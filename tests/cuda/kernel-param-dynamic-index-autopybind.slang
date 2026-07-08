//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none

// Same dynamic-index legalization for an `[AutoPyBindCUDA] [CudaKernel]` function:
// the generated `__kernel__`-prefixed wrapper still receives its by-value array
// parameter in `.param` space, so a runtime index must go through a local copy.

// CHECK: __global__ void __kernel__myKernel(FixedArray<float, 256>{{.*}}weights_0, TensorView idx_0, TensorView output_0)
// CHECK: FixedArray<float, 256>{{.*}}[[LOCAL:_S[0-9]+]] = weights_0;
// CHECK-NOT: weights_0[
// CHECK: [[LOCAL]][

[AutoPyBindCUDA]
[CudaKernel]
void myKernel(float weights[256], TensorView<uint> idx, TensorView<float> output)
{
    uint tid = cudaThreadIdx().x;
    output.store(tid, weights[idx.load(tid)]);
}
