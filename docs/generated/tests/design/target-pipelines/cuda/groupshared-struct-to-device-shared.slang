//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=4b4e0759c8a603e1b1c8bce1cbdccd4b05a30002e4de763fd5617a3d9f9b28cb
//META: purpose=Verifies a struct-typed groupshared declaration is lowered to __device__ __shared__ with field accesses preserved on CUDA emit.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: groupshared of an aggregate (struct) type. The
// `__device__ __shared__` lowering still applies and field
// accesses survive as `(&gs_0)->field_N` reads.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

struct Pair { int a; int b; }
groupshared Pair gs;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, uint3 ltid : SV_GroupThreadID)
{
    if (ltid.x == 0U) { gs.a = int(tid.x); gs.b = int(tid.y); }
    GroupMemoryBarrierWithGroupSync();
    outBuf[tid.x] = gs.a + gs.b;
}

// CHECK: __device__ __shared__
// CHECK: __syncthreads
