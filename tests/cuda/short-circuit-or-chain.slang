// CUDA short-circuit `||` chains must emit as source-level `||` expressions,
// not as one `bool` temporary per link of the chain. A materialized boolean
// temporary forces nvcc into `selp`/select chains instead of the
// compare-and-branch it generates for hand-written `||` — the subject of
// shader-slang/slang#14494.

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;
StructuredBuffer<float3> itemMax;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint it = tid.x;
    float3 qmax = itemMax[it];
    float3 qmin = itemMin[it];
    if (itemMin[it].x > qmax.x ||
        itemMin[it].y > qmax.y ||
        itemMin[it].z > qmax.z ||
        itemMax[it].x < qmin.x ||
        itemMax[it].y < qmin.y ||
        itemMax[it].z < qmin.z)
    {
        output[tid.x] = 0;
        return;
    }
    output[tid.x] = 1;
    //CHECK: ||
    //CHECK-NOT: bool _S
}
