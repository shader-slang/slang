---
generated: true
model: claude-opus-4-8[1m]
generated_at: 2026-06-11T13:43:23Z
source_commit: ef1068b5485e09b3a7afadba2e25f9541e29af42
watched_paths_digest: b6659b833f864303089712cf3ac328ce498f4412e693617dae58416e5c50162c
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/lower-to-ir

## Intent

White-box characterization tests for `source/slang/slang-lower-to-ir.cpp`
(AST -> Slang IR lowering; ~66% covered). They pin the **current observed
behaviour** of CLI-reachable statement- and expression-lowering branches,
not a spec. Strategy: target the under-tested control-flow and aggregate
lowering hooks in the hint area — `visitDeferStmt`, `visitSwitchStmt` /
`lowerSwitchCases` / `getLabelForCase`, the scalar-vs-vector split in
`visitSelectExpr`, aggregate (struct/array) initializer lowering, and
compound-assignment lowering in `visitAssignExpr`.

Observation points were chosen per the construct's stable contract:
behaviours observable in target text (the if/else shape a scalar `?:`
lowers to, the `OpSelect` a vector select lowers to, the shared-label /
fall-through structure of a `switch`, the side-effect survival of a
no-case `switch`, the scope-end placement of a `defer`) are pinned with
`SIMPLE` emit (HLSL or SPIR-V) — FileCheck, reported `ignored` locally and
validated in CI. Value-level behaviours that do not trip the known slangi
VM bugs (defer LIFO/early-return ordering, aggregate init, compound assign)
are pinned with `INTERPRET`, which validates locally.

All emitted text, instructions, and printed values were copied verbatim
from the local `slangc` / `slangi` at `source_commit`; none are spec
claims, so none carry `characterization-unverified=true`.

## Functional coverage

| Test                                                                                     | What it pins (current behaviour)                                                                                                                   | covers=                            |
| ---------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------- |
| [`compound-assignment-chain.slang`](compound-assignment-chain.slang)                     | A chain `x += 5; x *= 2; x -= 3; x /= 2;` from 10 read-modify-writes the same local in source order to 13 (integer division).                      | source/slang/slang-lower-to-ir.cpp |
| [`defer-lifo-and-scope-end.slang`](defer-lifo-and-scope-end.slang)                       | Two `defer` blocks in one block run at scope end in LIFO order: "body" then "second" then "first".                                                 | source/slang/slang-lower-to-ir.cpp |
| [`defer-runs-on-early-return.slang`](defer-runs-on-early-return.slang)                   | A function-scope `defer` fires before an early `return` and before the fall-through return — on both exit edges.                                   | source/slang/slang-lower-to-ir.cpp |
| [`defer-scope-end-hlsl-emit.slang`](defer-scope-end-hlsl-emit.slang)                     | A block `defer { outputBuffer[1]=x; }` lowers so the deferred store emits at block close (after x is mutated) and before code following the block. | source/slang/slang-lower-to-ir.cpp |
| [`scalar-ternary-lowers-to-ifelse.slang`](scalar-ternary-lowers-to-ifelse.slang)         | A scalar `(a<b)?a:b` inside a function lowers to a temporary plus an `if/else` (short-circuit form), not a single select.                          | source/slang/slang-lower-to-ir.cpp |
| [`struct-and-array-aggregate-init.slang`](struct-and-array-aggregate-init.slang)         | `P p = {3,4}` maps positionally to x=3,y=4 and `int arr[3]={10,20,30}` maps by index.                                                              | source/slang/slang-lower-to-ir.cpp |
| [`switch-fallthrough-and-shared-case.slang`](switch-fallthrough-and-shared-case.slang)   | Adjacent empty cases share one label, and a non-`break` case (`case 2`) falls through into the `default` body.                                     | source/slang/slang-lower-to-ir.cpp |
| [`switch-no-cases-evaluates-condition.slang`](switch-no-cases-evaluates-condition.slang) | A `switch` with no case/default still lowers (and keeps the side effects of) its condition but emits no switch instruction.                        | source/slang/slang-lower-to-ir.cpp |
| [`vector-select-lowers-to-select-inst.slang`](vector-select-lowers-to-select-inst.slang) | A vector `select(c,a,b)` (non-basic condition) lowers to a single `OpSelect` in SPIR-V, not an if/else.                                            | source/slang/slang-lower-to-ir.cpp |

## Unreachable gaps

- **`break` / `continue` `targetOuterStmtID` asserts** (`visitBreakStmt`,
  `visitContinueStmt`): both `SLANG_ASSERT(targetStmtID != kInvalidUniqueID)`
  and the `SLANG_ASSERT(targetBlock)` are unreachable from valid input —
  semantic checking resolves the target before lowering, so a CLI program
  cannot drive a break/continue with no resolved target.
- **Labeled `continue` is not reachable** from the CLI surface:
  `continue outer;` is a parse error (E20001 "unexpected token") before
  lowering, so the labeled-continue lowering path cannot be exercised by a
  `.slang` input. (Labeled `break outer;` parses and lowers fine.)
- **`switch` inside a function under slangi** aborts the VM bytecode
  emitter ("unimplemented: VM bytecode gen for inst"), so the switch
  fall-through / no-case behaviours are pinned via HLSL emit, not
  INTERPRET. Already captured as the pending finding
  `_meta/findings/slangi-switch-in-function-vm-bytecode-crash.yaml`; not
  re-filed here.
- **`visitThrowStmt` / `visitCatchStmt`** error-handler lowering is
  reachable but the slangi VM mis-evaluates `try`/`catch` (a documented
  quirk), and exception emit on several text targets is itself unstable;
  not targeted in this bundle to avoid pinning a known-bad path.
- **`visitGpuForeachStmt` / `visitCompileTimeForStmt`** are driven by
  intrinsics/`$` constructs that are not part of an ordinary slangc CLI
  shader; not targeted here.

## Doc gaps observed

NA
