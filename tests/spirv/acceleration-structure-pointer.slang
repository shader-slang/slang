//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-via-glsl

uniform RaytracingAccelerationStructure* accelStructAddr;
RWStructuredBuffer<int> output;

// CHECK: %[[REG:[A-Za-z0-9_]+]] = OpConvertUToAccelerationStructureKHR
// CHECK: OpRayQueryInitializeKHR %rayQuery{{.*}} %[[REG]]

[numthreads(1,1,1)]
void main()
{
    let accelStruct = *accelStructAddr;
    RayQuery rayQuery;
    RayDesc ray;
    ray.Direction = float3(0, 0, 1);
    ray.Origin = float3(0, 0, 0);
    ray.TMax = 1000;
    ray.TMin = 0;
    let rs = rayQuery.TraceRayInline(accelStruct, 0, 0, ray);
    output[0] = rayQuery.CandidateGeometryIndex();
}

