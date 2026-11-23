// Regression test for shader-slang/slang#14493.
//
// After inlining `find`, multi-level break lowering introduces a result that
// is undefined when the search is exhausted. The return-level flag selects
// the fallback on that path, so the undefined result is never observed.
// Demoting its phi into a variable without writing the undefined incoming
// lets downstream SSA construction carry an earlier bounce's result around
// the outer loop, keeping it live across TraceRay. Preserve the native phi
// and its undefined incoming instead.
//
// Both optimization modes must pass an undefined value directly into the
// search-result phi after the trace, without first putting it in mutable
// storage. The undefined value itself may be defined inside or outside the
// function. The vector entry point checks the same incoming value for a vector.

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly -minimum-slang-optimization
//TEST:SIMPLE(filecheck=VEC): -target spirv-asm -stage raygeneration -entry mainVec -emit-spirv-directly
//TEST:SIMPLE(filecheck=VEC): -target spirv-asm -stage raygeneration -entry mainVec -emit-spirv-directly -minimum-slang-optimization

RaytracingAccelerationStructure scene;
RWStructuredBuffer<float> buf;

[ForceInline]
float find(float u)
{
    for (uint i = 0; i < 4; ++i)
        if (u < buf[i])
            return buf[i];
    return 1.0;
}

[ForceInline]
float4 findVec(float u)
{
    for (uint i = 0; i < 4; ++i)
        if (u < buf[i])
            return buf[i].xxxx;
    return float4(1.0);
}

[shader("raygeneration")]
void main()
{
    RayDesc ray = { 0, 0, 1, 1e30 };
    float t = 0;
    for (int k = 0; k < 8; ++k)
    {
        TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, t);
        ray.TMax = find(t);
    }
    buf[0] = ray.TMax;
}

[shader("raygeneration")]
void mainVec()
{
    RayDesc ray = { 0, 0, 1, 1e30 };
    float t = 0;
    for (int k = 0; k < 8; ++k)
    {
        TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, t);
        ray.TMax = findVec(t).x;
    }
    buf[0] = ray.TMax;
}

// SPV-DAG: %[[UNDEF:[A-Za-z0-9_]+]] = OpUndef %float
// SPV-DAG: OpTraceRayKHR
// SPV: OpPhi %float {{.*}}%[[UNDEF]]

// VEC-DAG: %[[UNDEF:[A-Za-z0-9_]+]] = OpUndef %v4float
// VEC-DAG: OpTraceRayKHR
// VEC: OpPhi %v4float {{.*}}%[[UNDEF]]
