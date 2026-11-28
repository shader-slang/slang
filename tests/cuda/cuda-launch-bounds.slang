// [CudaLaunchBounds] and [CudaMaxRegisters] emit __launch_bounds__ and __maxnreg__ on the
// CUDA kernel. Other targets ignore them.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry solve -stage compute
//TEST:SIMPLE(filecheck=CUDA1): -target cuda -entry solveMaxThreadsOnly -stage compute
//TEST:SIMPLE(filecheck=CUDAREG): -target cuda -entry solveMaxRegisters -stage compute
// Compile the CUDA through NVRTC (ignored where it is absent). __maxnreg__ needs NVRTC 12.4 or
// later, and slang-test cannot require a version, so it only gets a source check.
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry solve -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry solve -stage compute
//TEST:SIMPLE(filecheck=SPV): -target spirv -entry solve -stage compute

// CUDA: extern "C" __global__ __launch_bounds__(256, 8) void solve(
// CUDA1: extern "C" __global__ __launch_bounds__(128) void solveMaxThreadsOnly(
// CUDAREG: extern "C" __global__ __maxnreg__(64) void solveMaxRegisters(
// PTX: .maxntid 256, 1, 1
// PTX: .minnctapersm 8
// HLSL-NOT: launch_bounds
// HLSL: [numthreads(256, 1, 1)]
// SPV: OpEntryPoint

[CudaLaunchBounds(256, 8)]
[numthreads(256, 1, 1)]
[shader("compute")]
void solve(uint3 tid: SV_DispatchThreadID, uniform float* x, uniform float* b, uniform uint n)
{
    if (tid.x < n)
        x[tid.x] = x[tid.x] * 0.5f + b[tid.x];
}

[CudaLaunchBounds(128)]
[numthreads(128, 1, 1)]
[shader("compute")]
void solveMaxThreadsOnly(uint3 tid: SV_DispatchThreadID, uniform float* x, uniform uint n)
{
    if (tid.x < n)
        x[tid.x] = x[tid.x] * 0.5f;
}

[CudaMaxRegisters(64)]
[numthreads(128, 1, 1)]
[shader("compute")]
void solveMaxRegisters(uint3 tid: SV_DispatchThreadID, uniform float* x, uniform uint n)
{
    if (tid.x < n)
        x[tid.x] = x[tid.x] * 0.5f;
}
