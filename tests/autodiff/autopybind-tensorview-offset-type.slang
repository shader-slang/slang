//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none
//TEST:SIMPLE(filecheck=TORCH): -target torch -line-directive-mode none

[AutoPyBindCUDA]
[CUDAKernel]
void copy(TensorView<float> input, TensorView<float, uint64_t> output)
{
    output[0] = input[0];
}

// CUDA: __global__ void __kernel__copy(TensorView {{.*}}, TensorViewT<{{.*}}> {{.*}})
// TORCH: void __kernel__copy(TensorView {{.*}}, TensorViewT<{{.*}}> {{.*}});
// TORCH-DAG: make_tensor_view(
// TORCH-DAG: make_tensor_view<{{.*}}>(
