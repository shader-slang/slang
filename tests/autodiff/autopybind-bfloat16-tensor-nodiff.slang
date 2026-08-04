//TEST:SIMPLE(filecheck=TORCH): -target torch -line-directive-mode none

// Verbatim regression test for the reported bf16 slangtorch failure:
// `unexpected: unknown scalar type in allocTorchTensor: TorchTensor`.
// The kernel uses `no_diff` modifiers on every parameter, the alternate
// `[CUDAKernel]` attribute spelling, and a trailing comma in the parameter
// list — none of which should affect host-side binding codegen.

[CUDAKernel][AutoPyBindCUDA]
void scale_bf16_nodiff(
    no_diff uint count,
    no_diff TensorView<BFloat16> input,
    no_diff TensorView<BFloat16> output,
    no_diff float scale,
)
{
    const uint tid = cudaBlockIdx().x * cudaBlockDim().x + cudaThreadIdx().x;
    if (tid >= count) {
        return;
    }

    // convert to fp32 for arithmetic,
    BFloat16 val = (BFloat16)(float(input[tid]) * scale);
    output[tid] = val;
}

// Both TensorView<BFloat16> parameters must reach MakeTensorView with bf16.
// TORCH-COUNT-2: torch::kBFloat16

// And the generated pybind module must export the kernel under its source name.
// TORCH: m.def("scale_bf16_nodiff", &scale_bf16_nodiff
