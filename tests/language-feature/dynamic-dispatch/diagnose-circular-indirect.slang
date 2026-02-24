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
    void process();
}

struct BarImpl : IBar
{
    IFoo foo;  // Contains IFoo
    void process() {}
}

// CHECK: error {{[0-9]+}}
struct FooImpl : IFoo
{
    IBar bar;  // Cycle: FooImpl -> IBar -> BarImpl -> IFoo -> FooImpl
    float getValue() { return 0; }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = createDynamicObject<IFoo>(0, 5);
    outputBuffer[0] = f.getValue();
}
