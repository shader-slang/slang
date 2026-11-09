//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -emit-spirv-directly

// CHECK: OpEntryPoint GLCompute

interface ISize
{
    static const int value;
}

struct VariadicSize<let Head : int, let each Tail : int> : ISize
{
    static const int value = Head + countof(Tail);
}

int getSize<T : ISize>()
{
    return T.value;
}

RWStructuredBuffer<int> result;

[numthreads(1, 1, 1)]
void computeMain()
{
    result[0] = getSize<VariadicSize<10, 20, 30>>();
}
