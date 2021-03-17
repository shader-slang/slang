#ifndef SLANG_TEST_SHADER_INPUT_LAYOUT_H
#define SLANG_TEST_SHADER_INPUT_LAYOUT_H

#include "source/core/slang-basic.h"
#include "source/core/slang-random-generator.h"

#include "source/core/slang-writer.h"

#include "slang-gfx.h"

namespace renderer_test {

using namespace gfx;

enum class ShaderInputType
{
    Buffer, Texture, Sampler, CombinedTextureSampler, Array, Uniform,
    Object,
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
    int mipMapCount = 0;            ///< 0 means the maximum number of mips will be bound

    Format format = Format::RGBA_Unorm_UInt8;            

    InputTextureContent content = InputTextureContent::One;
};

enum class InputBufferType
{
    ConstantBuffer, StorageBuffer,
    RootConstantBuffer,
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

struct ArrayDesc
{
    int size = 0;
};

enum class RTTIDataEntryType
{
    RTTIObject, WitnessTable
};
struct RTTIDataEntry
{
    RTTIDataEntryType type;
    Slang::String typeName;
    Slang::String interfaceName;
    unsigned int offset;
};

struct BindlessHandleDataEntry
{
    unsigned int offset;
    Slang::String name;
};

struct InputObjectDesc
{
    Slang::String typeName;
};

class ShaderInputLayoutEntry
{
public:
    ShaderInputType type;
    Slang::List<unsigned int> bufferData;
    Slang::List<RTTIDataEntry> rttiEntries;
    Slang::List<BindlessHandleDataEntry> bindlessHandleEntry;
    InputTextureDesc textureDesc;
    InputBufferDesc bufferDesc;
    InputSamplerDesc samplerDesc;
    ArrayDesc arrayDesc;
    InputObjectDesc objectDesc;
    bool isOutput = false;
    bool onlyCPULikeBinding = false;        ///< If true, only use on targets that have 'uniform' or 'CPU like' binding, like CPU and CUDA
    bool isBindlessObject = false;          ///< If true, this is a bindless object with no associated binding point in the shader.
    Slang::String name;                     ///< Optional name. Useful for binding through reflection.
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
    Slang::List<Slang::String> globalSpecializationArgs;
    Slang::List<Slang::String> entryPointSpecializationArgs;
    int numRenderTargets = 1;

    Slang::Index findEntryIndexByName(const Slang::String& name) const;

    void updateForTarget(SlangCompileTarget target);

    void parse(Slang::RandomGenerator* rand, const char* source);

        /// Writes a binding, if bindRoot is set, will try to honor the underlying type when outputting. If not will dump as uint32_t hex.
    static SlangResult writeBinding(const ShaderInputLayoutEntry& entry, slang::TypeLayoutReflection* typeLayout, const void* data, size_t sizeInBytes, Slang::WriterHelper writer);
};

void generateTextureDataRGB8(TextureData& output, const InputTextureDesc& desc);
void generateTextureData(TextureData& output, const InputTextureDesc& desc);


} // namespace render_test

#endif
