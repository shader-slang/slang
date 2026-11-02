//TEST:SIMPLE(filecheck=CHECK): -target cuh -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none

// Regression test for https://github.com/shader-slang/slang/issues/14033 :
// the CUDA header for an `__extern_cpp` compute entry point must declare the
// kernel so that host code can launch the kernel defined in the generated `.cu`.
// The declaration must have exactly the parameters of that definition: the
// `SV_DispatchThreadID` parameter is a varying input that CUDA legalization
// replaces with `blockIdx`/`threadIdx`, so it is not a kernel argument.
// The header declares only `__extern_cpp` entry points, so `InternalKernel`
// is defined in the `.cu` but does not appear in the header.

//CHECK-NOT: InternalKernel
//CHECK: struct fsymmat3
//CHECK-NOT: InternalKernel
//CHECK: extern "C" __global__ void UpdateInverse(fsymmat3 * {{[[:alnum:]_]+}}, fsymmat3 * {{[[:alnum:]_]+}}, uint {{[[:alnum:]_]+}});
//CHECK-NOT: {{UpdateInverse|InternalKernel}}

//CUDA: extern "C" __global__ void UpdateInverse(fsymmat3 * {{[[:alnum:]_]+}}, fsymmat3 * {{[[:alnum:]_]+}}, uint {{[[:alnum:]_]+}})
//CUDA-NEXT: {
//CUDA: extern "C" __global__ void InternalKernel(uint * {{[[:alnum:]_]+}})
//CUDA-NEXT: {

__extern_cpp struct fsymmat3
{
    __extern_cpp float v00, v01, v02, v11, v12, v22;
};

[shader("compute")]
[numthreads(128, 1, 1)]
__extern_cpp void UpdateInverse(
    uniform fsymmat3* inv,
    uniform fsymmat3* val,
    uniform uint n,
    uint3 tid: SV_DispatchThreadID)
{
    if (tid.x >= n)
        return;
    fsymmat3 r = val[tid.x];
    r.v00 = 1.0f / r.v00;
    inv[tid.x] = r;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void InternalKernel(uniform uint* p, uint3 tid: SV_DispatchThreadID)
{
    p[tid.x] = 0;
}
