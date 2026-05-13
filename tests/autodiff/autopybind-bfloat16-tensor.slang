//TEST:SIMPLE(filecheck=TORCH): -target torch -line-directive-mode none

// Verify that BFloat16 element types reach torch codegen for all three
// emitTorchScalarTypeName call sites in slang-emit-torch.cpp:
//   1. MakeTensorView           (host->device parameter binding)
//   2. AllocateTorchTensor empty(...)       (TorchTensor.alloc(dims))
//   3. AllocateTorchTensor empty_like(...)  (TorchTensor.emptyLike(other))

[AutoPyBindCUDA]
[CudaKernel]
void scaleKernel(TensorView<BFloat16> input, TensorView<BFloat16> output, float scale)
{
    uint tid = cudaThreadIdx().x;
    BFloat16 v = (BFloat16)(float(input[tid]) * scale);
    output[tid] = v;
}

// (1) MakeTensorView for input + output parameters.
// TORCH-DAG: make_tensor_view({{.*}}torch::kBFloat16
// TORCH-DAG: make_tensor_view({{.*}}torch::kBFloat16

[TorchEntryPoint]
TorchTensor<BFloat16> allocBF16(uint n)
{
    return TorchTensor<BFloat16>.alloc(n);
}

// (2) AllocateTorchTensor empty(...) branch.
// TORCH-DAG: torch::empty({{.*}}torch::kBFloat16

[TorchEntryPoint]
TorchTensor<BFloat16> cloneShape(TorchTensor<BFloat16> other)
{
    return TorchTensor<BFloat16>.emptyLike(other);
}

// (3) AllocateTorchTensor empty_like(...) branch.
// TORCH-DAG: torch::empty_like({{.*}}torch::kBFloat16
