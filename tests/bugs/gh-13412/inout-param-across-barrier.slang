// A store to an `inout` parameter is forwarded to a later read of it across a barrier (see
// shader-slang/slang#13412). `inout` has copy-in/copy-out semantics, so the callee's copy of `x`
// cannot be changed by another invocation during the barrier.
//
// Here the argument `s[t % 32]` is a groupshared element that threads `t` and `t + 32` share, and
// Metal passes its address directly instead of a copy. The returned `[[V]]` is this thread's own
// value, which is the copy semantics' answer. The shared element itself may hold the other
// thread's value after the barrier, because passing the address makes the callee's store visible
// before the call returns.

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
