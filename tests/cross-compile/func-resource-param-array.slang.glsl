#version 450
layout(row_major) uniform;
layout(row_major) buffer;

layout(std430, binding = 0) buffer _S1 {
    int _data[];
} a_0;

layout(std430, binding = 1) buffer _S2 {
    int _data[];
} b_0[3];

layout(std430, binding = 2) buffer _S3 {
    int _data[];
} c_0[4][3];

int f_0(uint _S4)
{
    return ((a_0)._data[(_S4)]);
}

int f_1(uint _S5, uint _S6)
{
    return ((b_0[_S5])._data[(_S6)]);
}

int g_0(uint _S7, uint _S8)
{
    return ((b_0[_S7])._data[(_S8)]);
}

int g_1(uint _S9, uint _S10, uint _S11)
{
    return ((c_0[_S9][_S10])._data[(_S11)]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint ii_0 = gl_GlobalInvocationID.x;
    uint jj_0 = gl_GlobalInvocationID.y;

    int tmp_0 = f_0(ii_0) + f_0(jj_0) + f_1(ii_0, jj_0) + g_0(ii_0, jj_0) + g_1(ii_0, jj_0, gl_GlobalInvocationID.z);

    ((a_0)._data[(ii_0)]) = tmp_0;
    return;
}

