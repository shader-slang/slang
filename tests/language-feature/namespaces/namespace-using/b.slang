//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj

module b;

import a;

// `using` is lookup-local to the declaring module and is not re-exported through
// `import` (shader-slang/slang#11443), so an importer that wants `ns`'s members
// unqualified must opt in with its own `using`. `ns.f()` below stays qualified and
// works without this, demonstrating that legitimate cross-module namespace access
// is unaffected.
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
