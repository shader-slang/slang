//TEST_IGNORE_FILE:
#version 450
layout(row_major) uniform;
layout(row_major) buffer;

struct F_0
{
    vec4 v0_0;
    vec2 v1_0;
};

float F_get_0(F_0 this_0)
{
    return this_0.v0_0.x + this_0.v1_0.x;
}


float E_get_0()
{
    return 1.00000000000000000000;
}

layout(location = 0)
in vec3 _S1;

layout(location = 1)
in vec4 _S2;

layout(location = 2)
in vec2 _S3;

struct GIn_0
{
    vec3 p0_0;
    F_0 field_0;
};

struct VOut_0
{
    vec4 projPos_0;
};



void main()
{
    GIn_0 _S4 = GIn_0(_S1, F_0(_S2, _S3));

    VOut_0 vout_0;
    vec3 _S5 = _S4.p0_0;

    float _S6 = F_get_0(_S4.field_0);
    float _S7 = E_get_0();

    vout_0.projPos_0 = vec4(_S5, _S6 + _S7);
    gl_Position = vout_0.projPos_0;
    return;
}