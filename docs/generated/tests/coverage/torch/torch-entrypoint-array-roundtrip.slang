//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T16:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerbuiltintypesforkernelentrypoints-removetorchkernels-handleautobindnames
//META: doc_section_digest=8395af4f0b26d3fcc84d23080f14f58fbfb44b12a02306327f726485b8a8edae
//META: purpose=A fixed-size float[2] [TorchEntryPoint] parameter and return both map to a 2-element std::tuple<float, float>.
//META: intent=characterization
//META: covers=source/slang/slang-ir-pytorch-cpp-binding.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the IRArrayType branch of translateToTupleType
// (slang-ir-pytorch-cpp-binding.cpp:57): a fixed-size array recurses on its
// element type and repeats the sub-element tuple arraySize times. For float[2]
// that is std::tuple<float, float>. The param rebuild uses makeValueFromTargetTuple's
// array branch (line 293, which also stamps a <name>_<i> name hint) and the
// return uses makeTargetTuple's array branch (line 171). The CHECK pins the host
// signature where both the array parameter and array result read as a 2-tuple.

//TEST:SIMPLE(filecheck=CHECK):-target torch -line-directive-mode none

[TorchEntryPoint]
export __extern_cpp float[2] runCompute(float input[2])
{
    float r[2];
    r[0] = input[0] + 1.0;
    r[1] = input[1] + 2.0;
    return r;
}

// CHECK: std::tuple<float, float> runCompute(std::tuple<float, float> input{{[0-9_]+}})
