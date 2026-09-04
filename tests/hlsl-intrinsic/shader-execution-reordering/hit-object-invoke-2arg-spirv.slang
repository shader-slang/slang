// #12553: the 2-argument HitObject::Invoke(hitObject, payload) (native-DXR SER spelling, no
// AccelerationStructure) must lower on SPIR-V (NV and EXT), GLSL (NV and EXT), and CUDA, emitting
// the same shader-execution op as the 3-argument overload.

//TEST:SIMPLE(filecheck=SPV): -target spirv -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderEXT
//TEST:SIMPLE(filecheck=NV):  -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=EXT): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderEXT
//TEST:SIMPLE(filecheck=GLSL_EXT): -target glsl -entry rayGenerationMain -stage raygeneration
//TEST:SIMPLE(filecheck=GLSL_NV): -target glsl -entry rayGenerationMain -stage raygeneration -capability GL_NV_shader_invocation_reorder
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry rayGenerationMain -stage raygeneration
//TEST:SIMPLE(filecheck=PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry rayGenerationMain -stage raygeneration

// Empty-payload entry point: when the payload type is eliminated by type legalization, the CUDA case
// resolves the single-argument optixInvoke wrapper (never instantiated by the payload-carrying form).
//TEST:SIMPLE(filecheck=EMPTYCUDA): -target cuda -entry rayGenEmpty -stage raygeneration
//TEST:SIMPLE(filecheck=EMPTYPTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry rayGenEmpty -stage raygeneration

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct MyPayload { float4 color; };
struct EmptyPayload {}

[shader("raygeneration")]
void rayGenerationMain()
{
    int idx = DispatchRaysIndex().x;

    RayDesc ray;
    ray.Origin = float3(idx, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    MyPayload payload = { float4(0) };

    HitObject hitFromTrace = HitObject::TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    // The 2-argument Invoke (no AccelerationStructure) must lower on every SER target, emitting
    // the same op the 3-argument form does. The CUDA/PTX checks are anchored to the entry function
    // so they match the emitted call, not the optixInvoke wrappers in the prelude.
    // CUDA: __raygen__rayGenerationMain
    // SPV: OpHitObjectExecuteShaderEXT
    // NV: OpHitObjectExecuteShaderNV
    // EXT: OpHitObjectExecuteShaderEXT
    // GLSL_EXT: hitObjectExecuteShaderEXT
    // GLSL_NV: hitObjectExecuteShaderNV
    // CUDA: optixInvoke(&{{.*}}, &{{.*}})
    // PTX: _optix_hitobject_invoke
    HitObject::Invoke(hitFromTrace, payload);

    outputBuffer[idx] = hitFromTrace.IsHit() ? 1 : 0;
}

[shader("raygeneration")]
void rayGenEmpty()
{
    int idx = DispatchRaysIndex().x;

    RayDesc ray;
    ray.Origin = float3(idx, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    EmptyPayload payload = {};

    HitObject hitFromTrace = HitObject::TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    // With an empty payload the CUDA case resolves the single-argument optixInvoke wrapper.
    // EMPTYCUDA: __raygen__rayGenEmpty
    // EMPTYCUDA: optixInvoke(&{{.*}})
    // EMPTYPTX: _optix_hitobject_invoke
    HitObject::Invoke(hitFromTrace, payload);

    outputBuffer[idx] = hitFromTrace.IsHit() ? 1 : 0;
}
