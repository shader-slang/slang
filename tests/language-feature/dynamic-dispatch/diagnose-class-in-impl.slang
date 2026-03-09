// Class types use reference semantics and are managed by the runtime.
// A class instance field cannot be packed into the fixed-size AnyValue
// byte buffer used for dynamic dispatch because the reference would
// become invalid after AnyValue round-tripping.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -conformance "Impl:IFoo=0" -conformance "GoodImpl:IFoo=1"
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain -conformance "Impl:IFoo=0" -conformance "GoodImpl:IFoo=1"

class MyClass
{
    int val;
}

interface IFoo
{
    int get();
}

// CHECK: type 'Impl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct Impl : IFoo
{
    MyClass obj;
    int get() { return 1; }
}

export struct GoodImpl : IFoo
{
    int v;
    int get() { return v; }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = createDynamicObject<IFoo>(1, 0);
    outputBuffer[0] = f.get();
}
