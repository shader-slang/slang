// A `[Differentiable]` function that calls a generic-extension method through an
// interface-typed parameter must reach the method's custom backward derivative when
// it is differentiated with `bwd_diff`.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type -Xslang -std -Xslang 2026
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -Xslang -std -Xslang 2026
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -Xslang -std -Xslang 2026

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IValue
{
    float get();
}

struct Value : IValue
{
    float get() { return 3.0; }
}

extension<T : IValue> T
{
    [Differentiable]
    [BackwardDerivative(read_bwd)]
    float read() { return this.get(); }
    void read_bwd(float grad) { outputBuffer[0] += grad; }
}

[Differentiable]
float scaleRead(IValue v, float s) { return v.read() * s; }

[numthreads(1, 1, 1)]
void computeMain()
{
    Value value;
    var s = diffPair(2.0);
    bwd_diff(scaleRead)(value, s, 1.0);
    outputBuffer[1] = s.d;

    // The custom backward derivative of `read` receives d(result)/d(read) = s = 2,
    // and d(result)/d(s) = read() = 3.
    // CHECK: 2.0
    // CHECK-NEXT: 3.0
}
