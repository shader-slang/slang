// Under `-fp-mode precise`, scalar float add, sub and mul in device code are emitted as `__fadd_rn`,
// `__fmul_rn` and so on, which are `__device__` functions. A `[CudaHost]` function is emitted as
// `__host__`, where nvcc rejects them, so its arithmetic keeps the plain operators. The kernel
// that it launches still gets the intrinsics.

//TEST:SIMPLE(filecheck=CHECK): -O0 -target cuda -fp-mode precise -line-directive-mode none

RWStructuredBuffer<float> output;

[CudaKernel]
void affineKernel()
{
    output[0] = output[1] * output[2] - output[3];
}

[CudaHost]
export float launchAffine(float a, float b, float c)
{
    __dispatch_kernel(affineKernel, uint3(1, 1, 1), uint3(1, 1, 1))();
    return a * b + c;
}

// CHECK-LABEL: __global__ void affineKernel()
// CHECK: __fsub_rn(__fmul_rn(
// CHECK-LABEL: __host__ float launchAffine(
// CHECK-NOT: _rn(
// CHECK: affineKernel<<<
// CHECK-NOT: _rn(
// CHECK: return
// CHECK-NOT: _rn(
// CHECK-NEXT: }
