---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T13:25:34Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: 1ec94893dbf01f578c85e79607171f593c06c00e297add42831695b6d50bc8d0
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
  macros, include paths). The raw file bytes are turned into source
  text by `SourceFile::decodeContentBlob`
  ([slang-source-loc.h](../../../../source/compiler-core/slang-source-loc.h),
  [slang-source-loc.cpp](../../../../source/compiler-core/slang-source-loc.cpp)),
  which strips a leading Unicode byte-order mark and decodes non-UTF-8
  encodings into a fresh blob; a BOM-free UTF-8 file is handed through
  unchanged. `SourceFile::setContents` routes through the same helper,
  so the lexer always sees BOM-free UTF-8.
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
`SourceLoc`'s copy constructor is explicitly `= default` rather than
user-written, so the type stays trivially copyable for aggregates that
embed it inside a union.

### Lexer flags and special-case rules

`LexerFlags` (declared in
[slang-lexer.h](../../../../source/compiler-core/slang-lexer.h))
currently includes `kLexerFlag_SuppressDiagnostics`, used to lex
without complaining about invalid or unsupported characters (for
example, when tokenizing inside an inactive `#if` block). The flag is
consulted through `Lexer::getDiagnosticSink`, which returns null
instead of the real sink when the flag is set, so the scan-time
diagnostic sites that go through that accessor are suppressed. The
malformed-UTF-8 diagnostic is an exception: it reports through
`lexer->m_sink` directly, so it still fires inside an inactive branch.

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
- Non-ASCII input is decoded as UTF-8 by the byte-level `_advance`
  helper in
  [slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp),
  which folds a valid multi-byte sequence into a single code point that
  the identifier rule accepts. A malformed sequence is diagnosed as
  `invalidUtf8ByteSequence` (with the offending bytes formatted in hex)
  and then substituted with a space, so a bad byte terminates the
  current token instead of derailing the rest of the scan. `_advance`
  stops at the first byte that is not a valid continuation byte rather
  than consuming it, which keeps the recovery point at the start of the
  next character.

### Literal scanning and value extraction

The lexer separates *scanning* a literal (recording its extent as raw
token text) from *decoding* its value, and the split is nearly clean:
scanning does the minimum work needed to find where the literal ends
plus a few purely syntactic checks (`invalidDigitForBase`,
`octalLiteral`, `quoteCannotBeDelimiter`), while everything about the
literal's *meaning* is deferred.

Scanning happens in the lex driver. `_lexStringLiteralBody(lexer, char quote)`
([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)
line 1731) handles both `"..."` and `'...'`; the closing quote
character is the parameter, so there is no separate
character-literal mode. Its only escape handling is stepping over
`\'`, `\"`, and `\\` so that an escaped quote does not falsely end the
token — it does not attempt to recognize octal, hex, or Unicode escape
forms. Accordingly the only diagnostics it raises are the two failures
that prevent finding an end at all: `endOfFileInLiteral` and
`newlineInLiteral`. Raw string literals are scanned separately by
`_lexRawStringLiteralBody` (line 1782), which looks for the matching
`)delimiter"` and performs no escape processing at all.

Decoding into a value is done on demand by the helper routines declared
in [slang-lexer.h](../../../../source/compiler-core/slang-lexer.h):

- `getIntegerLiteralValue` — parses the integer, optionally returning
  the suffix, decimal-base flag, and an overflow flag.
- `getFloatingPointLiteralValue` — parses the floating-point value.
  This helper also *classifies the suffix*: it splits the token into a
  number part and a suffix part, maps the suffix to a
  `FloatingPointLiteralType` (`Half` for `h`/`hf`/`fh`, `Float` for an
  empty suffix or `f`, `Double` for `l`/`lf`/`fl`, each in either
  case), and rounds the parsed value to that type's precision and
  range. The same enum reports the two malformed cases,
  `BadSignificand` and `BadSuffix`, with `outErrorContent` carrying the
  offending text. All four out-parameters — `outLiteralType`,
  `outIsOutOfRange`, `outPrecisionLost`, and `outErrorContent` — are
  non-optional references. `outIsOutOfRange` means the result is `0`
  (underflow) or `INFINITY` (overflow above the maximum for the
  *literal type*, not for `double`); `outPrecisionLost` is reported
  only for hex floats and only when the value is in range.
- `getStringLiteralTokenValue(token, sink)` — decodes escapes into the
  resulting bytes; `getFileNameTokenValue` is the variant for
  `#include`-style filenames, which does not process escapes.
- `getCharLiteralValue(token, sink)` — returns a 32-bit code point, or
  `-1` on failure (which is also diagnosed). This helper owns the
  one-character rule: it emits `illegalCharacterLiteral` for a body too
  short to hold a character (`''`) and again when the decode leaves
  unconsumed input, which is how a multi-character body is caught
  ([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)
  lines 1660-1725).

The string/char helpers take a `DiagnosticSink*` because all
literal-value errors are reported at decode time: out-of-range code
units and code points, malformed escape syntax, and the character-count
rule. Escape handling is centralized in `_decodeStringEscape`
([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)
line 1440), which takes the sink and the token's `SourceLoc` and
diagnoses its own failures (`invalidStringEscape` for an unknown escape
letter, a digitless or unterminated `\x{...}` / `\u{...}` form, or an
overflowing value; `invalidUnicodeStringEscape` when `\u` does not have
exactly 4 hex digits or `\U` does not have exactly 8) before returning
`-1`. Its grammar follows
[docs/language-reference/expressions-literal.md](../../../../docs/language-reference/expressions-literal.md):
octal (`\NNN`), hex (`\xNN`, any number of digits), and the Unicode
escapes `\uNNNN` (4 hex digits), `\UNNNNNNNN` (8 hex digits), and
`\u{...}` (braced, up to 32 bits). In string literals, `\xNN` maps to a
single byte while Unicode code-point escapes are encoded as UTF-8 byte
sequences (via `encodeUnicodePointToUTF8`). `getStringLiteralTokenValue`
copies bare non-escape bytes through verbatim without re-checking
UTF-8 well-formedness, because the lexer's `_advance` already
diagnosed a malformed sequence while scanning the literal body. In
character literals the body bytes are re-decoded with
`getUnicodePointFromUTF8` and malformed UTF-8 is rejected a second
time, so there is no practical difference between `\x` and `\u` there.
The corresponding diagnostics
(`invalidUtf8ByteSequence`, `invalidStringEscape`,
`invalidUnicodeStringEscape`, `outOfRangeCodeUnit`,
`outOfRangeCodePointForUtf8`) are defined in
[slang-lexer-diagnostic-defs.h](../../../../source/compiler-core/slang-lexer-diagnostic-defs.h).

### What the lexer does *not* do

- It does not classify keywords (deferred to lookup).
- It does not evaluate numeric literals during scanning; value
  extraction is a separate, on-demand step (see above).
- It does not skip whitespace or comments by default; `lexToken`
  emits them as their own token types, and the filtering is done by
  `lexAllSemanticTokens` and by the preprocessor's `ReadAllTokens`, so
  the parser never sees them.

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
  to record include dependencies),
- an optional `PreprocessorContentAssistInfo*`, used by the language
  server to collect code-assist information while preprocessing.

### Stack of input streams

The preprocessor maintains a stack of input streams, with the original
source file at the bottom and pushes for `#include`d files and macro
expansions on top. As tokens flow upward they pass through directive
recognition; each `#include`d file gets its own `InputFile` with its
own lexer and its own stack of `Conditional` state, so conditional
skipping and directive diagnostics are per-file and do not disturb the
parent.

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
(`#if`, `#ifdef`, `#ifndef`, `#else`, `#elif`, `#endif`) are actually
evaluated inside an
inactive block.

### Preprocessor directives

Directives are looked up by name in a callback table on the
preprocessor state, so adding a directive (`#pragma`, custom
extensions) is a matter of registering a new callback in
[slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp).
The `kDirectives` table in
[slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp)
holds the C / HLSL-style set (`#if` and friends, `#include`,
`#define`, `#undef`, `#warning`, `#error`, `#line`, `#pragma`) plus
the Slang language-selection directives `#language` / `#lang` and the
GLSL directives `#version` / `#extension`.

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
   No new `SourceView` is created for the invocation, so such a
   diagnostic points into the macro body rather than at the call site.
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
argument tokens still point at the call site. Only the pasted-token
case builds an initiating-location chain: when formatting a
diagnostic, `DiagnosticSink` walks `getInitiatingSourceLoc` for as
long as the view's `PathInfo::Type` is `TokenPaste`, emitting a
`seeTokenPasteLocation` note for each hop.

## Failure modes

- Invalid characters and malformed numeric / string / character
  literals raise diagnostics through the `DiagnosticSink`, split across
  two phases. Scan-time errors go through the sink passed into the
  lexer's `initialize`, and cover what the scanner can see without
  interpreting the literal: an invalid character, a malformed UTF-8
  sequence, end-of-file or newline inside a literal, a digit that is
  invalid for the literal's base, a legacy octal literal, and a quote
  used as a raw-string delimiter.
  Everything about a literal's value is a decode-time error, reported
  through the sink passed to the value-extraction helper:
  `getStringLiteralTokenValue` and `getCharLiteralValue` report
  ill-formed escape syntax, out-of-range code units and code points,
  and (for character literals) the empty or multi-character body, while
  `getIntegerLiteralValue` reports
  `integerLiteralTooLargeForAnyType`. `getFloatingPointLiteralValue`
  takes no sink at all — it
  returns its errors as a `FloatingPointLiteralType` of
  `BadSignificand` or `BadSuffix` plus the offending text, leaving the
  caller to choose the diagnostic.
- Both phases funnel through the file-local `diagnose` helper in
  [slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp),
  which drops a null sink and stops forwarding once the sink's error
  count exceeds `kMaxLexErrorCount` (100), so a pathologically
  malformed file cannot flood the sink.
- With `kLexerFlag_SuppressDiagnostics` set, the lexer still emits
  `TokenType::Invalid` tokens for malformed input but suppresses the
  diagnostics — used inside skipped preprocessor blocks where the
  tokens will be discarded by the inactive-branch filter. Because the
  suppression works by handing out a null sink from
  `Lexer::getDiagnosticSink`, it covers only the scan-time sites that
  use that accessor (the malformed-UTF-8 case bypasses it); a
  caller that later asks a value-extraction helper for a literal's
  value passes its own sink and will see the decode-time diagnostics.
- Preprocessor errors (unbalanced `#if`, unknown directive, missing
  include) likewise emit through the sink, and the preprocessor keeps
  going: a malformed directive is skipped to the end of its line, a
  failed `#include` returns from its handler, a bad macro invocation
  can expand to no tokens, and an unclosed conditional is diagnosed
  when its file reaches end-of-file. The single `EndOfFile` token in
  the output is appended by `ReadAllTokens` only after the whole input
  stack has been popped.

The detail of the diagnostic system is in
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md).
