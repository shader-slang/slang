//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -emit-spirv-directly

// Coverage: a SubpassInput array indexed with NonUniformResourceIndex must emit
// InputAttachmentArrayNonUniformIndexing.

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability InputAttachmentArrayNonUniformIndexing

[vk::input_attachment_index(0)] SubpassInput<float4> inputs[];

cbuffer Params { uint idx; }

float4 main() : SV_Target
{
    return inputs[NonUniformResourceIndex(idx)].SubpassLoad();
}
