// Regression test for #11197: SV_PrimitiveID is allowed in intersection,
// any-hit, and closest-hit stages.

//TEST:SIMPLE(filecheck=ISECT_SPIRV):-target spirv-assembly -emit-spirv-directly -entry main_isect -stage intersection
//TEST:SIMPLE(filecheck=AHIT_SPIRV):-target spirv-assembly -emit-spirv-directly -entry main_ahit -stage anyhit
//TEST:SIMPLE(filecheck=CHIT_SPIRV):-target spirv-assembly -emit-spirv-directly -entry main_chit -stage closesthit

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

// ISECT_SPIRV: BuiltIn PrimitiveId
// ISECT_SPIRV: %{{.*}} = OpVariable %_ptr_Input_int Input

[shader("anyhit")]
void main_ahit(inout Payload payload, int pid : SV_PrimitiveID)
{
    payload.val = float(pid);
}

// AHIT_SPIRV: BuiltIn PrimitiveId
// AHIT_SPIRV: %{{.*}} = OpVariable %_ptr_Input_int Input
// AHIT_SPIRV-NOT: OpTypePointer HitAttribute

[shader("closesthit")]
void main_chit(inout Payload payload, int pid : SV_PrimitiveID)
{
    payload.val = float(pid);
}

// CHIT_SPIRV: BuiltIn PrimitiveId
// CHIT_SPIRV: %{{.*}} = OpVariable %_ptr_Input_int Input
// CHIT_SPIRV-NOT: OpTypePointer HitAttribute
