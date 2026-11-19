//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -g2 -emit-spirv-directly

// RayQuery is an opaque builtin. This test uses three [noinline] function parameters as a simple
// fixture that forces each distinct instantiation to be retained and emitted into debug info.
// Both flag operands must appear in the name, so instantiations that differ only in ray flags or
// query flags are distinguished instead of sharing an incorrect or missing name.

RWStructuredBuffer<float> outBuf;

[noinline]
float useRayQueryNone(RayQuery<RAY_FLAG_NONE> q, float r)
{
    return r;
}
[noinline]
float useRayQueryForceOpaque(RayQuery<RAY_FLAG_FORCE_OPAQUE> q, float r)
{
    return r;
}
[noinline]
float useRayQueryMicromap(
    RayQuery<RAY_FLAG_NONE, RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS> q,
    float r)
{
    return r;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    RayQuery<RAY_FLAG_NONE> q0;
    RayQuery<RAY_FLAG_FORCE_OPAQUE> q1;
    RayQuery<RAY_FLAG_NONE, RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS> q2;
    outBuf[tid.x] = useRayQueryNone(q0, 1.0) + useRayQueryForceOpaque(q1, 1.0)
        + useRayQueryMicromap(q2, 1.0);
}

// CHECK-DAG: %[[DEBUG_INFO_NONE:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInfoNone

// CHECK-DAG: %[[RQ0_NAME:[0-9]+]] = OpString "RayQuery<0,0>"
// CHECK-DAG: %[[RQ0_LINKAGE:[0-9]+]] = OpString "@RayQuery<0,0>"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[RQ0_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[RQ0_LINKAGE]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}
// CHECK-DAG: %[[RQ1_NAME:[0-9]+]] = OpString "RayQuery<1,0>"
// CHECK-DAG: %[[RQ1_LINKAGE:[0-9]+]] = OpString "@RayQuery<1,0>"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[RQ1_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[RQ1_LINKAGE]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}
// CHECK-DAG: %[[RQ2_NAME:[0-9]+]] = OpString "RayQuery<0,1>"
// CHECK-DAG: %[[RQ2_LINKAGE:[0-9]+]] = OpString "@RayQuery<0,1>"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[RQ2_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[RQ2_LINKAGE]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}
