//TEST:SIMPLE(filecheck=CHECK): -std legacy -target hlsl -stage compute -entry main

interface IFoo
{
    int get();
}
struct A : IFoo
{
    int get()
    {
        return 7;
    }
}
struct B : IFoo
{
    int get()
    {
        return 109;
    }
}

int select<T : IFoo>(T value)
{
    return 1;
}
int select(dyn IFoo value)
{
    return 2;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    dyn IFoo value = A();
    if (threadId.x != 0)
        value = B();
    outputBuffer[threadId.x] = select(value);
}

// Call checking opens `value` before overload resolution. The constrained generic overload
// accepts the hidden payload type directly. The `dyn IFoo` overload would repack that
// payload into a box, so its conversion cost is higher and the generic overload wins.
// CHECK: return int(1);
// CHECK-NOT: return int(2);
