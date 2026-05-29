//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#collectoptixentrypointuniformparams
//META: doc_section_digest=83b5e2ba0599f98c93e210b43dbcde53e9857086c2c3137fc2c80137cf84aa44
//META: purpose=collectOptiXEntryPointUniformParams replaces collectEntryPointUniformParams for raygeneration entry points so a CUDA raygeneration shader emits as an extern "C" __global__ void __raygen__<name> kernel through the OptiX SBT pathway.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `## collectOptiXEntryPointUniformParams`: this CUDA-only Phase A
// pass handles SBT-bound uniform params for OptiX entry-point
// shapes, replacing the `collectEntryPointUniformParams` that the
// other shader targets run. The user-visible consequence is that
// a `[shader("raygeneration")]` kernel survives the pipeline and
// is wrapped by the CUDA emitter with the OptiX-style symbol
// prefix `__raygen__` (see `slang-emit-cuda.cpp` line ~494).

//TEST:SIMPLE(filecheck=CHECK):-target cuda -stage raygeneration -entry rayGenMain

uniform RWStructuredBuffer<float4> g_output;

[shader("raygeneration")]
void rayGenMain()
{
    uint2 idx = DispatchRaysIndex().xy;
    g_output[idx.x] = float4(float(idx.x), float(idx.y), 0.0, 1.0);
}

// CHECK: extern "C" __global__ void __raygen__rayGenMain
