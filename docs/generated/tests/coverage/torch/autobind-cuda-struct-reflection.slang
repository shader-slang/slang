//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T16:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerbuiltintypesforkernelentrypoints-removetorchkernels-handleautobindnames
//META: doc_section_digest=8395af4f0b26d3fcc84d23080f14f58fbfb44b12a02306327f726485b8a8edae
//META: purpose=An [AutoPyBindCUDA] kernel taking a struct of builtin types generates a host wrapper plus a __typeinfo__<Struct> reflection function.
//META: intent=characterization
//META: covers=source/slang/slang-ir-pytorch-cpp-binding.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the [AutoPyBindCUDA] host-wrapper pipeline:
// generateCUDAWrapperForFunc -> generateHostParamForCUDAParam, where
// translateToHostType keeps the struct shape (slang-ir-pytorch-cpp-binding.cpp:502)
// and castHostToCUDAType rebuilds it field-by-field (line 597). markTypeForPyExport
// (line 832) tags the struct, and generateReflectionForType's struct case (line 920)
// emits a __typeinfo__ function whose name comes from tryGetExportTypeName. The host
// wrapper takes two leading uint3 (block/grid) tuples then the flattened struct; the
// CHECKs pin the host signature, the reflection function, and both pybind registrations.

//TEST:SIMPLE(filecheck=CHECK):-target torch -line-directive-mode none

struct MyStruct
{
    float3x3 data;
    float3 vec;
}

[AutoPyBindCUDA]
[CudaKernel]
void plainCopy(MyStruct input, TensorView<float> output)
{
    uint3 d = cudaThreadIdx();
    if (d.x >= 1)
        return;
    output[0] = input.data[0][0];
    output[1] = input.vec[1];
}

// CHECK-DAG: void plainCopy(std::tuple<uint32_t, uint32_t, uint32_t> _blockSize{{[0-9_]+}}, std::tuple<uint32_t, uint32_t, uint32_t> _gridSize{{[0-9_]+}}, std::tuple<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>>, std::tuple<float, float, float>> input{{[0-9_]+}}, torch::Tensor output{{[0-9_]+}})
// CHECK-DAG: __typeinfo__MyStruct()
// CHECK-DAG: m.def("plainCopy", &plainCopy, "plainCopy");
// CHECK-DAG: m.def("__typeinfo__MyStruct", &__typeinfo__MyStruct, "__typeinfo__MyStruct");
