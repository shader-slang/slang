// Finding the trailing unsized array of an explicit constant buffer's element type stops at the
// type nesting limit for a generic struct whose last field instantiates it with new arguments,
// instead of walking forever.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry main

struct LoopTail<each T>
{
    float4 x[];
//CHECK:   ^ maximum type nesting level exceeded
    LoopTail<T, int> next;
}
ConstantBuffer<LoopTail<int>> loopTail;

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = 1;
}
