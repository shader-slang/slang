// Issue #10526: the opaque-local-variable check also covers acceleration
// structures (the `kIROp_RaytracingAccelerationStructureType` leaf category),
// which the other opaque-local tests do not exercise.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage raygeneration

RaytracingAccelerationStructure as0, as1;
uniform bool useSecond;

[noinline]
RaytracingAccelerationStructure pick(bool b)
{
    return b ? as1 : as0;
}
//CHECK: 56004{{.*}}opaque type in local variable

[shader("raygeneration")]
void main()
{
    RaytracingAccelerationStructure a = pick(useSecond);
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 1;
    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(a, 0, 0xff, ray);
    q.Proceed();
}
