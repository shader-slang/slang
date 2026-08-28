// The barrier itself is supported. This ordinary `[numthreads]` compute entry deliberately lacks
// the raw `[CUDAKernel]` ABI required by the current direct emitter, so it must be rejected at that
// exact boundary before creating a builder module or libNVVM program.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o nvvm-unsupported-ir.ptx

[numthreads(1, 1, 1)]
void computeMain()
{
    GroupMemoryBarrierWithGroupSync();
}

//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'CUDA kernel decoration'
