//TEST:SIMPLE(filecheck=CHECK): -target spirv

uniform RaytracingAccelerationStructure* accelStructAddr;
RWStructuredBuffer<int> output;

// CHECK: %[[REGA:[A-Za-z0-9_]+]] = OpConvertPtrToU
// CHECK: %[[REGB:[A-Za-z0-9_]+]] = OpConvertUToAccelerationStructureKHR{{.*}} %[[REGA]]
// CHECK: OpRayQueryInitializeKHR %rayQuery{{.*}} %[[REGB]]

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
    rayQuery.TraceRayInline(accelStruct, 0, 0, ray);
    output[0] = rayQuery.CandidateGeometryIndex();
}

