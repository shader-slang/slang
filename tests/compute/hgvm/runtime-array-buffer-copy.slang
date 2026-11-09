//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

struct Payload
{
    bool enabled;
    uint value;
}

[host]
void main()
{
    RuntimeArray<Payload> hostSource = RuntimeArray<Payload>::alloc(4);
    hostSource[0].enabled = true;
    hostSource[0].value = 17;
    hostSource[1].enabled = false;
    hostSource[1].value = 23;
    hostSource[2].enabled = true;
    hostSource[2].value = 42;

    StructuredBuffer<Payload> readOnlyBuffer = StructuredBuffer<Payload>::alloc(4);
    copy(readOnlyBuffer, 1, hostSource, 0, 3);

    RWStructuredBuffer<Payload> writableBuffer = RWStructuredBuffer<Payload>::alloc(4);
    copy(writableBuffer, 0, readOnlyBuffer, 1, 3);

    RuntimeArray<Payload> hostDestination = RuntimeArray<Payload>::alloc(4);
    copy(hostDestination, 1, writableBuffer, 0, 3);
    copy(hostDestination, 4, writableBuffer, 4, 0);

    print(hostDestination[0].enabled ? 1 : 0);
    print(hostDestination[0].value);
    print(hostDestination[1].enabled ? 1 : 0);
    print(hostDestination[1].value);
    print(hostDestination[2].enabled ? 1 : 0);
    print(hostDestination[2].value);
    print(hostDestination[3].enabled ? 1 : 0);
    print(hostDestination[3].value);
}

// CHECK: standard output = {
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 17
// CHECK-NEXT: 0
// CHECK-NEXT: 23
// CHECK-NEXT: 1
// CHECK-NEXT: 42
