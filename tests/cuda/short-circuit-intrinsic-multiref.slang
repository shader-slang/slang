// A short-circuit operand whose value feeds a target intrinsic that mentions
// its argument more than once does not fold into the emitted call text, so
// the pass must keep its diamond: hoisting the operand would turn its
// conditional load into an unconditional one. (Companion to
// shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

// The intrinsic definition references `$0` twice, so a `slang_ldg` result
// used as its argument is not folded into it by the emitter.
__target_intrinsic(cuda, "(($0).x + ($0).x)")
[__readNone]
[ForceInline]
float twiceX(float3 v);

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x > 1 && twiceX(itemMin[tid.x]) > 0.5)
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK-NOT: &&
}
