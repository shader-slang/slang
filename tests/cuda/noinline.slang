//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none
//TEST:SIMPLE(filecheck=CHECK_PTX): -target ptx -O0

// Test that [noinline] emits CUDA's `__noinline__` declaration specifier, that it is
// applied only to ordinary `__device__` functions, and that NVRTC accepts the result.

RWStructuredBuffer<int> outputBuffer;

// CHECK-DAG: __device__ __noinline__ int helperFunc
[noinline]
int helperFunc(int x)
{
    return x + 1;
}

// A function without the attribute must not pick up the qualifier, so the emitted
// specifier is driven by the decoration rather than applied to every device function.
// CHECK-DAG: {{^}}__device__ int plainHelper
int plainHelper(int x)
{
    return x * 2;
}

// A `[CudaKernel]` function takes the `__global__` branch, which is a separate
// classification from the ordinary device one, so it must not pick up the specifier either.
// CHECK-DAG: {{^}}__global__ void kernelFunc
[CudaKernel]
[noinline]
void kernelFunc(int x)
{
    outputBuffer[1] = x;
}

// The attribute is a no-op on an entry point: the specifier sequence stays exactly
// `extern "C" __global__ void`, with nothing inserted before the return type.
// CHECK-DAG: {{^}}extern "C" __global__ void computeMain
[noinline]
[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = helperFunc(42) + plainHelper(7);
}

// The ptx arm confirms NVRTC accepts the emitted specifier and compiles to valid PTX.
// CHECK_PTX: {{^}}.visible .entry computeMain
