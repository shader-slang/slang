---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-21T00:00:00Z
source_commit: 1e0d460c1cb410005c4f775ba11fbc803cc8c16d
watched_paths_digest: 12add2d77eb534b6741b9633b4251590baf28be0de7f2554aacada789942f5ec
source_doc: docs/llm-generated/ast-reference/statements.md
source_doc_digest: 18dc5b22d52dd6e4b69d2a6fd26470f083c80ab9ccd65073ee547bccc79af523
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ast-reference/statements

## Intent

Tests verify the documented user-observable roles of the concrete
`Stmt` subclasses enumerated in
[`docs/llm-generated/ast-reference/statements.md`](../../../docs/llm-generated/ast-reference/statements.md):
`BlockStmt`, `SeqStmt`, `IfStmt`, `ForStmt`, `WhileStmt`, `DoWhileStmt`,
`SwitchStmt`, `CaseStmt`, `DefaultStmt`, `BreakStmt`, `ContinueStmt`,
`LabelStmt`, `ReturnStmt`, `DiscardStmt`, `DeferStmt`, `ThrowStmt`,
`CatchStmt`, `DeclStmt`, `ExpressionStmt`, `EmptyStmt`,
`RequireCapabilityStmt`, `TargetSwitchStmt`, `StageSwitchStmt`,
`TargetCaseStmt`, and `CompileTimeForStmt`.

The doc is fundamentally about the internal AST shape of these
classes (parent class in the C++ hierarchy, field names,
`uniqueID`/`targetOuterStmtID` linkage); the user-observable surface
is the **control-flow consequence** of the statement kind. Each test
picks one statement kind and writes the smallest piece of Slang that
exercises that role, with control-flow execution under `INTERPRET`
(or `COMPARE_COMPUTE -cpu` where the interpreter is incomplete) and
diagnostic emission for the ill-formed forms.

Sibling bundle `syntax-reference/keywords-and-builtins/` already
covers parser-side keyword recognition for `if`/`for`/`while`/
`break`/`return`/`switch`/`discard`/etc. The angle here is **AST-level
consequences**: block scoping, branch slot nullability, loop variable
scoping, label-resolution targeting, defer scope-exit ordering, throw/
catch flow, target/stage switch resolution, compile-time-for
unrolling.

Internal AST shape claims (parent class in the C++ hierarchy, private
field names, `uniqueID` identity, synthesized-only classes that have
no user spelling) are recorded under `## Out of scope` because they
are unobservable through any allowed `slang-test` directive.

## Claims enumerated

| Claim ID | Anchor | Claim (one line) | Tests |
| --- | --- | --- | --- |
| C-01 | [#blockstmt-and-seqstmt](../../../docs/llm-generated/ast-reference/statements.md#blockstmt-and-seqstmt) | A `BlockStmt` introduces a new lexical scope via its `ScopeDecl`; a local declared inside `{...}` does not affect a same-named outer variable. | `blockstmt-introduces-scope.slang` |
| C-02 | [#blockstmt-and-seqstmt](../../../docs/llm-generated/ast-reference/statements.md#blockstmt-and-seqstmt) | A `SeqStmt` bundles several statements where a single `Stmt` slot is required; the canonical user surface is multiple declarators in one declaration in statement position. | `seqstmt-multi-declarator.slang` |
| C-03 | [#ifstmt](../../../docs/llm-generated/ast-reference/statements.md#ifstmt) | An `IfStmt` holds predicate, positive, and negative branches; only the matching branch executes. | `ifstmt-else-branch.slang` |
| C-04 | [#ifstmt](../../../docs/llm-generated/ast-reference/statements.md#ifstmt) | An `IfStmt` without an `else` leaves the `negativeStatement` slot null; a false predicate is a no-op. | `ifstmt-no-else-noop.slang` |
| C-05 | [#loop-family](../../../docs/llm-generated/ast-reference/statements.md#loop-family) | A `ForStmt` is a `ScopeStmt`; the `initialStatement` declares a loop variable scoped to the body, leaving a same-named outer variable preserved. | `forstmt-variable-scoped.slang` |
| C-06 | [#loop-family](../../../docs/llm-generated/ast-reference/statements.md#loop-family) | A `WhileStmt` tests its predicate before each iteration; a false initial condition skips the body. | `whilestmt-skips-zero-iterations.slang` |
| C-07 | [#loop-family](../../../docs/llm-generated/ast-reference/statements.md#loop-family) | A `DoWhileStmt` tests its predicate after each iteration; the body always runs at least once. | `dowhilestmt-runs-once.slang` |
| C-08 | [#switchstmt-casestmt-defaultstmt](../../../docs/llm-generated/ast-reference/statements.md#switchstmt-casestmt-defaultstmt) | A `SwitchStmt` dispatches to the matching `CaseStmt`; the body slices into per-case blocks, and `BreakStmt` returns control to the statement after the switch. | `switchstmt-case-dispatch.slang` |
| C-09 | [#switchstmt-casestmt-defaultstmt](../../../docs/llm-generated/ast-reference/statements.md#switchstmt-casestmt-defaultstmt) | A `CaseStmt` is "just a marker"; consecutive empty case markers fall through to the next non-empty body. | `switchstmt-fall-through.slang` |
| C-10 | [#nodes](../../../docs/llm-generated/ast-reference/statements.md#nodes) | A `BreakStmt` exits the innermost enclosing `BreakableStmt`; the outer loop continues. | `breakstmt-exits-innermost-loop.slang` |
| C-11 | [#nodes](../../../docs/llm-generated/ast-reference/statements.md#nodes) | A `BreakStmt` outside any loop or switch is rejected because no enclosing `BreakableStmt` matches its target. | `breakstmt-outside-loop-rejected.slang` |
| C-12 | [#nodes](../../../docs/llm-generated/ast-reference/statements.md#nodes) | A `ContinueStmt` skips the remainder of the current iteration of the innermost loop. | `continuestmt-skips-rest-of-iteration.slang` |
| C-13 | [#labelstmt-and-breakstmttargetlabel](../../../docs/llm-generated/ast-reference/statements.md#labelstmt-and-breakstmttargetlabel) | A `LabelStmt` attaches a label to an inner statement; a `BreakStmt` with that `targetLabel` breaks out of the labeled (non-innermost) loop. | `labelstmt-labeled-break.slang` |
| C-14 | [#nodes](../../../docs/llm-generated/ast-reference/statements.md#nodes) | A `ReturnStmt` yields a value and halts the function; statements after a taken return do not execute. | `returnstmt-halts-function.slang` |
| C-15 | [#nodes](../../../docs/llm-generated/ast-reference/statements.md#nodes) | A `ReturnStmt` in a non-void function requires an expression; `return;` is rejected. | `returnstmt-missing-expression-rejected.slang` |
| C-16 | [#nodes](../../../docs/llm-generated/ast-reference/statements.md#nodes) | A `DiscardStmt` is a fragment-only pixel-kill; using it from a compute entry point is rejected as a stage-unavailable feature. | `discardstmt-non-fragment-rejected.slang` |
| C-17 | [#deferstmt](../../../docs/llm-generated/ast-reference/statements.md#deferstmt) | A `DeferStmt` enqueues its inner statement to run when the enclosing scope exits; the deferred side effect lands after the surrounding statements. | `deferstmt-scope-exit-order.slang` |
| C-18 | [#deferstmt](../../../docs/llm-generated/ast-reference/statements.md#deferstmt) | A `DeferStmt`'s inner statement runs on every scope-exit path, including an early `return`. | `deferstmt-runs-on-early-return.slang` |
| C-19 | [#throwstmt-and-catchstmt](../../../docs/llm-generated/ast-reference/statements.md#throwstmt-and-catchstmt) | A `ThrowStmt` in an errorable function transfers control to the caller's matching `CatchStmt`; the handler runs and the protected body is short-circuited. | `throw-catch-handler-runs.slang` |
| C-20 | [#throwstmt-and-catchstmt](../../../docs/llm-generated/ast-reference/statements.md#throwstmt-and-catchstmt) | A `CatchStmt` with null `errorVar` is a catch-all; the handler runs for any thrown error and does not bind the error value. | `catchstmt-catch-all.slang` |
| C-21 | [#throwstmt-and-catchstmt](../../../docs/llm-generated/ast-reference/statements.md#throwstmt-and-catchstmt) | A `ThrowStmt` is only valid inside a function declared to throw; an uncaught `throw` in a non-throwing function is rejected. | `throwstmt-without-throws-rejected.slang` |
| C-22 | [#declstmt-and-expressionstmt](../../../docs/llm-generated/ast-reference/statements.md#declstmt-and-expressionstmt) | A `DeclStmt` lets a `DeclBase` appear in statement position; the local name is visible to later statements. | `declstmt-local-visible-later.slang` |
| C-23 | [#declstmt-and-expressionstmt](../../../docs/llm-generated/ast-reference/statements.md#declstmt-and-expressionstmt) | An `ExpressionStmt` holds an `Expr` used for its side effects; the value is discarded but side effects (function call, `printf`) are observed. | `expressionstmt-side-effect.slang` |
| C-24 | [#nodes](../../../docs/llm-generated/ast-reference/statements.md#nodes) | An `EmptyStmt` is a bare `;` accepted in statement position; it produces no effect. | `emptystmt-bare-semicolon.slang` |
| C-25 | [#requirecapabilitystmt](../../../docs/llm-generated/ast-reference/statements.md#requirecapabilitystmt) | A `RequireCapabilityStmt` (`__requireCapability(...)`) is a statement-level capability assertion scoped to the enclosing function. | `requirecapability-in-function.slang` |
| C-26 | [#compiletimeforstmt](../../../docs/llm-generated/ast-reference/statements.md#compiletimeforstmt) | A `CompileTimeForStmt` (`$for(i in Range(N))`) is unrolled at compile time; the body executes once per iteration with the loop variable substituted as a compile-time constant. | `compiletimefor-unrolls.slang` |
| C-27 | [#targetswitchstmt-stageswitchstmt-targetcasestmt](../../../docs/llm-generated/ast-reference/statements.md#targetswitchstmt-stageswitchstmt-targetcasestmt) | A `StageSwitchStmt` is statically resolved by entry-point stage at compile time; only the matching case body survives in emitted text. | `stageswitch-static-dispatch.slang` |
| C-28 | [#targetswitchstmt-stageswitchstmt-targetcasestmt](../../../docs/llm-generated/ast-reference/statements.md#targetswitchstmt-stageswitchstmt-targetcasestmt) | A `TargetSwitchStmt` is statically resolved by capability at compile time; non-matching `TargetCaseStmt` bodies are eliminated. | `targetswitch-static-dispatch.slang` |

## Tests in this bundle

| File | Intent | Doc anchor |
| --- | --- | --- |
| `blockstmt-introduces-scope.slang` | functional | `#blockstmt-and-seqstmt` |
| `seqstmt-multi-declarator.slang` | functional | `#blockstmt-and-seqstmt` |
| `ifstmt-else-branch.slang` | functional | `#ifstmt` |
| `ifstmt-no-else-noop.slang` | functional | `#ifstmt` |
| `forstmt-variable-scoped.slang` | functional | `#loop-family` |
| `whilestmt-skips-zero-iterations.slang` | functional | `#loop-family` |
| `dowhilestmt-runs-once.slang` | functional | `#loop-family` |
| `switchstmt-case-dispatch.slang` | functional | `#switchstmt-casestmt-defaultstmt` |
| `switchstmt-fall-through.slang` | functional | `#switchstmt-casestmt-defaultstmt` |
| `breakstmt-exits-innermost-loop.slang` | functional | `#nodes` |
| `breakstmt-outside-loop-rejected.slang` | negative | `#nodes` |
| `continuestmt-skips-rest-of-iteration.slang` | functional | `#nodes` |
| `labelstmt-labeled-break.slang` | functional | `#labelstmt-and-breakstmttargetlabel` |
| `returnstmt-halts-function.slang` | functional | `#nodes` |
| `returnstmt-missing-expression-rejected.slang` | negative | `#nodes` |
| `discardstmt-non-fragment-rejected.slang` | negative | `#nodes` |
| `deferstmt-scope-exit-order.slang` | functional | `#deferstmt` |
| `deferstmt-runs-on-early-return.slang` | functional | `#deferstmt` |
| `throw-catch-handler-runs.slang` | functional | `#throwstmt-and-catchstmt` |
| `catchstmt-catch-all.slang` | functional | `#throwstmt-and-catchstmt` |
| `throwstmt-without-throws-rejected.slang` | negative | `#throwstmt-and-catchstmt` |
| `declstmt-local-visible-later.slang` | functional | `#declstmt-and-expressionstmt` |
| `expressionstmt-side-effect.slang` | functional | `#declstmt-and-expressionstmt` |
| `emptystmt-bare-semicolon.slang` | functional | `#nodes` |
| `requirecapability-in-function.slang` | functional | `#requirecapabilitystmt` |
| `compiletimefor-unrolls.slang` | functional | `#compiletimeforstmt` |
| `stageswitch-static-dispatch.slang` | functional | `#targetswitchstmt-stageswitchstmt-targetcasestmt` |
| `targetswitch-static-dispatch.slang` | functional | `#targetswitchstmt-stageswitchstmt-targetcasestmt` |
| `blockstmt-empty.slang` | boundary | `#blockstmt-and-seqstmt` |
| `blockstmt-single-statement.slang` | boundary | `#blockstmt-and-seqstmt` |
| `blockstmt-deeply-nested-stress.slang` | stress | `#blockstmt-and-seqstmt` |
| `ifstmt-elseif-chain-stress.slang` | stress | `#ifstmt` |
| `ifstmt-predicate-evaluated-once.slang` | boundary | `#ifstmt` |
| `forstmt-zero-iterations.slang` | boundary | `#loop-family` |
| `forstmt-one-iteration.slang` | boundary | `#loop-family` |
| `forstmt-many-iterations-stress.slang` | stress | `#loop-family` |
| `forstmt-init-only-with-break.slang` | boundary | `#loop-family` |
| `whilestmt-true-with-break.slang` | boundary | `#loop-family` |
| `switchstmt-default-only.slang` | boundary | `#switchstmt-casestmt-defaultstmt` |
| `switchstmt-no-default.slang` | boundary | `#switchstmt-casestmt-defaultstmt` |
| `breakstmt-from-switch.slang` | boundary | `#switchstmt-casestmt-defaultstmt` |
| `continuestmt-outside-loop-rejected.slang` | negative | `#nodes` |
| `returnstmt-void-bare.slang` | boundary | `#nodes` |
| `returnstmt-inside-loop.slang` | boundary | `#nodes` |
| `deferstmt-multiple-lifo-order.slang` | boundary | `#deferstmt` |
| `deferstmt-at-function-entry.slang` | boundary | `#deferstmt` |
| `throwstmt-enum-value-preserved.slang` | boundary | `#throwstmt-and-catchstmt` |
| `catchstmt-nested-try.slang` | boundary | `#throwstmt-and-catchstmt` |
| `compiletimefor-zero-range.slang` | boundary | `#compiletimeforstmt` |
| `compiletimefor-large-range-stress.slang` | stress | `#compiletimeforstmt` |

## Doc gaps observed

- The `UnscopedForStmt` row in `## Nodes` documents the
  compatibility form "where the loop variable leaks into the
  surrounding scope", but the parser produces an `UnscopedForStmt`
  only when the source-language detection sets `SourceLanguage::HLSL`.
  A `.slang` source file (the only kind `slang-test` exercises with
  the `INTERPRET` directive) always parses as Slang and never reaches
  the `UnscopedForStmt` path. The doc could clarify that
  `UnscopedForStmt` is reachable only from HLSL input and not via a
  user-spellable `.slang` modifier; without that pointer an agent
  cannot anchor a test here.
- `RequireCapabilityStmt` is documented as a statement-level
  capability requirement, but the doc does not state which spelling
  produces it (`__requireCapability` is the parser token, while the
  module-scope `RequireCapabilityDecl` is `__require_capability`).
  A one-line note ("statement-level keyword is `__requireCapability`")
  would let the bundle's test anchor more directly than the current
  prose paragraph.
- The doc says "BreakStmt inside a SwitchStmt is matched via
  BreakableStmt::uniqueID" but does not state the user-observable
  consequence (that a labeled `break` can break out of a non-
  innermost switch as well as a non-innermost loop). The bundle
  exercises the labeled-loop case; the labeled-switch case is
  symmetric but has no separate doc claim and so no separate test.
- The doc lists `IntrinsicAsmStmt` as a node but notes it is "used by
  core-module intrinsics"; it has no user spelling. The same applies
  to `UnparsedStmt` ("reserved-content block deferred to a downstream
  compiler"). One line ("not user-spellable, exists only inside the
  core module / inline-HLSL paths") would let an agent skip these
  without doubt.
- The doc says `BreakableStmt` has a `uniqueID` "that `ChildStmt`s
  reference via `targetOuterStmtID`", and that `UniqueStmtIDNode`
  is "declared in this header as a `Decl` subclass for serialization
  convenience". The user-facing consequence is the matching of a
  `break` to its enclosing breakable, which we test indirectly via
  the labeled-break test; the `uniqueID` mechanism itself is
  internal and not separately observable.
- The doc points to `../syntax-reference/grammar.md#statements` for
  the grammar of each statement form, and to `../pipeline/04-ast-to-ir.md`
  for the IR lowering of `DeferStmt`, `ThrowStmt`, `CompileTimeForStmt`,
  and `TargetSwitchStmt`. The bundle exercises the AST-level
  consequence of each; the IR-side claims belong to
  `ir-reference/control-flow/`.

## Out of scope

The doc is overwhelmingly about internal AST shape. The following
claim families are not observable through `slangc` / `slang-test`
directives that this bundle can run; they are recorded here rather
than tested.

- The C++ parent class of any concrete `Stmt` (e.g. that
  `BlockStmt extends ScopeStmt`, that `ForStmt`/`WhileStmt`/
  `DoWhileStmt` all extend `LoopStmt`, that `JumpStmt extends
  ChildStmt`). Only the user-observable control-flow behavior is
  testable.
- Field names listed in the `## Nodes` table (e.g.
  `IfStmt::predicate`, `IfStmt::positiveStatement`,
  `IfStmt::negativeStatement`, `IfStmt::afterLoc`,
  `ForStmt::initialStatement`, `ForStmt::predicateExpression`,
  `ForStmt::sideEffectExpression`, `ForStmt::statement`,
  `WhileStmt::predicate`/`statement`, `DoWhileStmt::predicate`/
  `statement`, `SwitchStmt::condition`/`body`,
  `CaseStmt::expr`/`exprVal`, `BreakStmt::targetLabel`,
  `ReturnStmt::expression`, `BlockStmt::body`/`closingSourceLoc`,
  `LabelStmt::label`/`innerStmt`, `DeclStmt::decl`,
  `ExpressionStmt::expression`, `DeferStmt::statement`,
  `ThrowStmt::expression`, `CatchStmt::errorVar`/`tryBody`/
  `handleBody`, `RequireCapabilityStmt::requiredCaps`,
  `TargetCaseStmt::capability`/`capabilityToken`/`body`,
  `CompileTimeForStmt::varDecl`/`rangeBeginExpr`/`rangeEndExpr`/
  `rangeBeginVal`/`rangeEndVal`/`body`).
- The grammar production a parser callback is named (e.g.
  `parseTargetSwitchStmtImpl`, `parseCompileTimeForStmt`,
  `ParseDeferStatement`, `ParseRequireCapabilityStatement`).
- The `## Family hierarchy` mermaid diagram as a graph: abstract
  intermediates (`ScopeStmt`, `BreakableStmt`, `LoopStmt`,
  `ChildStmt`, `JumpStmt`, `CaseStmtBase`) carry no FIDDLE concrete
  tag and produce no user spelling of their own.
- The `BreakableStmt::uniqueID` and `ChildStmt::targetOuterStmtID`
  linkage. The user-visible consequence (a break/continue targets
  the right enclosing statement) is tested; the field-level identity
  is not.
- `UniqueStmtIDNode` (declared as a `Decl` subclass for
  serialization convenience, "not parsed as a statement").
- Statement kinds with no user-spellable form on the `.slang`
  source surface this bundle can exercise:
  - `IntrinsicAsmStmt` — used only by core-module intrinsics.
  - `UnparsedStmt` — reserved content deferred to a downstream
    compiler (e.g. inline HLSL); not reachable from a `.slang`
    source file via the `slang-test` directives this bundle uses.
  - `GpuForeachStmt` — the host-side `__GPU_FOREACH` grammar is not
    exercised by the agentic suite.
  - `UnscopedForStmt` — only produced when the parser is in HLSL
    source-language mode; not reachable from a `.slang` file (see
    doc gap above).
- `CompileTimeForStmt`'s `rangeBeginVal`/`rangeEndVal` as `IntVal*`
  filled in by checking — only the unrolled-body behavior is
  observable.
- IR-lowering side claims that the doc cross-links to
  `../pipeline/04-ast-to-ir.md` (defer materialization as scope-exit
  handlers, target-switch resolution, compile-time-for unrolling at
  the IR level). The IR-level behavior is tested in
  `ir-reference/control-flow/`; here we observe the source-level
  consequence only.
- `LabelStmt` on a non-loop, non-switch target. The doc only
  mentions labels on "enclosing labeled loop or switch", so other
  labelings (e.g. on a block) are an unstated claim.
