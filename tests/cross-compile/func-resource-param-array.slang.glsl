// func-resource-param-array.slang.glsl
#version 450

#define a a_0
#define b b_0
#define c c_0
#define ii ii_0
#define jj jj_0
#define kk kk_0

#define f_a f_0
#define f_b f_1
#define g_b g_0
#define g_c g_1

#define a_block _S1
#define b_block _S2
#define c_block _S3

#define f_a_i  	_S4
#define f_b_t 	_S5
#define f_b_i   _S6
#define g_b_i   _S7
#define g_b_j   _S8
#define g_c_t   _S9
#define g_c_i   _S10
#define g_c_j   _S11

#define tmp_f_a_ii	_S12
#define tmp_f_a_jj	_S13

#define tmp_f_b 	_S14
#define tmp_g_b 	_S15
#define tmp_g_c 	_S16

layout(std430, binding = 0) buffer a_block {
    int _data[];
} a;

layout(std430, binding = 1) buffer b_block {
    int _data[];
} b[3];

layout(std430, binding = 2) buffer c_block {
    int _data[];
} c[4][3];

int f_a(uint f_a_i)
{
    return a._data[f_a_i];
}

int f_b(uint f_b_t, uint f_b_i)
{
    return b[f_b_t]._data[f_b_i];
}

int g_b(uint g_b_i, uint g_b_j)
{
    return b[g_b_i]._data[g_b_j];
}

int g_c(uint g_c_t, uint g_c_i, uint g_c_j)
{
    return c[g_c_t][g_c_i]._data[g_c_j];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main()
{
    uint ii = gl_GlobalInvocationID.x;
    uint jj = gl_GlobalInvocationID.y;
    uint kk = gl_GlobalInvocationID.z;

    int tmp_f_a_ii = f_a(ii);

    int tmp_f_a_jj = f_a(jj);
    int tmp_0 = tmp_f_a_ii + tmp_f_a_jj;

    int tmp_f_b = f_b(ii, jj);
    int tmp_1 = tmp_0 + tmp_f_b;

    int tmp_g_b = g_b(ii, jj);
    int tmp_2 = tmp_1 + tmp_g_b;

    int tmp_g_c = g_c(ii, jj, kk);
    int tmp_3 = tmp_2 + tmp_g_c;

    a._data[ii] = tmp_3;

    return;
}
