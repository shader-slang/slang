// A short-circuit chain nested in its right operand (`a || (b || c)`) must
// also reconstruct: the inner diamond is rewritten first, and the merged
// blocks let the outer diamond match on the next fixpoint round.
// (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3 v = itemMin[tid.x];
    if (v.x > 0.0 || (v.y > 0.0 || v.z > 0.0))
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK-NOT: bool _S
    //CHECK: if({{.*}}||
}
