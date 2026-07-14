//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#cuda-python-ffi-attributes
//META: doc_section_digest=3b673e659791f55ec3bcf6e28fd250cbd2b9866ccde0280dfbe5a7d075f519d9
//META: purpose=`CudaKernelAttribute` (`[CudaKernel]`) marks a function as a CUDA kernel; the emitted CUDA carries the `__global__` decoration.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `CudaKernelAttribute` is the AST node behind the `[CudaKernel]`
// attribute. The CUDA emit path attaches the `__global__` function
// qualifier to such a function so it is callable from the host as a
// CUDA kernel.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none

[CudaKernel]
void writeOne(TensorView<float> output)
{
    output.store(cudaThreadIdx().x, 1.0f);
}

//CHECK: __global__ void writeOne
