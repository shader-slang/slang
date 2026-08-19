// Verify the `-cuda-noinline-threshold` heuristic: with a positive threshold, an ordinary
// `__device__` function whose IR body exceeds the threshold is emitted with `__noinline__`, while a
// small one, a `[CudaKernel]`, and the entry point are not. With no flag the emitted CUDA is
// unchanged (the heuristic is off by default). This checks the emitted source directly, so it needs
// no GPU.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none -cuda-noinline-threshold 10
//TEST:SIMPLE(filecheck=CHECK_OFF): -target cuda -line-directive-mode none

// The heuristic is gated to the CUDA target family; other backends consume `IRNoInlineDecoration`
// too, so the same flag must not perturb their output. HLSL would spell the decoration `[noinline]`.
//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -entry computeMain -stage compute -line-directive-mode none -cuda-noinline-threshold 10

RWStructuredBuffer<int> outputBuffer;

// A large ordinary device function: its long straight-line body pushes its IR instruction count
// well past the threshold, so it must gain the specifier.
// CHECK-DAG: {{^}}__device__ __noinline__ int bigHelper
int bigHelper(int x)
{
    int a = x;
    a = a * 2 + 1;
    a = a * 3 + 2;
    a = a * 5 + 3;
    a = a * 7 + 4;
    a = a * 11 + 5;
    a = a * 13 + 6;
    a = a * 17 + 7;
    a = a * 19 + 8;
    a = a * 23 + 9;
    a = a * 29 + 10;
    a = a * 31 + 11;
    a = a * 37 + 12;
    return a;
}

// A small ordinary device function: its IR body stays under the threshold, so the specifier must
// not appear — the emitted specifier is driven by size, not applied to every device function. The
// `int` immediately after `__device__` confirms nothing was inserted before the return type.
// CHECK-DAG: {{^}}__device__ int smallHelper
int smallHelper(int x)
{
    return x + 1;
}

// A `[CudaKernel]` takes the `__global__` branch, a separate classification the heuristic excludes
// even though its body is large, so it must not gain the specifier.
// CHECK-DAG: {{^}}__global__ void kernelFunc
[CudaKernel]
void kernelFunc(int x)
{
    int a = x;
    a = a * 2 + 1;
    a = a * 3 + 2;
    a = a * 5 + 3;
    a = a * 7 + 4;
    a = a * 11 + 5;
    a = a * 13 + 6;
    a = a * 17 + 7;
    outputBuffer[3] = a;
}

// The entry point is likewise excluded: its specifier sequence stays exactly `extern "C" __global__`.
// CHECK-DAG: {{^}}extern "C" __global__ void computeMain
[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = bigHelper(42) + smallHelper(7);
}

// CHECK_OFF-NOT: __noinline__

// CHECK_HLSL-NOT: [noinline]
