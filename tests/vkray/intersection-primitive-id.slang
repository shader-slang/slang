// intersection-primitive-id.slang

// Regression test for https://github.com/shader-slang/slang/issues/11197:
// `SV_PrimitiveID` must be accepted as an input to the intersection, any-hit,
// and closest-hit stages, matching `gl_PrimitiveID` in GLSL_EXT_ray_tracing,
// and must map to the `PrimitiveId` builtin in SPIR-V.

//TEST:SIMPLE(filecheck=CHECK_INTERSECTION): -emit-spirv-directly -stage intersection -entry mainIntersection -target spirv-assembly
//TEST:SIMPLE(filecheck=CHECK_ANYHIT): -emit-spirv-directly -stage anyhit -entry mainAnyHit -target spirv-assembly
//TEST:SIMPLE(filecheck=CHECK_CLOSESTHIT): -emit-spirv-directly -stage closesthit -entry mainClosestHit -target spirv-assembly
//TEST:SIMPLE(filecheck=CHECK_GLSL): -stage intersection -entry mainIntersection -target glsl

struct Payload
{
    float3 hitValue;
};

struct Attributes
{
    float2 bary;
};

// The builtin must be an Input-storage-class variable (not Private, which
// fails SPIR-V validation for BuiltIn PrimitiveId).

// CHECK_INTERSECTION: OpEntryPoint Intersection{{NV|KHR}} %mainIntersection
// CHECK_INTERSECTION: OpDecorate %[[VAR:[a-zA-Z0-9_]+]] BuiltIn PrimitiveId
// CHECK_INTERSECTION: %[[VAR]] = OpVariable %_ptr_Input_int Input
// CHECK_INTERSECTION: OpReportIntersectionKHR

// CHECK_GLSL: gl_PrimitiveID
[shader("intersection")]
void mainIntersection(uint primitiveId: SV_PrimitiveID)
{
    Attributes attr = {};
    attr.bary.x = primitiveId;
    ReportHit<Attributes>(1, 0, attr);
}

// CHECK_ANYHIT: OpEntryPoint AnyHit{{NV|KHR}} %mainAnyHit
// CHECK_ANYHIT: OpDecorate %[[VAR:[a-zA-Z0-9_]+]] BuiltIn PrimitiveId
// CHECK_ANYHIT: %[[VAR]] = OpVariable %_ptr_Input_int Input
[shader("anyhit")]
void mainAnyHit(inout Payload payload, in Attributes attr, uint primitiveId: SV_PrimitiveID)
{
    payload.hitValue.x = primitiveId;
}

// CHECK_CLOSESTHIT: OpEntryPoint ClosestHit{{NV|KHR}} %mainClosestHit
// CHECK_CLOSESTHIT: OpDecorate %[[VAR:[a-zA-Z0-9_]+]] BuiltIn PrimitiveId
// CHECK_CLOSESTHIT: %[[VAR]] = OpVariable %_ptr_Input_int Input
[shader("closesthit")]
void mainClosestHit(inout Payload payload, in Attributes attr, uint primitiveId: SV_PrimitiveID)
{
    payload.hitValue.x = primitiveId;
}
