// A `&&` nested in the right operand of an `||` (`a || (b && c)`) must also
// reconstruct into one expression: the inner diamond is rewritten first and
// the merged blocks let the outer diamond match on the next fixpoint round.
// (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x > 2 || (tid.x < 5 && itemMin[tid.x].x > 0.0))
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK-NOT: bool _S
    //CHECK: if({{.*}}||{{.*}}&&
}
