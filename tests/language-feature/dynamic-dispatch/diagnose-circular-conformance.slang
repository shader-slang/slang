// Direct circular dependency: struct contains the interface it implements.
//
// Dynamic dispatch uses a tagged union (AnyValue) to store conforming types at
// runtime. If a conforming type contains a field of the interface type it
// implements, the struct must be larger than the AnyValue it contains, but the
// AnyValue must be large enough to hold the struct. This is inherently
// unsatisfiable.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl
//TEST:SIMPLE(filecheck=CHECK): -target glsl
//TEST:SIMPLE(filecheck=CHECK): -target cpp
//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK): -target wgsl
//TEST:SIMPLE(filecheck=CHECK_NEG): -target spirv
//TEST:SIMPLE(filecheck=CHECK_NEG): -target hlsl

[anyValueSize(32)]
interface IValue
{
    float get();
}

// CHECK: error[E41003]
// CHECK: BadImpl
// CHECK: circular reference through interface
// CHECK: IValue
// CHECK_NEG-NOT: error[E41011]
struct BadImpl : IValue
{
    IValue child;
    float get() { return 0; }
}

IValue createValue()
{
    BadImpl b;
    return b;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IValue v = createValue();
    outputBuffer[0] = v.get();
}
