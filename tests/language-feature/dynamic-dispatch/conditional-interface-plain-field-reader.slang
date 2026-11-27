// SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

// Plain-interface-field variant of the late-reader test: the field is a bare
// IValue (no Conditional), and a late-instantiated generic reader must reuse
// the refined field type instead of re-widening from globally available
// witness tables.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputs
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
    IValue value;
}

interface IReader
{
    uint eval<T>(Holder holder, T input);
}

struct ReaderA : IReader
{
    uint eval<T>(Holder holder, T input)
    {
        return holder.value.read();
    }
}

struct ReaderB : IReader
{
    uint eval<T>(Holder holder, T input)
    {
        return holder.value.read() + 1;
    }
}

IReader chooseReader(uint id)
{
    if ((id & 1) == 0)
        return ReaderA();
    return ReaderB();
}

// Keep B in the module's global witness-table set for IValue, so the field's
// refined set ({A}) is strictly narrower than the global one.
IValue makeB(uint id)
{
    return B(uint4(id, 2, 3, 4));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Holder holder;
    holder.value = A(tid.x);
    uint result = 0;
    [unroll]
    for (uint i = 0; i < 2; i++)
    {
        result += chooseReader(i).eval(holder, i);
    }
    outputs[0] = result;
    // CHECK: 41
    outputs[1] = makeB(tid.x).read();
    // CHECK: 9
}
