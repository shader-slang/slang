// Issue #10526: the opaque-local-variable check also covers acceleration
// structures (the `kIROp_RaytracingAccelerationStructureType` leaf category),
// which the other opaque-local tests do not exercise.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage raygeneration

RaytracingAccelerationStructure as0, as1;
uniform bool useSecond;

[noinline]
RaytracingAccelerationStructure pick(bool b)
{
    return b ? as1 : as0;
}

[shader("raygeneration")]
void main()
{
    // The selected acceleration structure is forced through a function-local
    // variable, which Khronos targets forbid; the diagnostic is reported at the
    // (synthesized) local's first use.
    RaytracingAccelerationStructure a = pick(useSecond);
//CHECK:                                    ^ opaque type in local variable is not supported for Khronos targets
//CHECK:                                    ^ a resource or other opaque-typed value ('RaytracingAccelerationStructure') cannot be placed in a function-local variable for Khronos targets (SPIR-V/GLSL); this usually comes from selecting a resource with control flow (e.g. a '?:' or 'if'/'else') or returning one from a function
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 1;
    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(a, 0, 0xff, ray);
    q.Proceed();
}
