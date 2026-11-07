// Under `-fp-mode precise`, scalar arithmetic in device code uses the CUDA `_rn` intrinsics, which
// are device-only. A `[CudaHost]` function is emitted as `__host__` code for the host compiler, so
// its arithmetic must keep the infix form for the emitted source to compile.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -fp-mode precise -line-directive-mode none

RWStructuredBuffer<float> output;

[CudaKernel]
void scaleKernel(float s)
{
    output[0] = output[1] * s + output[2];
}

[CudaHost]
export void launchScaled(float a, float b, float c)
{
    __dispatch_kernel(scaleKernel, uint3(1, 1, 1), uint3(1, 1, 1))(a * b + c);
}

// CHECK-LABEL: __global__ void scaleKernel(
// CHECK: __fadd_rn(__fmul_rn(
// CHECK-LABEL: __host__ void launchScaled(
// CHECK-NOT: _rn(
// CHECK: scaleKernel<<<{{.*}}>>>(a_{{[0-9]+}} * b_{{[0-9]+}} + c_{{[0-9]+}});
