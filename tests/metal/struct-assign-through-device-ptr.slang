//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Repro for slangpy Metal failure: struct assignment through a device pointer
// in a ParameterBlock generates MSL that the Metal compiler rejects with
// "no viable overloaded '='".
//
// The root cause is: ParameterBlock type specialization causes Slang to emit
// two struct definitions for the same Slang type. The method then tries to
// assign between these structurally identical but nominally different types
// across address spaces (device vs thread), which Metal rejects.
//
// Without ParameterBlock, only one struct definition is emitted and Metal
// accepts the assignment.
//
// Regression between v2026.7.1 and current master.
//
// METALLIB: computeMain
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Element
{
    float x;
    int y;
};

struct Container
{
    RWStructuredBuffer<Element> data;

    void store(uint idx, Element value)
    {
        data[idx] = value;
    }

    Element load(uint idx)
    {
        return data[idx];
    }
};

ParameterBlock<Container> container;
RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Element e;
    e.x = 1.0;
    e.y = 2;
    container.store(tid.x, e);
    Element loaded = container.load(tid.x);
    output[0] = loaded.y;
}
