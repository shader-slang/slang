// SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

// A Conditional<IValue, true> stored in a struct field, passed through
// function arguments and return values, and a disabled Conditional<IValue,
// false> field that must keep the zero-size (empty struct) semantics.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputs
RWStructuredBuffer<uint> outputs;

interface IValue
{
    uint read();
}

struct A : IValue
{
    uint4 data;
    uint read() { return data.x + data.y + data.z + data.w; }
}

struct B : IValue
{
    uint4 data;
    uint read() { return data.x ^ data.y ^ data.z ^ data.w; }
}

IValue choose(uint id)
{
    uint4 data = uint4(id, 2, 3, 4);
    if ((id & 1) == 0)
        return A(data);
    return B(data);
}

struct Holder<let enabled : bool>
{
    Conditional<IValue, enabled> value;
    [mutating]
    void set(IValue input) { value.set(input); }
    uint read()
    {
        if (let input = value.get())
            return input.read();
        return 123;
    }
}

uint consume(Conditional<IValue, true> value)
{
    if (let input = value.get())
        return input.read();
    return 123;
}

Conditional<IValue, true> make(uint id)
{
    return Conditional<IValue, true>(choose(id));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Holder<true> holder;
    holder.set(choose(tid.x));
    outputs[0] = holder.read();
    // CHECK: 9
    holder.set(choose(tid.x + 1));
    outputs[1] = holder.read();
    // CHECK: 4
    outputs[2] = consume(make(tid.x));
    // CHECK: 9
    outputs[3] = consume(make(tid.x + 1));
    // CHECK: 4
    Holder<false> disabled;
    disabled.set(choose(tid.x));
    outputs[4] = disabled.read();
    // CHECK: 123
    Conditional<IValue, true> local = Conditional<IValue, true>(choose(tid.x));
    if (let value = local.get())
    {
        outputs[5] = value.read();
    }
    // CHECK: 9
    outputs[6] = sizeof(Holder<false>);
    // CHECK: 0
}
