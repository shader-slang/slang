// Regression test for #11197: SV_PrimitiveID is allowed in intersection,
// any-hit, and closest-hit stages.

//TEST:SIMPLE(filecheck=ISECT_SPIRV):-target spirv-assembly -entry main_isect -stage intersection
//TEST:SIMPLE(filecheck=AHIT_SPIRV):-target spirv-assembly -entry main_ahit -stage anyhit
//TEST:SIMPLE(filecheck=CHIT_SPIRV):-target spirv-assembly -entry main_chit -stage closesthit

struct Payload
{
    float val;
};

struct Attribs
{
    float v;
};

[shader("intersection")]
void main_isect(int pid : SV_PrimitiveID)
{
    Attribs attr = { float(pid) };
    ReportHit<Attribs>(1.0, 0, attr);
}

// ISECT_SPIRV: OpEntryPoint IntersectionKHR
// ISECT_SPIRV: BuiltIn PrimitiveId
// ISECT_SPIRV: %{{.*}} = OpVariable %_ptr_Input_int Input

[shader("anyhit")]
void main_ahit(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs : SV_IntersectionAttributes,
    int pid : SV_PrimitiveID)
{
    payload.val = float(pid) + attribs.barycentrics.x + attribs.barycentrics.y;
}

// AHIT_SPIRV: OpEntryPoint AnyHitKHR
// AHIT_SPIRV: BuiltIn PrimitiveId
// AHIT_SPIRV-DAG: OpTypePointer HitAttribute{{NV|KHR}}
// AHIT_SPIRV: %{{.*}} = OpVariable %_ptr_Input_int Input

[shader("closesthit")]
void main_chit(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs,
    int pid : SV_PrimitiveID)
{
    payload.val = float(pid) + attribs.barycentrics.x + attribs.barycentrics.y;
}

// CHIT_SPIRV: OpEntryPoint ClosestHitKHR
// CHIT_SPIRV: BuiltIn PrimitiveId
// CHIT_SPIRV-DAG: OpTypePointer HitAttribute{{NV|KHR}}
// CHIT_SPIRV: %{{.*}} = OpVariable %_ptr_Input_int Input
