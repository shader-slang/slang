//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -g2 -emit-spirv-directly

// RayQuery is an opaque builtin. This test uses two [noinline] function parameters as a simple
// fixture that forces each distinct instantiation to be retained and emitted into debug info.
// Its ray-flags operand must appear in the name, so the two instantiations below — which differ only
// in flags — are distinguished instead of both rendering the literal "unnamed" that a missing
// getTypeNameHint case would produce.

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

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    RayQuery<RAY_FLAG_NONE> q0;
    RayQuery<RAY_FLAG_FORCE_OPAQUE> q1;
    outBuf[tid.x] = useRayQueryNone(q0, 1.0) + useRayQueryForceOpaque(q1, 1.0);
}

// CHECK-DAG: %[[DEBUG_INFO_NONE:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInfoNone

// CHECK-DAG: %[[RQ0_NAME:[0-9]+]] = OpString "RayQuery<0>"
// CHECK-DAG: %[[RQ0_LINKAGE:[0-9]+]] = OpString "@RayQuery<0>"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[RQ0_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[RQ0_LINKAGE]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}
// CHECK-DAG: %[[RQ1_NAME:[0-9]+]] = OpString "RayQuery<1>"
// CHECK-DAG: %[[RQ1_LINKAGE:[0-9]+]] = OpString "@RayQuery<1>"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[RQ1_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[RQ1_LINKAGE]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}
