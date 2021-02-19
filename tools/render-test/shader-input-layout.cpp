// Stop warnings from Visual Studio
#define _CRT_SECURE_NO_WARNINGS 1

#include "shader-input-layout.h"
#include "core/slang-token-reader.h"
#include "core/slang-type-text-util.h"

#include "slang-gfx.h"

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

    static bool _isCPULikeBindingTarget(SlangCompileTarget target)
    {
        // CUDA and C++ are 'CPULike' in terms of their binding mechanism
        switch (target)
        {
            case SLANG_C_SOURCE:
            case SLANG_CPP_SOURCE:
            case SLANG_EXECUTABLE:
            case SLANG_SHARED_LIBRARY:
            case SLANG_HOST_CALLABLE:
            case SLANG_CUDA_SOURCE:
            case SLANG_PTX:
            {
                return true;
            }
            default: return false;
        }
    }

    void ShaderInputLayout::updateForTarget(SlangCompileTarget target)
    {
        if (!_isCPULikeBindingTarget(target))
        {
            int count = int(entries.getCount());
            for (int i = 0; i < count; ++i)
            {
                auto& entry = entries[i];
                if (entry.onlyCPULikeBinding)
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
        globalSpecializationArgs.clear();
        entryPointSpecializationArgs.clear();
        auto lines = Split(source, '\n');
        for (auto & line : lines)
        {
            if (line.startsWith("//TEST_INPUT:"))
            {
                auto lineContent = line.subString(13, line.getLength() - 13);
                TokenReader parser(lineContent);
                try
                {
                    if (parser.LookAhead("entryPointSpecializationArg")
                        || parser.LookAhead("type")
                        || parser.LookAhead("entryPointExistentialType"))
                    {
                        parser.ReadToken();
                        StringBuilder typeExp;
                        while (!parser.IsEnd())
                            typeExp << parser.ReadToken().Content;
                        entryPointSpecializationArgs.add(typeExp);
                    }
                    else if (parser.LookAhead("globalSpecializationArg")
                        || parser.LookAhead("global_type")
                        || parser.LookAhead("globalExistentialType"))
                    {
                        parser.ReadToken();
                        StringBuilder typeExp;
                        while (!parser.IsEnd())
                            typeExp << parser.ReadToken().Content;
                        globalSpecializationArgs.add(typeExp);
                    }
                    else
                    {
                        ShaderInputLayoutEntry entry;

                        if (parser.LookAhead("array"))
                        {
                            entry.type = ShaderInputType::Array;
                        }
                        else if (parser.LookAhead("cbuffer"))
                        {
                            entry.type = ShaderInputType::Buffer;
                            entry.bufferDesc.type = InputBufferType::ConstantBuffer;
                        }
                        else if (parser.LookAhead("object"))
                        {
                            entry.type = ShaderInputType::Object;
                        }
                        else if (parser.LookAhead("root_constants"))
                        {
                            entry.type = ShaderInputType::Buffer;
                            entry.bufferDesc.type = InputBufferType::RootConstantBuffer;
                        }
                        else if (parser.LookAhead("Uniform"))
                        {
                            entry.type = ShaderInputType::Uniform;
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
                        else
                        {
                            throw TextFormatException(String("Invalid input syntax at line ") + String(parser.NextToken().Position.Line));
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
                                    auto size = parser.ReadInt();
                                    if (entry.type == ShaderInputType::Array)
                                    {
                                        entry.arrayDesc.size = size;
                                    }
                                    else
                                    {
                                        entry.textureDesc.size = size;
                                    }
                                }
                                else if (word == "random")
                                {
                                    parser.Read("(");
                                    // Read the type
                                    String type = parser.ReadWord();
                                    SlangScalarType scalarType = _getScalarType(type.getUnownedSlice());
                                    if (scalarType == SLANG_SCALAR_TYPE_NONE)
                                    {
                                        StringBuilder scalarTypeNames;
                                        for (const auto& info : g_scalarTypeInfos)
                                        {
                                            if (scalarTypeNames.getLength() != 0)
                                            {
                                                scalarTypeNames << ", ";
                                            }
                                            scalarTypeNames << info.name;
                                        }

                                        throw TextFormatException(StringBuilder() << "Expecting " << scalarTypeNames << " " << parser.NextToken().Position.Line);
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
                                    uint32_t offset = 0;
                                    while (!parser.IsEnd() && !parser.LookAhead("]"))
                                    {
                                        RTTIDataEntry rttiEntry;
                                        if (parser.LookAhead("rtti"))
                                        {
                                            parser.ReadToken();
                                            parser.Read("(");
                                            rttiEntry.type = RTTIDataEntryType::RTTIObject;
                                            rttiEntry.typeName = parser.ReadWord();
                                            rttiEntry.offset = offset;
                                            parser.Read(")");
                                            offset += 8;
                                            entry.rttiEntries.add(rttiEntry);
                                            entry.bufferData.add(0);
                                            entry.bufferData.add(0);
                                            continue;
                                        }
                                        else if (parser.LookAhead("witness"))
                                        {
                                            parser.ReadToken();
                                            parser.Read("(");
                                            rttiEntry.type = RTTIDataEntryType::WitnessTable;
                                            rttiEntry.typeName = parser.ReadWord();
                                            parser.Read(",");
                                            rttiEntry.interfaceName = parser.ReadWord();
                                            rttiEntry.offset = offset;
                                            parser.Read(")");
                                            offset += 8;
                                            entry.rttiEntries.add(rttiEntry);
                                            entry.bufferData.add(0);
                                            entry.bufferData.add(0);
                                            continue;
                                        }
                                        else if (parser.LookAhead("handle"))
                                        {
                                            BindlessHandleDataEntry handleEntry;
                                            parser.ReadToken();
                                            parser.Read("(");
                                            handleEntry.name = parser.ReadWord();
                                            handleEntry.offset = offset;
                                            parser.Read(")");
                                            offset += 8;
                                            entry.bindlessHandleEntry.add(handleEntry);
                                            entry.bufferData.add(0);
                                            entry.bufferData.add(0);
                                            continue;
                                        }

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
                                        offset += 4;
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
                                else if(word == "mipMaps")
                                {
                                    parser.Read("=");
                                    entry.textureDesc.mipMapCount = int(parser.ReadInt());
                                }
                                else if( word == "type" )
                                {
                                    parser.Read("=");
                                    entry.objectDesc.typeName = parser.ReadWord();
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
                                if (parser.LookAhead("onlyCPULikeBinding"))
                                {
                                    entry.onlyCPULikeBinding = true;
                                    parser.ReadToken();
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
                                        throw TextFormatException(StringBuilder() << "Invalid input syntax at line " << parser.NextToken().Position.Line);
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
                                            throw TextFormatException(StringBuilder() << "Invalid input syntax at line " << parser.NextToken().Position.Line);
                                        }
                                    }

                                    entry.name = builder;
                                }
                                else if (parser.LookAhead("bindless"))
                                {
                                    parser.ReadToken();
                                    entry.isBindlessObject = true;
                                }
                                else
                                {
                                    fprintf(stderr, "Invalid TEST_INPUT syntax '%s'\n", parser.NextToken().Content.getBuffer());
                                    break;
                                }

                                if (parser.LookAhead(","))
                                    parser.Read(",");
                            }
                        }
                        entries.add(entry);
                    }
                }
                catch (const TextFormatException&)
                {
                    StringBuilder msg;
                    msg << "Invalid input syntax at line " << parser.NextToken().Position.Line;
                    throw TextFormatException(msg);
                }
            }
        }
    }

    /* static */SlangResult ShaderInputLayout::addBindSetValues(const Slang::List<ShaderInputLayoutEntry>& entries, const String& sourcePath, WriterHelper outStream, BindRoot& bindRoot)
    {
        BindSet* bindSet = bindRoot.getBindSet();
        SLANG_ASSERT(bindSet);

        for (Index entryIndex = 0; entryIndex < entries.getCount(); ++entryIndex)
        {
            auto& entry = entries[entryIndex];
            if (entry.isBindlessObject)
                continue;

            if (entry.name.getLength() == 0)
            {
                outStream.print("No 'name' specified for value in '%s'\n", sourcePath.getBuffer());
                return SLANG_FAIL;
            }

            BindLocation location = BindLocation::Invalid;
            SLANG_RETURN_ON_FAIL(bindRoot.parse(entry.name, sourcePath, outStream, location));

            auto& srcEntry = entries[entryIndex];

            auto typeLayout = location.getTypeLayout();
            const auto kind = typeLayout->getKind();
            switch (kind)
            {
                case slang::TypeReflection::Kind::Array:
                {
                    auto elementCount = int(typeLayout->getElementCount());
                    if (elementCount == 0)
                    {
                        if (srcEntry.type == ShaderInputType::Array)
                        {
                            // Set the size
                            SLANG_RETURN_ON_FAIL(bindRoot.setArrayCount(location, srcEntry.arrayDesc.size));
                        }
                        break;
                    }
                    break;
                }
                case slang::TypeReflection::Kind::Vector:
                case slang::TypeReflection::Kind::Matrix:
                case slang::TypeReflection::Kind::Scalar:
                case slang::TypeReflection::Kind::Struct:
                {
                    SLANG_RETURN_ON_FAIL(location.setUniform(srcEntry.bufferData.getBuffer(), srcEntry.bufferData.getCount() * sizeof(unsigned int)));
                    break;
                }
                default:
                    break;
                case slang::TypeReflection::Kind::ConstantBuffer:
                {
                    SLANG_RETURN_ON_FAIL(bindSet->setBufferContents(location, srcEntry.bufferData.getBuffer(), srcEntry.bufferData.getCount() * sizeof(unsigned int)));
                    break;
                }
                case slang::TypeReflection::Kind::ParameterBlock:
                {
                    auto elementTypeLayout = typeLayout->getElementTypeLayout();
                    SLANG_UNUSED(elementTypeLayout);
                    break;
                }
                case slang::TypeReflection::Kind::TextureBuffer:
                {
                    auto elementTypeLayout = typeLayout->getElementTypeLayout();
                    SLANG_UNUSED(elementTypeLayout);
                    break;
                }
                case slang::TypeReflection::Kind::ShaderStorageBuffer:
                {
                    auto elementTypeLayout = typeLayout->getElementTypeLayout();
                    SLANG_UNUSED(elementTypeLayout);
                    break;
                }
                case slang::TypeReflection::Kind::GenericTypeParameter:
                {
                    const char* name = typeLayout->getName();
                    SLANG_UNUSED(name);
                    break;
                }
                case slang::TypeReflection::Kind::Interface:
                {
                    const char* name = typeLayout->getName();
                    SLANG_UNUSED(name);
                    break;
                }
                case slang::TypeReflection::Kind::Resource:
                {
                    if (BindSet::isTextureType(typeLayout))
                    {
                        // We don't bother setting any data
                        BindSet::Value* value = bindSet->createTextureValue(typeLayout);
                        value->m_userIndex = entryIndex;
                        bindSet->setAt(location, value);
                        break;
                    }

                    auto type = typeLayout->getType();
                    auto shape = type->getResourceShape();

                    //auto access = type->getResourceAccess();

                    switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
                    {
                        default:
                            assert(!"unhandled case");
                            break;
                        case SLANG_BYTE_ADDRESS_BUFFER:
                        case SLANG_STRUCTURED_BUFFER:
                        {
                            size_t bufferSize = srcEntry.bufferData.getCount() * sizeof(unsigned int);

                            BindSet::Value* value = bindSet->createBufferValue(typeLayout, bufferSize, srcEntry.bufferData.getBuffer());
                            SLANG_ASSERT(value);

                            value->m_userIndex = entryIndex;

                            bindSet->setAt(location, value);
                            break;
                        }
                    }
                    if (shape & SLANG_TEXTURE_ARRAY_FLAG)
                    {

                    }
                    if (shape & SLANG_TEXTURE_MULTISAMPLE_FLAG)
                    {

                    }

                    break;
                }
            }
        }

        return SLANG_OK;
    }

    /* static */void ShaderInputLayout::getValueBuffers(const Slang::List<ShaderInputLayoutEntry>& entries, const BindSet& bindSet, List<BindSet::Value*>& outBuffers)
    {
        outBuffers.setCount(entries.getCount());

        for (Index i = 0; i< outBuffers.getCount(); ++i)
        {
            outBuffers[i] = nullptr;
        }

        const auto& values = bindSet.getValues();
        for (BindSet::Value* value : values)
        {
            if (value->m_userIndex >= 0)
            {
                outBuffers[value->m_userIndex] = value;
            }
        }
    }

    /* static */SlangResult ShaderInputLayout::writeBinding(BindRoot* bindRoot, const ShaderInputLayoutEntry& entry, const void* data, size_t sizeInBytes, WriterHelper writer)
    {
        typedef slang::TypeReflection::ScalarType ScalarType;

        slang::TypeReflection::ScalarType scalarType = slang::TypeReflection::ScalarType::None;

        slang::TypeLayoutReflection* typeLayout = nullptr;

        if (bindRoot && entry.name.getLength())
        {
            BindLocation location;
            if (SLANG_SUCCEEDED(bindRoot->parse(entry.name, "", writer, location)))
            {
                // We should have the type of the item
                typeLayout = location.m_typeLayout;
            }
        }

        slang::TypeLayoutReflection* elementTypeLayout = nullptr;

        if (typeLayout)
        {
            switch (typeLayout->getKind())
            {
                
                //case slang::TypeReflection::Kind::Struct:
                case slang::TypeReflection::Kind::Array:
                case slang::TypeReflection::Kind::Matrix:
                case slang::TypeReflection::Kind::Vector:
                {
                    elementTypeLayout = typeLayout->getElementTypeLayout();
                    break;
                }
                case slang::TypeReflection::Kind::Scalar:
                {
                    elementTypeLayout = typeLayout;
                    break;
                }
                case slang::TypeReflection::Kind::Resource:
                {
                    elementTypeLayout = typeLayout->getElementTypeLayout();
                    break;
                }   
                case slang::TypeReflection::Kind::TextureBuffer:
                case slang::TypeReflection::Kind::ShaderStorageBuffer:
                {
                    elementTypeLayout = typeLayout->getElementTypeLayout();
                    break;
                }
            }
        }

        if (elementTypeLayout)
        {
            scalarType = elementTypeLayout->getScalarType();
        }

        if (scalarType != ScalarType::None && scalarType != ScalarType::Void)
        {
            UnownedStringSlice text = TypeTextUtil::getScalarTypeName(scalarType);
            // Write out the type
            writer.put("type: ");
            writer.put(text);
            writer.put("\n");
        }

        switch (scalarType)
        {
            // TODO(JS):
            // Bool is here, because it's not clear across APIs how bool is laid out in memory
            // Float16 is here as we don't have a convert Float16 to float function laying around
            default:
            case ScalarType::None:
            case ScalarType::Void:
            case ScalarType::Bool:
            {
                auto ptr = (const uint32_t*)data;
                const size_t size = sizeInBytes / sizeof(ptr[0]);
                for (size_t i = 0; i < size; ++i)
                {
                    uint32_t v = ptr[i];
                    writer.print("%X\n", v);
                }
                break;
            }
            case ScalarType::Float16:
            {
                auto ptr = (const uint16_t*)data;
                const size_t size = sizeInBytes / sizeof(ptr[0]);
                for (size_t i = 0; i < size; ++i)
                {
                    const float v = HalfToFloat(ptr[i]);
                    writer.print("%f\n", v);
                }
                break;
            }
            case ScalarType::UInt32:
            {
                auto ptr = (const uint32_t*)data;
                const size_t size = sizeInBytes / sizeof(ptr[0]);
                for (size_t i = 0; i < size; ++i)
                {
                    uint32_t v = ptr[i];
                    writer.print("%u\n", v);
                }
                break;
            }
            case ScalarType::Int32:
            {
                auto ptr = (const int32_t*)data;
                const size_t size = sizeInBytes / sizeof(ptr[0]);
                for (size_t i = 0; i < size; ++i)
                {
                    int32_t v = ptr[i];
                    writer.print("%i\n", v);
                }
                break;
            }
            case ScalarType::Int64:
            {
                auto ptr = (const int64_t*)data;
                const size_t size = sizeInBytes / sizeof(ptr[0]);
                for (size_t i = 0; i < size; ++i)
                {
                    int64_t v = ptr[i];
                    writer.print("%" PRId64 "\n", v);
                }
                break;
            }
            case ScalarType::UInt64:
            {
                auto ptr = (const uint64_t*)data;
                const size_t size = sizeInBytes / sizeof(ptr[0]);
                for (size_t i = 0; i < size; ++i)
                {
                    uint64_t v = ptr[i];
                    writer.print("%" PRIu64 "\n", v);
                }
                break;
            }
            case ScalarType::Float32:
            {
                auto ptr = (const float*)data;
                const size_t size = sizeInBytes / sizeof(ptr[0]);
                for (size_t i = 0; i < size; ++i)
                {
                    const float v = ptr[i];
                    writer.print("%f\n", v);
                }
                break;
            }
            case ScalarType::Float64:
            {
                auto ptr = (const double*)data;
                const size_t size = sizeInBytes / sizeof(ptr[0]);
                for (size_t i = 0; i < size; ++i)
                {
                    const double v = ptr[i];
                    writer.print("%f\n", v);
                }
                break;
            }
        }

        return SLANG_OK;
    }

    /* static */SlangResult ShaderInputLayout::writeBindings(BindRoot* bindRoot, const ShaderInputLayout& layout, const List<BindSet::Value*>& buffers, WriterHelper writer)
    {
        const auto& entries = layout.entries;
        for (int i = 0; i < entries.getCount(); ++i)
        {
            const auto& entry = entries[i];
            if (entry.isOutput)
            {
                BindSet::Value* buffer = buffers[i];
                writeBinding(bindRoot, entries[i], buffer->m_data, buffer->m_sizeInBytes, writer);
            }
        }

        return SLANG_OK;
    }

    /* static */SlangResult ShaderInputLayout::writeBindings(BindRoot* bindRoot, const ShaderInputLayout& layout, const List<BindSet::Value*>& buffers, const String& fileName)
    {
        FILE * f = fopen(fileName.getBuffer(), "wb");
        if (!f)
        {
            return SLANG_FAIL;
        }
        FileWriter fileWriter(f, WriterFlags(0));
        return writeBindings(bindRoot, layout, buffers, &fileWriter);
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

        const Index maxMipLevels = Math::Log2Floor(output.textureSize) + 1;
        Index mipLevels = (inputDesc.mipMapCount <= 0) ? maxMipLevels : inputDesc.mipMapCount;
        mipLevels = (mipLevels > maxMipLevels) ? maxMipLevels : mipLevels;

        output.mipLevels = int(mipLevels); 
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
