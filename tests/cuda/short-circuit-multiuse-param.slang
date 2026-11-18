// A merge value used more than once keeps its diamond: the rewrite
// replaces the merge block's parameter with the reconstructed operation,
// which is only valid while the parameter has exactly one use. The common
// `bool passed = a && b; if (passed) ... if (passed) ...` shape therefore
// is therefore never reconstructed. (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    bool passed = tid.x > 1 && itemMin[tid.x].x > 0.0;
    if (passed)
    {
        output[tid.x] = 1;
    }
    if (passed)
    {
        output[tid.x] += 2;
    }
    //CHECK-NOT: &&
}
