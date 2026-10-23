// A store to groupshared memory is not forwarded to a later load across barriers when another
// thread writes that memory in between (shader-slang/slang#13412). On Metal,
// `introduceExplicitGlobalContext` makes `s` a local variable of the entry point in the
// `GroupShared` address space, so the root is a local `var` that is still shared with other
// threads.

//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none

groupshared uint s[64];
RWStructuredBuffer<uint> outb;

[noinline]
void writeNeighbour(uint t)
{
    s[(t + 1) % 64] = 100 + t;
}

// METAL-LABEL: void computeMain(
// METAL: writeNeighbour_0(
// METAL-NEXT: threadgroup_barrier(
// METAL-NEXT: outb{{.*}} = s_{{[0-9]+}}[t_{{[0-9]+}}];

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    s[t] = t;
    GroupMemoryBarrierWithGroupSync();
    writeNeighbour(t);
    GroupMemoryBarrierWithGroupSync();
    outb[t] = s[t];
}
