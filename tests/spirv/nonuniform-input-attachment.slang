//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -emit-spirv-directly

// Coverage: a SubpassInput array indexed with NonUniformResourceIndex must emit
// InputAttachmentArrayNonUniformIndexing and decorate the access chain and
// loaded input attachment NonUniform.

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability InputAttachmentArrayNonUniformIndexing

// The access chain into the SubpassInput array must be NonUniform.
// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %inputs %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[AC]] NonUniform
// CHECK-DAG: %[[LOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[AC]]
// CHECK-DAG: OpDecorate %[[LOAD]] NonUniform

[vk::input_attachment_index(0)] SubpassInput<float4> inputs[];

cbuffer Params { uint idx; }

float4 main() : SV_Target
{
    return inputs[NonUniformResourceIndex(idx)].SubpassLoad();
}
