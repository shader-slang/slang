//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Boundary: signed `int` MIN (encoded as `int(0x80000000)`) emits as the negative decimal `-2147483648` in Metal text.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: signed integer MIN edge. The literal `0x80000000`
// reinterpreted as `int` is the documented signed MIN; the Metal
// emitter renders it as `int(-2147483648)`. The runtime addend
// `tid.x` keeps the expression alive past constant folding.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) + int(0x80000000);
}

// CHECK: -2147483648
