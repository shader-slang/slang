//TEST:INTERPRET(hgvm): -api default

import slang.runtime;

struct Payload
{
    bool enabled;
    float3 position;
}

[host]
BufferView<Payload> makeView()
{
    let buffer = RWStructuredBuffer<Payload>::alloc(4);
    buffer[0] = Payload(false, float3(10, 20, 30));
    buffer[3] = Payload(false, float3(40, 50, 60));
    let source = RuntimeArray<Payload>::alloc(2);
    source[0] = Payload(true, float3(1, 2, 3));
    source[1] = Payload(false, float3(4, 5, 6));
    let view = buffer[range(1, 3)];
    view.copyFrom(source);
    assert(buffer[0].position.x == 10 && buffer[3].position.z == 60);
    return view;
}

[host]
void main()
{
    // The view outlives the local buffer handle and reads only when host() is called.
    let view = makeView();
    let snapshot = view.host();
    assert(view.getCount() == 2 && snapshot.getCount() == 2);
    assert(snapshot[0].enabled && snapshot[0].position.z == 3);
    snapshot[0].position.x = 99;
    assert(view.host()[0].position.x == 1);
    view.copyFrom(snapshot);
    assert(view.host()[0].position.x == 99);

    let destination = RWStructuredBuffer<Payload>::alloc(4);
    destination[range(0, 2)] = view;
    let saved = destination[range(0, 2)].host();
    assert(saved[0].position.x == 99 && saved[1].position.z == 6);
    destination[range(2, 4)].copyFrom(destination[range(0, 2)]);
    assert(destination[3].position.z == 6);

    // Rebinding a view leaves both allocations untouched.
    var rebound = destination[range(0, 2)];
    rebound = view;
    snapshot[0].position.x = 77;
    rebound.copyFrom(snapshot);
    assert(view.host()[0].position.x == 77 && destination[0].position.x == 99);

    let readOnly = StructuredBuffer<Payload>::alloc(2);
    copy(readOnly, 0, destination, 0, 2);
    let sourceView = readOnly[range(0, 2)];
    assert(sourceView.host()[0].position.x == 99);
    view.copyFrom(sourceView);
    assert(view.host()[0].position.x == 99);

    let empty = destination[range(4, 4)];
    assert(empty.host().getCount() == 0);
    empty.copyFrom(RuntimeArray<Payload>::alloc(0));
    destination[range(4, 4)] = empty;
}
