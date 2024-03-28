//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage fragment -entry main -allow-glsl
//TEST:SIMPLE(filecheck=CHECK_SPV): -target spirv -emit-spirv-directly -stage fragment -entry main -allow-glsl
#version 450
// CHECK_SPV-DAG: OpEntryPoint
// CHECK_GLSL-DAG: void main(

// CHECK_SPV-DAG: InputAttachmentIndex 0
// CHECK_SPV-DAG: InputAttachmentIndex 1
// CHECK_SPV-DAG: InputAttachmentIndex 2
// CHECK_SPV-DAG: InputAttachmentIndex 3
// CHECK_SPV-DAG: InputAttachmentIndex 4
// CHECK_SPV-DAG: InputAttachmentIndex 5

// CHECK_SPV-DAG: OpTypeImage %float SubpassData 2 0 0 2 Unknown
// CHECK_GLSL-CHECK: input_attachment_index = 0
// CHECK_GLSL-CHECK-DAG: subpassInput
layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput subpass;
// CHECK_SPV-DAG: OpTypeImage %float SubpassData 2 0 1 2 Unknown
// CHECK_GLSL-CHECK: input_attachment_index = 1
// CHECK_GLSL-CHECK-DAG: subpassInputMS
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInputMS subpassMS;
// CHECK_SPV-DAG: OpTypeImage %uint SubpassData 2 0 0 2 Unknown
// CHECK_GLSL-CHECK: input_attachment_index = 2
// CHECK_GLSL-CHECK-DAG: usubpassInput
layout (input_attachment_index = 2, set = 0, binding = 2) uniform usubpassInput usubpass;
// CHECK_SPV-DAG: OpTypeImage %uint SubpassData 2 0 1 2 Unknown
// CHECK_GLSL-CHECK: input_attachment_index = 3
// CHECK_GLSL-CHECK-DAG: usubpassInputMS
layout (input_attachment_index = 3, set = 0, binding = 3) uniform usubpassInputMS usubpassMS;
// CHECK_SPV-DAG: OpTypeImage %int SubpassData 2 0 0 2 Unknown
// CHECK_GLSL-CHECK: input_attachment_index = 4
// CHECK_GLSL-CHECK-DAG: isubpassInput
layout (input_attachment_index = 4, set = 0, binding = 4) uniform isubpassInput isubpass;
// CHECK_SPV-DAG: OpTypeImage %int SubpassData 2 0 1 2 Unknown
// CHECK_GLSL-CHECK: input_attachment_index = 5
// CHECK_GLSL-CHECK-DAG: isubpassInputMS
layout (input_attachment_index = 5, set = 0, binding = 5) uniform isubpassInputMS isubpassMS;

layout (location = 0) out vec4 outColor;

void main() {
// CHECK_SPV: OpImageRead
// CHECK_GLSL-CHECK: subpassLoad
// CHECK_SPV: OpImageRead
// CHECK_GLSL-CHECK: subpassLoad
// CHECK_SPV: OpImageRead
// CHECK_GLSL-CHECK: subpassLoad
// CHECK_SPV: OpImageRead
// CHECK_GLSL-CHECK: subpassLoad
// CHECK_SPV: OpImageRead
// CHECK_GLSL-CHECK: subpassLoad
// CHECK_SPV: OpImageRead
// CHECK_GLSL-CHECK: subpassLoad
    outColor = vec4(true
        && subpassLoad(subpass) == vec4(1)
        && subpassLoad(subpassMS, 0) == vec4(1)
        && subpassLoad(isubpass) == ivec4(1)
        && subpassLoad(isubpassMS, 0) == ivec4(1)
        && subpassLoad(usubpass) == uvec4(1)
        && subpassLoad(usubpassMS, 0) == uvec4(1)
    );
}