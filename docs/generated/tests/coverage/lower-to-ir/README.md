---
generated: true
model: claude-opus-5[1m]
generated_at: 2026-08-05T00:00:00+00:00
source_commit: 5634a0ea1b
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
claims, and only `require-capability-stmt.slang` carries
`characterization-unverified=true`.

**Deepening pass (2026-08-05).** A second sweep re-profiled the file against
the hand-written `tests/` suite plus the whole generated suite (1802 lines
still uncovered, 82.04% covered) and targeted the reachable remainder outside
the statement/aggregate area the first pass covered. Each candidate gap was
triaged by re-running the instrumented `slangc` under `LLVM_PROFILE_FILE` and
checking whether a candidate input actually executed the lines, so every test
below is known to move coverage rather than merely to look plausible; together
the ten new tests cover 86 previously-uncovered lines. The gaps this pass
targeted are, in order of size: `visitRequireCapabilityStmt` (the whole
function), the `traceBranchCoverage` arms of `visitWhileStmt`,
`lowerPackOffsetModifier`'s component-mask switch, the operand cases of
`visitSPIRVAsmExpr`, the argument loop of `visitIntrinsicAsmStmt`, the
derivative-group arms of `visitEmptyDecl`, the payload arm of
`visitMakeOptionalExpr`, and two arms of `visitBuiltinOperationIntVal`. Gaps
that turned out to be defensive, dead, or not drivable from the CLI are listed
under `## Unreachable gaps` rather than tested.

Two aborts were found while probing and are recorded as findings rather than
tests: `_meta/findings/conjunction-type-as-parameter-type-internal-error.yaml`
and `_meta/findings/intrinsic-asm-out-of-range-value-slot-assert.yaml`.

## Functional coverage

| Test                                                                                                   | What it pins (current behaviour)                                                                                                                   | covers=                            |
| ------------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------- |
| [`as-operator-same-type-optional.slang`](as-operator-same-type-optional.slang)                         | `v as T` where `T` is already `v`'s static type builds an always-present `Optional<T>` and folds away to the value itself (no type test survives). | source/slang/slang-lower-to-ir.cpp |
| [`compound-assignment-chain.slang`](compound-assignment-chain.slang)                                   | A chain `x += 5; x *= 2; x -= 3; x /= 2;` from 10 read-modify-writes the same local in source order to 13 (integer division).                      | source/slang/slang-lower-to-ir.cpp |
| [`defer-lifo-and-scope-end.slang`](defer-lifo-and-scope-end.slang)                                     | Two `defer` blocks in one block run at scope end in LIFO order: "body" then "second" then "first".                                                 | source/slang/slang-lower-to-ir.cpp |
| [`defer-runs-on-early-return.slang`](defer-runs-on-early-return.slang)                                 | A function-scope `defer` fires before an early `return` and before the fall-through return — on both exit edges.                                   | source/slang/slang-lower-to-ir.cpp |
| [`defer-scope-end-hlsl-emit.slang`](defer-scope-end-hlsl-emit.slang)                                   | A block `defer { outputBuffer[1]=x; }` lowers so the deferred store emits at block close (after x is mutated) and before code following the block. | source/slang/slang-lower-to-ir.cpp |
| [`derivative-group-linear-layout-decl.slang`](derivative-group-linear-layout-decl.slang)               | A file-scope `layout(derivative_group_linearNV) in;` decorates every entry point in the module and re-emits as the same GLSL layout qualifier.     | source/slang/slang-lower-to-ir.cpp |
| [`generic-value-param-bitwise-array-size.slang`](generic-value-param-bitwise-array-size.slang)         | `N ^ 1` and `~N` over a generic value parameter lower to constexpr bit-xor / bit-not IR ops and fold to 7 and 1 for N = 6.                         | source/slang/slang-lower-to-ir.cpp |
| [`intrinsic-asm-stmt-type-arg.slang`](intrinsic-asm-stmt-type-arg.slang)                               | `__intrinsic_asm "...", T;` lowers a type argument as an IR type and substitutes the target's spelling of that type into the `$[0]` slot.          | source/slang/slang-lower-to-ir.cpp |
| [`intrinsic-asm-stmt-value-arg.slang`](intrinsic-asm-stmt-value-arg.slang)                             | `__intrinsic_asm "...", expr;` lowers its value argument as an r-value and substitutes it for the `$0` slot in the emitted target text.            | source/slang/slang-lower-to-ir.cpp |
| [`packoffset-component-mask.slang`](packoffset-component-mask.slang)                                   | Every `packoffset` component mask (x/y/z/w) lowers to a PackOffsetDecoration and re-emits, with the `.x` mask elided.                              | source/slang/slang-lower-to-ir.cpp |
| [`require-capability-stmt.slang`](require-capability-stmt.slang)                                       | A `__requireCapability(...)` statement lowers into the entry point's capability set (upgrading the profile) without adding an OpCapability.        | source/slang/slang-lower-to-ir.cpp |
| [`scalar-ternary-lowers-to-ifelse.slang`](scalar-ternary-lowers-to-ifelse.slang)                       | A scalar `(a<b)?a:b` inside a function lowers to a temporary plus an `if/else` (short-circuit form), not a single select.                          | source/slang/slang-lower-to-ir.cpp |
| [`spirv-asm-builtin-var-operand.slang`](spirv-asm-builtin-var-operand.slang)                           | A `builtin(Name:Type)` operand inside `spirv_asm` lowers to a synthesized Input variable decorated with that SPIR-V BuiltIn.                       | source/slang/slang-lower-to-ir.cpp |
| [`spirv-asm-immediate-value-operand.slang`](spirv-asm-immediate-value-operand.slang)                   | A `!expr` immediate operand inside `spirv_asm` lowers the Slang constant as a literal enum operand rather than as an id.                           | source/slang/slang-lower-to-ir.cpp |
| [`struct-and-array-aggregate-init.slang`](struct-and-array-aggregate-init.slang)                       | `P p = {3,4}` maps positionally to x=3,y=4 and `int arr[3]={10,20,30}` maps by index.                                                              | source/slang/slang-lower-to-ir.cpp |
| [`switch-fallthrough-and-shared-case.slang`](switch-fallthrough-and-shared-case.slang)                 | Adjacent empty cases share one label, and a non-`break` case (`case 2`) falls through into the `default` body.                                     | source/slang/slang-lower-to-ir.cpp |
| [`switch-no-cases-evaluates-condition.slang`](switch-no-cases-evaluates-condition.slang)               | A `switch` with no case/default still lowers (and keeps the side effects of) its condition but emits no switch instruction.                        | source/slang/slang-lower-to-ir.cpp |
| [`vector-select-lowers-to-select-inst.slang`](vector-select-lowers-to-select-inst.slang)               | A vector `select(c,a,b)` (non-basic condition) lowers to a single `OpSelect` in SPIR-V, not an if/else.                                            | source/slang/slang-lower-to-ir.cpp |
| [`while-loop-branch-coverage-instrumentation.slang`](while-loop-branch-coverage-instrumentation.slang) | `-trace-branch-coverage` makes a `while` condition lower with a marker on each arm, backed by a synthesized `_slang_coverage` buffer.              | source/slang/slang-lower-to-ir.cpp |

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

Added by the 2026-08-05 deepening pass. Each entry names a gap that was
probed with the instrumented `slangc` and deliberately left untested, with
the reason.

- **`SLANG_UNIMPLEMENTED_X` type-expression visitors** — `visitAndTypeExpr`,
  `visitTupleTypeExpr`, `visitPointerTypeExpr`, `visitFuncTypeExpr`,
  `visitThisTypeExpr`, `visitModifiedTypeExpr`, `visitPackBranchTypeExpr`,
  `visitSharedTypeExpr`, `visitGenericAppExpr`, `visitLambdaExpr`,
  `visitOverloadedExpr` / `visitOverloadedExpr2`,
  `visitPartiallyAppliedGenericExpr`, `visitAggTypeCtorExpr`,
  `visitFloatBitCastExpr`. Every one of these bodies is a single
  "should not survive checking" abort; reaching one means the checker left a
  syntactic form in the tree it was supposed to resolve, which is a compiler
  bug rather than a testable behaviour. Defensive — do not target.
- **`emitCastToConcreteSuperTypeRec` fall-through asserts** (three
  `SLANG_ASSERT(!"unhandled")` arms). The transitive-witness recursion itself
  _is_ reachable (a three-level `struct A / B : A / C : B` chain passed to a
  function taking `A` executes it), but the arms that remain uncovered are the
  "witness operand was not a `SubtypeWitness`" fallbacks. Defensive.
- **`visitBreakStmt` / `visitContinueStmt` / `visitCaseStmtBase` /
  `visitTargetCaseStmt` asserts**, the `default:` arms of the `LoweredValInfo`
  flavor switches in `assign` / `getSimpleVal` / `subscriptValue`, and the
  `kMaxIRLayoutLoweringRecursionDepth` guard in layout lowering. Exhaustive
  switch defaults and impossible-state guards. Defensive.
- **Vector / matrix / struct arms of `visitInitializerListExpr`** — probed
  with `float2x2 m = {{1,2},{3,4}}`, `float3 v = {1,2,3}`, `P p = {3,4}`,
  empty `{}` forms, and each of those nested inside an array initializer. In
  all of them the checker rewrites the initializer list into a constructor
  call, so only the _array_ arm reaches lowering. The struct-with-base arm
  (`findBaseStructType`) was likewise unreachable. These are legacy arms kept
  for an older initializer-list path; a producer-side change would be needed
  to drive them, so no test was written. Reachable in principle, not from
  today's surface.
- **Value-pack flattening in `visitTupleExpr`** — a `TupleExpr` carrying an
  `IRMakeValuePack` element. `makeTuple(1, expand each args)` lowers as an
  ordinary call, not a `TupleExpr`, so the flatten arm was not reachable from
  the tuple/pack spellings tried.
- **`visitShapePackTransformExpr`, `visitExpandIntValPack`,
  `visitShapeConcat/Permute/Swap/ReduceIntValPack`, `visitLastIntVal`,
  `visitTrimLastIntValPack`, `visitFirstSubtypeWitness` /
  `visitLastSubtypeWitness`** — the tile-shape and pack-query `IntVal` family.
  Every spelling tried (`__last(D)` / `__trimLast(D)` under
  `where nonempty(D)`, `__shapePermute` on a `MockTile`-style generic) either
  folds at check time or is diagnosed before lowering; the one hand-written
  test of the shape ops is a `DIAGNOSTIC_TEST` that errors out first. Would
  need a genuinely unspecialized shape transform surviving to lowering.
- **`visitTypeCoercionConstraintDecl`, `visitNonEmptyPackConstraintDecl`,
  `visitGenericVariadicPackCountConstraintDecl`,
  `visitGlobalGenericValueParamDecl`** — the module-scope (`GlobalGenericParamDecl`
  parent) arms of the generic-constraint visitors. The constrained
  module-scope generic-parameter surface already has an open finding
  (`_meta/findings/generics-existentials-global-generic-param-constraint-sigsegv.yaml`);
  not re-probed here to avoid stacking tests on a crashing path.
- **`_lowerSubstitutionEnv`, `SpecializedComponentTypeIRGenContext::visitTypeConformance`,
  `visitRenamedEntryPoint`, `visitExistentialSpecializedType`** — driven by
  the `IComponentType` specialization / type-conformance API, not by any
  `slangc` command line. Needs a C++ unit test, not a `.slang` file.
- **`visitGLSLInterfaceBlockDecl` non-`buffer` arm and the
  `SynthesizedStructDecl` operand arms of `visitAggTypeDecl`** — probed with
  `-allow-glsl` `buffer` / `uniform` blocks; the decl is resolved to an alias
  earlier in `visitAggTypeDecl` and never reaches the else-if chain. The
  synthesized-struct arms are fed by internal (autodiff intermediate-context)
  decls with no user spelling.
- **`visitNamedExpressionType`** — `typealias` / `typedef` sugar is
  canonicalized before lowering; probed with both spellings in variable,
  parameter, and generic-argument positions and never reached.
- **`visitNullPtrLiteralExpr`** — `nullptr` compiles (it even reaches emit as
  a literal `nullptr` on the C++ target) but does not route through this
  visitor; the literal is resolved earlier. Not dead, but not reachable from
  the `nullptr` spelling.
- **The `throws` arm of `visitFuncType`** — needs a _function type_ carrying
  a non-bottom error type to be lowered. A `throws` function called through
  `try` is lowered as a call, not as a function-typed value; no spelling was
  found that lowers the function type itself. Left untargeted.
- **`lowerAutoPyBindCudaFunctionDecorations`, `visitDispatchKernelExpr`,
  the `TreatAsDifferentiable` / `ForwardDifferentiate` / `BackwardDifferentiate`
  visitors** — out of bundle: `coverage/torch` and `coverage/autodiff` own
  these surfaces.

## Doc gaps observed

NA
