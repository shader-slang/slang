// unknown-image-format.slang.glsl
//TEST_IGNORE_FILE:

#version 450
#extension GL_EXT_shader_image_load_formatted : require
layout(row_major) uniform;
layout(row_major) buffer;

struct SLANG_ParameterGroup_C_0
{
    uvec2 index_0;
};

layout(binding = 2)
layout(std140) uniform _S1
{
    uvec2 index_0;
} C_0;

layout(binding = 0)
uniform image2D gNoFormat_0;

layout(r32f)
layout(binding = 1)
uniform image2D gExplicitFormat_0;

layout(binding = 0, set = 1)
uniform image2D gBlock_noFormat_0;

layout(rgba8)
layout(binding = 1, set = 1)
uniform image2D gBlock_explicitFormat_0;

layout(binding = 3)
uniform image2D entryPointParams_noFormat_0;

layout(rgba16f)
layout(binding = 4)
uniform image2D entryPointParams_explicitFormat_0;

layout(location = 0)
out vec4 main_0;

void main()
{

    float _S3 = (imageLoad((gNoFormat_0), ivec2((C_0.index_0))).x);
    vec4 _S4 = vec4(_S3);

    float _S5 = (imageLoad((gExplicitFormat_0), ivec2((C_0.index_0))).x);
    vec4 result_0 = _S4 + _S5;

    vec4 _S6 = (imageLoad((gBlock_noFormat_0), ivec2((C_0.index_0))));
    vec4 result_1 = result_0 + _S6;

    vec4 _S7 = (imageLoad((gBlock_explicitFormat_0), ivec2((C_0.index_0))));
    vec4 result_2 = result_1 + _S7;

    vec4 _S8 = (imageLoad((entryPointParams_noFormat_0), ivec2((C_0.index_0))));
    vec4 result_3 = result_2 + _S8;

    vec4 _S9 = (imageLoad((entryPointParams_explicitFormat_0), ivec2((C_0.index_0))));
    main_0 = result_3 + _S9;

    return;
}
