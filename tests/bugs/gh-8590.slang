// Test for empty ray payload struct with HitObject.Invoke
// GitHub Issue: https://github.com/shader-slang/slang/issues/8590
// HLSL/DXIL and direct SPIR-V use the same one-uint physical payload representation.
// CUDA/OptiX needs no artificial payload storage. The logical RayPayload type stays unchanged.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile sm_6_5 -entry rayGenShaderMakeQueryInvokeNOP -stage raygeneration -capability hlsl_nvapi
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry rayGenShaderMakeQueryInvokeNOP -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry rayGenShaderMakeQueryInvokeNOP -stage raygeneration

[raypayload]
struct RayPayload
{
};

uniform RaytracingAccelerationStructure sceneBVH;

struct Result
{
    int queryWasSuccess;
};

uniform RWStructuredBuffer<Result> resultBuffer;

[shader("raygeneration")]
void rayGenShaderMakeQueryInvokeNOP()
{
    HitObject hitObjectNOP = HitObject.MakeNop();
    resultBuffer[0].queryWasSuccess = hitObjectNOP.IsNop();
    RayPayload payload = {};
    HitObject.Invoke(sceneBVH, hitObjectNOP, payload);
}

// HLSL: struct EmptyRayPayload{{[A-Za-z0-9_]*}}
// HLSL-NEXT: {
// HLSL-NEXT: uint _slang_dummy{{.*}};
// HLSL-NEXT: };
// HLSL: NvInvokeHitObject

// SPIRV: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// SPIRV: %[[payloadType:[A-Za-z0-9_]+]] = OpTypeStruct %[[uint]]{{$}}
// SPIRV: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer RayPayloadKHR %[[payloadType]]
// SPIRV: %[[payload:[A-Za-z0-9_]+]] = OpVariable %[[pointer]] RayPayloadKHR
// SPIRV: OpHitObjectExecuteShaderNV {{.*}} %[[payload]]

// CUDA-NOT: _slang_dummy
// CUDA: optixInvoke
