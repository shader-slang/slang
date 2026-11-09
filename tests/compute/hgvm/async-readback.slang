//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

[shader("compute")]
[numthreads(1, 1, 1)]
void writeValue(RWStructuredBuffer<uint> output, uint value)
{
    output[0] = value;
}

struct Payload
{
    bool enabled;
    uint value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writePayload(RWStructuredBuffer<Payload> output)
{
    Payload payload;
    payload.enabled = true;
    payload.value = 123;
    output[0] = payload;
}

[host]
void main()
{
    RWStructuredBuffer<uint> output = RWStructuredBuffer<uint>::alloc(1);
    dispatch(writeValue, uint3(1, 1, 1))(output, 42);

    // This call submits the write and device-to-readback copy without waiting on the host. The
    // following dispatch starts a later batch, so the deferred value must still capture 42.
    Defer<uint> pending = asyncReadBack(output, 0);
    Defer<uint> alias = pending;
    dispatch(writeValue, uint3(1, 1, 1))(output, 99);

    // Keep a second transfer pending at the same time. The readback pool must lease a distinct
    // slot until each defer captures its own bytes.
    Defer<uint> secondPending = asyncReadBack(output, 0);
    dispatch(writeValue, uint3(1, 1, 1))(output, 7);

    // Aliases share one transfer, and repeated waits reuse its captured bytes.
    print(alias.wait());
    print(pending.wait());
    print(secondPending.wait());

    // Both pool slots are free now, so this readback exercises the reuse path and captures the
    // third dispatch without allocating another staging resource.
    Defer<uint> reusedSlot = asyncReadBack(output, 0);
    print(reusedSlot.wait());

    // A synchronous readback is a later submission boundary and observes the third dispatch.
    print(readBack(output, 0));

    // Defer<T> has one managed handle for any T. Waiting applies the cached device-to-host layout
    // converter, including the four-byte shader bool to one-byte HGVM bool conversion here.
    RWStructuredBuffer<Payload> payloads = RWStructuredBuffer<Payload>::alloc(1);
    dispatch(writePayload, uint3(1, 1, 1))(payloads);
    Defer<Payload> pendingPayload = asyncReadBack(payloads, 0);
    Payload payload = pendingPayload.wait();
    print(payload.enabled);
    print(payload.value);
}

// CHECK: 42
// CHECK-NEXT: 42
// CHECK-NEXT: 99
// CHECK-NEXT: 7
// CHECK-NEXT: 7
// CHECK-NEXT: true
// CHECK-NEXT: 123
