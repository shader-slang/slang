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
    Buffer,
    Texture,
    Sampler,
    CombinedTextureSampler,
    Array,
    UniformData,
    Object,
    Aggregate,
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
//    ConstantBuffer,
    StorageBuffer,
//    RootConstantBuffer,
};

struct InputBufferDesc
{
    InputBufferType type = InputBufferType::StorageBuffer;
    int stride = 0; // stride == 0 indicates an unstructured buffer.
    Format format = Format::Unknown;
};

struct InputSamplerDesc
{
    bool isCompareSampler = false;
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
    class Val : public Slang::RefObject
    {
    public:
        Val(ShaderInputType kind)
            : kind(kind)
        {}

        ShaderInputType kind;
        bool isOutput = false;
    };
    typedef Slang::RefPtr<Val> ValPtr;

    class TextureVal : public Val
    {
    public:
        TextureVal()
            : Val(ShaderInputType::Texture)
        {}

        InputTextureDesc textureDesc;
    };

    class DataValBase : public Val
    {
    public:
        DataValBase(ShaderInputType kind)
            : Val(kind)
        {}

        Slang::List<unsigned int> bufferData;
    };

    class BufferVal : public DataValBase
    {
    public:
        BufferVal()
            : DataValBase(ShaderInputType::Buffer)
        {}

        InputBufferDesc bufferDesc;
    };

    class DataVal : public DataValBase
    {
    public:
        DataVal()
            : DataValBase(ShaderInputType::UniformData)
        {}
    };

    class SamplerVal : public Val
    {
    public:
        SamplerVal()
            : Val(ShaderInputType::Sampler)
        {}

        InputSamplerDesc samplerDesc;
    };

    class CombinedTextureSamplerVal : public Val
    {
    public:
        CombinedTextureSamplerVal()
            : Val(ShaderInputType::CombinedTextureSampler)
        {}

        Slang::RefPtr<TextureVal> textureVal;
        Slang::RefPtr<SamplerVal> samplerVal;
    };

    struct Field
    {
        Slang::String   name;
        ValPtr          val;
    };
    typedef Field Entry;

    class ParentVal : public Val
    {
    public:
        ParentVal(ShaderInputType kind)
            : Val(kind)
        {}

        virtual void addField(Field const& field) = 0;
    };

    class AggVal : public ParentVal
    {
    public:
        AggVal(ShaderInputType kind = ShaderInputType::Aggregate)
            : ParentVal(kind)
        {}

        Slang::List<Field> fields;

        virtual void addField(Field const& field) override;
    };

    class ObjectVal : public Val
    {
    public:
        ObjectVal()
            : Val(ShaderInputType::Object)
        {}

        Slang::String typeName;
        ValPtr contentVal;
    };

    class ArrayVal : public ParentVal
    {
    public:
        ArrayVal()
            : ParentVal(ShaderInputType::Array)
        {}

        Slang::List<ValPtr> vals;

        virtual void addField(Field const& field) override;
    };

    Slang::RefPtr<AggVal> rootVal;
    Slang::List<Slang::String> globalSpecializationArgs;
    Slang::List<Slang::String> entryPointSpecializationArgs;
    int numRenderTargets = 1;

    Slang::Index findEntryIndexByName(const Slang::String& name) const;

    void parse(Slang::RandomGenerator* rand, const char* source);

        /// Writes a binding, if bindRoot is set, will try to honor the underlying type when outputting. If not will dump as uint32_t hex.
    static SlangResult writeBinding(slang::TypeLayoutReflection* typeLayout, const void* data, size_t sizeInBytes, Slang::WriterHelper writer);
};

void generateTextureDataRGB8(TextureData& output, const InputTextureDesc& desc);
void generateTextureData(TextureData& output, const InputTextureDesc& desc);


} // namespace render_test

#endif
