// Test for empty ray payload struct with HitObject.Invoke
// GitHub Issue: https://github.com/shader-slang/slang/issues/8590
// The compiler should add a dummy field to empty ray payload structs for HLSL/DXIL/SPIRV targets.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile sm_6_5 -entry rayGenShaderMakeQueryInvokeNOP -stage raygeneration
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry rayGenShaderMakeQueryInvokeNOP -stage raygeneration -emit-spirv-directly -capability spirv_nv
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry rayGenShaderMakeQueryInvokeNOP -stage raygeneration

[raypayload]
struct RayPayload
{
    // Empty payload - should get a dummy field added for HLSL/DXIL
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

// HLSL: struct RayPayload
// HLSL: _slang_dummy
// HLSL: NvInvokeHitObject

// SPIRV: OpMemberName %RayPayload 0 "_slang_dummy"

// CUDA: optixInvoke
