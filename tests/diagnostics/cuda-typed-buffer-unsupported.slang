// A typed `Buffer<T>` access on CUDA/PTX must produce a capability diagnostic
// (E36107), not compile to an empty kernel. `RWStructuredBuffer` remains
// supported, so its use here must not be what triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target ptx -entry computeMain -stage compute

Buffer<float4> inBuf;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[tid.x] = inBuf[tid.x];
}

//CHECK: E36107
