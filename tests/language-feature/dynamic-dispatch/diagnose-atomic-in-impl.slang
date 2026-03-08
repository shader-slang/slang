// Atomic types have memory ordering semantics (acquire/release) that are
// incompatible with byte-level AnyValue packing. An Atomic<int> field
// would lose its atomic guarantees when packed/unpacked through AnyValue.
//
// Currently crashes with "Unimplemented type packing" instead of producing
// a user-facing diagnostic (#10419). The compiler should reject this with
// error 41014 like other unsupported types.

//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

interface IFoo
{
    int get();
}

// CHECK: type 'Impl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct Impl : IFoo
{
    Atomic<int> counter;
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
