//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Boundary: maximum threadgroup size `[numthreads(1024,1,1)]` is accepted and emits standard `[[kernel]]` text without a `numthreads(...)` attribute.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: upper-edge numthreads (1024 on the X axis is the
// typical Metal hardware cap). The pass-through still emits the
// kernel shape; threadgroup size is a dispatch-time concern.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1024, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: kernel
// CHECK: void main_0
// CHECK-NOT: numthreads(
