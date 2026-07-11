//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:40:21+00:00
//META: source_commit=4e394f5c941b8145021611e1d3016b1f27a4ba52
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=0410773935d5331fbece0da3f2383f2a7478f2afa84b7d440d2490b07f25489f
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
