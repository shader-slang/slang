// A store through a pointer must not be forwarded past barriers to a load of the same element
// when another thread writes that element between the barriers (shader-slang/slang#13412).
// Thread t-1 writes q[t] through a [noinline] helper, so q[t] must be reloaded.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute

struct P
{
    uint* p;
}
ConstantBuffer<P> cb;
RWStructuredBuffer<uint> outb;

[noinline]
void writeNeighbour(uint t)
{
    cb.p[(t + 1) % 64] = 100 + t;
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    uint* q = cb.p;
    q[t] = t;
    AllMemoryBarrierWithGroupSync();
    writeNeighbour(t);
    AllMemoryBarrierWithGroupSync();
    outb[t] = q[t];
}

// CUDA-LABEL: void computeMain(
// CUDA: writeNeighbour_0(
// CUDA: __syncthreads();
// CUDA-NOT: = t_{{[0-9]+}};
// CUDA: outb_0)[t_{{[0-9]+}}]) = *
// METAL-LABEL: void computeMain(
// METAL: writeNeighbour_0(
// METAL: threadgroup_barrier(
// METAL-NOT: = t_{{[0-9]+}};
// METAL: = *
// SPV: %computeMain = OpFunction
// SPV: OpFunctionCall %void %writeNeighbour
// SPV: OpControlBarrier
// SPV: OpLoad %uint
// SPV: OpFunctionEnd
