// raypayload-empty-struct.slang

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv

// Test error for empty ray payload struct
// Empty ray payloads are not supported by most target APIs (SPIR-V, DXR)

// CHECK: ([[# @LINE+1]]): error 40002:
struct [raypayload] RayPayload
{
};

uniform RWStructuredBuffer<int> resultBuffer;
uniform RaytracingAccelerationStructure sceneBVH;

[shader("raygeneration")]
void rayGenShader()
{
    HitObject hitObjectNOP = HitObject.MakeNop();
    resultBuffer[0] = hitObjectNOP.IsNop();
    RayPayload payload = {};
    HitObject.Invoke(sceneBVH, hitObjectNOP, payload);
}
