//TEST:SIMPLE(filecheck=TORCH): -target torch -line-directive-mode none

// Verify that BFloat16 element types reach torch codegen for both
// MakeTensorView (kernel parameter binding) and AllocateTorchTensor
// (TorchTensor<T>.alloc) without aborting in emitTorchScalarTypeName.

[AutoPyBindCUDA]
[CudaKernel]
void scaleKernel(TensorView<BFloat16> input, TensorView<BFloat16> output, float scale)
{
    uint tid = cudaThreadIdx().x;
    BFloat16 v = (BFloat16)(float(input[tid]) * scale);
    output[tid] = v;
}

// MakeTensorView for the input/output parameters must emit torch::kBFloat16.
// TORCH-COUNT-2: torch::kBFloat16

[TorchEntryPoint]
TorchTensor<BFloat16> allocBF16(uint n)
{
    return TorchTensor<BFloat16>.alloc(n);
}

// AllocateTorchTensor for the bf16 output buffer must also emit torch::kBFloat16.
// TORCH: torch::kBFloat16
