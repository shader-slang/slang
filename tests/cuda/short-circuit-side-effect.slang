// A short-circuit operand that can have side effects must keep its diamond:
// the right operand is only allowed to fold into an emitted `&&` when it is
// pure, so reordering it into an unconditionally evaluated expression would
// change program behavior. (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

int sideEffect()
{
    output[1] = output[1] + 1;
    return 1;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x > 1 && sideEffect() > 0)
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    // The diamond survives: the right operand has a side effect, so it must
    // not be hoisted into an unconditional `&&` operand.
    //CHECK-NOT: &&
}
