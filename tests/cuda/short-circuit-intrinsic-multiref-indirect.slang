// An interior chain instruction that the emitter will not fold into its use
// site must keep the whole memory-read chain's diamond: the emitter prints
// such an instruction as a statement before the `if`, and the load text
// folded into that statement would then be evaluated unconditionally.
// (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

// The intrinsic definition references `$0` twice, so the `* 2.0` between
// the load and the call is not folded into the emitted call text; the
// chain through that multiply must keep its diamond.
__target_intrinsic(cuda, "(($0).x + ($0).x)")
[__readNone]
[ForceInline]
float twiceX(float3 v);

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint count = output[0];
    if (tid.x < count && twiceX(itemMin[tid.x] * 2.0) > 0.5)
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
