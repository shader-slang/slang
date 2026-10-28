// shader-slang/slang#13530: finding the trailing unsized array of a global uniform parameter
// stops at the type nesting limit for a generic struct whose last field instantiates it with
// new arguments, instead of walking forever.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry main

struct LoopTail<each T>
{
    float4 x[];
//CHECK:   ^ maximum type nesting level exceeded
    LoopTail<T, int> next;
}
uniform LoopTail<int> loopTail;

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = 1;
}
