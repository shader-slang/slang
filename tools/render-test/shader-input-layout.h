#ifndef SLANG_TEST_SHADER_INPUT_LAYOUT_H
#define SLANG_TEST_SHADER_INPUT_LAYOUT_H

#include "core/basic.h"

#include "render.h"

namespace renderer_test {

using namespace gfx;

enum class ShaderInputType
{
    Buffer, Texture, Sampler, CombinedTextureSampler
};

enum class InputTextureContent
{
    Zero, One, ChessBoard, Gradient
};

struct InputTextureDesc
{
    int dimension = 2;
    int arrayLength = 0;
    bool isCube = false;
    bool isDepthTexture = false;
    bool isRWTexture = false;
    int size = 4;
    InputTextureContent content = InputTextureContent::One;
};

enum class InputBufferType
{
    ConstantBuffer, StorageBuffer
};

struct InputBufferDesc
{
    InputBufferType type = InputBufferType::ConstantBuffer;
    int stride = 0; // stride == 0 indicates an unstructured buffer.
    Format format = Format::Unknown;
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
    bool isOutput = false;
    int hlslBinding = -1;
    Slang::List<int> glslBinding;
};

struct TextureData
{
    Slang::List<Slang::List<unsigned int>> dataBuffer;
    int textureSize;
    int mipLevels;
    int arraySize;
};

class ShaderInputLayout
{
public:
    Slang::List<ShaderInputLayoutEntry> entries;
    Slang::List<Slang::String> globalTypeArguments;
    Slang::List<Slang::String> entryPointTypeArguments;
    int numRenderTargets = 1;
    void Parse(const char * source);
};

void generateTextureData(TextureData & output, const InputTextureDesc & desc);

} // namespace render_test

#endif
