// An ordinary compute entry is a CUDA kernel through its canonical entry-point decoration. It
// does not need the source-only `[CUDAKernel]` attribute before using supported executable IR.

//TEST:SIMPLE: -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o nvvm-ordinary-compute-entry.ptx.actual

[numthreads(1, 1, 1)]
void computeMain()
{
    GroupMemoryBarrierWithGroupSync();
}
