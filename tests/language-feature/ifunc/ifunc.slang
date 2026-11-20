//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

struct Functor : IFunc<int, int, bool>
{
    int operator()(int p, bool t)
    {
        return p + 1;
    }
}

struct MutatingFunctor : IMutatingFunc<int, int, bool>
{
    int data = 0;
    [mutating]
    int operator()(int p, bool t)
    {
        data++;
        return p + 1;
    }
}

int apply(IMutatingFunc<int, int, bool> f, int p)
{
    return f(p, true);
}

//TEST_INPUT:ubuffer(data=[0 3 2 2], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    // CHECK: 2
    outputBuffer[0] = apply(MutatingFunctor(), 1);
    // CHECK: 3
    outputBuffer[1] = apply(Functor(), 2);
}
