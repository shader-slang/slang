#ifndef SLANG_TEST_SHADER_INPUT_LAYOUT_H
#define SLANG_TEST_SHADER_INPUT_LAYOUT_H

#include "core/slang-basic.h"
#include "core/slang-random-generator.h"

#include "core/slang-writer.h"

#include "bind-location.h"

#include "render.h"

namespace renderer_test {

using namespace gfx;

enum class ShaderInputType
{
    Buffer, Texture, Sampler, CombinedTextureSampler, Array
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

    Format format = Format::RGBA_Unorm_UInt8;            

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

struct ArrayDesc
{
    int size = 0;
};

class ShaderInputLayoutEntry
{
public:
    ShaderInputType type;
    Slang::List<unsigned int> bufferData;
    InputTextureDesc textureDesc;
    InputBufferDesc bufferDesc;
    InputSamplerDesc samplerDesc;
    ArrayDesc arrayDesc;
    bool isOutput = false;
    bool isCPUOnly = false;

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

        /// Adds to bind set resources as defined in entries.
        /// Note: No actual resources are created on a device, these are just the 'Resource' structures that are held on the BindSet
        /// For buffers, the Resources will be setup with the contents of the entry.
        /// That if a resource is created that maps to an entry, the m_userData member of Resource will be set to it's index
    static SlangResult addBindSetValues(const Slang::List<ShaderInputLayoutEntry>& entries, const Slang::String& sourcePath, Slang::WriterHelper outError, BindRoot& bindRoot);

        /// Put into outBuffer the value buffers that were set via addbindSetValues (which will set m_userIndex to be the entries index)
    static void getValueBuffers(const Slang::List<ShaderInputLayoutEntry>& entries, const BindSet& bindSet, Slang::List<BindSet::Value*>& outBuffers);

        /// Write bindings from values in memory from buffers
    static SlangResult writeBindings(const ShaderInputLayout& layout, const Slang::List<BindSet::Value*>& buffers, const Slang::String& fileName);
};

void generateTextureDataRGB8(TextureData& output, const InputTextureDesc& desc);
void generateTextureData(TextureData& output, const InputTextureDesc& desc);


} // namespace render_test

#endif
