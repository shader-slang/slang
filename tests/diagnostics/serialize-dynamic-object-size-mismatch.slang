// `serializeDynamicObject` diagnoses a size mismatch (E41206) when the
// user-chosen payload storage type is not the same byte size as the
// interface's any-value payload.  Guards against silent data truncation
// or reading past the payload.  See #10221 / #10222.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain -conformance "FooA:IFoo=0"

[anyValueSize(16)]
interface IFoo
{
    float eval();
}

struct FooA : IFoo
{
    float x;
    float eval() { return x; }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    IFoo foo = createDynamicObject<IFoo>(0, 1.0);
    uint typeId;
    // Payload storage is 8 bytes (2 uints) but FooA's packed any-value is
    // 4 bytes (one float), so serializeDynamicObject cannot safely round-trip
    // the data through this U.
    uint[2] payloadTooBig;
    serializeDynamicObject(foo, typeId, payloadTooBig);
//CHECK:                  ^ serializeDynamicObject storage size mismatch
//CHECK:                  ^ `serializeDynamicObject` payload type 'array<uint,2>' has size '8' but interface 'IFoo' requires any-value size '4'
}
