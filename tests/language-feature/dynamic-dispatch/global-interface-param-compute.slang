// End-to-end correctness test for global-parameter dynamic dispatch.
// This is the primary runtime-correctness coverage for the fix in this
// PR — the compile-only `global-interface-param.slang` and
// `parameterblock-interface.slang` tests verify dispatch is *generated*,
// but only this compute test verifies it *executes correctly*.
//
// Path coverage: `ConstantBuffer<Params>` is an `IRPointerLikeType`,
// so loading `gParams.foo` exercises the same `specializeLoad` and
// `lowerExtractDynamicObject` paths fixed in this PR. Two distinct
// global params (gParamsA / gParamsB) bound to different concrete
// types verify the dispatch actually selects the right impl at the
// witness-table tag, not just that one impl happens to come through.
// (Direct `ConstantBuffer<IFoo>` binding is not supported by the CPU
// compute test framework for non-trivial dispatch, so the wrapper
// struct is the practical end-to-end vehicle.)
//
// Two complementary verifications:
//
// (1) COMPUTE — runs the shader on CPU and asserts the dispatched
//     method returned the actual concrete value (5 from ImplA,
//     14 from ImplB*2). The compile-only filechecks elsewhere in
//     this PR can't catch a regression where dispatch boilerplate
//     is emitted but produces the wrong value at runtime.
//
// (2) REPORT — uses the `-report-dynamic-dispatch-sites` diagnostic
//     (E50102) as the stable structural contract. We check that a
//     dispatch was generated for the multi-conformance call sites,
//     instead of matching synthesized function names that are
//     implementation-dependent and would rot if the dispatch shim
//     naming changes.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=COMPUTE):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=COMPUTE):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=COMPUTE):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=COMPUTE):-mtl -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=COMPUTE):-cuda -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -report-dynamic-dispatch-sites

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
    IFoo foo;
}

//TEST_INPUT:set gParamsA = new{ foo: new ImplA{ v: 5 } }
ConstantBuffer<Params> gParamsA;

//TEST_INPUT:set gParamsB = new{ foo: new ImplB{ v: 7 } }
ConstantBuffer<Params> gParamsB;

//TEST_INPUT: globalExistentialType __Dynamic
//TEST_INPUT: globalExistentialType __Dynamic

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // gParamsA is dispatched as ImplA.getValue() => v = 5.
    // REPORT: E50102{{.*}}generated dynamic dispatch code
    outputBuffer[0] = gParamsA.foo.getValue();
    // COMPUTE: 5

    // gParamsB is dispatched as ImplB.getValue() => v * 2 = 14.
    // REPORT: E50102{{.*}}generated dynamic dispatch code
    outputBuffer[1] = gParamsB.foo.getValue();
    // COMPUTE: 14
}
