// Regression test for https://github.com/shader-slang/slang/issues/11531 (nested case).
//
// Companion to ../extension-reopen-namespace/. The fix drives an extension's
// enclosing `namespace` ancestors to ScopesWired *outermost-first*, so for a
// nested reopened namespace the outer namespace is wired before the inner one
// (an inner fragment can find its siblings only once its outer namespace is
// wired). The single-level test cannot exercise that ordering (one level => the
// order is irrelevant); this two-level case locks it in, so a future refactor
// that flipped to innermost-first or dropped an outer level would be caught.
//
// As in the single-level test, the split across two `__include` fragments is
// load-bearing: the bug only reproduces when `outer::repro` is reopened across
// separate files.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

module nested_reopen_ns;

__include nested_reopen_ns_types;
__include nested_reopen_ns_ext;

// A concrete leaf conforming to outer::repro::ICollector so the conditional
// conformance added by the extension (Add<In1,In2> : ICollector) is satisfied.
struct Leaf : outer::repro::ICollector
{
    int v;
    int base_value() { return v; }
    int collect() { return v; }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    Leaf a = {3};
    Leaf b = {4};
    outer::repro::Add<Leaf, Leaf> sum = {a, b};
    // base_value() comes from the fragment-1 struct; collect() from the
    // fragment-2 extension. Both must type-check and run.
    outputBuffer[dispatchThreadID.x] = sum.base_value() + sum.collect();
    // CHECK: 14
}
