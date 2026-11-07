// A `[Differentiable]` method in a generic extension gives its `this` parameter the type
// `no_diff T`, as does any generic function with a `no_diff T` parameter, and it gives a
// non-differentiable parameter such as `int idx[1]` the type `no_diff int[1]`. When such a
// function is called through an interface-typed value whose only conforming type is known
// statically, type-flow specialization must resolve the call to that concrete type, as it
// does for the same function without `no_diff`.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type -Xslang -std -Xslang 2026
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -Xslang -std -Xslang 2026
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -Xslang -std -Xslang 2026
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -profile cs_6_0 -std 2025
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -profile cs_6_0 -std 2026
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -profile cs_6_0 -std 202c
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry computeMain -std 2025
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry computeMain -std 2026
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry computeMain -std 202c

interface IValue
{
    float get();
    [mutating] void set(float v);
}

struct Value : IValue
{
    float x;
    float get() { return x; }
    [mutating] void set(float v) { x = v; }
}

extension<T : IValue> T
{
    [Differentiable]
    [BackwardDerivative(read_bwd)]
    float read() { return this.get(); }
    void read_bwd(float grad) {}

    [Differentiable]
    [mutating]
    void bump() { this.set(this.get() + 1.0); }

    [Differentiable]
    float readAt(int idx[1]) { return this.get() + float(idx[0]); }
}

float readNoDiff<T : IValue>(no_diff T t) { return t.get(); }

float callRead(IValue v) { return v.read(); }

[Differentiable]
float scaleRead(IValue v, float s) { return v.read() * s; }

float callReadNoDiff(IValue v) { return readNoDiff(v); }

float callBump(IValue v)
{
    var w = v;
    w.bump();
    return w.get();
}

float callReadAt(IValue v)
{
    int idx[1] = { 2 };
    return v.readAt(idx);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// HLSL: void computeMain(
// SPIRV: OpEntryPoint
[numthreads(1, 1, 1)]
void computeMain()
{
    Value value = { 3.0 };

    // CHECK: 3.0
    outputBuffer[0] = callRead(value);
    // CHECK-NEXT: 6.0
    outputBuffer[1] = scaleRead(value, 2.0);
    // CHECK-NEXT: 3.0
    outputBuffer[2] = callReadNoDiff(value);
    // CHECK-NEXT: 4.0
    outputBuffer[3] = callBump(value);
    // CHECK-NEXT: 5.0
    outputBuffer[4] = callReadAt(value);
}
