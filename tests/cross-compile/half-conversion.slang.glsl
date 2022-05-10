//half-conversion.slang.glsl
//TEST_IGNORE_FILE:

#version 450

struct SLANG_ParameterGroup_C_0
{
    uvec4 u_0;
};

layout(binding = 0)
layout(std140) uniform _S1
{
    SLANG_ParameterGroup_C_0 _data;
} C_0;

vec4 f16tof32_0(uvec4 value_0)
{
    vec4 result_0;
    int i_0;
    i_0 = 0;
    for(;;)
    {
        if(i_0 < 4) {} else break;

        float _S2 = (unpackHalf2x16((value_0[i_0])).x);
        result_0[i_0] = _S2;
        i_0 = i_0 + int(1);
    }
    return result_0;
}

layout(location = 0)
out vec4 _S3;

void main()
{
    vec4 _S4 = f16tof32_0(C_0._data.u_0);
    _S3 = _S4;
    return;
}
