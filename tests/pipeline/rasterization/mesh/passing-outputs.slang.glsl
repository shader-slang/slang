#version 450
#extension GL_EXT_mesh_shader : require
layout(row_major) uniform;
layout(row_major) buffer;
struct Texes_0
{
    vec2 tex1_0;
    vec4 tex2_0;
};

struct Vertex_0
{
    vec4 pos_0;
    vec3 col_0;
    Texes_0 ts_0;
};

void just_two_0(out Vertex_0 v_0, out Vertex_0 w_0)
{
    Texes_0 _S1 = { vec2(0.00000000000000000000, 0.00000000000000000000), vec4(0.00000000000000000000, 0.00000000000000000000, 0.00000000000000000000, 0.00000000000000000000) };
    Vertex_0 _S2 = { vec4(0.00000000000000000000), vec3(1.00000000000000000000), _S1 };
    v_0 = _S2;
    w_0 = v_0;
    return;
}

void just_one_0(out Vertex_0 v_1)
{
    Texes_0 _S3 = { vec2(0.00000000000000000000, 0.00000000000000000000), vec4(0.00000000000000000000, 0.00000000000000000000, 0.00000000000000000000, 0.00000000000000000000) };
    Vertex_0 _S4 = { vec4(0.00000000000000000000), vec3(1.00000000000000000000), _S3 };
    v_1 = _S4;
    return;
}

void part_of_one_0(out vec4 p_0)
{
    p_0 = vec4(1.00000000000000000000, 2.00000000000000000000, 3.00000000000000000000, 4.00000000000000000000);
    return;
}

void write_struct_0(out Texes_0 t_0)
{
    t_0.tex1_0 = vec2(0.00000000000000000000);
    t_0.tex2_0 = vec4(1.00000000000000000000);
    return;
}

layout(location = 0)
out vec3  _S5[3];

layout(location = 1)
out vec2  _S6[3];

layout(location = 2)
out vec4  _S7[3];

out gl_MeshPerVertexEXT
{
    vec4 gl_Position;
} gl_MeshVerticesEXT[3];

void everything_0()
{
    vec3 _S8 = vec3(1.00000000000000000000);
    vec2 _S9 = vec2(0.00000000000000000000, 0.00000000000000000000);
    vec4 _S10 = vec4(0.00000000000000000000, 0.00000000000000000000, 0.00000000000000000000, 0.00000000000000000000);
    gl_MeshVerticesEXT[0U].gl_Position = vec4(0.00000000000000000000);
    _S5[0U] = _S8;
    _S6[0U] = _S9;
    _S7[0U] = _S10;
    return;
}

void a_0()
{
    everything_0();
    return;
}

void b_0()
{
    Vertex_0 _S11;
    Vertex_0 _S12;
    just_two_0(_S12, _S11);
    Vertex_0 _S13 = _S12;
    gl_MeshVerticesEXT[0U].gl_Position = _S13.pos_0;
    _S5[0U] = _S13.col_0;
    Texes_0 _S14 = _S13.ts_0;
    _S6[0U] = _S14.tex1_0;
    _S7[0U] = _S14.tex2_0;
    Vertex_0 _S15 = _S11;
    gl_MeshVerticesEXT[0U].gl_Position = _S15.pos_0;
    _S5[0U] = _S15.col_0;
    Texes_0 _S16 = _S15.ts_0;
    _S6[0U] = _S16.tex1_0;
    _S7[0U] = _S16.tex2_0;
    return;
}

void c_0(uint _S17)
{
    Vertex_0 _S18;
    just_one_0(_S18);
    Vertex_0 _S19 = _S18;
    gl_MeshVerticesEXT[_S17].gl_Position = _S19.pos_0;
    _S5[_S17] = _S19.col_0;
    Texes_0 _S20 = _S19.ts_0;
    _S6[_S17] = _S20.tex1_0;
    _S7[_S17] = _S20.tex2_0;
    return;
}

void d_0(uint _S21)
{
    Vertex_0 _S22;
    Vertex_0 _S23;
    just_two_0(_S23, _S22);
    Vertex_0 _S24 = _S23;
    gl_MeshVerticesEXT[_S21].gl_Position = _S24.pos_0;
    _S5[_S21] = _S24.col_0;
    Texes_0 _S25 = _S24.ts_0;
    _S6[_S21] = _S25.tex1_0;
    _S7[_S21] = _S25.tex2_0;
    Vertex_0 _S26 = _S22;
    gl_MeshVerticesEXT[0U].gl_Position = _S26.pos_0;
    _S5[0U] = _S26.col_0;
    Texes_0 _S27 = _S26.ts_0;
    _S6[0U] = _S27.tex1_0;
    _S7[0U] = _S27.tex2_0;
    return;
}

void e_0(uint _S28)
{
    part_of_one_0(gl_MeshVerticesEXT[_S28].gl_Position);
    Texes_0 _S29;
    write_struct_0(_S29);
    Texes_0 _S30 = _S29;
    _S6[_S28] = _S30.tex1_0;
    _S7[_S28] = _S30.tex2_0;
    part_of_one_0(_S7[_S28]);
    return;
}

layout(local_size_x = 3, local_size_y = 1, local_size_z = 1) in;
layout(max_vertices = 3) out;
layout(max_primitives = 1) out;
layout(triangles) out;
void main()
{
    SetMeshOutputsEXT(3U, 1U);
    if(gl_LocalInvocationIndex < 3U)
    {
        a_0();
        b_0();
        c_0(gl_LocalInvocationIndex);
        d_0(gl_LocalInvocationIndex);
        e_0(gl_LocalInvocationIndex);
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

