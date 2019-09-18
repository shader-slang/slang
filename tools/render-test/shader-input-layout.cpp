#include "shader-input-layout.h"
#include "core/slang-token-reader.h"

#include "render.h"

namespace renderer_test
{
    using namespace Slang;

#define SLANG_SCALAR_TYPES(x) \
    x("int", INT32) \
    x("uint", UINT32) \
    x("float", FLOAT32)

    struct TypeInfo
    {
        UnownedStringSlice name;
        SlangScalarType type;
    };

#define SLANG_SCALAR_TYPE_INFO(name, value) { UnownedStringSlice::fromLiteral(name), SLANG_SCALAR_TYPE_##value },
    static const TypeInfo g_scalarTypeInfos[] =
    {
        SLANG_SCALAR_TYPES(SLANG_SCALAR_TYPE_INFO)
    };
#undef SLANG_SCALAR_TYPES
#undef SLANG_SCALAR_TYPE_INFO

    static SlangScalarType _getScalarType(const UnownedStringSlice& slice)
    {
        for (const auto& info : g_scalarTypeInfos)
        {
            if (info.name == slice)
            {
                return info.type;
            }
        }
        return SLANG_SCALAR_TYPE_NONE;
    }

    Index ShaderInputLayout::findEntryIndexByName(const String& name) const
    {
        const Index count = Index(entries.getCount());
        for (Index i = 0; i < count; ++i)
        {
            const auto& entry = entries[i];
            if (entry.name == name)
            {
                return Index(i);
            }
        }
        return -1;
    }

    static bool _isCPUTarget(SlangCompileTarget target)
    {
        switch (target)
        {
            case SLANG_C_SOURCE:
            case SLANG_CPP_SOURCE:
            case SLANG_EXECUTABLE:
            case SLANG_SHARED_LIBRARY:
            case SLANG_HOST_CALLABLE:
            {
                return true;
            }
            default: return false;
        }
    }

    void ShaderInputLayout::updateForTarget(SlangCompileTarget target)
    {
        if (!_isCPUTarget(target))
        {
            int count = int(entries.getCount());
            for (int i = 0; i < count; ++i)
            {
                auto& entry = entries[i];
                if (entry.isCPUOnly)
                {
                    entries.removeAt(i);
                    i--;
                    count--;
                }
            }
        }
    }

    void ShaderInputLayout::parse(RandomGenerator* rand, const char * source)
    {
        entries.clear();
        globalGenericTypeArguments.clear();
        entryPointGenericTypeArguments.clear();
        globalExistentialTypeArguments.clear();
        entryPointExistentialTypeArguments.clear();
        auto lines = Split(source, '\n');
        for (auto & line : lines)
        {
            if (line.startsWith("//TEST_INPUT:"))
            {
                auto lineContent = line.subString(13, line.getLength() - 13);
                TokenReader parser(lineContent);
                try
                {
                    if (parser.LookAhead("type"))
                    {
                        parser.ReadToken();
                        StringBuilder typeExp;
                        while (!parser.IsEnd())
                            typeExp << parser.ReadToken().Content;
                        entryPointGenericTypeArguments.add(typeExp);
                    }
                    else if (parser.LookAhead("global_type"))
                    {
                        parser.ReadToken();
                        StringBuilder typeExp;
                        while (!parser.IsEnd())
                            typeExp << parser.ReadToken().Content;
                        globalGenericTypeArguments.add(typeExp);
                    }
                    else if (parser.LookAhead("globalExistentialType"))
                    {
                        parser.ReadToken();
                        StringBuilder typeExp;
                        while (!parser.IsEnd())
                            typeExp << parser.ReadToken().Content;
                        globalExistentialTypeArguments.add(typeExp);
                    }
                    else if (parser.LookAhead("entryPointExistentialType"))
                    {
                        parser.ReadToken();
                        StringBuilder typeExp;
                        while (!parser.IsEnd())
                            typeExp << parser.ReadToken().Content;
                        entryPointExistentialTypeArguments.add(typeExp);
                    }
                    else
                    {
                        ShaderInputLayoutEntry entry;

                        if (parser.LookAhead("cbuffer"))
                        {
                            entry.type = ShaderInputType::Buffer;
                            entry.bufferDesc.type = InputBufferType::ConstantBuffer;
                        }
                        else if (parser.LookAhead("ubuffer"))
                        {
                            entry.type = ShaderInputType::Buffer;
                            entry.bufferDesc.type = InputBufferType::StorageBuffer;
                        }
                        else if (parser.LookAhead("Texture1D"))
                        {
                            entry.type = ShaderInputType::Texture;
                            entry.textureDesc.dimension = 1;
                        }
                        else if (parser.LookAhead("Texture2D"))
                        {
                            entry.type = ShaderInputType::Texture;
                            entry.textureDesc.dimension = 2;
                        }
                        else if (parser.LookAhead("Texture3D"))
                        {
                            entry.type = ShaderInputType::Texture;
                            entry.textureDesc.dimension = 3;
                        }
                        else if (parser.LookAhead("TextureCube"))
                        {
                            entry.type = ShaderInputType::Texture;
                            entry.textureDesc.dimension = 2;
                            entry.textureDesc.isCube = true;
                        }
                        else if (parser.LookAhead("RWTexture1D"))
                        {
                            entry.type = ShaderInputType::Texture;
                            entry.textureDesc.dimension = 1;
                            entry.textureDesc.isRWTexture = true;
                        }
                        else if (parser.LookAhead("RWTexture2D"))
                        {
                            entry.type = ShaderInputType::Texture;
                            entry.textureDesc.dimension = 2;
                            entry.textureDesc.isRWTexture = true;
                        }
                        else if (parser.LookAhead("RWTexture3D"))
                        {
                            entry.type = ShaderInputType::Texture;
                            entry.textureDesc.dimension = 3;
                            entry.textureDesc.isRWTexture = true;
                        }
                        else if (parser.LookAhead("RWTextureCube"))
                        {
                            entry.type = ShaderInputType::Texture;
                            entry.textureDesc.dimension = 2;
                            entry.textureDesc.isCube = true;
                            entry.textureDesc.isRWTexture = true;
                        }
                        else if (parser.LookAhead("Sampler"))
                        {
                            entry.type = ShaderInputType::Sampler;
                        }
                        else if (parser.LookAhead("Sampler1D"))
                        {
                            entry.type = ShaderInputType::CombinedTextureSampler;
                            entry.textureDesc.dimension = 1;
                        }
                        else if (parser.LookAhead("Sampler2D"))
                        {
                            entry.type = ShaderInputType::CombinedTextureSampler;
                            entry.textureDesc.dimension = 2;
                        }
                        else if (parser.LookAhead("Sampler3D"))
                        {
                            entry.type = ShaderInputType::CombinedTextureSampler;
                            entry.textureDesc.dimension = 3;
                        }
                        else if (parser.LookAhead("SamplerCube"))
                        {
                            entry.type = ShaderInputType::CombinedTextureSampler;
                            entry.textureDesc.dimension = 2;
                            entry.textureDesc.isCube = true;
                        }
                        else if (parser.LookAhead("render_targets"))
                        {
                            numRenderTargets = parser.ReadInt();
                            continue;
                        }
                        parser.ReadToken();
                        // parse options
                        if (parser.LookAhead("("))
                        {
                            parser.Read("(");
                            while (!parser.IsEnd() && !parser.LookAhead(")"))
                            {
                                auto word = parser.ReadWord();
                                if (word == "depth")
                                {
                                    entry.textureDesc.isDepthTexture = true;
                                }
                                else if (word == "depthCompare")
                                {
                                    entry.samplerDesc.isCompareSampler = true;
                                }
                                else if (word == "arrayLength")
                                {
                                    parser.Read("=");
                                    entry.textureDesc.arrayLength = parser.ReadInt();
                                }
                                else if (word == "stride")
                                {
                                    parser.Read("=");
                                    entry.bufferDesc.stride = parser.ReadInt();
                                }
                                else if (word == "size")
                                {
                                    parser.Read("=");
                                    entry.textureDesc.size = parser.ReadInt();
                                }
                                else if (word == "random")
                                {
                                    parser.Read("(");
                                    // Read the type
                                    String type = parser.ReadWord();
                                    SlangScalarType scalarType = _getScalarType(type.getUnownedSlice());
                                    if (scalarType == SLANG_SCALAR_TYPE_NONE)
                                    {
                                        StringBuilder builder;
                                        for (const auto& info : g_scalarTypeInfos)
                                        {
                                            if (builder.getLength() != 0)
                                            {
                                                builder << ", ";
                                            }
                                            builder << info.name;
                                        }

                                        throw TextFormatException("Expecting " + builder + " " + parser.NextToken().Position.Line);
                                    }

                                    parser.Read(",");
                                    const int size = int(parser.ReadUInt());

                                    switch (scalarType)
                                    {
                                        case SLANG_SCALAR_TYPE_INT32:
                                        {
                                            bool hasRange = false;

                                            int32_t minValue = -0x7fffffff - 1;
                                            int32_t maxValue = 0x7fffffff;

                                            if (parser.LookAhead(","))
                                            {
                                                hasRange = true;
                                                parser.ReadToken();
                                                minValue = parser.ReadInt();

                                                if (parser.LookAhead(","))
                                                {
                                                    parser.ReadToken();
                                                    maxValue = parser.ReadInt();
                                                }
                                            }
                                            SLANG_ASSERT(minValue <= maxValue);
                                            maxValue = (maxValue >= minValue) ? maxValue : minValue;

                                            // Generate the data
                                            entry.bufferData.setCount(size);

                                            int32_t* dst = (int32_t*)entry.bufferData.getBuffer();
                                            for (int i = 0; i < size; ++i)
                                            {
                                                dst[i] = hasRange ? rand->nextInt32InRange(minValue, maxValue) : rand->nextInt32();
                                            }
                                            break;
                                        }
                                        case SLANG_SCALAR_TYPE_UINT32:
                                        {
                                            bool hasRange = false;
                                            uint32_t minValue = 0;
                                            uint32_t maxValue = 0xffffffff;

                                            if (parser.LookAhead(","))
                                            {
                                                parser.ReadToken();
                                                minValue = parser.ReadUInt();

                                                hasRange = true;

                                                if (parser.LookAhead(","))
                                                {
                                                    parser.ReadToken();
                                                    maxValue = parser.ReadUInt();
                                                }
                                            }

                                            SLANG_ASSERT(minValue <= maxValue);
                                            maxValue = (maxValue >= minValue) ? maxValue : minValue;

                                            // Generate the data
                                            entry.bufferData.setCount(size);

                                            uint32_t* dst = (uint32_t*)entry.bufferData.getBuffer();
                                            for (int i = 0; i < size; ++i)
                                            {
                                                dst[i] = hasRange ? rand->nextUInt32InRange(minValue, maxValue) : rand->nextUInt32();
                                            }

                                            break;
                                        }
                                        case SLANG_SCALAR_TYPE_FLOAT32:
                                        {
                                            float minValue = -1.0f;
                                            float maxValue = 1.0f;
                                            
                                            if (parser.LookAhead(","))
                                            {
                                                parser.ReadToken();
                                                minValue = parser.ReadFloat();

                                                if (parser.LookAhead(","))
                                                {
                                                    parser.ReadToken();
                                                    maxValue = parser.ReadFloat();
                                                }
                                            }

                                            SLANG_ASSERT(minValue <= maxValue);
                                            maxValue = (maxValue >= minValue) ? maxValue : minValue;

                                            // Generate the data
                                            entry.bufferData.setCount(size);

                                            float* dst = (float*)entry.bufferData.getBuffer();
                                            for (int i = 0; i < size; ++i)
                                            {
                                                dst[i] = (rand->nextUnitFloat32() * (maxValue - minValue)) + minValue;
                                            }
                                            break;
                                        }
                                    }

                                    // Read the range

                                    parser.Read(")");
                                }
                                else if (word == "data")
                                {
                                    parser.Read("=");

                                    parser.Read("[");
                                    while (!parser.IsEnd() && !parser.LookAhead("]"))
                                    {
                                        bool negate = false;
                                        if(parser.NextToken().Type == TokenType::OpSub)
                                        {
                                            parser.ReadToken();
                                            negate = true;
                                        }

                                        if (parser.NextToken().Type == TokenType::IntLiteral)
                                        {
                                            uint32_t val = parser.ReadUInt();
                                            if(negate) val = uint32_t(-int32_t(val));
                                            entry.bufferData.add(val);
                                        }
                                        else
                                        {
                                            auto floatNum = parser.ReadFloat();
                                            if(negate) floatNum = -floatNum;
                                            entry.bufferData.add(*(unsigned int*)&floatNum);
                                        }
                                    }
                                    parser.Read("]");
                                }
                                else if (word == "content")
                                {
                                    parser.Read("=");
                                    auto contentWord = parser.ReadWord();
                                    if (contentWord == "zero")
                                        entry.textureDesc.content = InputTextureContent::Zero;
                                    else if (contentWord == "one")
                                        entry.textureDesc.content = InputTextureContent::One;
                                    else if (contentWord == "chessboard")
                                        entry.textureDesc.content = InputTextureContent::ChessBoard;
                                    else
                                        entry.textureDesc.content = InputTextureContent::Gradient;
                                }
                                else if(word == "format")
                                {
                                    Format format = Format::Unknown;

                                    parser.Read("=");
                                    auto formatWord = parser.ReadWord();
                                    if(formatWord == "R_UInt32")
                                    {
                                        format = Format::R_UInt32;
                                    }
                                    else if (formatWord == "R_Float32")
                                    {
                                        format = Format::R_Float32;
                                    }
                                    else if (formatWord == "RGBA_Unorm_UInt8")
                                    {
                                        format = Format::RGBA_Unorm_UInt8;
                                    }

                                    entry.textureDesc.format = format;
                                    entry.bufferDesc.format = format;
                                }
                                if (parser.LookAhead(","))
                                    parser.Read(",");
                                else
                                    break;
                            }
                            parser.Read(")");
                        }
                        // parse bindings
                        if (parser.LookAhead(":"))
                        {
                            parser.Read(":");
                            while (!parser.IsEnd())
                            {
                                if (parser.LookAhead("isCPUOnly"))
                                {
                                    entry.isCPUOnly = true;
                                    parser.ReadToken();
                                }
                                else if (parser.LookAhead("dxbinding"))
                                {
                                    parser.ReadToken();
                                    parser.Read("(");
                                    entry.hlslBinding = parser.ReadInt();
                                    parser.Read(")");
                                }
                                else if (parser.LookAhead("glbinding"))
                                {
                                    parser.ReadToken();
                                    parser.Read("(");
                                    while (!parser.IsEnd() && !parser.LookAhead(")"))
                                    {
                                        entry.glslBinding.add(parser.ReadInt());
                                        if (parser.LookAhead(","))
                                            parser.Read(",");
                                        else
                                            break;
                                    }
                                    parser.Read(")");
                                }
                                else if (parser.LookAhead("out"))
                                {
                                    parser.ReadToken();
                                    entry.isOutput = true;
                                }
                                else if (parser.LookAhead("name"))
                                {
                                    parser.ReadToken();

                                    // Optionally consume '=' 
                                    if (parser.NextToken().Type == TokenType::OpAssign)
                                    {
                                        parser.ReadToken();
                                    }

                                    StringBuilder builder;

                                    Token nameToken = parser.ReadToken();
                                    if (nameToken.Type != TokenType::Identifier)
                                    {
                                        throw TextFormatException("Invalid input syntax at line " + parser.NextToken().Position.Line);
                                    }
                                    builder << nameToken.Content;

                                    while (!parser.IsEnd())
                                    {
                                        Token token = parser.NextToken(0);

                                        if (token.Type == TokenType::LBracket)
                                        {
                                            parser.ReadToken();
                                            int index = parser.ReadInt();
                                            SLANG_ASSERT(index >= 0);
                                            parser.ReadMatchingToken(TokenType::RBracket);

                                            builder << "[" << index << "]";
                                        }
                                        else if (token.Type == TokenType::Dot)
                                        {
                                            parser.ReadToken();
                                            Token identifierToken = parser.ReadMatchingToken(TokenType::Identifier);

                                            builder << "." << identifierToken.Content; 
                                        }
                                        else if (token.Type == TokenType::Comma)
                                        {
                                            // Break out
                                            break;
                                        }
                                        else
                                        {
                                            throw TextFormatException("Invalid input syntax at line " + parser.NextToken().Position.Line);
                                        }
                                    }

                                    entry.name = builder;
                                }

                                if (parser.LookAhead(","))
                                    parser.Read(",");
                            }
                        }
                        entries.add(entry);
                    }
                }
                catch (TextFormatException)
                {
                    throw TextFormatException("Invalid input syntax at line " + parser.NextToken().Position.Line);
                }
            }
        }
    }

    void generateTextureData(TextureData& output, const InputTextureDesc& desc)
    {
        switch (desc.format)
        {
            case Format::RGBA_Unorm_UInt8:
            {
                generateTextureDataRGB8(output, desc);
                break;
            }
            case Format::R_Float32:
            {
                TextureData work;
                generateTextureDataRGB8(work, desc);

                output.textureSize = work.textureSize;
                output.mipLevels = work.mipLevels;
                output.arraySize = work.arraySize;

                List<List<unsigned int>>& dstBuffer = output.dataBuffer;

                Index numMips = work.dataBuffer.getCount();
                dstBuffer.setCount(numMips);

                for (int i = 0; i < numMips; ++i)
                {
                    const Index numPixels = work.dataBuffer[i].getCount();
                    const unsigned int* srcPixels = work.dataBuffer[i].getBuffer();

                    dstBuffer[i].setCount(numPixels);

                    float* dstPixels = (float*)dstBuffer[i].getBuffer();

                    for (Index j = 0; j < numPixels; ++j)
                    {
                        // Copy out red
                        const unsigned int srcPixel = srcPixels[j];
                        const float value = (srcPixel & 0xff) * 1.0f / 255;
                        dstPixels[j] = value;
                    }
                }
                break;
            }
            default:
            {
                SLANG_ASSERT(!"Unhandled format");
                break;
            }
        }
    }

    template <typename F>
    void _iteratePixels(int dimension, int size, unsigned int * buffer, F f)
    {
        if (dimension == 1)
            for (int i = 0; i < size; i++)
                buffer[i] = f(i, 0, 0);
        else if (dimension == 2)
            for (int i = 0; i < size; i++)
                for (int j = 0; j < size; j++)
                    buffer[i*size + j] = f(j, i, 0);
        else if (dimension == 3)
            for (int i = 0; i < size; i++)
                for (int j = 0; j < size; j++)
                    for (int k = 0; k < size; k++)
                        buffer[i*size*size + j * size + k] = f(k, j, i);
    };

    void generateTextureDataRGB8(TextureData& output, const InputTextureDesc& inputDesc)
    {
        int arrLen = inputDesc.arrayLength;
        if (arrLen == 0)
            arrLen = 1;
        List<List<unsigned int>>& dataBuffer = output.dataBuffer;
        int arraySize = arrLen;
        if (inputDesc.isCube)
            arraySize *= 6;
        output.arraySize = arraySize;
        output.textureSize = inputDesc.size;
        output.mipLevels = Math::Log2Floor(output.textureSize) + 1;
        output.dataBuffer.setCount(output.mipLevels * output.arraySize);

        int slice = 0;
        for (int i = 0; i < arraySize; i++)
        {
            for (int j = 0; j < output.mipLevels; j++)
            {
                int size = output.textureSize >> j;
                int bufferLen = size;
                if (inputDesc.dimension == 2)
                    bufferLen *= size;
                else if (inputDesc.dimension == 3)
                    bufferLen *= size*size;
                dataBuffer[slice].setCount(bufferLen);

                _iteratePixels(inputDesc.dimension, size, dataBuffer[slice].getBuffer(), [&](int x, int y, int z) -> unsigned int
                {
                    if (inputDesc.content == InputTextureContent::Zero)
                    {
                        return 0x0;
                    }
                    else if (inputDesc.content == InputTextureContent::One)
                    {
                        return 0xFFFFFFFF;
                    }
                    else if (inputDesc.content == InputTextureContent::Gradient)
                    {
                        unsigned char r = (unsigned char)(x / (float)(size - 1) * 255.0f);
                        unsigned char g = (unsigned char)(y / (float)(size - 1) * 255.0f);
                        unsigned char b = (unsigned char)(z / (float)(size - 1) * 255.0f);
                        return 0xFF000000 + r + (g << 8) + (b << 16);
                    }
                    else if (inputDesc.content == InputTextureContent::ChessBoard)
                    {
                        unsigned int xSig = x < (size >> 1) ? 1 : 0;
                        unsigned int ySig = y < (size >> 1) ? 1 : 0;
                        unsigned int zSig = z < (size >> 1) ? 1 : 0;
                        auto sig = xSig ^ ySig ^ zSig;
                        if (sig)
                            return 0xFFFFFFFF;
                        else
                            return 0xFF808080;
                    }
                    return 0x0;
                });
                slice++;
            }
        }
    }
}
