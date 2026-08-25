//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry VendorClosestHit -stage closesthit -target hlsl -capability hlsl_nvapi
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry VendorClosestHit -stage closesthit -target spirv-assembly -emit-spirv-directly -capability spvShaderInvocationReorderNV

// HLSL-DAG: [shader("closesthit")]void VendorClosestHit(
// HLSL-DAG: NvRtGetClusterID
// HLSL-DAG: NvRtSphereObjectPositionAndRadius
// HLSL-DAG: NvRtLssObjectPositionsAndRadii
// HLSL-DAG: NvRtIsSphereHit
// HLSL-DAG: NvRtIsLssHit
// SPIRV-DAG: OpCapability RayTracingClusterAccelerationStructureNV
// SPIRV-DAG: OpCapability RayTracingLinearSweptSpheresGeometryNV
// SPIRV: OpEntryPoint ClosestHitKHR %VendorClosestHit "main"
// SPIRV-DAG: OpDecorate {{.*}} BuiltIn ClusterIDNV
// SPIRV-DAG: OpDecorate {{.*}} BuiltIn HitSpherePositionNV
// SPIRV-DAG: OpDecorate {{.*}} BuiltIn HitLSSPositionsNV

import slang.raytracing;

struct VendorPayload
{
    int clusterID;
    float4 sphere;
    float2x4 lss;
    bool isSphere;
    bool isLss;
}

struct VendorTraceContext : rt::ITraceContext
{
    typealias Payload = VendorPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct VendorRecord {}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = VendorTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = VendorRecord;
}

struct VendorClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.clusterID = GetClusterID();
        input.payload.sphere = GetSpherePositionAndRadius();
        input.payload.lss = GetLssPositionsAndRadii();
        input.payload.isSphere = IsSphereHit();
        input.payload.isLss = IsLssHit();
    }
}
