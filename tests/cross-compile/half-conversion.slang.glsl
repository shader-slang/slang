#version 450
layout(row_major) uniform;
layout(row_major) buffer;
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
    int i_0 = 0;
    for(;;)
    {
        if(i_0 < 4)
        {
        }
        else
        {
            break;
        }
        result_0[i_0] = (unpackHalf2x16((value_0[i_0])).x);
        i_0 = i_0 + 1;
    }
    return result_0;
}

layout(location = 0)
out vec4 _S2;

void main()
{
    _S2 = f16tof32_0(C_0._data.u_0);
    return;
}
