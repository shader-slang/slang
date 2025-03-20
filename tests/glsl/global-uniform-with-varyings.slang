//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv -entry main -stage vertex
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target spirv -entry main -stage vertex

#version 330 core
layout(location=0) in float3 a_position;
layout(location=1) in float4 a_color;

out float4 v_color;

uniform matrix<float,4,4> u_transform;

void main() {
    gl_Position = u_transform * vec4(a_position, 1.0);
    v_color = a_color;

    // CHECK_SPIRV: OpEntryPoint
    // CHECK_GLSL: main
}
