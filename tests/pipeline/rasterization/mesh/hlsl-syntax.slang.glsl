#version 450
#extension GL_EXT_mesh_shader : require
layout(row_major) uniform;
layout(row_major) buffer;
const vec3  colors_0[3] = { vec3(1.0, 1.0, 0.0), vec3(0.0, 1.0, 1.0), vec3(1.0, 0.0, 1.0) };
const vec2  positions_0[3] = { vec2(0.0, -0.5), vec2(0.5, 0.5), vec2(-0.5, 0.5) };
layout(location = 0)
out vec3  verts_color_0[3];

out gl_MeshPerVertexEXT
{
    vec4 gl_Position;
} gl_MeshVerticesEXT[3];


out uvec3  gl_PrimitiveTriangleIndicesEXT[1];
void foo_0(uint _S2)
{
    if(_S2 < 3U)
    {
        gl_MeshVerticesEXT[_S2].gl_Position = vec4(positions_0[_S2], 0.0, 1.0);
        verts_color_0[_S2] = colors_0[_S2];
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
