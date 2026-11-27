// SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

// Generic readers instantiated after the field has acquired a refined payload
// type: the later instantiation must reuse the field's refined info instead of
// re-widening it from globally available witness tables.

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

struct Holder
{
    Conditional<IValue, true> value;
}

interface IReader
{
    uint eval<T>(Holder holder, T input);
}

struct ReaderA : IReader
{
    uint eval<T>(Holder holder, T input)
    {
        if (let value = holder.value.get())
            return value.read();
        return 123;
    }
}

struct ReaderB : IReader
{
    uint eval<T>(Holder holder, T input)
    {
        if (let value = holder.value.get())
            return value.read() + 1;
        return 123;
    }
}

IValue choose(uint id)
{
    if ((id & 1) == 0)
        return A(id);
    return B(uint4(id, 2, 3, 4));
}

IReader chooseReader(uint id)
{
    if ((id & 1) == 0)
        return ReaderA();
    return ReaderB();
}

uint run(uint valueID, uint readerID)
{
    Holder holder;
    holder.value = Conditional<IValue, true>(choose(valueID));
    return chooseReader(readerID).eval(holder, valueID);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputs[0] = run(tid.x, tid.x);
    // CHECK: 20
    outputs[1] = run(tid.x + 1, tid.x + 1);
    // CHECK: 11
    outputs[2] = run(tid.x, tid.x + 1);
    // CHECK: 21
    outputs[3] = run(tid.x + 1, tid.x);
    // CHECK: 10
}
