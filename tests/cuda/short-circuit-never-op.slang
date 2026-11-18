// A pure chain instruction the C-family emitters classify
// FoldPolicy::Never (here a MakeStruct from an inlined constructor call)
// keeps its diamond: the struct construction cannot print as a
// sub-expression, so the pass must not hoist it out of the conditional
// arm. (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

struct S
{
    float x;
    float y;
};

[ForceInline]
S makeS(float3 v)
{
    return S(v.x, v.y);
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x > 1 && makeS(itemMin[tid.x]).x > 0.0)
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK-NOT: &&
}
