// A `[ForceInline]` `[Differentiable]` function and its synthesized derivative must both be
// force-inlined on non-CUDA targets: the derivative inherits the force-inline hint, so no separate
// `square`-derived function survives to be called (shader-slang/slang#12623).

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

[ForceInline]
[Differentiable]
float square(float x)
{
    return x * x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var dp = diffPair(3.0f, 1.0f);
    var result = fwd_diff(square)(dp);
    outputBuffer[tid.x] = result.d;
}

// The force-inlined function and its derivative are inlined away: no `OpFunctionCall` to a separate
// `square`-derived function survives.
// CHECK: OpEntryPoint
// CHECK-NOT: OpFunctionCall
