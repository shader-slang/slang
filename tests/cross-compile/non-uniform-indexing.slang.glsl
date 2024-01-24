#version 450
#extension GL_EXT_nonuniform_qualifier : require
layout(row_major) uniform;
layout(row_major) buffer;
layout(binding = 0)
uniform texture2D  t_0[10];

layout(binding = 1)
uniform sampler s_0;

layout(location = 0)
out vec4 main_0;

layout(location = 0)
in vec3 uv_0;

void main()
{
    main_0 = (texture(sampler2D(t_0[nonuniformEXT(int(uv_0.z))],s_0), (uv_0.xy)));
    return;
}

