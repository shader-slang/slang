# Slice 167: Counter-backed structured-buffer views

## Motivation

The frozen census contained two healthy MVP workloads for ordinary HLSL Append and Consume
resources. Consider the source operation:

```slang
AppendStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    output.Append(1.0);
    GroupMemoryBarrier();
}
```

Append/Consume legalization does not leave one opaque resource. It creates a struct containing an
element `RWStructuredBuffer<T>` and a counter `RWStructuredBuffer<Atomic<int>>`, then generates
helpers that query dimensions, update counter element zero, and access the selected data element.
Direct NVVM rejected the first field type before reaching any of those already-supported generic
operations.

## Proposed solution

Admit the exact two-operand structured-buffer form intentionally created by
`lowerStructuredBufferType`, alongside the established exact three-operand form. Continue checking
the opcode, element family, explicit data layout, and bounded operand count before provider
mutation.

Lower `IRStructuredBufferGetDimensions` from its existing sources of truth: extract the runtime
count from the selected `{data, count}` view, derive the constant stride from the selected CUDA
storage layout, and construct the canonical unsigned `uint2`. Map exact canonical GenericAsm
`__threadfence_block` to a newly appended typed operation ID and LLVM's `nvvm.membar.cta` intrinsic.
Do not add a callback, resource-name check, compatibility fallback, or serialized-IR rewrite.

## Change summary

- The structured-buffer classifier accepts only canonical two- and three-operand forms.
- Direct preflight and emission support exact structured-buffer dimension queries through generic
  aggregate extraction, integer conversion, constant, and vector-construction operations.
- Provider ABI revision 32 appends the CTA memory-fence semantic to the existing typed catalog.
- The Append and Consume fixtures gain direct O0/O3 differential lanes.
- Separate frozen/discovery census artifacts, a 32-gate measurement manifest, the design record,
  capability ledger, completed plan, and this report retain the evidence.

## Concepts and vocabulary

**Counter-backed resource aggregate** is the canonical struct created by Append/Consume lowering.
Its element view and counter view are ordinary typed structured-buffer values; the aggregate itself
is transported structurally.

**Raw structured-buffer view** is the selected provider value `{data, count}`. `data` is the global
element pointer and `count` is an unsigned 64-bit runtime element count.

**CTA memory fence** orders memory accesses at thread-block scope but does not synchronize threads.
It is represented by `llvm.nvvm.membar.cta`, unlike `llvm.nvvm.barrier0` or device-scope
`llvm.nvvm.membar.gl`.

## Process report

The first input-shape audit traced the failure to `lowerStructuredBufferType` in
`slang-ir-lower-append-consume-structured-buffer.cpp`. For each legalized field it calls the IR
builder with exactly the element and explicit data-layout operands. The base structured-buffer type
contract permits that form; a separate ordinary legalization path may append a third
layout-conformance operand. The two spellings are therefore intentional canonical producer forms,
not malformed alternatives. `_getNVVMSupportedRawBufferType` now accepts exactly two or three
operands and applies the same supported element and explicit-layout checks to either. Counts below
two or above three, missing/unsupported layouts, and unsupported elements remain rejected by the
existing adjacent-negative test before any provider call.

With that contract fixed, both real workloads advanced to `IRStructuredBufferGetDimensions`. Its
producer already supplies the selected resource and an unsigned `uint2` result. The selected raw
view carries the only runtime fact—the element count—as field one. The established
`_getNVVMStructuredBufferStorageLayout` computes the CUDA size/alignment of the exact structured
element, which is the authoritative stride. `_getNVVMStructuredBufferDimensions` validates that
complete relation once. Preflight requires the existing unsigned i64-to-u32 conversion, value
availability validates the resource operand, and emission extracts count, converts it, constructs
the stride constant, and forms the result vector. This does not rediscover syntax or invent a
layout operand.

The next exact producer was `GroupMemoryBarrier()` from `hlsl.meta.slang`, whose selected target
body is canonical GenericAsm `__threadfence_block`. Reusing `WORKGROUP_BARRIER` would incorrectly
add thread synchronization through `barrier0`; reusing `DEVICE_MEMORY_BARRIER` would widen scope to
`membar.gl`. The operation is valid and the semantic catalog is the established owner for complete
GenericAsm assembly/signature matches. ABI revision 32 therefore appends one operation ID, and the
provider maps it to `llvm.nvvm.membar.cta`. The generic operation callback already expresses the
request, so the interface table does not grow.

An early synthetic compiler test tried to pass complete Append/Consume aggregates as fake-provider
kernel parameters. It failed in the fake's deliberately small nested resource-struct parameter
model before reaching either new operation. Expanding that fake representation would only serve
the fixture and would not prove LLVM or CUDA behavior, so the synthetic fixture was removed. The
real promoted tests are the correct ownership proof for the combined resource ABI, dimension
query, counter atomics, helpers, control flow, and fence. The existing real-provider builder test
proves exact `membar.cta` serialization and post-terminator rejection, while the established
adjacent-negative compiler test protects malformed structured-buffer shapes.

All four promoted runtime lanes pass. Frozen corpus v1 retains exactly 452 workloads and 427
healthy references, advancing from 400/400/400 to 402/402/402 O0/O3/both with exactly Append and
Consume as gains and zero old-correct regressions. Discovery retains exactly 82 workloads/72
healthy references at 66/66/66, with no gain or regression. The separate classifications and
Pareto artifacts remain checked in. The selected NVVM prefix passes 433/433, and the promoted
fixtures pass 4/4.

The 32-gate exploratory measurement run produced 160 PTX rows and 160 assembled cubins. Append
measured 238.7 ms and 4,152-byte PTX at direct O0 SM70, and 248.7 ms and 1,212 bytes at direct O3
SM70, versus 360.5 ms and 9,225 bytes through NVRTC O3. Consume measured 232.4 ms and 3,944-byte
PTX at direct O0 SM70, and 242.0 ms and 1,376 bytes at direct O3 SM70, versus 351.4 ms and 9,425
bytes through NVRTC O3. Direct O3 PTX assembled with CUDA 12.9 for SM70, SM80, and SM90. These
three-repetition compile measurements remain exploratory rather than benchmark claims.

The final new-helper/special-case inventory retains three bounded entries. The two-or-three operand
branch survives because both exact forms have named canonical producers and share all semantic
validation. `_getNVVMStructuredBufferDimensions` survives because it centralizes one exact IR
relation using existing sources of truth. The appended CTA-fence catalog row survives because its
semantics cannot be represented by either prior barrier operation. No fixture-name check, fallback,
syntax reconstruction, arbitrary operand-graph walk, provider callback, text manipulation, or
downstream patch for malformed IR remains.
