//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -line-directive-mode none -DEXCLUDE_CUDA_KERNEL
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -emit-spirv-directly -line-directive-mode none -DEXCLUDE_CUDA_KERNEL
//TEST:SIMPLE(filecheck=METAL): -target metal -line-directive-mode none -DEXCLUDE_CUDA_KERNEL
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -line-directive-mode none -DEXCLUDE_CUDA_KERNEL

// Test that forward-mode autodiff correctly handles a no_diff scalar parameter
// used in a vector construction (via lerp(float3, float3, float)) inside a
// generic function with IDifferentiable interface constraints.
//
// Previously this crashed with "unexpected IR opcode during code emit" because
// the forward-mode pass emitted void_constant (RTTIType) instead of a zero
// literal for the differential of the no_diff parameter.

interface IA : IDifferentiable {
    [Differentiable]
    float3 f(float3 x);
}

struct SA : IA {
    float s;
    [Differentiable]
    float3 f(float3 x) { return s * x; }
}

[Differentiable]
float3 my_func<A : IA>(A a, float3 x, float3 y, no_diff float alpha) {
    return a.f(lerp(x, y, alpha));
}

// CUDA-DAG: __global__ void __kernel__kern
// HLSL: computeMain
// SPV: OpEntryPoint
// METAL: computeMain
// WGSL: computeMain

RWStructuredBuffer<float> buffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    SA a;
    a.s = 2.0;
    float v = buffer[tid];
    float3 r = my_func(a, float3(v,v,v), float3(v+1,v+1,v+1), 0.5);
    buffer[tid] = r.x;
}

#ifndef EXCLUDE_CUDA_KERNEL
[CUDAKernel]
[Differentiable]
[AutoPyBindCUDA]
void kern(DiffTensorView io) {
    uint tid = cudaBlockIdx().x * cudaBlockDim().x + cudaThreadIdx().x;
    SA a; a.s = 2.0;
    float v = io.loadOnce(uint2(tid, 0));
    float3 r = my_func(a, float3(v,v,v), float3(v+1,v+1,v+1), 0.5);
    io.storeOnce(uint2(tid, 0), r.x);
}
#endif
