// With debug info enabled the arm keeps its shape: the debug markers in
// the value arm are left in place by the pass (hoisting them would detach
// them from the code they describe), so the diamond survives.
// (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda -g

RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x < 3 && itemMin[tid.x].x > 0.0)
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK: if(
    //CHECK-NOT: &&
}
