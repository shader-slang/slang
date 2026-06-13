// Regression test for https://github.com/shader-slang/slang/issues/11531 (nested case).
//
// Companion to ../extension-reopen-namespace/. This exercises a *nested* reopened
// namespace (`outer::repro` reopened across fragments), so both the outer and
// inner `NamespaceDecl`s must reach ScopesWired before the extension header is
// resolved. (Iteration order of the wiring pre-pass does not matter:
// `ensureDecl(ns, ScopesWired)` is monotonic and `visitNamespaceDecl` wires each
// namespace against its sibling-chain peers, so driving the discovered namespaces
// in any order converges. This therefore guards the nested-namespace wiring
// itself -- a future refactor that narrowed the pre-pass back to
// enclosing-namespaces-only, or dropped the nested level, would be caught -- not a
// particular ordering.)
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
