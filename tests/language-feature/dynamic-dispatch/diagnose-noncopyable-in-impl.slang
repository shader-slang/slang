// Non-copyable types have their copy semantics disabled, but AnyValue
// marshalling copies data at the byte level, bypassing the type system's
// copy restrictions. The dyn interface validation rejects non-copyable
// members (error 33081), but regular interfaces currently allow them.
//
// This test documents the inconsistency (#10421): the compiler silently
// accepts non-copyable fields in regular interface implementations, even
// though AnyValue packing effectively copies the data. Whether this is
// intentional or a bug depends on whether non-copyable semantics should
// be enforced at the marshalling level.

//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -conformance "Impl:IFoo=0" -conformance "GoodImpl:IFoo=1"
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain -conformance "Impl:IFoo=0" -conformance "GoodImpl:IFoo=1"

[__NonCopyableType]
struct NonCopyData
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
    NonCopyData data;
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
