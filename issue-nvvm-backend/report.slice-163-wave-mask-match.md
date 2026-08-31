# Slice 163: Canonical wave-mask match

## Motivation

Three healthy frozen-v1 workloads stopped at the same exact final-IR operation. Consider the
functional divergent switch:

```slang
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int val = 0;
    switch (int(tid.x))
    {
    case 0: val = 100; break;
    case 1: val = 200; break;
    case 2: val = 300; break;
    default: val = 400; break;
    }

    int total = WaveActiveSum(val);
    outputBuffer[tid.x] = total;
}
```

`ActiveMaskSynthesisContext::transformRegion` sees the structured switch and inserts
`IRWaveMaskMatch` immediately before it. Each lane needs the mask of active peers whose switch
selector equals its own selector, so the case regions can carry the correct mask and reconverge.
All three selected workloads reach this canonical final shape at O0 and O3:

```text
UInt matchingMask = waveMaskMatch(UInt activeMask, Int selector)
```

Direct NVVM rejected it before provider mutation with `direct NVVM lowering does not support Slang
IR instruction or shape 'waveMaskMatch'`. The affected rows were the active-mask switch with and
without a source default plus the functional divergent-switch workload.

## Proposed solution

Represent canonical scalar wave-mask match as one typed semantic value operation. Append
`SLANG_NVVM_VALUE_OP_WAVE_MASK_MATCH` to the existing generic value-operation vocabulary and
advance the forward-only provider ABI to revision 31. Do not add a callback or another wave-specific
interface.

Admit exactly `UInt = waveMaskMatch(UInt mask, T value)` for signed i32, unsigned i32, and float32.
Map the operation to LLVM 14's `llvm.nvvm.match.any.sync.i32`; bitcast float32 to i32 before the
intrinsic. Use the shared semantic catalog for both capability preflight and emission, and keep
vectors, aggregates, wider scalars, and adjacent wave operations outside this slice.

## Change summary

- The provider ABI and semantic catalog append one typed wave-mask-match operation with i32, u32,
  and f32 rows.
- The LLVM 14 provider emits the native NVVM match-any intrinsic through the existing generic
  value-operation callback.
- The direct emitter resolves `IRWaveMaskMatch` through the shared catalog, preflights the exact
  row, validates mask/value availability, and emits the typed operation.
- Builder tests serialize all three b32 rows; a fake-emitter test proves active-mask synthesis
  reaches the typed operation and returns an unsigned-i32 mask.
- Three existing compute fixtures gain permanent direct-NVVM O0 and O3 differential lanes.
- Frozen/discovery census artifacts remain separate, the representative measurement manifest grows
  from 21 to 24 gates, and the design, ledger, plan, and report retain the evidence.

## Concepts and vocabulary

**Wave-mask match** means the per-lane partition mask returned by CUDA/NVVM match-any. For each
active lane, the result names active lanes whose b32 value has the same bits as that lane's value.

**Active-mask synthesis** is the producer-side pass that threads explicit wave masks through
structured control flow. For a switch it uses the selector equivalence classes to derive the mask
on entry to each case region.

**Match-all predicate** is the existing operation that asks whether all active lanes have the same
value. Its Boolean result is not enough to recover each lane's equivalence-class mask.

**Semantic operation ID** is a typed request carried through the generic provider callback. Adding
an ID changes the forward ABI revision but does not add an interface-table field or function
pointer.

## Process report

The failure census initially grouped the three rows as a generic atomic/wave cluster. The exact
shape audit showed they all stop at the same named operation and producer. In
`ActiveMaskSynthesisContext::transformRegion`, the `kIROp_Switch` branch has the region-entry mask
and the integer switch condition. `IRBuilder::emitWaveMaskMatch` constructs the two-operand
instruction before the switch. Final linked IR preserves an unsigned-i32 mask result, unsigned-i32
mask operand, and signed-i32 selector operand in all three targets. This is deliberate canonical IR,
not an alternative spelling or malformed upstream representation.

The existing provider vocabulary was then audited for an economical compiler-owned recipe.
`SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL` calls `llvm.nvvm.match.all.sync.i32p` and extracts its
Boolean predicate. That predicate says only whether the entire active mask agrees; it cannot tell
different lanes which equivalence class they belong to. A software reconstruction would require a
multi-step lane scan with repeated shuffle, compare, ballot, and select operations and would be
larger, slower, and more difficult to prove under divergence. LLVM 14 directly exposes
`llvm.nvvm.match.any.sync.i32`, the exact canonical operation required here. This is the concrete
expressibility gap that justifies revising the ABI.

Revision 31 appends one semantic ID and increments the operation count. It does not add or reorder
an API table member. `NVVMSemantics::kCatalog` is the single type-contract source used by
`NVVMIRBuilder::isOperationSupported`, preflight, fake observation, and real emission. The three
catalog rows mirror the established match-all b32 family: signedness is irrelevant to bit equality,
and float32 is explicitly bitcast to i32 in the provider. No source generic-assembly spelling is
associated with these rows because the producer is the canonical IR instruction itself.

`_getNVVMValueOperation` maps only `kIROp_WaveMaskMatch` to the new semantic. The requirements pass
then resolves the exact catalog row before provider creation. Function validation proves the
unsigned-i32 wave mask through `_validateWaveMaskValue` and validates the selected scalar operand
through the existing value path. Emission shares the ordinary generic value-operation block, so
the same resolved descriptor and two already-lowered operands reach the provider. There is no
fixture-name test, syntax reconstruction, assembly parsing, compatibility fallback, or downstream
repair.

The self-review inventory has three notable changes. The new operation ID survives because no
revision-30 result represents a per-lane match mask and LLVM supplies the exact primitive. The
three catalog overloads survive because they share one b32 provider contract and focused builder
tests prove the signed, unsigned, and float bit-transport cases. The new compiler switch cases
survive because each is routed through the shared resolver rather than reproducing a type
classification. No new helper walks arbitrary IR or recreates producer context. Removing the
mapping restores the original first unsupported shape in all three promoted workloads.

Validation used rebuilt Release host and isolated LLVM 14 provider binaries, with every build and
test run outside the sandbox. The three real workloads pass O0 and O3 differential execution; all
four focused units pass; and the selected direct-NVVM prefix passes 432/432. Generated direct O0
and O3 PTX contains `match.any.sync.b32` and assembles for all measured architectures.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references and improves from
391/395/391 to 394/398/394 O0/O3/both-mode correct. Across all frozen rows, native CUDA is 449
correct and three infrastructure; direct O0 is 407 correct, 32 preflight, eight runtime mismatch,
and five provider; direct O3 is 412 correct, 32 preflight, and eight runtime mismatch. The only
gains are the three selected rows and there are no old-correct losses. Discovery remains exactly
82 workloads/72 healthy references at 64/64/64, with no gains or losses; each direct mode remains
64 correct, eight preflight, two provider, seven infrastructure, and one runtime mismatch.

All 24 representative direct-O3 gates assemble with CUDA 12.9 for SM70, SM80, and SM90. The active
mask switch gate measures 247.9 ms and 1924-byte PTX at direct O3 SM70 versus 353.9 ms and 9999
bytes through NVRTC O3; direct O0 measures 247.7 ms and emits 5420-byte PTX. The no-default variant
measures 236.3 ms and 1707-byte PTX versus 351.3 ms and 9787 bytes. The functional reconvergence
gate measures 243.1 ms and 1937-byte PTX versus 363.2 ms and 10831 bytes. These timings remain
exploratory rather than a controlled benchmark.
The repository formatting driver was also attempted; this machine lacks gersemi, clang-format,
prettier, and shfmt, so no automated formatter ran. Manual diff review and `git diff --check` are
clean.
