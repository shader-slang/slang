#version 450
#extension GL_EXT_nonuniform_qualifier : require
layout(row_major) uniform;
layout(row_major) buffer;
layout(binding = 0)
uniform texture2D  t_0[10];

layout(binding = 1)
uniform sampler s_0;

layout(location = 0)
out vec4 _S1;

layout(location = 0)
in vec3 _S2;

void main()
{
    _S1 = (texture(sampler2D(t_0[nonuniformEXT(int(_S2.z))],s_0), (_S2.xy)));
    return;
}

