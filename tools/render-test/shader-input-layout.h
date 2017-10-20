#ifndef SLANG_TEST_SHADER_INPUT_LAYOUT_H
#define SLANG_TEST_SHADER_INPUT_LAYOUT_H

#include "core/basic.h"

namespace renderer_test
{
    enum class ShaderInputType
    {
        Buffer, Texture, Sampler, CombinedTextureSampler
    };
    struct InputTextureDesc
    {
        int dimension = 2;
        int arrayLength = 0;
        bool isCube = false;
        bool isDepthTexture = false;
    };
    enum class InputBufferType
    {
        ConstantBuffer, StorageBuffer
    };
    struct InputBufferDesc
    {
        InputBufferType type = InputBufferType::ConstantBuffer;
        int stride = 0; // stride == 0 indicates an unstructured buffer.
    };
    struct InputSamplerDesc
    {
        bool isCompareSampler = false;
    };
    class ShaderInputLayoutEntry
    {
    public:
        ShaderInputType type;
        Slang::List<unsigned int> bufferData;
        InputTextureDesc textureDesc;
        InputBufferDesc bufferDesc;
        InputSamplerDesc samplerDesc;
        int hlslRegister = -1;
        int glslBinding = -1;
        int glslLocation = -1;
    };
    class ShaderInputLayout
    {
    public:
        Slang::List<ShaderInputLayoutEntry> entries;
        void Parse(const char * source);
    };
}

#endif