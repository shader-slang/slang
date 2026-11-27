// Synthesized memberwise constructors have attributed parameters; Conditional
// payload propagation must inspect the matched payload type, not the outer
// attributed operand.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputs
RWStructuredBuffer<uint> outputs;

interface IValue
{
    uint read();
}

struct A : IValue
{
    uint x;
    uint read() { return x + 20; }
}

struct B : IValue
{
    uint4 x;
    uint read() { return x.x + x.y + x.z + x.w; }
}

extern static const bool ENABLED = true;

struct Holder
{
    uint x;
    Conditional<IValue, ENABLED> value = Conditional<IValue, ENABLED>(A(0));
}

IValue choose(uint id)
{
    if ((id & 1) == 0)
        return A(id);
    return B(uint4(id, 2, 3, 4));
}

uint read(Holder holder)
{
    if (let value = holder.value.get())
        return value.read();
    return 123;
}

uint run(uint id)
{
    Holder original;
    original.x = id + 100;
    original.value = Conditional<IValue, ENABLED>(choose(id));
    Holder copied = Holder(original.x, original.value);
    return copied.x + read(copied);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputs[0] = run(tid.x);
    // CHECK: 120
    outputs[1] = run(tid.x + 1);
    // CHECK: 111
    outputs[2] = run(tid.x + 2);
    // CHECK: 124
    outputs[3] = run(tid.x + 3);
    // CHECK: 115
}
