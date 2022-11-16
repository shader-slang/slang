#version 450
#extension GL_EXT_mesh_shader : require
layout(row_major) uniform;
layout(row_major) buffer;
const vec3  colors_0[3] = { vec3(1.00000000000000000000, 1.00000000000000000000, 0.00000000000000000000), vec3(0.00000000000000000000, 1.00000000000000000000, 1.00000000000000000000), vec3(1.00000000000000000000, 0.00000000000000000000, 1.00000000000000000000) };
const vec2  positions_0[3] = { vec2(0.00000000000000000000, -0.50000000000000000000), vec2(0.50000000000000000000, 0.50000000000000000000), vec2(-0.50000000000000000000, 0.50000000000000000000) };
layout(location = 0)
out vec3  _S1[3];

out gl_MeshPerVertexEXT
{
    vec4 gl_Position;
} gl_MeshVerticesEXT[3];

struct Vertex_0
{
    vec4 pos_0;
    vec3 color_0;
};

void foo_0(uint _S2)
{
    if(_S2 < 3U)
    {
        Vertex_0 _S3 = { vec4(positions_0[_S2], 0.00000000000000000000, 1.00000000000000000000), colors_0[_S2] };
        gl_MeshVerticesEXT[_S2].gl_Position = _S3.pos_0;
        _S1[_S2] = _S3.color_0;
    }
    else
    {
    }
    return;
}

layout(local_size_x = 3, local_size_y = 1, local_size_z = 1) in;
layout(max_vertices = 3) out;
layout(max_primitives = 1) out;
layout(triangles) out;
void main()
{
    SetMeshOutputsEXT(3U, 1U);
    foo_0(gl_LocalInvocationIndex);
    if(gl_LocalInvocationIndex < 1U)
    {
        gl_PrimitiveTriangleIndicesEXT[gl_LocalInvocationIndex] = uvec3(0U, 1U, 2U);
    }
    else
    {
    }
    return;
}
