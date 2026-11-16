// A side-effectful right operand keeps its diamond on the `||` path too:
// all the other negative tests use `&&`, but `||` swaps which arm carries
// the constant, so the arm checks run against a different shape and need
// their own pin. (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

[ForceInline]
bool record(float v)
{
    output[0] = output[0] + 1;
    return v > 0.5;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x > 1 || record(itemMin[tid.x].x))
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK-NOT: ||
}
