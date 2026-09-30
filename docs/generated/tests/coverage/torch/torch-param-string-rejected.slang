//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T16:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerbuiltintypesforkernelentrypoints-removetorchkernels-handleautobindnames
//META: doc_section_digest=1b4008e777a39c271b0416d939d21f6b248cfe804bde82afe67b956117c6cf82
//META: purpose=A [TorchEntryPoint] taking a non-tensor-mappable parameter (String) is rejected with E55102.
//META: intent=characterization
//META: covers=source/slang/slang-ir-pytorch-cpp-binding.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the parameter-type guard in generateCppBindingForFunc
// (slang-ir-pytorch-cpp-binding.cpp:407): translateToTupleType returns null for a
// `string` parameter (catch-all else, line 92), so the pass diagnoses
// InvalidTorchKernelParamType (E55102) and bails. This is the parameter
// counterpart to the return-type guard (E55101). The CHECK pins the error code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target torch -line-directive-mode none

[TorchEntryPoint]
export __extern_cpp void runCompute(String s)
//CHECK:                                   ^ invalid pytorch kernel parameter type
//CHECK:                                   ^ 'string' is not a valid parameter type for a pytorch kernel function.
{
}
