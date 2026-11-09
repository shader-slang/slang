//TEST:INTERPRET(hgvm): -api default

import slang.runtime.core.simple_assert;

struct Payload
{
    bool enabled;
    float3 position;
    uint tag;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void modify(RWStructuredBuffer<Payload> data, RWStructuredBuffer<uint> counter)
{
    data[1].position.x += 4.0f;
    data[1].tag += 10;
    InterlockedAdd(counter[0], 1u);
}

[host]
void main()
{
    let data = RWStructuredBuffer<Payload>::alloc(3);
    data[0] = Payload(false, float3(10.0f, 20.0f, 30.0f), 100u);
    data[1] = Payload(true, float3(1.0f, 2.0f, 3.0f), 7u);
    data[2] = Payload(false, float3(40.0f, 50.0f, 60.0f), 200u);
    let counter = RWStructuredBuffer<uint>::alloc(1);
    counter[0] = 0u;

    dispatch(modify, uint3(1, 1, 1))(data, counter);
    assert(counter[0] == 1u);
    assert(data[1].enabled && data[1].position.x == 5.0f && data[1].tag == 17u);

    var updated = data[1];
    updated.position = float3(9.0f, 8.0f, 11.0f);
    updated.enabled = false;
    updated.tag++;
    data[1] = updated;
    dispatch(modify, uint3(1, 1, 1))(data, counter);
    let result = data[1];
    assert(!result.enabled && !any(result.position != float3(13.0f, 8.0f, 11.0f)));
    assert(result.tag == 28u && counter[0] == 2u);
    assert(data[0].tag == 100u && data[2].tag == 200u);
    assert(!any(data[0].position != float3(10.0f, 20.0f, 30.0f)));
    assert(!any(data[2].position != float3(40.0f, 50.0f, 60.0f)));

    // Bulk transfers remain explicit. Read-only buffer indexing uses the same host readback.
    let readOnly = StructuredBuffer<Payload>::alloc(3);
    copy(readOnly, 0, data, 0, 3);
    assert(readOnly[1].tag == 28u);
    updated = data[1];
    updated.tag = 99u;
    data[1] = updated;
    copy(readOnly, 1, data, 1, 1);
    assert(readOnly[1].tag == 99u);
    counter[0] += 2u;
    counter[0]++;
    assert(counter[0] == 5u);
}
