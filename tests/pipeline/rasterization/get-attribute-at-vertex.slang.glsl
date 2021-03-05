// get-attribute-at-vertex.slang.glsl
//TEST_IGNORE_FILE:

#version 450

#extension GL_NV_fragment_shader_barycentric : require

pervertexNV layout(location = 0)
in vec4 _S1[3];

layout(location = 0)
out vec4 _S2;

void main()
{
    vec4 _S3;

    _S3 = gl_BaryCoordNV.x * _S1[0]
        + gl_BaryCoordNV.y * _S1[1]
        + gl_BaryCoordNV.z * _S1[2];

    _S2 = _S3;

    return;
}
