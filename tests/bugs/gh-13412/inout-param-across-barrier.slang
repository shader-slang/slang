// An `inout` parameter is still forwarded across a barrier on Metal after the fix for
// shader-slang/slang#13412: `inout` has copy-in/copy-out semantics, so the callee may keep its own
// copy of `x` across the barrier. This holds even when the argument is a groupshared element.

//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none

groupshared float s[64];
RWStructuredBuffer<float> buf;

[noinline]
float f(inout float x, uint t)
{
    x = float(t);
    GroupMemoryBarrierWithGroupSync();
    return x;
}

// METAL-LABEL: float f_0(
// METAL: float [[V:_S[0-9]+]] = float(t_0);
// METAL-NEXT: *x_0 = [[V]];
// METAL-NEXT: threadgroup_barrier(
// METAL-NEXT: return [[V]];

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    buf[t] = f(s[t % 32], t);
}
