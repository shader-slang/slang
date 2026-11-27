// Storing an interface value in a `Conditional` struct field must behave like
// a plain interface field: retain the selected implementation and dispatch on it.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputs
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

IValue choose(uint4 data)
{
    if ((data.x & 1) == 0)
        return A(data);
    return B(data);
}

struct Holder
{
    Conditional<IValue, true> wrapped;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint4 data = uint4(tid.x, 2, 3, 4);
    Holder holder;
    holder.wrapped = Conditional<IValue, true>(choose(data));
    if (let value = holder.wrapped.get())
    {
        outputs[tid.x] = value.read();
    }
    else
    {
        outputs[tid.x] = 123;
    }
}
// CHECK: 9
// CHECK: 4
// CHECK: 11
// CHECK: 6
