// A plain `a && b && c` chain (no buffer loads, no nesting) must emit as
// one source-level short-circuit expression — the shape every other
// positive test reaches only with memory reads in the mix.
// (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint x = tid.x;
    //CHECK-NOT: bool _S
    if (x > 0 && x < 3 && (x % 2) == 1)
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK: if({{.*}}&&
}
