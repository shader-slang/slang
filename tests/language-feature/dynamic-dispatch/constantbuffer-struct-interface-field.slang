// A ConstantBuffer wrapping a struct with an interface-typed field is
// the practical workaround for the ConstantBuffer<IFoo> crash (#10314).
// The struct wrapper forces the existential tuple (RTTI + witness table
// + AnyValue) into a known layout within the constant buffer, allowing
// the compiler to generate correct pack/unpack and dispatch code.
//
// Two concrete types (ImplA returning the value, ImplB doubling it) are
// stored in separate constant buffers to verify that the dispatch reads
// the correct concrete type from each buffer independently.

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
    IFoo foo;
    int scale;
}

//TEST_INPUT:set gParamsA = new{ foo: new ImplA{ v: 5 }, scale: 3 }
ConstantBuffer<Params> gParamsA;

//TEST_INPUT:set gParamsB = new{ foo: new ImplB{ v: 4 }, scale: 2 }
ConstantBuffer<Params> gParamsB;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: globalExistentialType __Dynamic
//TEST_INPUT: globalExistentialType __Dynamic

[numthreads(1, 1, 1)]
void computeMain()
{
    // ImplA: getValue() returns v=5, scale=3 => 5*3 = 15
    outputBuffer[0] = gParamsA.foo.getValue() * gParamsA.scale;
    // CHECK: 15

    // ImplB: getValue() returns v*2=8, scale=2 => 8*2 = 16
    outputBuffer[1] = gParamsB.foo.getValue() * gParamsB.scale;
    // CHECK: 16

    // Direct dispatch without scale
    outputBuffer[2] = gParamsA.foo.getValue();
    // CHECK: 5

    outputBuffer[3] = gParamsB.foo.getValue();
    // CHECK: 8
}
