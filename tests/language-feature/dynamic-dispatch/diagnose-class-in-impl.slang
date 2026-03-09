// Class types use reference semantics and are managed by the runtime.
// A class instance field cannot be packed into the fixed-size AnyValue
// byte buffer used for dynamic dispatch because the reference would
// become invalid after AnyValue round-tripping.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -conformance "Impl:IFoo=0" -conformance "GoodImpl:IFoo=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain -conformance "Impl:IFoo=0" -conformance "GoodImpl:IFoo=1"

class MyClass
{
    int val;
}

interface IFoo
{
    int get();
}

export struct Impl : IFoo
//CHECK:      ^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^ type 'Impl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    MyClass obj;
    int get() { return 1; }
}

export struct GoodImpl : IFoo
{
    int v;
    int get() { return v; }
}

uniform int dynId;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = createDynamicObject<IFoo>(dynId, 0);
    outputBuffer[0] = f.get();
}
