//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

// Reverse differentiation initializes its synthesized context with a raw default construction.
// HGBC removes that initialization because its instruction set has no default-construction opcode.

struct Payload
{
    RWStructuredBuffer<float> buffer;

    static Payload alloc(uint count)
    {
        return Payload(RWStructuredBuffer<float>::alloc(count));
    }
}

void consume_bwd(Payload payload, inout DifferentialPair<float> scale)
{
    updateDiff(scale, scale.d + readBack(payload.buffer, 0));
}

[BackwardDerivative(consume_bwd)]
void consume(Payload payload, float scale)
{
}

[Differentiable]
void run(float scale)
{
    Payload payload = Payload::alloc(1);
    consume(payload, scale);
}

[host]
public void main(float scale)
{
    var pair = diffPair(scale);
    bwd_diff(run)(pair);
}

// CHECK: OpAllocateResource Buffer Buffer Undefined DeviceLocal UnorderedAccess
// CHECK: OpReadBackBufferElement
