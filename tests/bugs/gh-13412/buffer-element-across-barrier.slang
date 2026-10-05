// A read of a structured-buffer element before the first barrier must stay there when another
// thread writes that element between two barriers (shader-slang/slang#13412). Thread t-1
// writes buf[t] through a [noinline] helper, so `a` must be the value read before barrier 1.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none

RWStructuredBuffer<uint> buf;
RWStructuredBuffer<uint> outBuf;

[noinline]
void writeNeighbour(uint t)
{
    buf[(t + 1) % 64] = 100 + t;
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    uint a = buf[t];
    AllMemoryBarrierWithGroupSync();
    writeNeighbour(t);
    AllMemoryBarrierWithGroupSync();
    outBuf[t] = a;
}

// HLSL-LABEL: void computeMain(
// HLSL: uint a_0 = buf_0[
// HLSL-NEXT: AllMemoryBarrierWithGroupSync();
// CUDA-LABEL: void computeMain(
// CUDA: uint a_0 = *
// CUDA-NEXT: __syncthreads();
// METAL-LABEL: void computeMain(
// METAL: uint a_0 = *
// METAL-NEXT: threadgroup_barrier(
// GLSL-LABEL: void main(
// GLSL: uint a_0 = buf_0._data[
// GLSL-NEXT: controlBarrier(
