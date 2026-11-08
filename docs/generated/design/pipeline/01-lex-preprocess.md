---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-02T21:19:11Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: 25c315302bec8a98147ed08e896ab7b882f1e7ddd78ee373f4e252ec03861394
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
runs to completion and produces a flat token list, after which parsing
begins, which allows the parser arbitrary lookahead.

## Lexer

The lexer is implemented under
[source/compiler-core/](../../../../source/compiler-core):

- [slang-lexer.h](../../../../source/compiler-core/slang-lexer.h) /
  [slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp) —
  the `Lexer` struct, its `initialize` / lex driver, and the
  `TokenList`, `TokenSpan`, and `TokenReader` types.
- [slang-token.h](../../../../source/compiler-core/slang-token.h) /
  [slang-token.cpp](../../../../source/compiler-core/slang-token.cpp) —
  the `Token`, `TokenType`, and `TokenFlags` data types.
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

The token-kind catalog is in
[../syntax-reference/tokens.md](../syntax-reference/tokens.md).

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
- Numeric, string, and character literal text is kept undecoded in the
  token's raw text (backslash-newline continuations are removed from
  the content when `ScrubbingNeeded` is set). The lexer scans the literal's extent but does not
  decode its value; value extraction is deferred to the helpers below.
- Non-ASCII input is decoded as UTF-8 by the byte-level `_advance`
  helper in
  [slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp),
  which folds a valid multi-byte sequence into a single code point that
  the identifier rule accepts. A malformed sequence is diagnosed as
  `invalidUtf8ByteSequence` (offending bytes in hex) and substituted
  with a space, so a bad byte terminates the current token. `_advance`
  stops at the first invalid continuation byte rather than consuming
  it, so recovery resumes at the start of the next character.

### Literal scanning and value extraction

The lexer separates *scanning* a literal (recording its extent as raw
token text) from *decoding* its value. Scanning finds where the
literal ends plus a few purely syntactic checks (`invalidDigitForBase`,
`octalLiteral`, `quoteCannotBeDelimiter`), while the literal's *meaning*
is deferred. Of those three checks only
`octalLiteral` is a warning: scanning continues into
`_lexNumber(lexer, 8)`, so `017` still yields a usable base-8 literal
token ([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)
line 2097). `invalidDigitForBase` (line 551) and
`quoteCannotBeDelimiter` (line 1797) are errors.

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
  offending text; the parser's `parseFloatingPointLiteralExpr`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
  9098) turns them into `InvalidFloatingPointLiteralNumber` and
  `InvalidFloatingPointLiteralSuffix` (lines 9131-9143), the only
  caller in the watched sources. `outIsOutOfRange` means the result is
  `0` (underflow) or `INFINITY` (overflow above the maximum for the
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

An empty suffix classifies as `Float`, not `Double`, so the suffix
alone can decide whether a literal is representable:

```slang
double a = 1e300lf; // Double: stays finite
float b = 1e300;    // Float: out of range, decodes to INFINITY
```

`1e300` is far above the `float` maximum
([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)
lines 1273-1284, and line 1331 for the `float` parse).

Literal-value errors are reported at decode time through the helpers'
`DiagnosticSink*`. Escape handling is centralized in `_decodeStringEscape`
([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)
line 1440), which takes the sink and the token's `SourceLoc` and
diagnoses its own failures (`invalidStringEscape` for an unknown escape
letter, a digitless or unterminated `\x{...}` / `\u{...}` form, or an
overflowing value; `invalidUnicodeStringEscape` when `\u` does not have
exactly 4 hex digits or `\U` does not have exactly 8) before returning
`-1`. The grammar is: octal (`\NNN`), hex (`\xNN`, any number of digits), and the Unicode
escapes `\uNNNN` (4 hex digits), `\UNNNNNNNN` (8 hex digits), and
`\u{...}` (braced, up to 32 bits). In string literals, `\xNN` maps to a
single byte while Unicode code-point escapes are encoded as UTF-8
(via `encodeUnicodePointToUTF8`). `getStringLiteralTokenValue` copies
bare non-escape bytes through without re-checking UTF-8, since
`_advance` already diagnosed malformed sequences while scanning; in
character literals the body is re-decoded with `getUnicodePointFromUTF8`
and malformed UTF-8 is rejected again. The diagnostics are defined in
[slang-lexer-diagnostic-defs.h](../../../../source/compiler-core/slang-lexer-diagnostic-defs.h).

### What the lexer does *not* do

- It does not classify keywords (deferred to lookup).
- It does not evaluate literals during scanning (see above).
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
- an optional `PreprocessorHandler*` (callbacks for
  end-of-translation-unit and file-dependency events),
- an optional `PreprocessorContentAssistInfo*`, used by the language
  server to collect code-assist information while preprocessing.

### Stack of input streams

The preprocessor maintains a stack of input streams: the original
source file at the bottom, with `#include`d files and macro expansions
pushed on top. Each `#include`d file gets its own `InputFile` with its
own lexer and `Conditional` state, so conditional skipping and
directive diagnostics are per-file.

### Macro expansion

Macro definitions store a pre-lexed token sequence chopped into
`MacroDefinition::Op` entries; expansion replays those ops. A
parameter reference is compiled to a parameter op (`ExpandedParam`,
`UnexpandedParam`, or `StringizedParam`) carrying the parameter index.
At invocation, `MacroInvocation` replays the matching argument's token
range by index (`_getArgTokens`,
[slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp)),
wrapping it in an `ExpansionInputStream` for the `ExpandedParam` case so
argument tokens are themselves macro-expanded — there is no per-invocation
environment of pseudo-macros.

An invocation whose argument count does not match the parameter list is
diagnosed with `WrongNumberOfArgumentsToMacro` and skipped, so it
contributes no tokens (lines 1870 and 1892); a variadic macro requires
only its non-variadic parameters and accepts any number of trailing
arguments. Expansion is self-suppressing rather than depth-limited:
every in-flight invocation is on a "busy" list, and
`_maybeBeginMacroInvocation` leaves the identifier unexpanded when
`MacroInvocation::isBusy` finds its macro there (lines 1462 and 1728).
A macro naming itself in its own body reaches the parser as a plain
identifier, neither re-expanded nor diagnosed; the same list breaks
mutual recursion across nested expansions.

Inactive `#if` branches still flow through the lexer (so line
accounting stays correct), but their contents are not expanded; only
the conditional directives (`#if`, `#ifdef`, `#ifndef`, `#else`,
`#elif`, `#endif`) are evaluated inside an inactive block.

### Preprocessor directives

Directives are looked up by name in a callback table, so adding one is
a matter of registering a new callback in
[slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp).
The `kDirectives` table holds the C / HLSL-style set (`#if` and friends, `#include`,
`#define`, `#undef`, `#warning`, `#error`, `#line`, `#pragma`) plus
the Slang language-selection directives `#language` / `#lang` and the
GLSL directives `#version` / `#extension`.

Every directive consumes its own line, so none contributes tokens to
the output list. Of the four non-C entries, `#language` / `#lang`
(`HandleLanguageDirective`, line 4593) take an optional `slang` name
followed by a version —

```
#lang [slang] <version>
#language [slang] <version>
```

— and record the source language that `preprocessSource` returns
through its `SourceLanguageDirective` out-parameter (language, the
location of the first directive, and the Slang version) and the
version through `outLanguageVersion`
([slang-preprocessor.h](../../../../source/slang/slang-preprocessor.h)).
Only the first selection in a source segment is kept:
`_tryApplySourceLanguageDirective` (line 4516) diagnoses a later
directive naming a different language
(`ConflictingSourceLanguageDirectives`) or a different Slang version
(`ConflictingSlangLanguageVersionDirectives`), returns false, and
leaves the first selection and `languageVersion` in force. Combining
the selections of separately preprocessed primary files is done by
`_applySourceLanguageDirective` in
[slang-compile-request.cpp](../../../../source/slang/slang-compile-request.cpp)
(line 276), outside this document's scope.
The version operand is resolved by name: whether it arrived as an
identifier or an integer literal, its *text* is looked up by
`TypeTextUtil::findLanguageVersion`, where each version has several
spellings (`legacy` / `default` / `2018`, `2025` / `202a`, `2026` /
`202b` / `latest`, `202c` / `next`). So `#lang 2026`, `#lang 202b` and
`#lang latest` are the same directive. Only `slang` is accepted as the
optional language name; `glsl` is no longer recognized.

Which diagnostic an unusable operand produces depends on what the
parser can still infer about it, which is why there are three:

| Operand | Diagnostic |
| --- | --- |
| Neither an identifier nor an integer literal | `ExpectedIntegralVersionNumber` |
| An unrecognized identifier, with no `slang` name given | `UnknownLanguage` — with the language unspecified, the lone identifier is read as an attempt to name one |
| An unrecognized integer literal, or an unrecognized identifier *after* an explicit `slang` | `UnknownLanguageVersion` — the language is settled, so the token can only have been a version |

`#version` (line 4558) takes an integer and records GLSL as the
source language (through the same first-selection rule) when it names a
valid GLSL version; `#extension` (line 4496) is accepted and discarded.

`#pragma` dispatches on a second name through `kPragmaDirectives`
(line 4445), which holds two entries, `once` and `warning`.
`#pragma once` records the containing file's unique identity in
`pragmaOnceUniqueIdentities` (line 4272), and `HandleIncludeDirective`
returns early for a later `#include` resolving to that identity (line
3680). An unrecognized sub-directive is *not* an error:
`findPragmaDirective` falls back to `handleUnknownPragmaDirective`
(line 4242), which warns with `UnknownPragmaDirectiveIgnored` and skips
the line — unlike an unknown `#`-directive, which
`HandleInvalidDirective` (line 4677) reports as an error.

`#if` and `#elif` conditions go through a recursive-descent evaluator
whose value type is a plain signed `int` (`PreprocessorExpressionValue`,
line 2984); any non-zero result selects the branch. An identifier
naming no object-like macro evaluates to `0` after the
`UndefinedIdentifierInPreprocessorExpression` warning (line 3117).
`EvaluateInfixOp` (line 3194) applies the C++ operator directly; only
`/` and `%` check operands (a zero divisor is diagnosed and yields
`0`), so overflow of `+`, `-`, `*`, or `<<` is not diagnosed.

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
that file. The handler's `handleFileDependency` callback records dependencies.

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
restarting from 0 and colliding. A `SLANG_RELEASE_ASSERT` on the handed-back counter guards
monotonicity (e.g. against `uint32_t` wrap), since a violation would
silently mis-resolve `#pragma warning` state.

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
   list. `_getArgTokens` replays the recorded token range through a
   `PretokenizedInputStream` without rewriting anything (lines 2351
   and 2494), so each keeps the `SourceLoc` it was physically lexed
   with at the invocation. A parameter used more than once replays the
   *same* location every time: given `#define TWICE(x) ((x) + (x))`, a
   diagnostic about the argument in `TWICE(undeclared)` points at
   `undeclared` in the invocation, not at either `(x)` in the body —
   where the category-1 rule would put it.
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

This split lets diagnostics (see
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md))
point inside the macro body when the macro is at fault, while
argument tokens point at the call site. Only the pasted-token
case builds an initiating-location chain: when formatting a
diagnostic, `DiagnosticSink` walks `getInitiatingSourceLoc` for as
long as the view's `PathInfo::Type` is `TokenPaste`, emitting a
`seeTokenPasteLocation` note for each hop.

## Failure modes

- Invalid characters and malformed literals raise diagnostics in two
  phases. Scan-time errors go through the sink passed to the lexer's
  `initialize` (invalid character, malformed UTF-8, end-of-file or
  newline inside a literal, invalid digit for the base, legacy octal,
  bad raw-string delimiter). Value errors are decode-time, reported
  through the sink passed to the value-extraction helper: both `getStringLiteralTokenValue` and `getCharLiteralValue` report
  ill-formed escape syntax; string decoding additionally reports
  out-of-range code units and code points (`outOfRangeCodeUnit`,
  `outOfRangeCodePointForUtf8`), and character decoding reports the
  empty or multi-character body, while
  `getIntegerLiteralValue` reports
  `integerLiteralTooLargeForAnyType`. `getFloatingPointLiteralValue`
  takes no sink; it returns `BadSignificand` / `BadSuffix` and the
  parser chooses the diagnostic.
- Both phases funnel through the file-local `diagnose` helper in
  [slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp),
  which drops a null sink and stops forwarding once the sink's error
  count exceeds `kMaxLexErrorCount` (100), so a pathologically
  malformed file cannot flood the sink.
- With `kLexerFlag_SuppressDiagnostics` set (see Lexer flags above),
  the lexer still emits `TokenType::Invalid` tokens but suppresses
  scan-time diagnostics, except malformed UTF-8. A caller that later
  asks a value-extraction helper for a literal's value passes its own
  sink and will see the decode-time diagnostics.
- Preprocessor errors (unbalanced `#if`, unknown directive, missing
  include, cyclic include, a `#else` / `#elif` / `#endif` with no open
  conditional) likewise emit through the sink, and the preprocessor
  keeps going: a malformed directive is skipped to the end of its
  line, a failed `#include` returns from its handler, a bad macro
  invocation can expand to no tokens, and an unclosed conditional is
  diagnosed once per still-open conditional at end-of-file, as
  `EndOfFileInPreprocessorConditional`
  ([slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp)
  line 4836). The stray closer is a separate diagnostic,
  `DirectiveWithoutIf` (line 3530); `CyclicInclude` fires
  when a resolved include is already open on the include stack (line
  3710), and that `#include` is skipped. The single `EndOfFile` token in
  the output is appended by `ReadAllTokens` only after the whole input
  stack has been popped.

See [../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md)
for the diagnostic system.
