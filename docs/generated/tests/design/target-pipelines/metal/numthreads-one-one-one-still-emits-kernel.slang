//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=3b19519e792b092406320b661c62d171b0f8cdff6f282ae48cf30ac091136ae8
//META: purpose=Boundary: minimum threadgroup size `[numthreads(1,1,1)]` is accepted and still emits the standard `[[kernel]] void main_0` shape (no `numthreads` attribute survives in Metal text).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: smallest valid numthreads `(1,1,1)` lower edge.
// The kernel still emits with `[[kernel]] void main_0`, and no
// `numthreads(` token appears in the Metal text.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: kernel
// CHECK: void main_0
// CHECK-NOT: numthreads(
