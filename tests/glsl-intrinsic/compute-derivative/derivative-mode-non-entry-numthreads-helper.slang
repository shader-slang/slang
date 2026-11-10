// The GLSL module-scope `layout(derivative_group_quadsNV) in;` qualifier decorates every entry
// point of the module. `helper` below carries `[numthreads(3, 1, 1)]` but is only called, so it is
// not an entry point: it gets neither the derivative-group decoration (whose quad check would
// reject its 3x1 thread count with E31210) nor an entry-point `LocalSize` execution mode.
// `realMain` is the selected entry point and has a valid 2x2 thread count.

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
    outputBuffer.result = asint(helper(dispatchThreadID));
}

// CHECK: OpEntryPoint GLCompute %[[EP:[A-Za-z0-9_]+]] "realMain"
// CHECK-NOT: OpExecutionMode {{%[A-Za-z0-9_]+}} LocalSize 3 1 1
// CHECK: OpExecutionMode %[[EP]] DerivativeGroupQuads{{NV|KHR}}
// CHECK-NOT: OpExecutionMode {{%[A-Za-z0-9_]+}} LocalSize 3 1 1
