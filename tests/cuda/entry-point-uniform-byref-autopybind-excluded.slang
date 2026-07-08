//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none

// Companion to entry-point-uniform-byref-cuda-kernel-excluded.slang for the
// [AutoPyBindCUDA] arm of the by-reference gate's kernel-attribute exclusion: the
// parameter matches the descriptor-table predicate (struct carrying a fixed-size
// array of pointer-backed structs), but the auto-generated Python binding launches
// the kernel with by-value arguments, so the parameter must stay by value.

struct MyTensor
{
    float* data;
    uint stride;
}

struct TensorList
{
    MyTensor tensors[32];
}

// CHECK: __global__ void __kernel__myKernel(TensorList_0 list_0, TensorView output_0)

[AutoPyBindCUDA]
[CudaKernel]
void myKernel(TensorList list, TensorView<float> output)
{
    uint tid = cudaThreadIdx().x;
    output.store(tid, *(list.tensors[tid % 32].data));
}
