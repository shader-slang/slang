// Indirect circular dependencies through multiple interfaces are also invalid.
//
// The circularity check must trace through all conforming types. If interface A
// has a conforming type that contains interface B, and interface B has a
// conforming type that contains interface A, this creates a cycle that prevents
// either from having a finite packed size.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl
//TEST:SIMPLE(filecheck=CHECK): -target glsl
//TEST:SIMPLE(filecheck=CHECK): -target cuda
//TEST:SIMPLE(filecheck=CHECK): -target cpp
//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK): -target wgsl
//TEST:SIMPLE(filecheck=NEG): -target spirv
//TEST:SIMPLE(filecheck=NEG): -target hlsl

[anyValueSize(32)]
interface IFoo
{
    float getValue();
}

[anyValueSize(32)]
interface IBar
{
    float process();
}

// CHECK-DAG: error[E41003]
// CHECK-DAG: BarImpl
// CHECK-DAG: circular reference through interface
// CHECK-DAG: IBar
struct BarImpl : IBar
{
    IFoo foo;
    float process() { return foo.getValue(); }
}

// CHECK-DAG: error[E41003]
// CHECK-DAG: FooImpl
// CHECK-DAG: circular reference through interface
// CHECK-DAG: IFoo
struct FooImpl : IFoo
{
    IBar bar;
    float getValue() { return bar.process(); }
}

IFoo createFoo()
{
    FooImpl f;
    return f;
}

IBar createBar()
{
    BarImpl b;
    return b;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = createFoo();
    IBar b = createBar();
    // NEG-NOT: error[E41011]
    outputBuffer[0] = f.getValue() + b.process();
}
