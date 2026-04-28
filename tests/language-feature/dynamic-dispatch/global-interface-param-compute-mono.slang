// Single-conformance counterpart to global-interface-param-compute.slang.
//
// With only one concrete impl in source AND a single conformance
// registered, the compiler must monomorphize the call: there is no
// runtime choice to make, so no dispatch shim should be emitted.
//
// Two complementary verifications:
//
// (1) COMPUTE — runs the shader on CPU and asserts the call to
//     getValue() actually returned the concrete value (6).
//
// (2) MONO — uses `-report-dynamic-dispatch-sites` to assert the
//     *absence* of an E50102 dispatch note for the call site. This
//     pins the monomorphization contract via the stable diagnostic
//     instead of the synthesized name patterns the existing tests
//     in this PR rely on.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=COMPUTE):-cpu -output-using-type
//TEST:SIMPLE(filecheck=MONO):-target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -report-dynamic-dispatch-sites

[anyValueSize(16)]
interface IFoo
{
    int getValue();
}

export struct ImplA : IFoo
{
    int v;
    int getValue() { return v + 1; }
}

struct Params
{
    IFoo foo;
}

//TEST_INPUT:set gParams = new{ foo: new ImplA{ v: 5 } }
ConstantBuffer<Params> gParams;

//TEST_INPUT: globalExistentialType __Dynamic

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Monomorphized: no dispatch shim emitted, the call is direct.
    // MONO-NOT: E50102
    outputBuffer[0] = gParams.foo.getValue();
    // COMPUTE: 6
}
