// Regression test for the #12392 cascade fix in `getAllEntryPointsNoOverride`.
//
// The GLSL module-scope `layout(derivative_group_quadsNV) in;` qualifier decorates every *entry
// point* of the module. `helper` below carries `[numthreads]` but is NOT an entry point (it is only
// called, and is not the function we compile). It must not receive the derivative-group decoration,
// and its (3,1,1) thread count must not be validated against the quad requirement — doing so would
// raise a spurious `E31210`. `realMain` is the selected entry point and has a valid (2,2,1) count.

//TEST:SIMPLE(filecheck=CHECK): -allow-glsl -fvk-use-entrypoint-name -target spirv -entry realMain -stage compute

buffer MyBlockName
{
    int result;
} outputBuffer;

[numthreads(3, 1, 1)]
float helper(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    return ddx(1);
}

layout(derivative_group_quadsNV) in;

[shader("compute")]
[numthreads(2, 2, 1)]
void realMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK: OpExecutionMode {{.*}} DerivativeGroupQuads{{NV|KHR}}
    outputBuffer.result = asint(helper(dispatchThreadID));
}
