// A late-instantiated generic reader reaches the Conditional field through a
// pointer (a local copy), with an unrolled loop in the entry point forcing a
// second specialization iteration — so the reader runs in a fresh typeflow
// context where the field's refined type must be reused directly (the
// analyzeFieldAddress fallback). The stored payload set ({A}) is narrower
// than the globally available one ({A, B}).

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
        // Read through a local so the access goes through a field address
        // rather than a field extract from the parameter.
        Holder local = holder;
        if (let value = local.value.get())
            return value.read();
        return 123;
    }
}

struct ReaderB : IReader
{
    uint eval<T>(Holder holder, T input)
    {
        Holder local = holder;
        if (let value = local.value.get())
            return value.read() + 1;
        return 123;
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
    holder.value = Conditional<IValue, true>(A(tid.x));
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
