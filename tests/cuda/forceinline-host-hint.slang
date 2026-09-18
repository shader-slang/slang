// A user `[ForceInline]` on a `[CudaHost]` function must keep its inlining hint on CUDA: the
// function is deferred just like a device one (shader-slang/slang#12623) and survives as a real
// `__host__` function, so it must be emitted as `__forceinline__ __host__` rather than a bare
// `__host__` that silently drops the hint. Emitted as CUDA source (the `[TorchEntryPoint]` keeps the
// host helper reachable); this is the CUDA-source path, not the NVRTC/ptx path (which rejects
// `__host__`).

//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none

// CHECK-DAG: __forceinline__ __host__ {{.*}}hostHelper
[CudaHost]
[ForceInline]
float hostHelper(float x)
{
    return x * 2.0f + 1.0f;
}

[TorchEntryPoint]
float entryPoint(float x)
{
    return hostHelper(x);
}
