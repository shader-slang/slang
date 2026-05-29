// Unsized (runtime-sized) arrays cannot be packed into AnyValue because
// their size is indeterminate at compile time. The AnyValue region is a
// fixed-size byte buffer, so types with unknown size cannot participate
// in dynamic dispatch.
//
// The compiler rejects this with error 41014 like other unsupported types.
// See #10420.

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
    float data[];
    int get() { return int(data[0]); }
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
