// An interface-typed field filled through an untyped object input
// (`new{ ... }`, with no type name) must get the conformance ID that the
// compiled program uses for the concrete type.
//
// The shader constructs an `ImplB` itself, so the compiler numbers
// `ImplB : IFoo` before `ImplA : IFoo`, the reverse of the order in which the
// input below lists them. render-test used to create the untyped `Params`
// object with slang-rhi's device session, which numbers conformances in the
// order it is asked about them (`ImplA` first), so `p.a` dispatched to `ImplB`
// and `p.b` to `ImplA`.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

[anyValueSize(16)]
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
    IFoo a;
    IFoo b;
}

//TEST_INPUT:set gParams = new{ a: new ImplA{ v: 5 }, b: new ImplB{ v: 3 } }
ConstantBuffer<Params> gParams;

//TEST_INPUT: globalExistentialType __Dynamic
//TEST_INPUT: globalExistentialType __Dynamic

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Constructed in the shader; ImplA is only reached through gParams.
    IFoo local = ImplB(7);

    // CHECK: type: int32_t
    // gParams.a is an ImplA: 5.
    outputBuffer[0] = gParams.a.getValue();
    // CHECK-NEXT: 5

    // gParams.b is an ImplB: 3 * 2.
    outputBuffer[1] = gParams.b.getValue();
    // CHECK-NEXT: 6

    // local is an ImplB: 7 * 2.
    outputBuffer[2] = local.getValue();
    // CHECK-NEXT: 14
}
