// Indirect circular dependencies through multiple interfaces are also invalid.
//
// The circularity check must trace through all conforming types. If interface A
// has a conforming type that contains interface B, and interface B has a
// conforming type that contains interface A, this creates a cycle that prevents
// either from having a finite packed size.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

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

// CHECK: error[E41003]
// CHECK: BarImpl
// CHECK: circular reference through interface
// CHECK: IBar
struct BarImpl : IBar
{
    IFoo foo;
    float process() { return foo.getValue(); }
}

// CHECK: error[E41003]
// CHECK: FooImpl
// CHECK: circular reference through interface
// CHECK: IFoo
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
    outputBuffer[0] = f.getValue() + b.process();
}
