// Non-copyable types have their copy semantics disabled, but AnyValue
// marshalling copies data at the byte level, bypassing the type system's
// copy restrictions. Both dyn interface validation (error 33081) and
// regular interface validation (error 41014) reject non-copyable members.
//
// This test verifies that error 41014 is emitted when a struct implementing
// a regular interface contains non-copyable fields, either directly or
// nested inside another struct. See #10421.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -conformance "Impl:IFoo=0" -conformance "ImplNested:IFoo=1" -conformance "GoodImpl:IFoo=2"
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain -conformance "Impl:IFoo=0" -conformance "ImplNested:IFoo=1" -conformance "GoodImpl:IFoo=2"

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

struct WrapperStruct
{
    NonCopyData wrapped;
    int padding;
}

export struct ImplNested : IFoo
//CHECK:      ^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^ type 'ImplNested' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    WrapperStruct container;
    int get() { return 2; }
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
