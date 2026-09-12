//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain

// Regression test for shader-slang/slang#11356: __bwd_diff applied to a bound
// member access (`obj.method`) on a non-static [Differentiable][NoDiffThis]
// method must lower correctly. PR #10827 made the resolved BwdDiffFuncType
// include `this` as an explicit first parameter for non-static member methods,
// but the front-end conversion of `__bwd_diff(obj.method)` to a static-style
// call left the receiver out of the argument list, so argCount was one short
// of the resolved paramCount and slangc segfaulted in the SPIR-V/WGSL emit
// path. The second TEST line pins that emit path so a future regression of
// the SPIR-V crash itself is caught (the -cpu run alone goes through a
// different lowering pipeline).
// SPIRV: OpEntryPoint

struct MyData
{
    float scale;

    [Differentiable]
    [NoDiffThis]
    float apply(float x) { return scale * x * x; }
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    MyData d = { 3.0 };
    var dpx = diffPair(2.0, 0.0);
    __bwd_diff(d.apply)(dpx, 1.0);

    // d/dx(scale * x^2) at scale=3, x=2 is 2 * scale * x = 12.
    outputBuffer[0] = dpx.d;
    // CHECK: 12.0
}
