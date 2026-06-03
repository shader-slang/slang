// Regression test for #11197: SV_PrimitiveID is allowed in intersection,
// any-hit, and closest-hit stages per the GLSL_EXT_ray_tracing spec, but
// the front-end accessor table for `sv_primitiveid` only listed the
// rasterization stages, and `_processEntryPointVaryingParameter` rejected
// any `in` parameter on intersection regardless of semantic.

//TEST:SIMPLE(filecheck=ISECT):-target spirv-assembly -entry main_isect -stage intersection
//TEST:SIMPLE(filecheck=AHIT):-target spirv-assembly -entry main_ahit -stage anyhit
//TEST:SIMPLE(filecheck=CHIT):-target spirv-assembly -entry main_chit -stage closesthit

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
    Attribs a = { float(pid) };
    ReportHit<Attribs>(1.0, 0, a);
}
// ISECT: BuiltIn PrimitiveId

[shader("anyhit")]
void main_ahit(inout Payload payload, in Attribs attr, int pid : SV_PrimitiveID)
{
    payload.val = attr.v + float(pid);
}
// AHIT: BuiltIn PrimitiveId

[shader("closesthit")]
void main_chit(inout Payload payload, in Attribs attr, int pid : SV_PrimitiveID)
{
    payload.val = attr.v + float(pid);
}
// CHIT: BuiltIn PrimitiveId
