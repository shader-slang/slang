// Invalid [CudaLaunchBounds] / [CudaMaxRegisters] values, and both on one kernel (CUDA rejects
// __launch_bounds__ together with __maxnreg__).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda

RWStructuredBuffer<uint> outputBuffer;

[CudaLaunchBounds(0)]
/*CHECK:
 ^^^^^^^^^^^^^^^^ invalid CUDA kernel attribute value
 ^^^^^^^^^^^^^^^^ value '0' is out of range for attribute 'CudaLaunchBounds'
*/
[numthreads(64, 1, 1)]
[shader("compute")]
void zeroThreads(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = tid.x;
}

[CudaLaunchBounds(64, -1)]
/*CHECK:
 ^^^^^^^^^^^^^^^^ invalid CUDA kernel attribute value
 ^^^^^^^^^^^^^^^^ value '-1' is out of range for attribute 'CudaLaunchBounds'
*/
[numthreads(64, 1, 1)]
[shader("compute")]
void negativeBlocks(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = tid.x;
}

[CudaMaxRegisters(0)]
/*CHECK:
 ^^^^^^^^^^^^^^^^ invalid CUDA kernel attribute value
 ^^^^^^^^^^^^^^^^ value '0' is out of range for attribute 'CudaMaxRegisters'
*/
[numthreads(64, 1, 1)]
[shader("compute")]
void zeroRegisters(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = tid.x;
}

[CudaLaunchBounds(64)]
[CudaMaxRegisters(32)]
/*CHECK:
 ^^^^^^^^^^^^^^^^ duplicate modifier
 ^^^^^^^^^^^^^^^^ modifier 'CudaMaxRegisters' is redundant or conflicting with existing modifier 'CudaLaunchBounds'
*/
[numthreads(64, 1, 1)]
[shader("compute")]
void both(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = tid.x;
}
