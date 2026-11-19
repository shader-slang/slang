// Regression test for shader-slang/slang#12785 (positive, cross-block).
//
// The reporter's real (MaterialX-style) pattern reuses a read-only-resource-load call across a
// basic-block boundary: an unconditional `first` call dominates a `repeated` call inside a
// divergent branch, with no memory-writing/synchronizing instruction in between. The dominated
// `repeated` call must reuse the result of the dominating `first` call.
//
// This does not conflict with gh-3429.slang: there neither branch's load dominates the other, so
// neither may be commoned; here `first` strictly dominates `repeated`.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute

StructuredBuffer<float> inputData;
RWStructuredBuffer<float> outputData;

[noinline]
float loadValue(uint index)
{
    return inputData[index];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;

    float first = loadValue(index);
    float acc = first;
    if ((index & 1) != 0)
    {
        // Dominated by `first`, with nothing writing or synchronizing memory in between: this call
        // reuses `first` rather than emitting a second `loadValue`.
        float repeated = loadValue(index);
        acc += repeated;
    }
    outputData[index] = acc;
}

// Exactly one call site (definition is `loadValue_0(uint index_0)`, which does not match).
// CHECK-COUNT-1: loadValue_0(index
// CHECK-NOT: loadValue_0(index
