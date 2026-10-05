// A read of a structured-buffer element must stay before a barrier when another thread writes
// that element after the barrier (shader-slang/slang#13412). There is no user-written call:
// the barrier is the call that may write `b[63 - t]`.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none

RWStructuredBuffer<float> b;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    float v = b[63 - t];
    AllMemoryBarrierWithGroupSync();
    b[t] = v;
}

// HLSL: float v_0 = b_0[63U - t_0];
// HLSL-NEXT: AllMemoryBarrierWithGroupSync();
// HLSL-NEXT: b_0[t_0] = v_0;
// GLSL: float v_0 = b_0._data[uint(63U - t_0)];
// GLSL-NEXT: controlBarrier(
// GLSL-NEXT: b_0._data[uint(t_0)] = v_0;
// WGSL: var v_0 : f32 = b_0[u32(63) - t_0];
// WGSL-NEXT: workgroupBarrier();
// WGSL-NEXT: b_0[t_0] = v_0;
// METAL: float v_0 = *(b_0+(63U - t_0));
// METAL-NEXT: threadgroup_barrier(
// METAL-NEXT: *(b_0+t_0) = v_0;
// CUDA: float v_0 = *
// CUDA-NEXT: __syncthreads();
// CUDA-NEXT: = v_0;
