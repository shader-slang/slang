# Prompt: docs/generated/tests/ast-reference/statements/

See [`_common.md`](_common.md) for universal rules. See
[`ast-reference-declarations.md`](ast-reference-declarations.md) and
[`ast-reference-expressions.md`](ast-reference-expressions.md) for the
sibling AST-reference prompts; the "claims to observations" translation
rule established there applies here unchanged.

## Target

Produce the test bundle at `docs/generated/tests/ast-reference/statements/`,
anchored to
[`docs/generated/design/ast-reference/statements.md`](../../../docs/generated/design/ast-reference/statements.md).

Audience: nightly CI. The bundle exercises the concrete `Stmt`
subclasses enumerated in the doc — `BlockStmt`, `SeqStmt`, `IfStmt`,
`ForStmt`, `UnscopedForStmt`, `WhileStmt`, `DoWhileStmt`, `SwitchStmt`,
`CaseStmt`, `DefaultStmt`, `BreakStmt`, `ContinueStmt`, `ReturnStmt`,
`DiscardStmt`, `DeferStmt`, `ThrowStmt`, `CatchStmt`, `LabelStmt`,
`EmptyStmt`, `DeclStmt`, `ExpressionStmt`, `RequireCapabilityStmt`,
`TargetSwitchStmt`, `StageSwitchStmt`, `TargetCaseStmt`,
`CompileTimeForStmt` — through their **observable consequences** at
parse / check / control-flow-execution time.

This bundle is **medium-sized** (size cap 40). Aim for 12–25 tests so
each major statement kind gets one observable test plus a handful of
diagnostic tests for the natural rejection forms.

## The translation rule (carried from `ast-reference-base.md`)

`statements.md` enumerates one concrete `Stmt` subclass per syntactic
statement form. Slang as a compiler does **not** expose its AST to the
user. So a claim such as "`BlockStmt` is a `ScopeStmt` carrying a
`ScopeDecl`" is testable only via its observable surface: a name
declared inside a `{}` block is not visible outside the block; the
same name can be reused after the block closes.

- **Testable** ⇔ "if the doc's claim about this statement kind were
  false, the program-text behavior we wrote would change in a way
  `slangc` reports (a value, a diagnostic, or a control-flow path
  taken)."
- **Not testable through slangc** ⇔ "the claim is about which C++
  class the parser allocated for the node, which field on that class
  holds the body, the `uniqueID` identity, or which intermediate AST
  class is later rewritten by IR lowering."

### Observable claims (write tests for these)

The doc's `## Nodes` table lists ~25 concrete statement kinds. Most
claims are user-observable as the value computed, the branch taken, or
the diagnostic produced. Group them by family for coverage:

- **BlockStmt + scoping** (`#blockstmt-and-seqstmt`) — `{ ... }`
  introduces a new lexical scope; a local declared inside is not
  visible outside; a same-named outer variable is shadowed inside.
- **SeqStmt + multi-declarator** (`#blockstmt-and-seqstmt`) — the
  parser bundles `int a, b;` into a `DeclGroup`-bearing `SeqStmt` /
  `DeclStmt` and both names are independently observable. (This is
  the only kind whose observable surface overlaps with the
  declarations bundle's `declgroup-multi-var.slang`; the angle here
  is the **statement-position** containment, not the decl-group
  structure.)
- **IfStmt** (`#ifstmt`) — `if (cond) ... else ...` picks the right
  branch; `if (cond) ...` without an `else` leaves the `else` slot
  null so the negative side is a no-op. Use both forms.
- **SwitchStmt + CaseStmt + DefaultStmt**
  (`#switchstmt-casestmt-defaultstmt`) — `switch (e) { case k: ...
  default: ... }` dispatches by value, the body slices into per-case
  blocks, fall-through behavior is observable. The interpreter
  cannot evaluate `switch`; prefer `-target cpp` text-emit
  observation.
- **Loop family** (`#loop-family`) — `for (init; cond; step) body`,
  `while (cond) body`, `do body while (cond)` each iterate as
  expected; the loop variable of `for` is scoped to the body, so a
  same-named outer variable is preserved.
- **UnscopedForStmt** (`#nodes`) — the legacy HLSL form that does
  **not** scope the loop variable to the body. The observable form
  is the doc's note that this is a compatibility form; in
  user-written Slang it requires `[unscoped_for]` and the loop var
  leaks. A simple parse-only test on `-target hlsl` is enough.
- **BreakStmt / ContinueStmt** (`#nodes`) — `break` exits the
  innermost enclosing `BreakableStmt`; `continue` skips to the next
  iteration of the innermost `LoopStmt`.
- **LabelStmt + labeled break** (`#labelstmt-and-breakstmttargetlabel`)
  — `label: for (...) { break label; }` breaks out of a non-innermost
  loop. The observable is the iteration count.
- **ReturnStmt** (`#nodes`) — `return e` halts the function and yields
  `e`; `return;` in a `void` function is accepted; `return` without
  an expression in a non-void function is rejected.
- **DiscardStmt** (`#nodes`) — `discard;` is only valid in a fragment
  shader; outside fragment context it diagnoses. The fragment-stage
  acceptance is already covered by
  `syntax-reference/keywords-and-builtins/statement-discard-fragment.slang`;
  the AST-side angle is the **rejection** of `discard` in a compute
  shader.
- **DeferStmt** (`#deferstmt`) — `defer S;` enqueues `S` to run at
  scope exit, including early return / break paths. The observable
  is the order in which side effects land.
- **ThrowStmt + CatchStmt** (`#throwstmt-and-catchstmt`) — a function
  declared `throws E` may `throw e;` and the caller's `do { try f();
  } catch (e: E) { ... }` runs the handler. A null `errorVar` is a
  catch-all.
- **DeclStmt** (`#declstmt-and-expressionstmt`) — a `DeclBase` (local
  variable) appears in statement position; the name is visible to
  later statements.
- **ExpressionStmt** (`#declstmt-and-expressionstmt`) — an arbitrary
  `Expr` is used for its side effects. The canonical observation is
  a function call whose return value is unused.
- **EmptyStmt** (`#nodes`) — a bare `;` is accepted.
- **RequireCapabilityStmt** (`#requirecapabilitystmt`) — the
  statement-level form takes a capability atom and applies it to the
  enclosing function. The doc spells the **statement** form; the
  **declaration** form is the subject of
  `ast-reference/declarations/`'s `requirecapabilitydecl.slang`. The
  angle here is the in-function spelling.
- **TargetSwitchStmt / StageSwitchStmt + TargetCaseStmt**
  (`#targetswitchstmt-stageswitchstmt-targetcasestmt`) — static
  dispatch by capability / pipeline stage. The observable is the
  emit text: only one case body survives.
- **CompileTimeForStmt** (`#compiletimeforstmt`) — `$for(i in
  Range(N))` is unrolled at compile time; the body repeats N times
  with `i` substituted as a compile-time constant.

### Negative / diagnostic claims (one per family that has one)

- **DiscardStmt** — `discard` in a compute shader diagnoses.
- **ReturnStmt** — missing return value in a non-void function
  diagnoses (the diagnostic attaches to the function signature line,
  not the `return;` line per the lessons captured).
- **BreakStmt** — `break` outside any loop or `switch` diagnoses.
- **ContinueStmt** — `continue` outside any loop diagnoses.
- **ThrowStmt** — `throw e;` in a function not declared `throws E`
  diagnoses (covered minimally; the deep error-handling tests are
  the subject of language-feature tests).

### Not testable through slangc (do NOT write tests for these)

The doc carries many claims about the **internal AST shape** of these
statements. These are unobservable through `slangc` text I/O. Record
them under `## Untested claims` in `README.md`. Examples:

- That `IfStmt::negativeStatement` is a raw `Stmt*` (only the
  branch-selection behavior is observable).
- That `BreakableStmt` carries a `UniqueStmtIDNode* uniqueID` and
  `ChildStmt` references it via `targetOuterStmtID` (only the
  user-visible matching of `break` / labeled `break` to the right
  enclosing statement is observable).
- That `LoopStmt` is the C++ parent of `ForStmt`, `WhileStmt`, and
  `DoWhileStmt`.
- That `ForStmt::initialStatement` is parsed as a `Stmt` rather than
  an `Expr` so a `DeclStmt` can introduce the loop variable (the
  observable is just "the for-init declaration is in scope inside
  the body").
- That `BlockStmt` carries a `closingSourceLoc` (only diagnostic
  positions reference it, and even those are not part of the user
  surface for this bundle).
- That `IfStmt::afterLoc` records the source location after the
  `if` (a language-server signal).
- The internal layout of `SeqStmt::stmts` as `List<Stmt*>`.
- That `IntrinsicAsmStmt` is used only by core-module intrinsics
  (no user-spellable form).
- That `UnparsedStmt` is reserved content for downstream compilers
  (no user-spellable form via the `slang-test` surface this bundle
  runs).
- That `GpuForeachStmt` exists (the `__GPU_FOREACH` keyword is a
  host-side compute-foreach grammar that is not exercised by the
  agentic suite).
- That `UniqueStmtIDNode` is declared as a `Decl` subclass for
  serialization convenience.

If you find yourself thinking "this would verify that the AST node
allocated is class X" or "this would assert that field F is a
`Stmt*`", stop — that is a source-targeting probe in disguise.
Re-frame as "the documented role of this statement kind".

## Avoid duplication with sibling bundles

- `syntax-reference/keywords-and-builtins/` — already covers
  **keyword recognition** for `if`, `else`, `for`, `while`, `do`,
  `break`, `continue`, `return`, `switch`, `case`, `default`,
  `discard`. The angle here is **AST-level consequences** (scoping,
  fall-through, label resolution, defer-ordering, branch slot
  nullability, control-flow termination, target switch resolution),
  not "the parser recognizes the keyword".
- `pipeline/02-parse-ast/` — covers **parse-stage** observable
  claims (body deferral, statement-vs-expression disambiguation).
  Do not duplicate that angle here.
- `ast-reference/declarations/` — covers `RequireCapabilityDecl`
  (module-scope form). The angle here for `RequireCapabilityStmt`
  is the function-body form.
- `ast-reference/expressions/` — covers `Expr` kinds. Avoid pulling
  an expression-level claim into a statement-level test.
- `ir-reference/control-flow/` — covers IR-level control-flow
  instructions. Avoid asserting IR shape; assert the source-level
  effect.

If a claim is best tested in a sibling bundle, do not duplicate it
here. If two bundles seem to want the same test, prefer the more
specific one and cite the doc anchor for the angle this bundle
takes.

## Allowed secondary doc citations

- `docs/generated/design/ast-reference/base.md`
- `docs/generated/design/syntax-reference/grammar.md`
- `docs/generated/design/pipeline/02-parse-ast.md`

If you would cite anything else, stop and record a doc-gap finding in
`README.md`.

## Source files you may consult for _verification only_

- `source/slang/slang-ast-stmt.h`
- `source/slang/slang-ast-base.h`
- `source/slang/slang-parser.cpp`

You may look at these files to verify that a claim in the doc
corresponds to a real parse path (e.g. that `__target_switch` is the
actual keyword spelling for `TargetSwitchStmt`, that `defer` opens a
`DeferStmt`). You may **not** mine them for behavioral claims that the
doc does not make, and you may **not** write tests that probe internal
class identity.

## Test directives

Most statement-level claims are **target-independent** (the statement
shape and its control-flow effects are the same regardless of
backend). So:

- `//TEST:INTERPRET(filecheck=CHECK):` — **primary** directive. Use
  `printf` to observe the branch taken, the iteration count, the
  defer-ordering, or the value returned.
- `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` — for negative claims:
  `discard` outside fragment, `break` outside loop, missing return,
  `throw` without `throws`.
- `//TEST:SIMPLE(filecheck=CHECK):-target <X>` — use **only** when
  the statement's lowering observably differs per target. The
  clearest cases are:
  - `SwitchStmt` (the interpreter does not evaluate `switch`; use
    `-target cpp` to confirm the `switch` survives).
  - `DiscardStmt` (only the fragment-stage HLSL backend emits it;
    already covered by the sibling bundle — only the rejection in a
    non-fragment context is the AST-side angle).
  - `TargetSwitchStmt` / `StageSwitchStmt` (the unselected case
    bodies do not appear in emitted text).
  - `UnscopedForStmt` (HLSL-only legacy).
- Multi-target directives are not necessary for most statement
  claims; control flow lowers identically across the text backends
  that this suite can run.

## Cast and observation reminders (carried from `_common.md` / pilot)

- Use **constructor-style casts** (`int('A')`), not C-style
  (`(int)'A'`).
- `slangi` `printf` does **not** support `%s`. For string-typed
  observation, use `-target cpp` text emit.
- `slangi` cannot evaluate `switch` bytecode in all configurations;
  for switch-shape claims, observe the C++ emit (`-target cpp`)
  instead.
- `static const int x = N;` at file scope is the cleanest pattern
  for asserting a compile-time-known result.
- The runner's "Suggested annotations" output is the source of
  truth for diagnostic caret positions. Hand-counting carets is
  unreliable.
- Caret `^` placement in `//CHECK:` requires column `>= 10`. For
  earlier columns, use the `/*CHECK: ... */` block-comment form.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ast-reference/statements.md` (or one of the listed secondary
      docs).
- [ ] The bundle exercises **all major statement families** named in
      `## Nodes` whose role is user-observable: block scoping,
      `if`/`else` branch selection, loop iteration / variable
      scoping, `switch` / `case` / `default` dispatch, `break` /
      `continue` / labeled break, `return` (with and without
      value), `discard` (fragment-only), `defer` (scope-exit
      ordering), `throw` / `catch` (errorable functions), decl /
      expression / empty statements, `__target_switch` /
      `__stage_switch` static dispatch, `$for ... in Range(...)`
      compile-time unrolling, `__requireCapability` in function
      body.
- [ ] At least one negative / diagnostic test per family that has a
      natural negative form (`discard` outside fragment, `break`
      outside loop, missing return value, `throw` without `throws`).
- [ ] No test asserts the C++ identity of an AST node, the parent
      class in the C++ hierarchy, or the layout of a field list.
- [ ] No test depends on a GPU. `INTERPRET` and diagnostic-only
      directives carry almost the whole bundle.
- [ ] No test was written by inspecting an uncovered source line. If
      you find yourself thinking "this would cover the branch at
      `slang-parser.cpp:NNNN`", stop and re-read the doc.
- [ ] Synthesized / non-user-spellable nodes (`UnparsedStmt`,
      `IntrinsicAsmStmt`, `GpuForeachStmt`, `UniqueStmtIDNode`) are
      recorded under `## Untested claims` in `README.md`, not as tests.
- [ ] `README.md` `## Doc gaps observed` is honest. If a claim
      wanted a test but had no good anchor (or the anchor was too
      coarse), record it.
