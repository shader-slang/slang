// The discriminating case for the `[__readNone]` exemption: a read-none
// call whose own user fails a fold veto (the multi-component `.xy`
// swizzle of a vector-typed value) and whose operands are not memory
// reads. With the exemption the call is never fold-gated, so the chain
// reconstructs; deleting `isReadNoneCall` would make the call the fold
// path's root and the swizzle veto would keep the diamond. Unconditional
// evaluation of the read-none call is unobservable by definition.
// (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

// The intrinsic definition references `$0` twice, so the call cannot fold
// into its user's text.
__target_intrinsic(cuda, "(($0) + ($0))")
[__readNone]
[ForceInline]
float3 vtwice(float3 v);

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x > 1 && any(vtwice(float3(1.0, 2.0, 3.0)).xy > 1.5))
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK: if({{.*}}&&
}
