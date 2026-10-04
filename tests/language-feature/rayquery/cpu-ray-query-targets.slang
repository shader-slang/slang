// The C++ host-callable path has a concrete triangle RayQuery implementation. The direct LLVM
// path intentionally remains unavailable until it has its own ABI and resumable lowering.

//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=LLVM,non-exhaustive): -target host-callable -emit-cpu-via-llvm -entry computeMain -stage compute

// CPP-NOT: struct RayDesc_
// CPP: RayDesc
// CPP: RayQuery<0>
// LLVM: E36107

RaytracingAccelerationStructure scene;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    RayDesc ray = {float3(0.0), 0.0, float3(0.0, 0.0, 1.0), 10.0};
    RayQuery<RAY_FLAG_NONE> query;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
    while (query.Proceed())
        query.CommitNonOpaqueTriangleHit();
}
