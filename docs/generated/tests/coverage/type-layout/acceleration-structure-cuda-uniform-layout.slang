//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `RaytracingAccelerationStructure` on the CUDA target is laid out as a single pointer-sized uniform and emits as an OptixTraversableHandle field of the global-parameter block.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `CUDAObjectLayoutRulesImpl::GetObjectLayout` groups
// `ShaderStorageBuffer` and `AccelerationStructure` into one arm that returns a
// single `sizeof(CUDAPtr)`-sized uniform, as opposed to the neighbouring buffer
// arms which return pointer-plus-count. Reaching it needs an acceleration
// structure on a CUDA/OptiX pipeline stage — a compute entry point rejects
// `TraceRay` at capability checking — so the test uses a `raygeneration` entry
// point. The CHECK pins the acceleration structure appearing as one scalar
// `OptixTraversableHandle` member of `GlobalParams_0`, which is what a
// single-pointer uniform layout produces.
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry rayGenMain -stage raygeneration

RaytracingAccelerationStructure scene;
RWStructuredBuffer<float> outp;
struct Payload { float hitT; };
[shader("raygeneration")]
void rayGenMain()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 1000;
    Payload p;
    p.hitT = 0;
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, p);
    outp[0] = p.hitT;
}

//CUDA: struct GlobalParams{{[A-Za-z0-9_]*$}}
//CUDA: OptixTraversableHandle scene{{[A-Za-z0-9_]*}};
//CUDA: RWStructuredBuffer<float> outp{{[A-Za-z0-9_]*}};
