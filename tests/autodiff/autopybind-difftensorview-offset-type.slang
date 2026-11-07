//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none
//TEST:SIMPLE(filecheck=TORCH): -target torch -line-directive-mode none

[AutoPyBindCUDA]
[Differentiable]
[CUDAKernel]
void scale(DiffTensorView<float, uint64_t> input, DiffTensorView output)
{
    uint64_t i = (uint64_t(1) << 32) + 1;
    output[0] = 3 * input[i];
}

// CUDA: TensorViewT<unsigned long long> diff
// CUDA: TensorViewT<unsigned long long> primal
// CUDA-DAG: void __kernel__scale_bwd_diff(
// CUDA-DAG: void __kernel__scale_fwd_diff(
// CUDA-DAG: void __kernel__scale(
// TORCH: make_tensor_view<TensorViewT<unsigned long long>::OffsetType>(
// TORCH-DAG: m.def("scale",
// TORCH-DAG: m.def("scale_fwd_diff",
// TORCH-DAG: m.def("scale_bwd_diff",
// TORCH-DAG: m.def("__typeinfo__DiffTensorView", &__typeinfo__DiffTensorView,
// TORCH-DAG: m.def("__typeinfo__DiffTensorView_1", &__typeinfo__DiffTensorView_1,
// TORCH-DAG: m.def("__typeinfo__AtomicAdd", &__typeinfo__AtomicAdd,
// TORCH-DAG: m.def("__typeinfo__AtomicAdd_1", &__typeinfo__AtomicAdd_1,
