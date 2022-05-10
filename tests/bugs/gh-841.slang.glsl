//TEST_IGNORE_FILE:
#version 450

layout(location = 0)
out vec4 _S1;

layout(location = 0)
in vec4 _S2;

flat layout(location = 1)
in uint _S3;

struct RasterVertex_0
{
    vec4 c_0;
    uint u_0;
};

void main()
{
    RasterVertex_0 _S4 = RasterVertex_0(_S2, _S3);
    vec4 result_0 = _S4.c_0;

    vec4 result_1;
    if(bool(_S4.u_0 & uint(1)))
    {
        result_1 = result_0 + 1.0;
    }
    else
    {
        result_1 = result_0;
    }
    _S1 = result_1;
    return;
}
