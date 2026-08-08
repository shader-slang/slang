//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T16:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerbuiltintypesforkernelentrypoints-removetorchkernels-handleautobindnames
//META: doc_section_digest=8395af4f0b26d3fcc84d23080f14f58fbfb44b12a02306327f726485b8a8edae
//META: purpose=A float2x2 [TorchEntryPoint] value maps to a nested std::tuple of two row tuples, each of two floats.
//META: intent=characterization
//META: covers=source/slang/slang-ir-pytorch-cpp-binding.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the IRMatrixType branch of translateToTupleType
// (slang-ir-pytorch-cpp-binding.cpp:21): a matrix is lowered to a tuple of row
// tuples (rowCount tuples, each colCount elements). For float2x2 this is
// std::tuple<std::tuple<float,float>, std::tuple<float,float>>. The return path
// uses makeTargetTuple's matrix branch (line 108) and the param path uses
// makeValueFromTargetTuple's matrix branch (line 236, the only branch that
// SLANG_ASSERTs row/col are int-literals). The CHECK pins the nested host shape.

//TEST:SIMPLE(filecheck=CHECK):-target torch -line-directive-mode none

[TorchEntryPoint]
export __extern_cpp float2x2 runCompute(float2x2 input)
{
    return input;
}

// CHECK: std::tuple<std::tuple<float, float>, std::tuple<float, float>> runCompute(std::tuple<std::tuple<float, float>, std::tuple<float, float>> input{{[0-9_]+}})
