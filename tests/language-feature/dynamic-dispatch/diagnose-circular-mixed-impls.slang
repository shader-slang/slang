// Mixed cyclic and non-cyclic implementations for the same interface.
//
// When an interface has both a non-cyclic implementation (GoodImpl) and a
// cyclic one (BadImpl), only the cyclic implementation should get E41003.
// Size inference should still work using the non-cyclic implementation.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl
//TEST:SIMPLE(filecheck=CHECK): -target glsl
//TEST:SIMPLE(filecheck=CHECK): -target cpp
//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK): -target wgsl
//TEST:SIMPLE(filecheck=NEG): -target spirv
//TEST:SIMPLE(filecheck=NEG): -target hlsl

[anyValueSize(32)]
interface IValue
{
    float get();
}

struct GoodImpl : IValue
{
    float data;
    float get() { return data; }
}

// CHECK: error[E41003]
// CHECK: BadImpl
// CHECK: circular reference through interface
// CHECK: IValue
struct BadImpl : IValue
{
    IValue child;
    float get() { return 0; }
}

IValue createGood()
{
    GoodImpl g;
    g.data = 42.0;
    return g;
}

IValue createBad()
{
    BadImpl b;
    return b;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IValue v = createGood();
    IValue w = createBad();
    // NEG-NOT: GoodImpl{{.*}}circular
    outputBuffer[0] = v.get() + w.get();
}
