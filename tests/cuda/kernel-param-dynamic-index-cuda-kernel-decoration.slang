//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none

// The dynamic-index legalization keys off the CUDA kernel ABI, not the
// `[shader("compute")]` entry-point form: a `[CudaKernel]`-attributed function also
// receives its parameters in `.param` space, so a runtime index into a by-value
// array parameter must likewise be rewritten through a function-local copy.

// CHECK: __global__ void myKernel(FixedArray<float, 256>{{.*}}weights_0, TensorView idx_0, TensorView output_0)
// CHECK: FixedArray<float, 256>{{.*}}[[LOCAL:_S[0-9]+]] = weights_0;
// CHECK-NOT: weights_0[
// CHECK: [[LOCAL]][

[CudaKernel]
void myKernel(float weights[256], TensorView<uint> idx, TensorView<float> output)
{
    uint tid = cudaThreadIdx().x;
    output.store(tid, weights[idx.load(tid)]);
}
