//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/subpass-input/input-attachment-index-overlapping-error3.slang -target glsl -stage fragment -entry main
//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/subpass-input/input-attachment-index-overlapping-error3.slang -target spirv -emit-spirv-directly -stage fragment -entry main
#version 450
// CHECK: warning{{.*}}39001
buffer MyBlockName
{
    uvec4 idata[2];
} keepAliveBuffer;

layout (input_attachment_index = 0, binding = 1) uniform isubpassInput isubpass;
layout (input_attachment_index = 1, binding = 1) uniform isubpassInputMS isubpassMS;

layout (location = 0) out vec4 outColor;

void main() {
    keepAliveBuffer.idata[0] = subpassLoad(isubpass);
    keepAliveBuffer.idata[1] = subpassLoad(isubpassMS, 0);

    outColor = vec4(0);
}