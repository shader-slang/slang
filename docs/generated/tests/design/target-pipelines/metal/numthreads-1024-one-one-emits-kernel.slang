//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=3b19519e792b092406320b661c62d171b0f8cdff6f282ae48cf30ac091136ae8
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
