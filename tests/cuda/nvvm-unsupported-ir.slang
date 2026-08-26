// Slice 6 accepts only an empty compute body. This barrier remains a call in canonical linked IR,
// so the direct emitter must reject it before creating a builder module or libNVVM program.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o nvvm-unsupported-ir.ptx

[numthreads(1, 1, 1)]
void computeMain()
{
    GroupMemoryBarrierWithGroupSync();
}

//CHECK: E52017
