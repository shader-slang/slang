// Structural joins of Conditional payloads with different sizes and witness
// sets: function returns merged at a call site, inout overwrites, arrays,
// nested Conditional, and a Conditional inside an Optional.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputs
RWStructuredBuffer<uint> outputs;

interface IValue
{
    uint read();
}

struct Small : IValue
{
    uint value;
    uint read() { return value + 10; }
}

struct Large : IValue
{
    uint4 value;
    uint read() { return value.x + value.y + value.z + value.w; }
}

Conditional<IValue, true> makeSmall(uint value)
{
    return Conditional<IValue, true>(Small(value));
}

Conditional<IValue, true> makeLarge(uint value)
{
    return Conditional<IValue, true>(Large(uint4(value, 2, 3, 4)));
}

Conditional<IValue, true> selectValue(bool large, uint value)
{
    if (large)
        return makeLarge(value);
    return makeSmall(value);
}

uint read(Conditional<IValue, true> value)
{
    if (let payload = value.get())
        return payload.read();
    return 123;
}

void overwrite(inout Conditional<IValue, true> value, bool large, uint input)
{
    if (large)
        value = makeLarge(input);
    else
        value = makeSmall(input);
}

struct Holder
{
    Conditional<IValue, true> value;
}

uint nested(Conditional<Conditional<IValue, true>, true> value)
{
    if (let inner = value.get())
        return read(inner);
    return 123;
}

extern static const bool ENABLED = true;

struct ConditionalHolder<let enabled : bool>
{
    Conditional<IValue, enabled> value;
    uint read()
    {
        if (let payload = value.get())
            return payload.read();
        return 123;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputs[0] = read(makeSmall(tid.x));
    // CHECK: 10
    outputs[1] = read(makeLarge(tid.x));
    // CHECK: 9
    outputs[2] = read(selectValue((tid.x & 1) != 0, tid.x));
    // CHECK: 10
    outputs[3] = read(selectValue((tid.x & 1) == 0, tid.x));
    // CHECK: 9

    Holder holder;
    holder.value = makeSmall(tid.x);
    outputs[4] = read(holder.value);
    // CHECK: 10
    holder.value = makeLarge(tid.x);
    outputs[5] = read(holder.value);
    // CHECK: 9

    Conditional<IValue, true> value = makeSmall(tid.x);
    overwrite(value, true, tid.x);
    outputs[6] = read(value);
    // CHECK: 9
    overwrite(value, false, tid.x);
    outputs[7] = read(value);
    // CHECK: 10

    Conditional<IValue, true> values[2] = {makeSmall(tid.x), makeLarge(tid.x)};
    outputs[8] = read(values[tid.x & 1]);
    // CHECK: 10
    outputs[9] = read(values[(tid.x + 1) & 1]);
    // CHECK: 9

    outputs[10] = nested(Conditional<Conditional<IValue, true>, true>(makeSmall(tid.x)));
    // CHECK: 10
    outputs[11] = nested(Conditional<Conditional<IValue, true>, true>(makeLarge(tid.x)));
    // CHECK: 9

    ConditionalHolder<ENABLED> enabled;
    enabled.value = Conditional<IValue, ENABLED>(Small(tid.x));
    outputs[12] = enabled.read();
    // CHECK: 10
    ConditionalHolder<!ENABLED> disabled;
    disabled.value = Conditional<IValue, !ENABLED>(Large(uint4(tid.x, 2, 3, 4)));
    outputs[13] = disabled.read();
    // CHECK: 123
    outputs[14] = sizeof(ConditionalHolder<!ENABLED>);
    // CHECK: 0

    Optional<Conditional<IValue, true>> optional = makeSmall(tid.x);
    if (let present = optional)
    {
        outputs[15] = read(present);
    }
    // CHECK: 10
    optional = makeLarge(tid.x);
    if (let present = optional)
    {
        outputs[16] = read(present);
    }
    // CHECK: 9
}
