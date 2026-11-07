// In legacy mode, a `[Differentiable]` generic-extension method called through an
// interface-typed value that can hold two conforming types is dispatched dynamically
// instead of being rejected with E33180. Type-flow then rebuilds the specialization with
// `no_diff` around the union of `A` and `B` for `this`, and around `int[1]` for `idx`.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type -Xslang -std -Xslang legacy
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -Xslang -std -Xslang legacy
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -Xslang -std -Xslang legacy
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -std legacy -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IValue
{
    float get();
}

struct A : IValue
{
    float get() { return 3.0; }
}

struct B : IValue
{
    float get() { return 5.0; }
}

extension<T : IValue> T
{
    [Differentiable]
    [BackwardDerivative(read_bwd)]
    float read() { return this.get(); }
    void read_bwd(float grad) {}

    [Differentiable]
    float readAt(int idx[1]) { return this.get() + float(idx[0]); }
}

IValue makeValue(uint id)
{
    if (id == 0)
        return A();
    return B();
}

float callRead(IValue v)
{
    return v.read();
    // REPORT-DAG: note[E50102]: generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float callReadAt(IValue v)
{
    int idx[1] = { 2 };
    return v.readAt(idx);
}

[Differentiable]
float scaleRead(IValue v, float s) { return v.read() * s; }

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // CHECK: 3.0
    outputBuffer[0] = callRead(makeValue(tid.x));
    // CHECK-NEXT: 5.0
    outputBuffer[1] = callRead(makeValue(tid.x + 1));
    // CHECK-NEXT: 7.0
    outputBuffer[2] = callReadAt(makeValue(tid.x + 1));
    // CHECK-NEXT: 10.0
    outputBuffer[3] = scaleRead(makeValue(tid.x + 1), 2.0);
}
