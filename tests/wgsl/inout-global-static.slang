// Regression test for shader-slang/slang#12173.
// A by-ref (`inout`/`out`) parameter is emitted as `ptr<function, T>`. A global `static`
// lowers to a module-scope `var<private>`, so passing it directly would form a
// `ptr<private, T>` at the call site — an address-space mismatch WGSL rejects. The WGSL
// legalizer must bridge such a module-scope argument through a `function`-space temp and
// write the temp back into the global after the call.

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain -warnings-disable 30056

RWStructuredBuffer<uint> output;

static uint4 thenBranchesV = {0, 0, 0, 0};
static uint4 elseBranchesV = {0, 0, 0, 0};
static uint4 outV = {0, 0, 0, 0};

// CHECK: fn incV{{.*}}( {{.*}} : ptr<function, vec4<u32>>)
uint4 incV(inout uint4 v)
{
    v = v + uint4(1U, 0U, 0U, 0U);
    return v;
}

// CHECK: fn setV{{.*}}( {{.*}} : ptr<function, vec4<u32>>)
void setV(out uint4 v)
{
    v = uint4(7U, 0U, 0U, 0U);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Each `inout` global-static arg is bridged through a `function`-space temp
    // (`&(_S...)`, never `&(thenBranchesV_0)` directly), and the same temp is written
    // back into the global after the call to preserve the `inout` update.
    // CHECK: incV{{.*}}(&([[T0:_S[0-9]+]]))
    // CHECK: thenBranchesV{{.*}} = [[T0]]
    // CHECK: incV{{.*}}(&([[T1:_S[0-9]+]]))
    // CHECK: elseBranchesV{{.*}} = [[T1]]
    output[tid.x + 8U] =
        (uint4(tid.x) >= uint4(2U) ? 6767U * incV(thenBranchesV) : 1111U * incV(elseBranchesV)).x;

    // The `out` global-static arg is likewise bridged, and the writeback is the load-bearing
    // half that stores the temp back into the module-scope global.
    // CHECK: setV{{.*}}(&([[T2:_S[0-9]+]]))
    // CHECK: outV{{.*}} = [[T2]]
    setV(outV);
    output[tid.x] = outV.x;
}
