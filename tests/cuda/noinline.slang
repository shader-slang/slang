//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none
//TEST:SIMPLE(filecheck=CHECK_PTX): -target ptx
//TEST:SIMPLE(filecheck=CHECK_NVVM): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o -

// Test that [noinline] emits CUDA's `__noinline__` declaration specifier and that it is
// applied to ordinary `__device__` functions only. `[CudaHost]` is excluded from this file
// deliberately: NVRTC rejects `__host__` outright ("not allowed in JIT mode"), so pinning it
// here would break the ptx arm below.

RWStructuredBuffer<int> outputBuffer;

// CHECK-DAG: {{^}}__device__ __noinline__ int helperFunc
[noinline]
int helperFunc(int x)
{
    return x + 1;
}

// `[CudaDeviceExport]` is the shipping-relevant case: a `[noinline]` device function callable
// from host CUDA, whose lowering also attaches export/keep-alive decorations. It still takes
// the ordinary device branch, so it must gain the specifier too.
// CHECK-DAG: {{^}}__device__ __noinline__ int exportedFunc
[CudaDeviceExport]
[noinline]
int exportedFunc(int x)
{
    return x + 3;
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
    // Call the helper more than once: with a single call site the downstream compiler inlines
    // it even with `__noinline__`, emitting no PTX function for the ptx arm below to observe.
    outputBuffer[0] = helperFunc(42) + plainHelper(7) + exportedFunc(1);
    outputBuffer[2] = helperFunc(7) + helperFunc(9);
}

// The ptx arm compiles the emitted CUDA through NVRTC, so an invalid specifier sequence fails
// here rather than being silently accepted. The `.func` witnesses the specifier taking effect:
// without `__noinline__` the helper is inlined and no `.func` is emitted at all. It precedes
// the entry point in the output, so it is checked first.
// CHECK_PTX: {{^}}.func{{.*}}helperFunc
// CHECK_PTX: {{^}}.visible .entry computeMain

// Direct NVVM keeps ordinary helpers internal, applies `noinline` to `helperFunc`, and uses the
// exact source name and external linkage recorded by `[CudaDeviceExport]`. At the test harness's
// O0 setting the unconstrained plain helper remains present, but it is still internal.
// CHECK_NVVM: {{^}}.func{{.*}}helperFunc
// CHECK_NVVM: {{^}}.func{{.*}}plainHelper
// CHECK_NVVM: {{^}}.visible .func{{.*}}exportedFunc
// CHECK_NVVM: {{^}}.visible .entry computeMain
