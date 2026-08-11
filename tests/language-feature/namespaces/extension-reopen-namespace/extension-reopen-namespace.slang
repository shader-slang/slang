// Regression test for https://github.com/shader-slang/slang/issues/11531
//
// An `extension` declared in one `implementing` fragment of a module must be able
// to resolve, by unqualified name, declarations made in an earlier fragment that
// reopens the same `namespace`. Before the fix the extension header
// (generic-parameter constraints, target type, and `where`-clauses) was checked
// before the namespace fragments were wired into each other's sibling scope chain,
// so `IBase`/`ICollector`/`Add` resolved to nothing (error 30015, cascading 30855).
//
// The split across two `__include` fragments below is load-bearing: the bug only
// reproduces when `repro` is reopened across separate files. An extension in the
// same fragment as its referenced types resolves fine, so merging reopen_ns_types
// and reopen_ns_ext into one file would silently neuter this regression.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

module reopen_ns;

__include reopen_ns_types;
__include reopen_ns_ext;

// A concrete leaf conforming to repro::ICollector so that the conditional
// conformance added by the extension (Add<In1,In2> : ICollector) is satisfied.
struct Leaf : repro::ICollector
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
    repro::Add<Leaf, Leaf> sum = {a, b};
    // base_value() comes from the fragment-1 struct; collect() from the
    // fragment-2 extension. Both must type-check and run.
    outputBuffer[dispatchThreadID.x] = sum.base_value() + sum.collect();
    // CHECK: 14
}
