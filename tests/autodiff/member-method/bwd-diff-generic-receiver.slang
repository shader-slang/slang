//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain

// Regression test for shader-slang/slang#11356: __bwd_diff applied to a bound
// member access where the receiver is a generic type parameter constrained by
// an interface. The witness-table-dispatched call has the same shape mismatch
// as the concrete case (Repro A); before the fix it surfaced as
// `unimplemented: assignment` during IR lowering instead of a segfault. The
// second TEST line pins the SPIR-V emit path so a regression of the original
// witness-table crash is caught even when the -cpu run uses different lowering.
// SPIRV: OpEntryPoint

interface IShape
{
    [Differentiable]
    [NoDiffThis]
    float apply(float x);
}

struct MyShape : IShape
{
    float scale;

    [Differentiable]
    [NoDiffThis]
    float apply(float x) { return scale * x * x; }
}

float gradientThrough<S: IShape>(S shape, float x)
{
    var dpx = diffPair(x, 0.0);
    __bwd_diff(shape.apply)(dpx, 1.0);
    return dpx.d;
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    MyShape s = { 3.0 };
    outputBuffer[0] = gradientThrough(s, 2.0);
    // d/dx(scale * x^2) at scale=3, x=2 is 2 * scale * x = 12.
    // CHECK: 12.0
}
