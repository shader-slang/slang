//DISABLE_TEST:SIMPLE(filecheck=CHECK):  -allow-glsl -stage geometry -entry main -target glsl
//DISABLE_TEST:SIMPLE(filecheck=CHECK):  -allow-glsl -stage geometry -entry main -target spirv -emit-spirv-directly
#version 460

layout(points) in;
layout(line_strip, max_vertices = 2) out;

void main()
{
    gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.0, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.0, 0.0, 0.0);
    EmitStreamVertex(0);

    gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.0, 0.0, 0.0);
    EmitStreamVertex(0);

    EndPrimitive();
}