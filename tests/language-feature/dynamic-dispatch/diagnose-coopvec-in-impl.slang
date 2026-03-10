// Cooperative vectors represent data distributed across a subgroup of
// threads. They cannot be packed into a per-thread AnyValue byte buffer
// because their storage model is fundamentally different from ordinary
// per-thread data.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -conformance "Impl:IFoo=0" -conformance "GoodImpl:IFoo=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain -conformance "Impl:IFoo=0" -conformance "GoodImpl:IFoo=1"

interface IFoo
{
    int get();
}

export struct Impl : IFoo
//CHECK:      ^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^ type 'Impl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    CoopVec<float, 4> v;
    int get() { return 1; }
}

export struct GoodImpl : IFoo
{
    float v;
    int get() { return int(v); }
}

uniform int dynId;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = createDynamicObject<IFoo>(dynId, 0);
    outputBuffer[0] = f.get();
}
