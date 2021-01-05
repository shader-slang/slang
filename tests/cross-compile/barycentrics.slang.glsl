#version 450

#extension GL_NV_fragment_shader_barycentric : enable

layout(location = 0)
out vec4 _S1;

void main()
{
    _S1 = vec4(gl_BaryCoordNV, float(0));
    return;
}
