// Companion to serialize-dynamic-object-size-mismatch.slang covering
// the *undersized* direction of the E41206 diagnostic. The oversized
// case (storage > payload) already lives in the sibling file; this
// file pins the symmetric undersized case (storage < payload), which
// is the more dangerous direction because it implies silent data
// truncation rather than just unused trailing bytes. See
// #10221 / #10222.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain -conformance "FooA:IFoo=0"

[anyValueSize(16)]
interface IFoo
{
    float eval();
}

// 16 bytes — large enough that a 4-byte storage is meaningfully
// undersized regardless of whether typeflow specializes the
// payload to the singleton struct or to the union form.
struct FooA : IFoo
{
    float a, b, c, d;
    float eval() { return a + b + c + d; }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    IFoo foo = createDynamicObject<IFoo>(0, float4(1, 2, 3, 4));
    uint typeId;
    // Payload storage is 4 bytes (1 uint) but FooA's packed any-value
    // is 16 bytes (four floats). Going through this U would silently
    // drop 12 bytes of payload — exactly the case the diagnostic
    // exists to prevent.
    uint[1] payloadTooSmall;
    serializeDynamicObject(foo, typeId, payloadTooSmall);
//CHECK:                  ^ serializeDynamicObject storage size mismatch
//CHECK:                  ^ `serializeDynamicObject` payload type 'array<uint,1>' has size '4' but the specialized payload size for interface 'IFoo' is '16'
}
