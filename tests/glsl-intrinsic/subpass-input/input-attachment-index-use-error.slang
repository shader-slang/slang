//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage fragment -entry main -allow-glsl
#version 450

buffer MyBlockName
{
    vec4 fdata[2];
    ivec4 udata[2];
    uvec4 idata[2];
} keepAliveBuffer;

// CHECK: error 31207
layout (input_attachment_index = 1, set = 0, binding = 1) uniform vec3 image;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0);
}