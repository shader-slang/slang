//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj

module b;

import a;

// Load-bearing: the unqualified `f()` in `outVal = f() + ns.f()` below resolves
// ONLY because of this `using ns;`. `using` is lookup-local to the declaring module
// and is not re-exported through `import` (shader-slang/slang#11443), so this
// importer must opt in to use `ns`'s members unqualified — delete this line and
// `f()` becomes an undefined identifier (E30015). The qualified `ns.f()` resolves
// without it; legitimate cross-module namespace access is unaffected.
using ns;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int tid = dispatchThreadID.x;
    int inVal = tid;
    int outVal = f() + ns.f();
    outputBuffer[tid] = outVal;
    // CHECK: 2
}
