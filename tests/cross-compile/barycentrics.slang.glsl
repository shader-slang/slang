#version 450

#extension GL_EXT_fragment_shader_barycentric : enable

layout(location = 0)
out vec4 main_0;

void main()
{
    main_0 = vec4(gl_BaryCoordEXT, float(0));
    return;
}
