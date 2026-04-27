//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Verify that struct assignment through a device pointer in a ParameterBlock
// compiles correctly. If the resource-element lowering creates a spurious
// _default storage type for the struct, the Metal compiler will reject the
// cross-type assignment with "no viable overloaded '='".
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
