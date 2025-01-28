//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage fragment

#version 450
layout(location=1) in vec4 color_0;
in vec2 texcoord_0;

out vec4 out1;

void main()
{
    out1 = vec4(color_0.rg, texcoord_0);
}