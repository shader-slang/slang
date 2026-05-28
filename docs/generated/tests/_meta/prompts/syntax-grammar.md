# Prompt: docs/generated/tests/syntax-reference/grammar/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/syntax-reference/grammar/`,
anchored to
[`docs/generated/design/syntax-reference/grammar.md`](../../../docs/generated/design/syntax-reference/grammar.md).

Audience: nightly CI. The bundle exercises the **surface grammar** of
Slang — the EBNF-style productions reverse-engineered from the parser
in `slang-parser.cpp`. Tests verify that the documented productions
parse (positive) or that ill-formed inputs are rejected with a
diagnostic (negative). The grammar is, by construction, a parse-stage
concern, so `pipeline_stage=parse` dominates this bundle.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. One test file per **verifiable grammar production** in the source
   doc. A production is "verifiable" via `slangc` / `slangi` if its
   syntactic surface can be observed:
   - **decl-production claims** — `StructDecl`, `EnumDecl`,
     `InterfaceDecl`, `TypeAliasDecl`, `TypedefDecl`, `FuncDecl`,
     `ConstructorDecl`, `PropertyDecl`, `SubscriptDecl`, `CBufferDecl`,
     `NamespaceDecl`, etc. — verify by exercising the smallest body
     that observes the production parses and links.
   - **expression-precedence and associativity claims** — the
     precedence ladder under `## Expressions`. Verify by computing a
     result that distinguishes one bracketing from another (e.g.
     `2 + 3 * 4 == 14` proves `*` binds tighter than `+`;
     `a = b = 0` proves `=` is right-associative).
   - **statement-production claims** — `if`, `for`, `while`,
     `do ... while`, `do ... catch`, `switch`/`case`/`default`,
     `break`, `continue`, `return`, `discard`, `defer`, `throw`,
     `Block`. Verify by running the statement and observing a side
     effect.
   - **type-spelling claims** — array, pointer (`*`), optional
     (`?`), reference (`&`), tuple, function-type. The grammar lists
     these; only test the ones whose surface syntax actually parses
     in Slang (e.g. `int[4]` works in many positions;
     `func(int)->int` is the function-type form). If a spelling is
     only valid in particular contexts, anchor the test to the
     context-permitted use.
   - **generics-production claims** — `GenericDecl`,
     `GenericParams`, `GenericArgs`, `WhereClause` (the conformance
     `Type ':' Type` and equality `Type '==' Type` forms). The
     `<`-as-generic vs `<`-as-comparison story lives at
     `### `<` disambiguation` (anchor: `#disambiguation`) — write one
     test for each side of that fork.
   - **modifier-production claims** — `ModifierList`, attribute
     bracket form `[name(args)]`. Most semantic effects of modifiers
     are checker-stage and belong to other bundles; this bundle
     restricts to syntax-surface claims (the bracket-form attribute
     parses and accepts comma-separated names; modifier keywords
     attach to the following decl).

3. Each test is small, single-purpose, and named after the production
   or claim it verifies. Examples:
   - `decl-struct-empty.slang`
   - `decl-enum-with-cases.slang`
   - `decl-typealias.slang`
   - `decl-namespace.slang`
   - `decl-interface-method.slang`
   - `expr-mul-binds-tighter-than-add.slang`
   - `expr-assign-right-associative.slang`
   - `expr-ternary-right-associative.slang`
   - `stmt-do-while.slang`
   - `stmt-switch-case-default.slang`
   - `type-array-suffix.slang`
   - `type-tuple.slang`
   - `generic-call-followed-by-paren.slang`
   - `less-than-in-body-parses-as-comparison.slang`
   - `where-clause-conformance.slang`
   - `attribute-comma-list.slang`
   - `decl-unexpected-token-rejected.slang` (negative)

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/syntax-reference/grammar.md`

Secondary (allowed citations; use sparingly and only when the primary
doc explicitly hands off):

- `docs/generated/design/syntax-reference/tokens.md` — for terminal
  spellings the grammar references via aliases (`IDENT`,
  `INT_LIT`, ...).
- `docs/generated/design/syntax-reference/keywords-and-builtins.md` —
  for the keyword vocabulary referenced by `ModifierKeyword` and the
  decl-opener keywords.
- `docs/generated/design/pipeline/02-parse-ast.md` — for the
  two-stage parse and `<`-disambiguation story the grammar hands
  off to.

If you would cite anything else, stop and record a doc-gap finding in
`README.md`.

## Source files you may consult for _verification only_

You may look at these files to verify that a claim in the doc is
realizable (e.g. the exact spelling of a production, the diagnostic
code emitted for a parse error). You may **not** mine them for
behavioral claims that the doc does not make.

- `source/slang/slang-parser.h`
- `source/slang/slang-parser.cpp`
- `source/compiler-core/slang-lexer.cpp`

If you find yourself thinking "this would cover the branch at
`slang-parser.cpp:NNNN`", stop and re-read the doc.

## Test directives

Grammar claims are largely target-independent: once the AST is built,
all backends see the same shape. So:

- Prefer `//TEST:INTERPRET(filecheck=CHECK):` for positive parses that
  print an observable value distinguishing one bracketing from
  another. Use the smallest body that prints `ok` or a small integer.
- For "is rejected" claims, use
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`. When the caret lands in
  columns 1–9, use the `/*CHECK*/` block-comment form documented in
  `_common.md` rather than a `//CHECK` line.
- **Do not** add multi-target `//TEST:SIMPLE` directives for
  target-independent grammar claims — they are pure repetition of
  identical pre-backend behavior.

Do not use any GPU-only directive.

## Coverage strategy

Aim for **20–35 tests** covering the major production families:
top-level structure, declarations, statements, expressions
(precedence + associativity), types, generics + where-clauses, and
modifiers/attributes. Mix INTERPRET positives with at least one or
two DIAGNOSTIC negatives. Avoid duplicating
`syntax-reference/tokens` (which is purely lexer-level) and
`pipeline/02-parse-ast` (which is parser-stage). The angle of this
bundle is the **production-shape** specifically.

## Cast and observation reminders (carried from `_common.md` / pilot)

- Use **constructor-style casts** (`int('A')`), not C-style
  (`(int)'A'`). The C-style form returns 0 in `slangi`.
- `slangi` `printf` does not support `%s`. For string observation,
  use `//TEST:SIMPLE(filecheck=CHECK):-target cpp` and FileCheck
  the emitted C++ source.
- `static const int x = N;` at file scope is the cleanest pattern
  for asserting a compile-time-known result.
- `slangi` cannot host `cbuffer` or non-const `static` module
  globals; for `cbuffer` claims switch to `-target hlsl`.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `syntax-reference/grammar.md` (or one of the listed secondary
      docs).
- [ ] The bundle exercises **declaration**, **expression**,
      **statement**, **type**, **generic**, and **modifier**
      production families.
- [ ] At least one test exercises the `### `<` disambiguation`
      (`#disambiguation`) anchor on each side — positive generic-call
      and `<`-as-comparison.
- [ ] At least one DIAGNOSTIC test for a parse-stage rejection.
- [ ] At least one INTERPRET test demonstrates precedence
      (e.g. `*` over `+`) and one demonstrates right-associativity
      (`=` or `?:`).
- [ ] No test depends on a GPU.
- [ ] No test was written by inspecting an uncovered source line.
