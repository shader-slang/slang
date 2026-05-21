# Prompt: tests-agentic/pipeline/02-parse-ast/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/pipeline/02-parse-ast/`,
anchored to
[`docs/llm-generated/pipeline/02-parse-ast.md`](../../../docs/llm-generated/pipeline/02-parse-ast.md).

Audience: nightly CI. The bundle exercises the second compilation
stage end-to-end — how the flat post-preprocess token list becomes a
strongly-typed AST. Lex- and preprocess-surface claims belong to
`tests-agentic/pipeline/01-lex-preprocess/` and must not be
duplicated here; this bundle's tests succeed or fail because of
something the **parser** did, not the lexer.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. One test file per **verifiable claim** in
   `pipeline/02-parse-ast.md`. A claim is "verifiable" via `slangc` /
   `slangi` if its consequences can be observed in compilation
   behavior. The dominant claim families for this bundle are:
   - **decl-shape claims** — that `struct`, `interface`,
     `extension`, `namespace`, `typealias`, `func`, etc. parse into
     `Decl` nodes with the expected member and parameter
     structure. (The keyword-as-syntax-table story itself is a
     claim; verify it by registering a check on a representative
     keyword.)
   - **stmt-shape claims** — that block, if, while, for, return,
     etc. statements parse into their corresponding `Stmt` nodes,
     including the **two-stage parsing** invariant: function bodies
     are captured as `UnparsedStmt` and re-parsed at check time, so
     a body-only construct that depends on outer-scope checking
     (notably the `<`-as-generic vs `<`-as-less-than choice) parses
     in the body-parse stage.
   - **expr-shape claims** — that operator precedence and
     associativity produce the AST shape the language documents
     (e.g. `*` binds tighter than `+`, assignment is right-associative),
     that postfix forms (`.`, `[]`, `()`) bind tightly, and that the
     **freshly-parsed expr-or-type unification** holds: at parse
     time `A(B)` is a single `InvokeExpr` whether `A` later resolves
     to a function or a type.
   - **type-shape claims** — that array, pointer, optional (`?`),
     and function-type spellings parse where Slang supports them.
     `tests-agentic/syntax-reference/grammar.md` is the authoritative
     surface; do not invent shapes the doc does not list.
   - **generics claims** — that a generic declaration after a decl
     keyword is unambiguous, and that `<` in body position is
     resolved by the heuristic the doc describes. Verify the
     positive cases (`max<int>(1,2)` parses and runs;
     `a < b` parses and runs). The exact set of "generic-followers"
     punctuation in `grammar.md#-disambiguation` is the doc-side
     authority — anchor any disambiguation test there as a secondary
     citation.
   - **modifier claims** — that a modifier (`static`, `const`,
     `[unroll]`) collected before a decl keyword attaches to the
     `Decl`. Most observable modifier behaviors are
     semantic-checker-stage, not parse-stage; restrict tests here to
     ones whose pass/fail depends only on parsing (e.g.
     attribute-syntax acceptance).
   - **error-recovery claims** — that the parser emits a diagnostic
     for an unexpected token and continues, so a second error later
     in the same file is also reported. The `## Failure modes`
     section explicitly states this contract.

3. Each test is small, single-purpose, and named after the claim it
   verifies. Examples:
   - `decl-struct-empty.slang`
   - `decl-func-arrow-return.slang`
   - `expr-precedence-mul-over-add.slang`
   - `expr-assign-right-associative.slang`
   - `generic-call-in-body.slang`
   - `less-than-in-body-parses-as-comparison.slang`
   - `unparsed-stmt-resolves-at-check.slang`
   - `stmt-if-else-parse.slang`
   - `modifier-attached-to-decl.slang`
   - `unexpected-token-diagnostic.slang`

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/pipeline/02-parse-ast.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/llm-generated/pipeline/01-lex-preprocess.md` —
  for the input contract that the parser consumes.
- `docs/llm-generated/syntax-reference/grammar.md` —
  for the concrete production shapes (most useful for
  `<`-disambiguation, generic-application, and type-spelling
  claims that the parser doc hands off to grammar).
- `docs/llm-generated/ast-reference/base.md`
- `docs/llm-generated/ast-reference/declarations.md`
- `docs/llm-generated/ast-reference/expressions.md`
- `docs/llm-generated/ast-reference/statements.md` — for the
  node-family hand-offs the parser doc names.

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Source files you may consult for _verification only_

You may look at these files to verify that a claim in the doc is
realizable (e.g. the exact spelling of a decl keyword, the
diagnostic code emitted for an unexpected token). You may **not**
mine them for behavioral claims that the doc does not make.

- `source/slang/slang-parser.h`
- `source/slang/slang-parser.cpp`
- `source/slang/slang-ast-base.h`
- `source/slang/slang-ast-decl.h`
- `source/slang/slang-ast-expr.h`
- `source/slang/slang-ast-stmt.h`
- `source/slang/slang-ast-type.h`
- `source/slang/slang-ast-modifier.h`
- `source/slang/slang-ast-builder.h`
- `source/slang/slang-ast-builder.cpp`

If you find yourself thinking "this would cover the branch at
`slang-parser.cpp:NNNN`", stop and re-read the doc.

## Test directives

Parser-stage claims are largely target-independent: once the AST is
built, all backends see the same shape. So:

- Prefer `//TEST:INTERPRET(filecheck=CHECK):` for behaviors that
  print an observable value (a numeric result that proves a
  precedence or generic-call parse came out right).
- For pure shape claims that have no runtime observable (e.g.
  "this declaration parses"), use the smallest body that runs
  through the interpreter and prints `ok` or a small integer that
  can only be produced if parsing succeeded.
- For "is rejected" claims (the `## Failure modes` family and any
  diagnostic produced by error recovery), use
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`. When the token caret
  lands in columns 1–9, use the `/*CHECK*/` block-comment form
  documented in `_common.md` rather than a `//CHECK` line.
- **Do not** add multi-target `//TEST:SIMPLE` directives for
  target-independent parser claims — they are pure repetition of
  identical pre-backend behavior. A multi-target test is only
  justified for the rare parser claim that produces a different
  emit per target.

Do not use any GPU-only directive.

## Cast and observation reminders (carried from `_common.md` / pilot)

- Use **constructor-style casts** (`int('A')`), not C-style
  (`(int)'A'`). The C-style form returns 0 in `slangi`.
- `slangi` `printf` does not support `%s`. For string observation,
  use `//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute`
  and FileCheck the emitted C++ source for the string literal.
- `static const int x = N;` at file scope is the cleanest pattern
  for asserting a compile-time-known result.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `pipeline/02-parse-ast.md` (or one of the listed secondary
      docs).
- [ ] The bundle exercises **all four major node families**
      (Decl, Expr, Stmt, Modifier) explicitly named in
      `## AST data model`.
- [ ] At least one diagnostic test exercises the
      `## Failure modes` claim about error recovery (the parser
      emits a diagnostic and continues so that a second later
      error is also reported).
- [ ] At least one test exercises the
      `## Generics ambiguity` claim — both the positive (`<` _is_
      a generic application) and the disambiguation case (`<` is
      a less-than comparison in a body context).
- [ ] At least one test exercises the
      `## Two-stage parsing` claim — i.e. a construct whose parse
      depends on body-parse-time resolution.
- [ ] Tests in this bundle do not duplicate the
      `pipeline/01-lex-preprocess`, `syntax-reference/tokens`, or
      `syntax-reference/keywords-and-builtins` bundles. The
      keyword-set tests there cover decl-keyword spellings; this
      bundle's decl tests should cover shape that those tests do
      not (e.g. parameter lists, where-clauses, attribute syntax).
- [ ] No test depends on a GPU. `INTERPRET`, `-target cpp` text
      emit, and diagnostic-only directives are the only ones used.
- [ ] No test was written by inspecting an uncovered source line.
- [ ] `README.md` `## Doc gaps observed` is honest. If you wanted
      a test you could not anchor — for example a precise
      precedence-table claim more specific than what the doc
      states — write down which claim the doc would need to add.
