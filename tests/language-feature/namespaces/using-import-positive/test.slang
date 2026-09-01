//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

module test;

import helper;

// Positive: qualified access to an imported module's namespace member resolves
// (the module's own scope is still re-exported through `import`).
int viaQualified(int x) { return Foo::moduleHelper(x); }

// Positive: an importer's OWN top-level `using namespace` still brings the
// members into this importer's scope. This is local to the importer; it does not
// leak to modules that import `test`. See shader-slang/slang#11443.
using namespace Foo;
int viaLocalUsing(int x) { return moduleHelper(x); }

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int tid = dispatchThreadID.x;
    // viaQualified(10) = 11, viaLocalUsing(20) = 21 -> 32 for every thread.
    outputBuffer[tid] = viaQualified(10) + viaLocalUsing(20);
    // CHECK: 32
}
