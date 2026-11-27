// Regression test for https://github.com/shader-slang/slang/issues/13772
//
// SPV_NV_linear_swept_spheres has two capabilities that Vulkan gates behind two separate
// features: `RayTracingSpheresGeometryNV` (`spheres`) and
// `RayTracingLinearSweptSpheresGeometryNV` (`linearSweptSpheres`). Each sphere or LSS hit query
// must declare only the capability of the primitive it reads, so that a shader using one kind
// of primitive does not require the feature of the other.
//
// `OpCapability` is a module-level union, so each function is compiled alone through a `-D`
// selector. Under CI's SLANG_RUN_SPIRV_VALIDATION=1, spirv-val also rejects a query that lacks
// its capability, such as `HitObject.IsSphereHit()` without `RayTracingSpheresGeometryNV`.
// The MIXED case checks that a module using both primitives still declares both capabilities.

//TEST:SIMPLE(filecheck=SPHERE): -target spirv -stage closesthit -entry main -DGLOBAL_SPHERE_POSITION_AND_RADIUS
//TEST:SIMPLE(filecheck=SPHERE): -target spirv -stage closesthit -entry main -DGLOBAL_IS_SPHERE_HIT
//TEST:SIMPLE(filecheck=LSS): -target spirv -stage closesthit -entry main -DGLOBAL_LSS_POSITIONS_AND_RADII
//TEST:SIMPLE(filecheck=LSS): -target spirv -stage closesthit -entry main -DGLOBAL_IS_LSS_HIT
//TEST:SIMPLE(filecheck=SPHERE): -target spirv -stage closesthit -entry main -capability spvShaderInvocationReorderNV -DHIT_OBJECT_SPHERE_POSITION_AND_RADIUS
//TEST:SIMPLE(filecheck=SPHERE): -target spirv -stage closesthit -entry main -capability spvShaderInvocationReorderNV -DHIT_OBJECT_IS_SPHERE_HIT
//TEST:SIMPLE(filecheck=LSS): -target spirv -stage closesthit -entry main -capability spvShaderInvocationReorderNV -DHIT_OBJECT_LSS_POSITIONS_AND_RADII
//TEST:SIMPLE(filecheck=LSS): -target spirv -stage closesthit -entry main -capability spvShaderInvocationReorderNV -DHIT_OBJECT_IS_LSS_HIT
//TEST:SIMPLE(filecheck=MIXED): -target spirv -stage closesthit -entry main -DMIXED

// The -NOT lines on both sides of the positive match cover the whole module.
// SPHERE-NOT: OpCapability RayTracingLinearSweptSpheresGeometryNV
// SPHERE: OpCapability RayTracingSpheresGeometryNV
// SPHERE-NOT: OpCapability RayTracingLinearSweptSpheresGeometryNV

// LSS-NOT: OpCapability RayTracingSpheresGeometryNV
// LSS: OpCapability RayTracingLinearSweptSpheresGeometryNV
// LSS-NOT: OpCapability RayTracingSpheresGeometryNV

// MIXED-DAG: OpCapability RayTracingSpheresGeometryNV
// MIXED-DAG: OpCapability RayTracingLinearSweptSpheresGeometryNV

struct Payload
{
    float4 sphere;
    float2x4 lss;
    bool isHit;
};

[shader("closesthit")]
void main(inout Payload payload, BuiltInTriangleIntersectionAttributes attr)
{
#if defined(GLOBAL_SPHERE_POSITION_AND_RADIUS)
    payload.sphere = GetSpherePositionAndRadius();
#elif defined(GLOBAL_IS_SPHERE_HIT)
    payload.isHit = IsSphereHit();
#elif defined(GLOBAL_LSS_POSITIONS_AND_RADII)
    payload.lss = GetLssPositionsAndRadii();
#elif defined(GLOBAL_IS_LSS_HIT)
    payload.isHit = IsLssHit();
#elif defined(MIXED)
    payload.isHit = IsSphereHit() || IsLssHit();
#else
    HitObject hitObject = HitObject.MakeNop();
#if defined(HIT_OBJECT_SPHERE_POSITION_AND_RADIUS)
    payload.sphere = hitObject.GetSpherePositionAndRadius();
#elif defined(HIT_OBJECT_IS_SPHERE_HIT)
    payload.isHit = hitObject.IsSphereHit();
#elif defined(HIT_OBJECT_LSS_POSITIONS_AND_RADII)
    payload.lss = hitObject.GetLssPositionsAndRadii();
#elif defined(HIT_OBJECT_IS_LSS_HIT)
    payload.isHit = hitObject.IsLssHit();
#endif
#endif
}
