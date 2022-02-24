//TEST_IGNORE_FILE:

#version 450
#extension GL_ARB_fragment_shader_interlock : require
layout(row_major) uniform;
layout(row_major) buffer;

layout(rgba32f)
layout(binding = 0)
uniform image2D entryPointParams_texture_0;

layout(location = 0)
in vec4 _S1;

layout(location = 0)
out vec4 _S2;

void main()
{
    beginInvocationInterlockARB();

    vec4 _S3 = (imageLoad((entryPointParams_texture_0), ivec2((uvec2(_S1.xy)))));
    imageStore((entryPointParams_texture_0), ivec2((uvec2(_S1.xy))), _S3 + _S1);

    endInvocationInterlockARB();

    _S2 = _S3;
    return;
}
