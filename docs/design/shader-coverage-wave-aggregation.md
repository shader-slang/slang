# Shader coverage: wave-aggregated counter increments (design notes)

Tracking issue: [#11509](https://github.com/shader-slang/slang/issues/11509).
Status: **implemented for SPIR-V (both uint32 and the default uint64),
opt-in and OFF by default; follow-ups remain.** Enable with
`-trace-coverage-wave-aggregation` (`CompilerOptionName::TraceCoverageWaveAggregation`).
It is opt-in because the perf payoff is workload/GPU-dependent and did not
materialize on an NVIDIA RTX A3000 laptop (the per-line subgroup reduce/elect
overhead roughly cancels the reduced atomic contention — see "Measured result"
at the end); the per-lane atomic remains the default. The emit-side recipe
below is wired up: coverage emits
two IR ops (`coverageActiveLaneCount`, `coverageElectFirstLane`) inside an
`if`-guard, and the SPIR-V backend lowers them to `OpGroupNonUniformIAdd
(Reduce)` / `OpGroupNonUniformElect`. The gate is the **direct SPIR-V backend**
(`isSPIRV`), not all Khronos targets — GLSL keeps the per-lane atomic. For
uint64 counters the uint active-lane count is widened with `OpUConvert` before
the 64-bit atomic. Verified: both widths emit the aggregated sequence and pass
`spirv-val`; 63 coverage + 11 unit tests pass (regression test
`coverage-wave-aggregated-spirv.slang` covers both widths). **Remaining:**
HLSL/CUDA/Metal emitter lowering of the two ops; re-measure the demo speedup.
The rest of this document is the original investigation/design.

## Problem

The coverage instrumentation pass (`source/slang/slang-ir-coverage-instrument.cpp`)
emits one atomic increment per covered line/branch **per thread**:

```text
slotPtr = RWStructuredBufferGetElementPtr(__slang_coverage, slot)
AtomicAdd(slotPtr, 1, relaxed)
```

When many threads execute the same marker — e.g. a counter inside a hot
loop — every thread issues an atomic-add to the same buffer slot, and the
adds serialize. On the `shader-coverage-image-pipeline` demo (2.07M threads,
per-line counters inside the bilateral filter's inner loop) the hottest
counter slots receive 18-37 million atomics each, and the instrumented
shader runs ~20-30x slower than the uninstrumented one on an NVIDIA RTX
A3000. The same demo is only ~3x slower on Apple Silicon, because Apple GPUs
coalesce atomics within a SIMD-group before they reach memory. The goal is to
do that coalescing explicitly so all targets benefit.

## Desired transform

Replace the per-lane increment with a wave/subgroup-aggregated one — one
atomic per wave instead of one per active lane:

```text
laneCount = <number of lanes active at this marker>
if (<this lane is the elected lane>)
    AtomicAdd(slotPtr, laneCount, relaxed)
```

`laneCount` is the count of lanes active *at the marker*, so divergent
control flow (a marker only some lanes reach) stays correct: the sum of
active lanes equals the number of executions, exactly matching the per-lane
baseline. Only the increment *amount* changes (`1` -> `laneCount`); the
per-line/branch counter slots, the manifest, the LCOV output, and host
readback are all unaffected.

This reduces atomic traffic by up to the wave width (≈32 on NVIDIA/AMD,
≈32/64 on others), which should close most of the gap with Apple.

## The stage problem (why this is non-trivial)

`instrumentCoverage` runs inside `linkAndOptimizeIR`, **after** `linkIR` and
after stdlib specialization. At that point the high-level wave intrinsics are
not available as primitives:

- `WaveActiveSum`, `WaveIsFirstLane`, `WaveMaskSum`, etc. are **stdlib
  functions** in `hlsl.meta.slang` implemented with per-target
  `__target_switch` / `__intrinsic_asm` bodies — not single IR opcodes.
- The only wave-related IR opcodes that exist are `kIROp_WaveGetActiveMask`,
  `kIROp_WaveMaskBallot`, and `kIROp_WaveMaskMatch`. There is no IR opcode for
  a wave reduction, lane index, lane count, or bit-count, so the aggregate
  cannot simply be emitted as a few intrinsic insts the way the plain
  `AtomicAdd` is.

So the pass cannot just inline `WaveActiveSum(1)` / `WaveIsFirstLane()`.

## Candidate approaches

1. **Emit a call to a Slang-defined coverage-increment helper (recommended).**
   Define a small generic helper, e.g.
   `void __coverageCounterIncrement<T>(RWStructuredBuffer<T> buf, uint slot)`,
   in a coverage runtime module written in Slang. Its body uses the
   high-level wave intrinsics with a capability-gated fallback:

   ```slang
   // pseudo-code
   void __coverageCounterIncrement<T>(RWStructuredBuffer<T> buf, uint slot)
   {
       // Wave-capable targets: one atomic per wave.
       let count = T(WaveActiveSum(1u));
       if (WaveIsFirstLane())
           InterlockedAdd(buf[slot], count);
       // Targets without wave ops fall back (via capability/__target_switch)
       // to a plain per-lane InterlockedAdd(buf[slot], T(1)).
   }
   ```

   The coverage pass emits a *call* to this helper instead of an inline
   `AtomicAdd`. The normal capability system and target lowering expand the
   wave intrinsics per target and select the fallback where wave ops are
   unsupported (CPU, pre-SM6.0 HLSL).

   **Resolved — how the helper survives to the post-link pass.** Slang already
   force-copies specific `[KnownBuiltin(name)]` functions through `linkIR`
   even when unused (see `KnownBuiltinDeclName::NullDifferential` in
   `slang-ir-link.cpp`). The coverage helper(s) get the same treatment:
   annotate with `[KnownBuiltin(...)]`, add the name to `KnownBuiltinDeclName`,
   and add it to the force-copy switch in `slang-ir-link.cpp`. The pass then
   finds the function by its known-builtin decoration and emits a call.

   **Important constraint — the helper must be non-generic.** Generic
   specialization runs *before* `instrumentCoverage`, so a generic
   `helper<T>` emitted at that point would never be specialized. Define two
   concrete helpers instead — one taking `RWStructuredBuffer<uint>` and one
   `RWStructuredBuffer<uint64_t>` — and have the pass pick the variant that
   matches `counterElementType`. The subsequent inlining + target-lowering
   passes in `linkAndOptimizeIR` (which run after `instrumentCoverage`) expand
   the call and the wave intrinsics normally.

   **CPU fallback.** Coverage runs on the CPU (C++ source) target, which has
   no waves, so the helper body must `__target_switch` to a plain
   `InterlockedAdd(buf[slot], 1)` on cpu/default; only the GPU SIMT cases use
   the wave-aggregated form. (WGSL and CPU-via-LLVM skip coverage entirely.)

2. **Emit the aggregate from low-level IR ops.** Use `WaveGetActiveMask` +
   `WaveMaskBallot` + a popcount + an elected-lane test built from the
   ballot's lowest set bit vs the lane index. Rejected for now: lane-index /
   bit-count are not IR opcodes either, and the `WaveMask` is `uint4` for >32
   lanes, so this re-implements a chunk of stdlib lowering by hand and is
   error-prone across wave widths.

3. **Instrument before stdlib lowering.** Move coverage instrumentation
   earlier so high-level wave intrinsics are callable. Rejected: coverage is
   deliberately late (post-specialization) so it counts the final, specialized
   code; moving it changes coverage semantics.

**Recommendation (superseded — see spike result below): approach 1.** It reuses
the capability system and per-target lowering, keeps the coverage pass
target-agnostic, and gives the fallback for free.

## SPIR-V de-risk spike result (approach 1 does NOT work)

A spike implemented approach 1 for SPIR-V end to end: a `[KnownBuiltin]`
`export`-ed `__coverageCounterIncrementWaveAggregated(RWStructuredBuffer<uint>,
uint)` helper in `hlsl.meta.slang` with the validated wave body; a force-copy
case in `slang-ir-link.cpp`; and a gated call-emission in the pass
(`shouldUseWaveAggregation()` → emit `Call(helper, buffer, slot)` for
Khronos/uint, else the per-lane atomic).

**Outcome: the helper never reaches the post-link pass.** The compiled SPIR-V
still showed only per-lane `OpAtomicIAdd %uint … %uint_1`; `shouldUseWaveAggregation()`
returned false because the helper was absent from the module. Investigation
(`-dump-ir-after linkIR`) showed the helper — and even `NullDifferential`, the
force-copy precedent — are **absent from the linked IR for a plain compute
compile**. Root cause: `linkIR` is demand-driven; unreferenced symbols are not
retained even with `[KnownBuiltin]` + `export` + a `shouldCopy` force-copy case.
`NullDifferential` only persists in autodiff flows because the autodiff pass
*emits a reference to it* before dead-code elimination. The coverage helper has
no reference until the pass emits the call — which is too late — and there is no
on-demand "import a core-module function by name into the current module" API
for a post-link pass to use.

**Conclusion: pivot to approach 2 (emit-side lowering).** The coverage pass
keeps emitting a primitive it can always produce — either the existing
`kIROp_AtomicAdd` tagged with a decoration, or a dedicated
`kIROp_CoverageCounterIncrement` — and the per-target emitters lower it to the
wave-aggregated native sequence (the SPIR-V form is already validated:
`OpGroupNonUniformIAdd Reduce` + `OpGroupNonUniformElect` + `OpAtomicIAdd`). The
conditional is awkward to synthesize in the emitter (it works per-inst, not in
blocks), so the most practical shape is: the pass emits the *blocks* (an `if`
guarding the atomic) and a small marked op for "active-lane count", which the
emitter lowers to the subgroup reduce/elect. Start with SPIR-V (highest value;
NVIDIA is the worst case), then HLSL/CUDA.

**Revised estimate:** ~1 week for the SPIR-V path (emitter work + the IR-op/
decoration + tests), i.e. the "worst case" bucket from the original estimate —
the clean helper-call route is closed. An alternative worth weighing is moving
coverage-marker *lowering* earlier (before stdlib specialization) so high-level
wave intrinsics are usable; rejected so far because it changes which
(specialized) code coverage counts.

## SPIR-V emit-side implementation recipe (concrete)

Investigation has reduced the emit-side path to a mechanical checklist; every
API below was located in the tree. Two hard sub-problems, both solved:

**A. Emitting the subgroup ops (no GroupNonUniform op currently goes through
the SPIR-V op-switch — they only come via `spirv_asm`).** Use the generic
`emitInst(parent, irInst, SpvOp, operands...)` in `slang-emit-spirv.cpp`
(line ~1308). Scope/group-operation operands follow the atomic-emission
pattern at `slang-emit-spirv.cpp:5468`:
  - active-lane count → `OpGroupNonUniformIAdd`:
    `emitInst(parent, inst, SpvOpGroupNonUniformIAdd, inst->getFullType(),
    emitIntConstant(SpvScopeSubgroup, uint), SpvLiteralInteger::from32(SpvGroupOperationReduce),
    emitIntConstant(1, uint))` — note `Scope` is an `<id>` constant but
    `GroupOperation` is a **literal** (`SpvLiteralInteger::from32`, line ~334).
    Then `requireSPIRVCapability(SpvCapabilityGroupNonUniformArithmetic)`.
  - elect → `OpGroupNonUniformElect`:
    `emitInst(parent, inst, SpvOpGroupNonUniformElect, boolType,
    emitIntConstant(SpvScopeSubgroup, uint))` +
    `requireSPIRVCapability(SpvCapabilityGroupNonUniform)`.

**B. The `if (elect)` conditional (mid-block insertion).** The coverage marker
sits mid-block, so emitting an `IfElse` terminator there requires **splitting
the block** — move the instructions after the marker into a new "after" block,
terminate the original block with `emitIfElse(elect, trueBlock, afterBlock,
afterBlock)`, fill `trueBlock` with the atomic + `emitBranch(afterBlock)`.
`emitIfElseWithBlocks` (`slang-ir.cpp:6235`) creates/append the blocks but does
**not** split, so the pass must do the split (or factor the increment into a
helper-shaped block layout). Alternatively make the increment one IR op and emit
the control flow in the backend — rejected: creating SpvInst blocks mid-emit is
more exotic than an IR-level split.

### Steps
1. Two IR ops in `slang-ir-insts.lua` (e.g. `coverageActiveLaneCount` → uint,
   `coverageElectFirstLane` → bool), no operands. **Watch the cascade:** adding
   an op typically also needs a `slang-ir-insts-stable-names.lua` entry and may
   hit exhaustive switches (side-effect/hoistability classification — these must
   be marked non-hoistable / has-side-effect-ish so CSE/code-motion can't lift
   them across control flow or merge two subgroup reads).
2. SPIR-V backend: two `case`s per **A** in the op switch near `kIROp_AtomicAdd`.
3. Coverage pass (`emitCoverageCounterIncrement`, gated by
   `shouldUseWaveAggregation()` = helper-free now: `isKhronosTarget(target) &&
   counterElementType is uint`): split the block per **B**, emit the two ops +
   the guarded atomic.
4. Tests: a SPIR-V FileCheck test asserting `OpGroupNonUniformIAdd`/`Elect`; a
   CPU count-equivalence test (CPU path unaffected = reference).
5. Validate on `tmp/spike-cov.slang` (already used during the spike) that the
   output matches the validated reference sequence and passes `spirv-val`.

Then HLSL/CUDA follow by lowering the same two ops in their emitters
(`WaveActiveSum`/`WaveIsFirstLane`, `_waveSum`/elect).

## Validated emission

The wave-aggregated increment body was prototyped and compiled with the local
`slangc` to confirm it lowers correctly on every relevant target:

```slang
void coverageIncrement(uint slot)
{
    uint activeLaneCount = WaveActiveSum(1u);   // lanes active at this marker
    if (WaveIsFirstLane())
        InterlockedAdd(counters[slot], activeLaneCount);
}
```

- **SPIR-V** (passes `spirv-val`): `OpGroupNonUniformIAdd %uint ... Reduce
  %uint_1` (the wave sum), `OpGroupNonUniformElect` (first-lane), then
  `OpAtomicIAdd` of the aggregated count. Capabilities `GroupNonUniform` /
  `GroupNonUniformArithmetic` are auto-declared.
- **HLSL**: `WaveActiveSum(1U)`, `WaveIsFirstLane()`, `InterlockedAdd(...)`.
- **CUDA**: `_waveSum(...)` over the active mask, `atomicAdd(...)`.

The divergent test case (only some lanes call the helper) confirms the
reduction is over the lanes active at the call site, matching per-execution
counts. So the increment *code* is settled; the remaining work is the
plumbing that makes the coverage pass emit it (above).

## Target gating

Wave aggregation only applies where the target has wave/subgroup ops; the
fallback is the current per-lane atomic.

- **HLSL/DXIL** — wave intrinsics require SM6.0+; older profiles use the
  fallback.
- **SPIR-V** — `GroupNonUniform*` subgroup ops (already used by the stdlib
  intrinsics); the capability is auto-declared.
- **CUDA** — warp intrinsics.
- **Metal** — `simd_*`; note Apple already coalesces, so the win is smaller.
- **CPU (LLVM / C++ source)** — no wave concept; always the per-lane
  `_slang_atomic_add_*` path.

With approach 1 this gating is mostly handled by the helper's `__target_switch`
and `[require(...)]` capability annotations rather than by branching in the
C++ pass.

## Correctness / testing

- Aggregated counts must equal the per-lane baseline. Add tests that compile
  the same shader with and without aggregation and compare counter values
  (CPU path is unaffected, so it is the reference).
- Cover divergent control flow: a marker inside a branch only some lanes take
  must add only the active-lane count.
- Confirm the existing 61 coverage tests and the metadata unit tests still
  pass (the metadata/manifest/LCOV contracts do not change).
- Re-measure the image-pipeline demo before/after to quantify the speedup.

## Where it plugs in

The single emission site is `CoverageInstrumenter::lowerMarkerOp` in
`source/slang/slang-ir-coverage-instrument.cpp`. The atomic emission has been
factored into `emitCoverageCounterIncrement()` so the aggregated path is a
localized change behind a target-capability check.

## Measured result (why it is opt-in / off by default)

On an NVIDIA RTX A3000 laptop GPU, the `shader-coverage-image-pipeline` demo
(1080p bilateral filter, 251 instrumented counters) showed **no speedup** from
aggregation: coverage-on stayed ~325-450 ms/config vs ~12 ms uninstrumented
(~25-35x), essentially unchanged from the per-lane atomic. The aggregation does
a full subgroup reduce + elect + branch on *every* covered line (millions of
executions in the hot loop), and that overhead roughly cancels the reduced
atomic contention. Apple GPUs coalesce atomics in hardware "for free", which is
why Metal saw only ~3x; doing it explicitly in the shader is not free.

The optimization is therefore left **off by default**, pending a measurement on
a desktop/datacenter NVIDIA GPU (or different driver) where the atomic-contention
gap might favor it. Note that coverage is a test/dev feature, so its runtime is
not performance-critical; the important problem — GPU-watchdog crashes on long
sweeps — is solved separately by tiling the dispatch (PR #11451), not by this.
