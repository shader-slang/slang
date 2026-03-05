// An array of interface-typed values inside a struct wrapped by a
// ConstantBuffer is the pattern used in material/light systems where
// a fixed-size collection of polymorphic objects is passed as uniform
// data (e.g., an array of material layers or light sources).
//
// Each array element is an independent existential slot with its own
// RTTI, witness table, and AnyValue data. The compiler must generate
// dispatch code for each element's method call independently.

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

struct Params
{
    IFoo values[2];
}

//TEST_INPUT:set gParams = new{ values: [ new ImplA{ v: 3 }, new ImplB{ v: 5 } ] }
ConstantBuffer<Params> gParams;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: globalExistentialType __Dynamic

[numthreads(1, 1, 1)]
void computeMain()
{
    // ImplA: getValue() returns v=3
    outputBuffer[0] = gParams.values[0].getValue();
    // CHECK: 3

    // ImplB: getValue() returns v*2=10
    outputBuffer[1] = gParams.values[1].getValue();
    // CHECK: 10

    // Sum of both elements
    outputBuffer[2] = gParams.values[0].getValue() + gParams.values[1].getValue();
    // CHECK: 13

    // Index derived from runtime dispatch result to prevent constant folding
    int idx = gParams.values[0].getValue() - 2;
    outputBuffer[3] = gParams.values[idx].getValue();
    // CHECK: 10
}
