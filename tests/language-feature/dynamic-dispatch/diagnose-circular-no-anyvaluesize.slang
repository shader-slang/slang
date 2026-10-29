// Circular interface conformance without an `[anyValueSize(...)]` attribute should produce
// a clear error, not an internal compiler error.
//
// When an interface declaration has no `[anyValueSize(...)]` attribute, the compiler infers
// the AnyValue size from conforming types. If a struct conforms to an interface and contains
// a field of the corresponding existential box type, size inference has no base case.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl
//TEST:SIMPLE(filecheck=CHECK): -target glsl
//TEST:SIMPLE(filecheck=CHECK): -target cpp
//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK): -target wgsl

interface IFoo
{
    float getValue();
}

// CHECK-NOT: internal error
// CHECK: error[E41003]
// CHECK: BadImpl
// CHECK: circular reference through interface
// CHECK: IFoo
struct BadImpl : IFoo
{
    IFoo child;
    float getValue() { return child.getValue(); }
}

IFoo factory(uint id)
{
    return BadImpl();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo obj = factory(0);
    outputBuffer[0] = obj.getValue();
}
