---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-29T13:37:51Z
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
watched_paths_digest: e9924a6d79a718b4f741ca99add14031255b67e8c578cc6bf6b39f89c956daaf
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Lex and Preprocess

This document covers the first compilation stage: turning a source
buffer into a flat array of `Token` that the parser can consume. The
intended reader is a developer modifying token classification, source-
location encoding, literal-value extraction, or preprocessor directives.

## Inputs and outputs

- **Input**: a source buffer (typically loaded from disk by the
  include system) plus the active `Linkage` configuration (predefined
  macros, include paths).
- **Output**: a fully expanded `TokenList` (see
  [slang-lexer.h](../../../../source/compiler-core/slang-lexer.h)),
  meaning a sequence of `Token` values with all `#include`, macro
  expansion, and conditional preprocessor directives already resolved.

The preprocessor does **not** stream tokens to the parser. Phase 1
runs to completion and produces a flat token list, after which phase 2
(parsing) begins. This decoupling is what allows the parser to use
arbitrary lookahead.

## Lexer

The lexer is implemented under
[source/compiler-core/](../../../../source/compiler-core):

- [slang-lexer.h](../../../../source/compiler-core/slang-lexer.h) /
  [slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp) —
  the `Lexer` struct and its `initialize` / lex driver.
- [slang-token.h](../../../../source/compiler-core/slang-token.h) /
  [slang-token.cpp](../../../../source/compiler-core/slang-token.cpp) —
  the `Token`, `TokenType`, `TokenFlags`, `TokenList`, `TokenSpan`,
  and `TokenReader` data types.
- [slang-token-defs.h](../../../../source/compiler-core/slang-token-defs.h)
  — X-macro list of every `TokenType` value.

### Token data model

Every token carries:

- a `TokenType` (one byte; declared via the X-macro in
  [slang-token-defs.h](../../../../source/compiler-core/slang-token-defs.h));
- raw text (`charsCount` plus a union pointing either to raw chars or
  to an interned `Name*`);
- a `SourceLoc` (a single 32-bit integer; decoded by
  [slang-source-loc.h](../../../../source/compiler-core/slang-source-loc.h));
- a small `TokenFlags` byte — `AtStartOfLine`, `AfterWhitespace`,
  `ScrubbingNeeded`, and `Name` (which discriminates the union).

The complete token-kind catalog with categories is in
[../syntax-reference/tokens.md](../syntax-reference/tokens.md); this
document does not duplicate it.

### Source-location encoding

Source locations are kept as a single `uint32_t` that the
`SourceManager`
([slang-source-loc.h](../../../../source/compiler-core/slang-source-loc.h),
[slang-source-loc.cpp](../../../../source/compiler-core/slang-source-loc.cpp))
decodes back into file / line / column on demand. The encoding is
chosen so that every token carries one cheap field rather than three.

### Lexer flags and special-case rules

`LexerFlags` (declared in
[slang-lexer.h](../../../../source/compiler-core/slang-lexer.h))
currently includes `kLexerFlag_SuppressDiagnostics`, used to lex
without complaining about invalid or unsupported characters (for
example, when tokenizing inside an inactive `#if` block).

The lexer handles several C-isms:

- Backslash line continuations (the source location of the resulting
  token still maps back to the original physical line).
- No distinction between identifiers and keywords. Every keyword token
  arrives at the parser as `TokenType::Identifier` (see the X-macro
  list in
  [slang-token-defs.h](../../../../source/compiler-core/slang-token-defs.h));
  the parser resolves the keyword status via lookup, as described in
  [02-parse-ast.md](02-parse-ast.md).
- Numeric, string, and character literal text is kept verbatim in the
  token's raw text. The lexer scans the literal's extent but does not
  decode its value; value extraction is deferred to the helpers below.

### Literal scanning and value extraction

The lexer separates *scanning* a literal (recording its extent as raw
token text) from *decoding* its value. Scanning happens in the lex
driver — for example `_lexStringLiteralBody`
([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp))
handles both `"..."` and `'...'`, walking escape sequences far enough
to find the closing quote. Scanning records the raw text but still
validates enough literal structure to report malformed syntax: it
diagnoses empty or multi-character character literals
(`illegalCharacterLiteral`), an EOF or newline inside a literal
(`endOfFileInLiteral`, `newlineInLiteral`), and ill-formed `\x`, `\u`,
`\U`, or braced Unicode escapes (`invalidStringEscape`,
`invalidUnicodeStringEscape`). Decoding into a value is done on demand
by the helper routines declared in
[slang-lexer.h](../../../../source/compiler-core/slang-lexer.h):

- `getIntegerLiteralValue` — parses the integer, optionally returning
  the suffix, decimal-base flag, and an overflow flag.
- `getFloatingPointLiteralValue` — parses the floating-point value and
  optionally reports `outIsOutOfRange` (underflow returns `0`,
  overflow returns `INFINITY`) and `outPrecisionLost` (significand
  truncated).
- `getStringLiteralTokenValue(token, sink)` — decodes escapes into the
  resulting bytes; `getFileNameTokenValue` is the variant for
  `#include`-style filenames, which does not process escapes.
- `getCharLiteralValue(token, sink)` — returns a 32-bit code point, or
  `-1` on failure (which is also diagnosed).

The string/char helpers take a `DiagnosticSink*` because value
conversion errors — out-of-range code units or code points, and the
escape decoding the helpers themselves perform — are reported at decode
time, separately from the structural escape validation already done
during scanning. Escape handling is centralized in `_decodeStringEscape`
([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp))
and follows the grammar in
[docs/language-reference/expressions-literal.md](../../../../docs/language-reference/expressions-literal.md):
octal (`\NNN`), hex (`\xNN`, any number of digits), and the Unicode
escapes `\uNNNN` (4 hex digits), `\UNNNNNNNN` (8 hex digits), and
`\u{...}` (braced, up to 32 bits). In string literals, `\xNN` maps to a
single byte while Unicode code-point escapes are encoded as UTF-8 byte
sequences (via `encodeUnicodePointToUTF8`); bare non-ASCII bytes are
passed through without enforcing well-formed UTF-8. In character
literals the source bytes are decoded with `getUnicodePointFromUTF8`
and malformed UTF-8 is rejected, so there is no practical difference
between `\x` and `\u` there. The corresponding diagnostics
(`invalidUtf8ByteSequence`, `invalidStringEscape`,
`invalidUnicodeStringEscape`, `outOfRangeCodeUnit`,
`outOfRangeCodePointForUtf8`) are defined in
[slang-lexer-diagnostic-defs.h](../../../../source/compiler-core/slang-lexer-diagnostic-defs.h).

### What the lexer does *not* do

- It does not classify keywords (deferred to lookup).
- It does not evaluate numeric literals during scanning; value
  extraction is a separate, on-demand step (see above).
- It does not skip whitespace or comments by default; whitespace and
  comments are emitted as their own token types so that downstream
  passes (preprocessor, parser) can decide whether to filter them.

## Preprocessor

Implemented in
[source/slang/slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp);
public surface in
[slang-preprocessor.h](../../../../source/slang/slang-preprocessor.h).

The `Preprocessor` is configured through `PreprocessorDesc`, which
takes:

- a `DiagnosticSink*` for messages,
- a `NamePool*` for interning identifier text,
- an `ISlangFileSystemExt*` and `SourceManager*` for I/O,
- an optional `IncludeSystem*` for `#include` resolution
  ([slang-include-system.h](../../../../source/compiler-core/slang-include-system.h)),
- an optional `Dictionary<String, String>` of predefined macros,
- an optional `PreprocessorHandler*` (a callback for events like
  end-of-translation-unit and file dependencies — used by the build
  to record include dependencies).

### Stack of input streams

The preprocessor maintains a stack of input streams, with the original
source file at the bottom and pushes for `#include`d files and macro
expansions on top. As tokens flow upward they pass through directive
recognition; the "stream" abstraction means that lexer behavior
(`#include` mode, conditional skipping) can be set per stream without
disturbing the parent.

### Macro expansion

Macro definitions store an already-lexed token sequence, pre-chopped
into `MacroDefinition::Op` entries; expansion "replays" those ops. A
parameter reference is compiled to a parameter op (`ExpandedParam`,
`UnexpandedParam`, or `StringizedParam`) carrying the parameter index.
At invocation, `MacroInvocation` replays the matching argument's token
range by index (`_getArgTokens`,
[slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp)),
wrapping it in an `ExpansionInputStream` for the `ExpandedParam` case so
argument tokens are themselves macro-expanded — there is no per-invocation
environment of pseudo-macros.

Inactive `#if` branches still flow through the lexer (so column /
line accounting stays correct), but their contents are not expanded;
only directives that may toggle the active / inactive state
(`#if`, `#else`, `#elif`, `#endif`) are actually evaluated inside an
inactive block.

### Preprocessor directives

Directives are looked up by name in a callback table on the
preprocessor state, so adding a directive (`#pragma`, custom
extensions) is a matter of registering a new callback in
[slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp).
The list of supported directives is the standard C / HLSL set —
verify by reading the directive table in
[slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp).

### `#include` resolution

`#include` strings are resolved by `IncludeSystem` from
[slang-include-system.cpp](../../../../source/compiler-core/slang-include-system.cpp),
which consults the `Linkage`'s search paths. For angle-form includes,
`HandleIncludeDirective`
([slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp))
concatenates the raw tokens between `OpLess` and `OpGreater` into the
path string and selects `IncludeSystem::Mode::System`; quoted includes
take the path from a single `StringLiteral` token. Resolution returns a
`SourceFile`; the preprocessor then pushes a fresh input stream for
that file. The handler receives a `handleFileDependency` callback so
the front-end can build dependency records for the build system.

### `#pragma warning` state across files

`#pragma warning(push/pop/disable/...)` state is tracked by a
`WarningStateTracker`
([slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp)),
which records, per diagnostic id, a timeline keyed on an absolute
source-location axis. Because each `__include`d file is preprocessed in
its own `preprocessSource` pass with a fresh `Preprocessor`, the
tracker carries a `persistedAbsoluteSourceLocCounter`: `preprocessSource`
seeds its `absoluteSourceLocCounter` from the persisted value at entry
and hands the advanced value back at exit, so the timeline's absolute
axis stays globally monotonic across files instead of every pass
restarting from 0 and colliding. A `SLANG_RELEASE_ASSERT` on the handed-
back counter guards monotonicity (e.g. against `uint32_t` wrap on very
large translation units), because a violation would silently mis-resolve
`#pragma warning` state in shipping builds.

## Source-location preservation

Tokens emitted by macro expansion fall into three categories whose
source locations are chosen differently:

1. **Raw body tokens** — tokens copied verbatim from the macro
   definition. Their `SourceLoc` is the location of the corresponding
   token in the macro *definition*, replayed by
   `MacroInvocation::readToken`
   ([slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp)).
   The `SourceManager` records that the resulting expansion is part of
   a macro invocation, so diagnostics can walk back to the invocation
   site even though the spelling location points into the macro body.
2. **Argument tokens** — tokens taken from the call-site argument
   list. They retain the call-site `SourceLoc`, since they are
   physically lexed from the invocation.
3. **Constructed tokens** — synthesized fresh, with three different
   source-location rules
   ([slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp)):
   - *Builtins* (`__LINE__`, `__FILE__`) are pushed by
     `_pushStreamForSourceLocBuiltin`, which gives the synthesized token
     `m_macroInvocationLoc`, attributing it to the invocation site (the
     reported line/file value, however, derives from the initiating
     top-level location).
   - *Stringized parameters* (`#x`) take the location of the `#` token
     in the macro *definition* (`m_macro->tokens.m_tokens[tokenIndex].loc`).
   - *Pasted tokens* (`x##y`) are re-lexed from a fresh
     `PathInfo::makeTokenPaste()` source view whose origin is the `##`
     token location (`tokenPasteLoc`).

This split is what lets diagnostics (see
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md))
point inside the macro body when the macro itself is at fault, while
still letting users trace the inclusion / expansion chain back to
the call site. The `SourceManager` distinguishes "expansion"
locations from "spelling" locations through the same compact integer
encoding, and exposes helpers to walk that chain when formatting
diagnostics.

## Failure modes

- Invalid characters and malformed numeric / string / character
  literals raise diagnostics through the `DiagnosticSink`. Scan-time
  errors (invalid character, end-of-file / newline in a literal,
  empty / multi-character character literal, and ill-formed escape
  syntax) go through the sink passed into the lexer's `initialize`;
  decode-time errors (malformed UTF-8, out-of-range code unit / code
  point, helper-specific escape decoding) go
  through the sink passed to the value-extraction helpers
  (`getStringLiteralTokenValue`, `getCharLiteralValue`). With
  `kLexerFlag_SuppressDiagnostics` set, the lexer still emits
  `TokenType::Invalid` tokens for malformed input but suppresses the
  diagnostics — used inside skipped preprocessor blocks where the
  tokens will be discarded by the inactive-branch filter.
- Preprocessor errors (unbalanced `#if`, unknown directive, missing
  include) likewise emit through the sink. When the preprocessor
  cannot recover, it produces an `EndOfFile` token early and the
  parser sees a truncated stream.

The detail of the diagnostic system is in
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md).
