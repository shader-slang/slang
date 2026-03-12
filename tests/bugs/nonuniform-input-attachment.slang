//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -emit-spirv-directly -allow-glsl

#version 450

// CHECK: OpCapability InputAttachmentArrayNonUniformIndexing
// CHECK-DAG: %[[ATTACH_PTR:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_UniformConstant_{{[A-Za-z0-9_]+}} %subpasses %{{[A-Za-z0-9_]+}}
// CHECK-DAG: %[[ATTACH:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[ATTACH_PTR]]
// CHECK-DAG: OpDecorate %[[ATTACH_PTR]] NonUniform
// CHECK-DAG: OpDecorate %[[ATTACH]] NonUniform

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput subpasses[4];
layout(location = 0) flat in int idx;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = subpassLoad(subpasses[NonUniformResourceIndex(idx)]);
}
