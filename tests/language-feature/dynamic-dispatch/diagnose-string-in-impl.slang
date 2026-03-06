// String is a variable-length reference type that cannot be packed into
// the fixed-size AnyValue byte buffer used for dynamic dispatch.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

interface IFoo
{
    int get();
}

// CHECK: type 'Impl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct Impl : IFoo
{
    String s;
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
    IFoo f = createDynamicObject<IFoo>(0, 0);
    outputBuffer[0] = f.get();
}
