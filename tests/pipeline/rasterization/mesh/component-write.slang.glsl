#version 450
#extension GL_EXT_mesh_shader : require
layout(row_major) uniform;
layout(row_major) buffer;
const vec3  colors_0[3] = { vec3(1.00000000000000000000, 1.00000000000000000000, 0.00000000000000000000), vec3(0.00000000000000000000, 1.00000000000000000000, 1.00000000000000000000), vec3(1.00000000000000000000, 0.00000000000000000000, 1.00000000000000000000) };
const vec2  positions_0[3] = { vec2(0.00000000000000000000, -0.50000000000000000000), vec2(0.50000000000000000000, 0.50000000000000000000), vec2(-0.50000000000000000000, 0.50000000000000000000) };
layout(location = 0)
out vec3  _S1[];

layout(local_size_x = 3, local_size_y = 1, local_size_z = 1) in;
layout(max_vertices = 3) out;
layout(max_primitives = 1) out;
layout(triangles) out;
void main()
{
    SetMeshOutputsEXT(3U, 1U);
    if(gl_LocalInvocationIndex < 3U)
    {
        gl_MeshVerticesEXT[gl_LocalInvocationIndex].gl_Position = vec4(positions_0[gl_LocalInvocationIndex], 0.00000000000000000000, 1.00000000000000000000);
        _S1[gl_LocalInvocationIndex] = colors_0[gl_LocalInvocationIndex];
    }
    else
    {
    }
    if(gl_LocalInvocationIndex < 1U)
    {
        gl_PrimitiveTriangleIndicesEXT[gl_LocalInvocationIndex] = uvec3(0U, 1U, 2U);
    }
    else
    {
    }
    return;
}

