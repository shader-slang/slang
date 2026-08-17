//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=5cfb04763d734c87abb0c612990b5db758ffc4cce1785fc806d10db967096a26
//META: purpose=checkStaticAssert runs as a direct call at the end of Phase B, after specialization, so a static_assert whose condition depends on a generic value parameter is evaluated with the specialized value and a satisfied assertion compiles cleanly to a CUDA kernel.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies checkStaticAssert's placement after specialization (Phase B
// row 63): the assertion condition `N > 0` names a generic value
// parameter, so it can only be evaluated once specializeModule has
// substituted the concrete argument. Instantiating with N=4 satisfies
// the condition; the CHECKs assert the specialized function body holds
// the folded literal `int(4)` and that the kernel definition follows,
// which is only possible if the static_assert was checked against the
// specialized value rather than rejected pre-specialization.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

int pickPositive<let N : int>()
{
    static_assert(N > 0, "N must be positive");
    return N;
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = pickPositive<4>() + int(tid.x);
}

// The specialized function (carrying the folded literal) is emitted
// before the kernel in the CUDA output, so the CHECKs follow that order.
// CHECK: int(4)
// CHECK: extern "C" __global__ void main
