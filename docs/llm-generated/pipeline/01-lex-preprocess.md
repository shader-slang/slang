---
generated: true
model: claude-opus-4.7
generated_at: 2026-05-07T14:35:56+00:00
source_commit: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
watched_paths_digest: ffca628ccbe37729fbf4b931ff5a00d755e36eb2c4c24e63869e5ee91bba63b3
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Lex and Preprocess

This document covers the first compilation stage: turning a source
buffer into a flat array of `Token` that the parser can consume. The
intended reader is a developer modifying token classification, source-
location encoding, or preprocessor directives.

## Inputs and outputs

- **Input**: a source buffer (typically loaded from disk by the
  include system) plus the active `Linkage` configuration (predefined
  macros, include paths).
- **Output**: a fully expanded `TokenList` (see
  [slang-lexer.h](../../../source/compiler-core/slang-lexer.h)),
  meaning a sequence of `Token` values with all `#include`, macro
  expansion, and conditional preprocessor directives already resolved.

The preprocessor does **not** stream tokens to the parser. Phase 1
runs to completion and produces a flat token list, after which phase 2
(parsing) begins. This decoupling is what allows the parser to use
arbitrary lookahead.

## Lexer

The lexer is implemented under
[source/compiler-core/](../../../source/compiler-core/):

- [slang-lexer.h](../../../source/compiler-core/slang-lexer.h) /
  [slang-lexer.cpp](../../../source/compiler-core/slang-lexer.cpp) —
  the `Lexer` struct and its `initialize` / lex driver.
- [slang-token.h](../../../source/compiler-core/slang-token.h) /
  [slang-token.cpp](../../../source/compiler-core/slang-token.cpp) —
  the `Token`, `TokenType`, `TokenFlags`, `TokenList`, `TokenSpan`,
  and `TokenReader` data types.
- [slang-token-defs.h](../../../source/compiler-core/slang-token-defs.h)
  — X-macro list of every `TokenType` value.

### Token data model

Every token carries:

- a `TokenType` (one byte; declared via the X-macro in
  [slang-token-defs.h](../../../source/compiler-core/slang-token-defs.h));
- raw text (`charsCount` plus a union pointing either to raw chars or
  to an interned `Name*`);
- a `SourceLoc` (a single 32-bit integer; decoded by
  [slang-source-loc.h](../../../source/compiler-core/slang-source-loc.h));
- a small `TokenFlags` byte — `AtStartOfLine`, `AfterWhitespace`,
  `ScrubbingNeeded`, and `Name` (which discriminates the union).

The complete token-kind catalog with categories is in
[../syntax-reference/tokens.md](../syntax-reference/tokens.md); this
document does not duplicate it.

### Source-location encoding

Source locations are kept as a single `uint32_t` that the
`SourceManager`
([slang-source-loc.h](../../../source/compiler-core/slang-source-loc.h),
[slang-source-loc.cpp](../../../source/compiler-core/slang-source-loc.cpp))
decodes back into file / line / column on demand. The encoding is
chosen so that every token carries one cheap field rather than three.

### Lexer flags and special-case rules

`LexerFlags` (declared in
[slang-lexer.h](../../../source/compiler-core/slang-lexer.h))
currently includes `kLexerFlag_SuppressDiagnostics`, used to lex
without complaining about invalid or unsupported characters (for
example, when tokenizing inside an inactive `#if` block).

The lexer handles several C-isms:

- Backslash line continuations (the source location of the resulting
  token still maps back to the original physical line).
- Special handling of `<...>` after a `#include` directive, so that
  `<foo/bar.h>` lexes as a single string-like token instead of a
  comparison expression.
- No distinction between identifiers and keywords. Every keyword token
  arrives at the parser as `TokenType::Identifier` (see the X-macro
  list in
  [slang-token-defs.h](../../../source/compiler-core/slang-token-defs.h));
  the parser resolves the keyword status via lookup, as described in
  [02-parse-ast.md](02-parse-ast.md).
- Numeric literal suffixes are kept as part of the token's raw text;
  the lexer does not extract literal values.

### What the lexer does *not* do

- It does not classify keywords (deferred to lookup).
- It does not evaluate numeric literals.
- It does not skip whitespace or comments by default; whitespace and
  comments are emitted as their own token types so that downstream
  passes (preprocessor, parser) can decide whether to filter them.

## Preprocessor

Implemented in
[source/slang/slang-preprocessor.cpp](../../../source/slang/slang-preprocessor.cpp);
public surface in
[slang-preprocessor.h](../../../source/slang/slang-preprocessor.h).

The `Preprocessor` is configured through `PreprocessorDesc`, which
takes:

- a `DiagnosticSink*` for messages,
- a `NamePool*` for interning identifier text,
- an `ISlangFileSystemExt*` and `SourceManager*` for I/O,
- an optional `IncludeSystem*` for `#include` resolution
  ([slang-include-system.h](../../../source/compiler-core/slang-include-system.h)),
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

Macro definitions store an already-lexed token sequence; expansion
"replays" those tokens. Function-style macros allocate a fresh
environment that maps parameter names to pseudo-macros for the
arguments, so parameter expansion follows the same recursive
expansion mechanism as ordinary macro expansion.

Inactive `#if` branches still flow through the lexer (so column /
line accounting stays correct), but their contents are not expanded;
only directives that may toggle the active / inactive state
(`#if`, `#else`, `#elif`, `#endif`) are actually evaluated inside an
inactive block.

### Preprocessor directives

Directives are looked up by name in a callback table on the
preprocessor state, so adding a directive (`#pragma`, custom
extensions) is a matter of registering a new callback in
[slang-preprocessor.cpp](../../../source/slang/slang-preprocessor.cpp).
The list of supported directives is the standard C / HLSL set —
verify by reading the directive table in
[slang-preprocessor.cpp](../../../source/slang/slang-preprocessor.cpp).

### `#include` resolution

`#include` strings are resolved by `IncludeSystem` from
[slang-include-system.cpp](../../../source/compiler-core/slang-include-system.cpp),
which consults the `Linkage`'s search paths. Resolution returns a
`SourceFile`; the preprocessor then pushes a fresh input stream for
that file. The handler receives a `handleFileDependency` callback so
the front-end can build dependency records for the build system.

## Source-location preservation

Tokens emitted by macro expansion fall into three categories whose
source locations are chosen differently:

1. **Raw body tokens** — tokens copied verbatim from the macro
   definition. Their `SourceLoc` is the location of the corresponding
   token in the macro *definition*, replayed by
   `MacroInvocation::readToken`
   ([slang-preprocessor.cpp lines
   2435-2444](../../../source/slang/slang-preprocessor.cpp)). The
   `SourceManager` records that the resulting expansion is part of a
   macro invocation, so diagnostics can walk back to the invocation
   site even though the spelling location points into the macro body.
2. **Argument tokens** — tokens taken from the call-site argument
   list. They retain the call-site `SourceLoc`, since they are
   physically lexed from the invocation.
3. **Constructed tokens** — `__LINE__`, `__FILE__`, stringized (`#x`)
   and pasted (`x##y`) tokens, which are synthesized fresh. These use
   `m_macroInvocationLoc` so they are attributed to the invocation
   site ([slang-preprocessor.cpp lines
   2332-2334](../../../source/slang/slang-preprocessor.cpp)).

This split is what lets diagnostics (see
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md))
point inside the macro body when the macro itself is at fault, while
still letting users trace the inclusion / expansion chain back to
the call site. The `SourceManager` distinguishes "expansion"
locations from "spelling" locations through the same compact integer
encoding, and exposes helpers to walk that chain when formatting
diagnostics.

## Failure modes

- Invalid characters and malformed numeric / string literals raise
  diagnostics through the `DiagnosticSink` passed into the lexer's
  `initialize`. With `kLexerFlag_SuppressDiagnostics` set, the lexer
  still emits `TokenType::Invalid` tokens for these inputs but
  suppresses the diagnostics — used inside skipped preprocessor
  blocks where the tokens will be discarded by the inactive-branch
  filter.
- Preprocessor errors (unbalanced `#if`, unknown directive, missing
  include) likewise emit through the sink. When the preprocessor
  cannot recover, it produces an `EndOfFile` token early and the
  parser sees a truncated stream.

The detail of the diagnostic system is in
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md).
