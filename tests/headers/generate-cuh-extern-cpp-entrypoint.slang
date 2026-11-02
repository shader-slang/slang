//TEST:SIMPLE(filecheck=CHECK): -target cuh -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none

// Regression test for https://github.com/shader-slang/slang/issues/14033 :
// the CUDA header for an `__extern_cpp` compute entry point must declare the
// kernel so that host code can launch the kernel defined in the generated `.cu`.
// The declaration must have exactly the parameters of that definition: the
// `SV_DispatchThreadID` parameter is a varying input that CUDA legalization
// replaces with `blockIdx`/`threadIdx`, so it is not a kernel argument.

//CHECK: struct fsymmat3
//CHECK: extern "C" __global__ void UpdateInverse(fsymmat3 * {{[[:alnum:]_]+}}, fsymmat3 * {{[[:alnum:]_]+}}, uint {{[[:alnum:]_]+}});
//CHECK-NOT: {

//CUDA: extern "C" __global__ void UpdateInverse(fsymmat3 * {{[[:alnum:]_]+}}, fsymmat3 * {{[[:alnum:]_]+}}, uint {{[[:alnum:]_]+}})
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
