# Prompt: pipeline/01-lex-preprocess.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/generated/design/pipeline/01-lex-preprocess.md` — a
detailed description of the lexing and preprocessing stage.

Audience: a developer modifying token classification, source-location
encoding, or preprocessor directives.

## Required structure

1. `# Lex and Preprocess` (title)
2. `## Inputs and outputs` — what the stage starts with (a source
   buffer) and what it produces (a flat array of `Token`).
3. `## Lexer`:
   - File pointers to the watched paths
     ([slang-lexer.h](../../../../source/compiler-core/slang-lexer.h),
     [slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp),
     [slang-token.h](../../../../source/compiler-core/slang-token.h)).
   - The `Token` and `TokenKind` data model (raw text, source location,
     flags). Do not enumerate every token kind here — that is in
     [../syntax-reference/tokens.md](../syntax-reference/tokens.md).
   - The source-location encoding (single integer, decoded via
     [slang-source-loc.h](../../../../source/compiler-core/slang-source-loc.h)).
   - Special-case rules (line continuations, `<...>` after `#include`,
     handling of identifiers vs. keywords — note that keywords are
     resolved later, not in the lexer).
4. `## Preprocessor`:
   - File pointer
     ([slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp)).
   - The "stack of input streams" model.
   - Macro expansion ("replay" of stored token sequences) and
     environments for argument lookup.
   - Why preprocessing runs to completion before parsing starts (no
     direct interaction between the parser and preprocessor — the
     output is a flat token array).
   - How `#include` resolution interacts with the include search system
     in `source/compiler-core/slang-include-system.cpp`.
5. `## Source-location preservation` — how source locations survive
   macro expansion so that diagnostics later point at the right column.
6. `## Failure modes` — how lex / preprocess errors propagate (cite the
   diagnostic sink path; link
   [../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md)).

## Quality checklist (in addition to the universal one)

- [ ] Every claim about lexer or preprocessor behavior cites a file
      under `source/compiler-core/` or `source/slang/slang-preprocessor*`.
- [ ] Token-kind enumeration is **not** duplicated; defer to
      [../syntax-reference/tokens.md](../syntax-reference/tokens.md).
- [ ] Document length under 24 KB.
