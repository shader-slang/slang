# Prompt: tests-agentic/syntax-reference/tokens/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/syntax-reference/tokens/`,
anchored to
[`docs/llm-generated/syntax-reference/tokens.md`](../../../docs/llm-generated/syntax-reference/tokens.md).

Audience: nightly CI. The bundle exercises lexer-surface behaviors —
how the source character stream becomes a token stream — without
testing the parser's interpretation of those tokens. The doc lists
token kinds, flags, and special-case lexing rules; tests verify the
observable consequences.

## Required structure

1. `BUNDLE.md` with the structure named in `_common.md`.
2. One test file per **verifiable lexer claim** in the source doc. A
   claim is "verifiable" via `slangc` if its consequences can be
   observed in compilation behavior — for tokens, that usually means:
   - **literal-typing claims** — "the `f` suffix makes the literal
     `float`": verify by declaring a variable, calling a typed
     function, or printing the type via interpreter.
   - **comment-skipping claims** — "`//` ends at newline" /
     "`/* ... */` is removed" / "nested block comments are not
     supported": verify by placing valid code where the lexer should
     find it (positive) or by triggering a diagnostic where the
     lexer should fail to remove a token (negative).
   - **line-continuation claims** — `\` immediately before newline
     is folded out: verify by splitting a literal or identifier across
     two physical lines.
   - **string-literal claims** — including the raw-string form:
     verify by `printf` via the interpreter and FileChecking the
     output, paying attention to escape semantics.
   - **special-token claims** — `...`, `::`, `=>`, `->`: verify in
     the smallest valid usage (variadic generic, qualified name,
     lambda, function return).

3. Each test is small, single-purpose, and named after the claim it
   verifies. Examples:
   - `numeric-literal-suffix-u.slang`
   - `block-comment-no-nesting.slang`
   - `raw-string-no-escape.slang`
   - `line-continuation-identifier.slang`

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/syntax-reference/tokens.md`

Secondary (allowed citations; use sparingly and only when the primary
doc explicitly hands off):

- `docs/llm-generated/syntax-reference/keywords-and-builtins.md`
- `docs/llm-generated/syntax-reference/grammar.md`
- `docs/llm-generated/pipeline/01-lex-preprocess.md`

If you would cite anything else, stop and record a doc-gap finding in
`BUNDLE.md`.

## Source files you may consult for _verification only_

You may look at these files to verify that a claim in the doc is
realizable (e.g. the exact spelling of an operator token, the
diagnostic code emitted for a lex error). You may **not** mine them
for behavioral claims that the doc does not make.

- `source/compiler-core/slang-token.h`
- `source/compiler-core/slang-token.cpp`
- `source/compiler-core/slang-token-defs.h`
- `source/compiler-core/slang-lexer.h`
- `source/compiler-core/slang-lexer.cpp`

## Test directives

Tokens are best exercised on the interpreter or CPU compute (we want
to verify the lexer-level behavior, not target-specific emit). Prefer:

- `//TEST:INTERPRET(filecheck=CHECK):` — print observable values
  with `printf` and check output.
- `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type`
  — when a buffer write is the cleanest assertion.
- `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` — for "is rejected" or
  "produces error" claims.
- `//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm` — only when the
  lexer-level claim must be observed in emitted code (rare for this
  bundle).

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `syntax-reference/tokens.md` (or one of the listed secondary
      docs).
- [ ] Lexer claims dominate the bundle; do not include tests whose
      pass/fail depends on parser interpretation choices unless the
      doc explicitly makes a lexer claim about them.
- [ ] At least one negative / diagnostic test for the "block comments
      are not nested" claim and at least one for "raw strings do not
      process escapes".
- [ ] No test depends on a GPU.
- [ ] No test was written by inspecting an uncovered source line. If
      you find yourself thinking "this would cover the branch at
      `slang-lexer.cpp:NNNN`", stop and re-read the doc.
