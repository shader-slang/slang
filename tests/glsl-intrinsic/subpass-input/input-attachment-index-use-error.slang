//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage fragment -entry main -allow-glsl -DGLSL
//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage fragment -entry main -allow-glsl -DHLSL
#version 450

// CHECK: error 31207
#ifdef GLSL
layout (input_attachment_index = 1, set = 0, binding = 1) uniform vec3 errorBinding;
#endif
#ifdef HLSL
[[vk::input_attachment_index(1)]] RWTexture1D<float4> errorBinding;
#endif

layout (location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0);
}