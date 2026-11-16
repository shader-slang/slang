// A call to a `[__readNone]` target intrinsic that mentions its argument
// once can wrap a memory read in a reconstructed `||` operand: the read
// itself is fold-gated (the single `$0` reference passes the multi-reference
// veto), and the read-none callee provably reads nothing on its own. The
// test exercises that exemption branch, though it cannot fail on the
// exemption's removal alone — a single-`$0` read-none intrinsic passes the
// fold gate either way. (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

// The intrinsic definition references `$0` once.
__target_intrinsic(cuda, "fabsf(($0))")
[__readNone]
[ForceInline]
float absX(float v);

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x > 1 || absX(itemMin[tid.x].x) > 0.5)
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
