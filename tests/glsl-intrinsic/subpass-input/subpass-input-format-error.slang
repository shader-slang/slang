//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage fragment -entry main -allow-glsl
//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -stage fragment -entry main -allow-glsl
#version 450

// CHECK: error 31209

layout (input_attachment_index = 0, set = 0, binding = 0, rgba32f) uniform subpassInput subpass;

layout (location = 0) out vec4 outColor;

void main() {

    outColor = vec4(true
        && subpassLoad(subpass) == vec4(1)
    );
}