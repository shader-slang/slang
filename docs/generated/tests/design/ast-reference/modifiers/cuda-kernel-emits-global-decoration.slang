//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#cuda--python--ffi-attributes
//META: doc_section_digest=123b7667975a062eadd9a683c067ebc0122e7a4eb83b99fbe3e0b85a0e15878c
//META: purpose=`[CudaKernel]` marks a function as a CUDA kernel; the emitted CUDA carries the `__global__` decoration.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the attribute changes the emitted CUDA function qualifier, which is
// what makes the function launchable from the host. CHECK asserts the emitted
// CUDA declares the function `__global__`; an ordinary Slang function would be
// emitted as a plain device function instead.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none

[CudaKernel]
void writeOne(TensorView<float> output)
{
    output.store(cudaThreadIdx().x, 1.0f);
}

//CHECK: __global__ void writeOne
