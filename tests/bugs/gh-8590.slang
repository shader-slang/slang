//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -emit-spirv-directly

// CHECK: OpCapability RayTracingKHR

[raypayload]
struct RayPayload
{
};

uniform RaytracingAccelerationStructure sceneBVH;

struct Result
{
    int queryWasSuccess;
    int invokeWasSuccess;
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
