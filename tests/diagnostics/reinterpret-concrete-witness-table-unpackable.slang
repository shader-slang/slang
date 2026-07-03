// Verify that the unresolved-generic scan in lowerReinterpret does not walk
// into concrete witness-table entries and mistake their generic-side operands
// for unresolved use-site types. A concrete specialization with an unpackable
// String payload must still report the user-facing AnyValue diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

interface IHasValue
{
    int get();
}

struct GenericImpl<T> : IHasValue
{
    T payload;
    int get() { return 0; }
}

RWStructuredBuffer<int> outBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    GenericImpl<String> value;
    value.payload = "hello";
    outBuffer[0] = reinterpret<int>(value);
//CHECK:                           ^ type cannot be packed for dynamic dispatch
//CHECK:                           ^ type 'GenericImpl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
}
