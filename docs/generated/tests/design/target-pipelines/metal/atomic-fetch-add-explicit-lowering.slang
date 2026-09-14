//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=5fdd0ac6daa767e15b8f1dad12d2d7a2846fe891f83f100e2466f794797eb465
//META: purpose=`InterlockedAdd` on a device buffer lowers to `atomic_fetch_add_explicit` through an `(atomic_uint device*)` cast with `memory_order_relaxed`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Metal atomic shape that `validateAtomicOperations` admits
// on this target: MSL has no HLSL-style `InterlockedAdd`, so the operation
// must be re-expressed as a C++11-style explicit atomic on a pointer that
// has been cast into MSL's `atomic_uint` view of device memory. CHECK
// asserts all three parts on the emitted call -- the function name, the
// pointer cast, and the relaxed ordering argument.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<uint> counter;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint prev;
    InterlockedAdd(counter[0], 1u, prev);
}

// CHECK: atomic_fetch_add_explicit
// CHECK-SAME: atomic_uint device*
// CHECK-SAME: memory_order_relaxed
