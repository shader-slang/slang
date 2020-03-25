// precise-keyword.slang.glsl
//TEST_IGNORE_FILE:

#version 450

layout(location = 0)
out vec4 _S1;

layout(location = 0)
in vec2 _S2;

void main()
{
    precise float z_0;

    if(_S2.x > float(0))
    {
        float _S3 = _S2.x * _S2.y + _S2.x;
        z_0 = _S3;
    }
    else
    {
        float _S4 = _S2.y * _S2.x + _S2.y;
        z_0 = _S4;
    }
    _S1 = vec4(z_0);
    return;
}