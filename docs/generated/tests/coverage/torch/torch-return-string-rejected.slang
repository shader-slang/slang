//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T16:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerbuiltintypesforkernelentrypoints-removetorchkernels-handleautobindnames
//META: doc_section_digest=1b4008e777a39c271b0416d939d21f6b248cfe804bde82afe67b956117c6cf82
//META: purpose=A [TorchEntryPoint] returning a non-tensor-mappable type (String) is rejected with E55101.
//META: intent=characterization
//META: covers=source/slang/slang-ir-pytorch-cpp-binding.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the return-type guard in generateCppBindingForFunc
// (slang-ir-pytorch-cpp-binding.cpp:377): when translateToTupleType returns null
// for the function result type (here `string`, which hits the catch-all else at
// line 92), the pass diagnoses InvalidTorchKernelReturnType (E55101) and bails.
// The CHECK pins the error code on the return-type token.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target torch -line-directive-mode none

[TorchEntryPoint]
export __extern_cpp String runCompute()
//CHECK:                   ^^^^^^^^^^ invalid pytorch kernel return type
//CHECK:                   ^^^^^^^^^^ 'string' is not a valid return type for a pytorch kernel function.
{
    return "hello";
}
