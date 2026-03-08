// ParameterBlock differs from ConstantBuffer in binding semantics
// (descriptor sets on Vulkan, root signatures on DX12), but should
// produce identical dispatch behavior for interface-typed struct fields.
// This test uses both a single interface field and an array of interface
// values to verify that ParameterBlock correctly handles existential
// layouts and dynamic dispatch.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

[anyValueSize(8)]
interface IFoo
{
    int getValue();
}

export struct ImplA : IFoo
{
    int v;
    int getValue() { return v; }
}

export struct ImplB : IFoo
{
    int v;
    int getValue() { return v * 2; }
}

struct SingleParams
{
    IFoo foo;
    int offset;
}

struct ArrayParams
{
    IFoo values[2];
}

//TEST_INPUT:set gSingle = new{ foo: new ImplA{ v: 7 }, offset: 100 }
ParameterBlock<SingleParams> gSingle;

//TEST_INPUT:set gArray = new{ values: [ new ImplA{ v: 2 }, new ImplB{ v: 3 } ] }
ParameterBlock<ArrayParams> gArray;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// One globalExistentialType per ParameterBlock global (gSingle, gArray)
//TEST_INPUT: globalExistentialType __Dynamic
//TEST_INPUT: globalExistentialType __Dynamic

[numthreads(1, 1, 1)]
void computeMain()
{
    // Single field: ImplA returns v=7, plus offset=100
    outputBuffer[0] = gSingle.foo.getValue() + gSingle.offset;
    // CHECK: 107

    // Array[0]: ImplA returns v=2
    outputBuffer[1] = gArray.values[0].getValue();
    // CHECK: 2

    // Array[1]: ImplB returns v*2=6
    outputBuffer[2] = gArray.values[1].getValue();
    // CHECK: 6

    // Combined: 7 + 2 + 6 = 15
    outputBuffer[3] = gSingle.foo.getValue() + gArray.values[0].getValue() + gArray.values[1].getValue();
    // CHECK: 15
}
