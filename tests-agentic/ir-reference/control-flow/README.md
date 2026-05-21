---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T18:00:00+00:00
source_commit: bbd84dc65e58598bfa71fafe72764b4076b0869b
watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_doc: docs/llm-generated/ir-reference/control-flow.md
source_doc_digest: c78fb8a143ec9c0010b90e49799d399562fcba32c0b8e6cb2c7a312c06cd7fab
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/control-flow

## Intent

Tests verify the per-opcode catalog of the IR control-flow family
described in
[`docs/llm-generated/ir-reference/control-flow.md`](../../../docs/llm-generated/ir-reference/control-flow.md):
that each documented IR control-flow opcode (`block` parent,
`param` block-parameter (Slang IR's phi replacement), the
`TerminatorInst` family — `return_val`, `unconditionalBranch`,
`loop`, `ifElse`, `switch`, `unreachable`, plus the in-block
`discard`) appears in `-dump-ir` output for the obvious AST-statement
surface that produces it, and that the operand shape matches the
doc.

The primary observation mechanism is `-target spirv-asm -dump-ir -o
/dev/null -entry main -stage compute` (or `-stage fragment` for
`discard`) followed by a FileCheck against the IR dump. Anchors are
user-named functions (`func %main`, `func %helper`, `func %findIndex`)
and user globals (`%a`, `%b`, `%n`, `%sel`); the IR-dump preamble
is large and any unanchored pattern risks false positives.

Conditions and loop predicates are derived from `uniform` globals so
that constant folding does not collapse the structured terminators
to a single block. Switch case labels in one test use
`static const int K = N;` to confirm the doc's claim that case
values appear as inline literal operands regardless of declaration
form.

## Claims enumerated

| Claim ID | Anchor | Claim (one line) | Tests |
| --- | --- | --- | --- |
| C-01 | [#ifelse](../../../docs/llm-generated/ir-reference/control-flow.md#ifelse) | `if (a) ... else ...` lowers to `ifElse(cond, trueBlock, falseBlock, mergeBlock)` with an explicit four-operand structured-join. | [`ifelse-structured-merge.slang`](ifelse-structured-merge.slang) |
| C-02 | [#ifelse](../../../docs/llm-generated/ir-reference/control-flow.md#ifelse) | An `if` without `else` still produces a four-operand `ifElse` with a synthesized merge block. | [`ifelse-empty-else-arm.slang`](ifelse-empty-else-arm.slang) |
| C-03 | [#ifelse](../../../docs/llm-generated/ir-reference/control-flow.md#ifelse) | Nested `if` statements produce nested `ifElse` terminators, each with its own four-operand tuple. | [`nested-ifelse.slang`](nested-ifelse.slang) |
| C-04 | [#terminators-returns-and-yields](../../../docs/llm-generated/ir-reference/control-flow.md#terminators-returns-and-yields) | `return expr;` in a value-returning function terminates with `return_val(val)`. | [`return-val-int.slang`](return-val-int.slang) |
| C-05 | [#terminators-returns-and-yields](../../../docs/llm-generated/ir-reference/control-flow.md#terminators-returns-and-yields) | A `void` function terminates with `return_val(void_constant)`. | [`return-val-void-constant.slang`](return-val-void-constant.slang) |
| C-06 | [#terminators-returns-and-yields](../../../docs/llm-generated/ir-reference/control-flow.md#terminators-returns-and-yields) | An early `return` from inside a loop body produces `return_val` as the terminator of the in-loop block. | [`early-return-from-loop.slang`](early-return-from-loop.slang) |
| C-07 | [#loop](../../../docs/llm-generated/ir-reference/control-flow.md#loop) | A `for` loop lowers to `loop(body, break, continue, ...)` with initial induction values as trailing operands. | [`loop-for-induction-param.slang`](loop-for-induction-param.slang) |
| C-08 | [#loop](../../../docs/llm-generated/ir-reference/control-flow.md#loop) | A `while` loop lowers to the same `loop` opcode as `for`. | [`loop-while.slang`](loop-while.slang) |
| C-09 | [#loop](../../../docs/llm-generated/ir-reference/control-flow.md#loop) | A `do`-`while` loop also lowers to `loop`; the trailing predicate is inverted so the `ifElse` branches to break on true. | [`loop-do-while.slang`](loop-do-while.slang) |
| C-10 | [#loop](../../../docs/llm-generated/ir-reference/control-flow.md#loop) | The back-edge from a loop iteration is an `unconditionalBranch` carrying updated induction values. | [`loop-induction-update-on-branch.slang`](loop-induction-update-on-branch.slang) |
| C-11 | [#terminators-unconditional-branches](../../../docs/llm-generated/ir-reference/control-flow.md#terminators-unconditional-branches) | `break;` and `continue;` lower to `unconditionalBranch`es targeting the loop's break / continue labels. | [`loop-break-continue.slang`](loop-break-continue.slang) |
| C-12 | [#terminators-unconditional-branches](../../../docs/llm-generated/ir-reference/control-flow.md#terminators-unconditional-branches) | `unconditionalBranch` carries argument operands that bind to the target block's `param`s (SSA phi handoff). | [`unconditional-branch-with-args.slang`](unconditional-branch-with-args.slang) |
| C-13 | [#terminators-switches](../../../docs/llm-generated/ir-reference/control-flow.md#terminators-switches) | A `switch` lowers to `switch(value, breakLabel, defaultLabel, caseVal1, caseBlock1, ...)`. | [`switch-multi-case.slang`](switch-multi-case.slang) |
| C-14 | [#terminators-switches](../../../docs/llm-generated/ir-reference/control-flow.md#terminators-switches) | Case-label values from `static const int` declarations resolve to inline `<N> : Int` literal operands on the switch terminator. | [`switch-static-const-case-label.slang`](switch-static-const-case-label.slang) |
| C-15 | [#terminators-no-continuation](../../../docs/llm-generated/ir-reference/control-flow.md#terminators-no-continuation) | When every switch arm `return`s, the synthesized merge block is terminated by `unreachable`. | [`switch-unreachable-merge.slang`](switch-unreachable-merge.slang) |
| C-16 | [#block-and-param](../../../docs/llm-generated/ir-reference/control-flow.md#block-and-param) | A block `param` is Slang IR's encoding of an SSA phi — predecessor branches supply matching argument operands. | [`block-param-from-switch-merge.slang`](block-param-from-switch-merge.slang) |
| C-17 | [#conditionalbranch-vs-ifelse](../../../docs/llm-generated/ir-reference/control-flow.md#conditionalbranch-vs-ifelse) | The front-end emits structured `ifElse` (with explicit merge) — never `conditionalBranch` — for surface-level conditions. | [`loop-conditional-via-ifelse.slang`](loop-conditional-via-ifelse.slang) |
| C-18 | [#discard](../../../docs/llm-generated/ir-reference/control-flow.md#discard) | `discard` is not a terminator; it sits as an ordinary in-block instruction followed by the block's real terminator (an `unconditionalBranch`). | [`discard-non-terminator.slang`](discard-non-terminator.slang) |

## Tests in this bundle

| File | Intent | Doc anchor |
| --- | --- | --- |
| [`ifelse-structured-merge.slang`](ifelse-structured-merge.slang) | functional | `#ifelse` |
| [`ifelse-empty-else-arm.slang`](ifelse-empty-else-arm.slang) | functional | `#ifelse` |
| [`nested-ifelse.slang`](nested-ifelse.slang) | functional | `#ifelse` |
| [`return-val-int.slang`](return-val-int.slang) | functional | `#terminators-returns-and-yields` |
| [`return-val-void-constant.slang`](return-val-void-constant.slang) | functional | `#terminators-returns-and-yields` |
| [`early-return-from-loop.slang`](early-return-from-loop.slang) | functional | `#terminators-returns-and-yields` |
| [`loop-for-induction-param.slang`](loop-for-induction-param.slang) | functional | `#loop` |
| [`loop-while.slang`](loop-while.slang) | functional | `#loop` |
| [`loop-do-while.slang`](loop-do-while.slang) | functional | `#loop` |
| [`loop-induction-update-on-branch.slang`](loop-induction-update-on-branch.slang) | functional | `#loop` |
| [`loop-break-continue.slang`](loop-break-continue.slang) | functional | `#terminators-unconditional-branches` |
| [`unconditional-branch-with-args.slang`](unconditional-branch-with-args.slang) | functional | `#terminators-unconditional-branches` |
| [`switch-multi-case.slang`](switch-multi-case.slang) | functional | `#terminators-switches` |
| [`switch-static-const-case-label.slang`](switch-static-const-case-label.slang) | functional | `#terminators-switches` |
| [`switch-unreachable-merge.slang`](switch-unreachable-merge.slang) | functional | `#terminators-no-continuation` |
| [`block-param-from-switch-merge.slang`](block-param-from-switch-merge.slang) | functional | `#block-and-param` |
| [`loop-conditional-via-ifelse.slang`](loop-conditional-via-ifelse.slang) | functional | `#conditionalbranch-vs-ifelse` |
| [`discard-non-terminator.slang`](discard-non-terminator.slang) | functional | `#discard` |
| [`ifelse-empty-then-arm.slang`](ifelse-empty-then-arm.slang) | boundary | `#ifelse` |
| [`ifelse-deeply-nested-5.slang`](ifelse-deeply-nested-5.slang) | stress | `#ifelse` |
| [`ifelse-ternary-expression.slang`](ifelse-ternary-expression.slang) | boundary | `#ifelse` |
| [`switch-single-case-with-default.slang`](switch-single-case-with-default.slang) | boundary | `#terminators-switches` |
| [`switch-no-default.slang`](switch-no-default.slang) | boundary | `#terminators-switches` |
| [`switch-default-before-cases.slang`](switch-default-before-cases.slang) | boundary | `#terminators-switches` |
| [`switch-eight-case-arms.slang`](switch-eight-case-arms.slang) | stress | `#terminators-switches` |
| [`switch-duplicate-case-labels-rejected.slang`](switch-duplicate-case-labels-rejected.slang) | negative | `#terminators-switches` |
| [`loop-single-iteration-bound.slang`](loop-single-iteration-bound.slang) | boundary | `#loop` |
| [`loop-nested-two-deep.slang`](loop-nested-two-deep.slang) | stress | `#loop` |
| [`loop-infinite-while-true.slang`](loop-infinite-while-true.slang) | boundary | `#loop` |
| [`early-return-in-switch-case.slang`](early-return-in-switch-case.slang) | boundary | `#terminators-returns-and-yields` |
| [`early-return-at-depth-three.slang`](early-return-at-depth-three.slang) | boundary | `#terminators-returns-and-yields` |
| [`all-paths-return-ifelse.slang`](all-paths-return-ifelse.slang) | boundary | `#terminators-no-continuation` |
| [`block-param-from-ifelse-merge.slang`](block-param-from-ifelse-merge.slang) | boundary | `#block-and-param` |
| [`block-param-loop-header-induction.slang`](block-param-loop-header-induction.slang) | boundary | `#block-and-param` |
| [`break-from-inner-loop-only.slang`](break-from-inner-loop-only.slang) | boundary | `#terminators-unconditional-branches` |
| [`break-from-switch-inside-loop.slang`](break-from-switch-inside-loop.slang) | boundary | `#terminators-unconditional-branches` |
| [`discard-rejected-in-compute-stage.slang`](discard-rejected-in-compute-stage.slang) | negative | `#discard` |

## Out of scope (no-GPU runner)

- **`conditionalBranch`** — listed in the doc as `(synthesized)`
  with no AST origin. The front-end emits structured `ifElse` for
  every surface conditional; no portable Slang surface produces
  `conditionalBranch` at the LOWER-TO-IR stage.
- **`yield`** — terminates the body of a `generic` value; observable
  only inside generic-instantiation IR, not via natural surface
  code.
- **`tryCall` / `throw`** — error-flow opcodes. The natural
  `try` / `throw` surface is unstable in current Slang dumps; exercise
  via `cross-cutting/ir-instructions` if at all.
- **`defer`** — `DeferStmt` lowers to `defer`, but the natural
  surface form is non-trivial to write portably.
- **`targetSwitch`** — synthesized by the multi-target lowering
  pass; not produced from surface code on the LOWER-TO-IR stage.
- **`missingReturn`** — synthesized by lowering when a non-`void`
  function falls off the end; the natural surface is rejected as a
  diagnostic before IR is dumped.
- **`gpuForeach`** — emitted by host-shader lowering only.
- **`GenericAsm`** / **`RequirePrelude`** /
  **`RequireTargetExtension`** / **`RequireComputeDerivative`** /
  **`StaticAssert`** / **`Printf`** /
  **`RequireMaximallyReconverges`** / **`RequireQuadDerivatives`** —
  all `(synthesized)` or backend hints per the doc; no portable
  surface anchors them at LOWER-TO-IR.

## Doc gaps observed

- The doc lists `conditionalBranch` with no AST origin and notes
  that it is the lower-level form, but does not state explicitly
  that the front-end *never* emits it directly from surface code.
  A note pinning the producer (some later IR-pass / legalization
  step) would make the cross-bundle boundary with
  `cross-cutting/ir-instructions` clearer.
- The doc's `loop` section names the three structured-join operands
  but does not call out that a `do`-`while` lowering **inverts** the
  test (computing `not(cond)` and branching to break on true). A
  one-line note in the `## loop` heading would prevent readers from
  expecting `cmpLT` + branch-to-body in the do-while case.
- The doc says `discard` "is not technically a terminator" but does
  not name the terminator that *does* end its block in practice
  (`unconditionalBranch` to the post-`if` merge). A short example
  of the surrounding shape would help.
- The doc's `## terminators-no-continuation` heading describes
  `unreachable` semantically but does not name a natural surface
  that produces it (every-arm-returns inside a switch is the
  observed case). An example anchor would help.
- The doc does not specify the operand layout of `ifElse` when one
  arm is empty: in practice the lowering reuses the merge block as
  the empty arm's target (e.g. `ifElse(cond, merge, falseBlock,
  merge)` for an empty then-arm). Stating this collapsing rule
  would clarify how the four-operand invariant interacts with
  degenerate surface forms.
- The doc does not call out that **both-empty** `if`/`else` blocks
  (degenerate, both arms empty) are eliminated at LOWER-TO-IR — no
  `ifElse` is emitted at all. Same for a zero-iteration
  `for (int i=0; i<0; i++)` and an empty switch body. Naming this
  collapsing behavior would prevent readers from expecting a
  terminator that the IR never produces.
- The doc does not call out that dead code following a
  `return` (e.g. `return x; return 0;`) is silently dropped during
  lowering — the second `return_val` does not appear in the IR
  dump, and no `unreachable` is synthesized either. A note alongside
  the `return_val` row would clarify why "unreachable after return"
  is not an observable surface anchor for `unreachable`.
- The doc does not mention that a `while (true)` loop with the only
  exit being an internal `return` produces a structured `loop`
  terminator whose **continueLabel and body are the same block**;
  the break label is reachable only via the return path and is
  terminated by `unreachable`. Naming this shape would help readers
  recognize infinite-loop CFGs.
- The doc does not mention the `[loopMaxIters(N)]` decoration
  emitted on `loop` terminators whose trip count is statically
  known. The decoration is observable in `-dump-ir` output and
  consumers (unroller, structured emitters) read it. A one-line
  note alongside `loop` would document its presence.
- The doc's `#terminators-switches` entry says case-value operands
  are `(caseValue, caseBlock)` pairs but does not pin the
  uniqueness invariant: duplicate case values are rejected by the
  front-end (E30605 in the checker), not by an IR validator. A
  cross-reference to the diagnostic would explain why the IR never
  sees a malformed switch.
- The doc's `#discard` entry describes `discard` as the "HLSL
  fragment-shader instruction" but does not pin the corresponding
  diagnostic code emitted when `discard` is used from a compute or
  vertex entry point (E36107). A cross-reference would make the
  stage restriction enforceable from the doc.
