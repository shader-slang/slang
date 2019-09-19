#define _CRT_SECURE_NO_WARNINGS 1

#include "cpu-compute-util.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-std-writers.h"
#include "../../source/core/slang-token-reader.h"

#define SLANG_PRELUDE_NAMESPACE CPPPrelude
#include "../../prelude/slang-cpp-types.h"

namespace renderer_test {
using namespace Slang;

/* static */SlangResult CPUComputeUtil::writeBindings(const ShaderInputLayout& layout, const List<CPUMemoryBinding::Buffer>& buffers, const String& fileName)
{
    FILE * f = fopen(fileName.getBuffer(), "wb");
    if (!f)
    {
        return SLANG_FAIL;
    }

    const auto& entries = layout.entries;

    for (int i = 0; i < entries.getCount(); ++i)
    {
        const auto& entry = entries[i];
        if (entry.isOutput)
        {
            const auto& buffer = buffers[i];

            unsigned int* ptr = (unsigned int*)buffer.m_data;

            const int size = int(entry.bufferData.getCount());
            // Must be the same size or less than allocated buffer
            SLANG_ASSERT(size * sizeof(unsigned int) <= buffer.m_sizeInBytes);

            for (int i = 0; i < size; ++i)
            {
                unsigned int v = ptr[i];

                fprintf(f, "%X\n", v);
            }
        }
    }
    fclose(f);
    return SLANG_OK;
}


template <int COUNT>
struct OneTexture2D : public CPUComputeUtil::Resource, public CPPPrelude::ITexture2D
{
    void setOne(void* out)
    {
        float* dst = (float*)out;
        for (int i = 0; i < COUNT; ++i)
        {
            dst[i] = 1.0f;
        }
    }

    virtual void Load(const CPPPrelude::int3& v, void* out) SLANG_OVERRIDE
    {
        setOne(out);
    }
    virtual void Sample(CPPPrelude::SamplerState samplerState, const CPPPrelude::float2& loc, void* out) SLANG_OVERRIDE
    {
        setOne(out);
    }
    virtual void SampleLevel(CPPPrelude::SamplerState samplerState, const CPPPrelude::float2& loc, float level, void* out) SLANG_OVERRIDE
    {
        setOne(out);
    }

    OneTexture2D()
    {
        m_interface = static_cast<CPPPrelude::ITexture2D*>(this);
    }
};

static CPUComputeUtil::Resource* _newOneTexture2D(int elemCount)
{
    switch (elemCount)
    {
        case 1: return new OneTexture2D<1>();
        case 2: return new OneTexture2D<2>();
        case 3: return new OneTexture2D<3>();
        case 4: return new OneTexture2D<4>();
        default: return nullptr;
    }
}

/* static */SlangResult CPUComputeUtil::calcBindings(const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout, Context& outContext)
{
    auto request = compilationAndLayout.output.request;
    auto reflection = (slang::ShaderReflection*) spGetReflection(request);

    const auto& sourcePath = compilationAndLayout.sourcePath;

    auto& binding = outContext.binding;

    binding.init(reflection, 0);

    auto& buffers = outContext.buffers;
    buffers.clear();

    // Okay we need to find all of the bindings and match up to those in the layout
    const ShaderInputLayout& layout = compilationAndLayout.layout;

    {
        auto outStream = StdWriters::getOut();
        auto& entries = layout.entries;
        buffers.setCount(entries.getCount());

        for (int entryIndex = 0; entryIndex < entries.getCount(); ++entryIndex)
        {
            auto& entry = entries[entryIndex];

            if (entry.name.getLength() == 0)
            {
                outStream.print("No 'name' specified for resources in '%s'\n", sourcePath.getBuffer());
                return SLANG_FAIL;
            }

            // We will parse the 'name' as may be path to a resource
            TokenReader parser(entry.name);

            CPUMemoryBinding::Location location;

            {
                Token nameToken = parser.ReadToken();
                if (nameToken.Type != TokenType::Identifier)
                {
                    outStream.print("Invalid input syntax at line %d", int(parser.NextToken().Position.Line));
                    return SLANG_FAIL;
                }
                location = binding.find(nameToken.Content.getBuffer());
                if (location.isInvalid())
                {
                    outStream.print("Unable to find entry in '%s' for '%s' (for CPU name must be specified) \n", sourcePath.getBuffer(), entry.name.getBuffer());
                    return SLANG_FAIL;
                }
            }

            while (!parser.IsEnd())
            {
                Token token = parser.NextToken(0);

                if (token.Type == TokenType::LBracket)
                {
                    parser.ReadToken();
                    int index = parser.ReadInt();
                    SLANG_ASSERT(index >= 0);

                    location = location.toIndex(index);
                    if (location.isInvalid())
                    {
                        outStream.print("Unable to find entry in '%d' in '%s'\n", index, entry.name.getBuffer());
                        return SLANG_FAIL;
                    }
                    parser.ReadMatchingToken(TokenType::RBracket);
                }
                else if (token.Type == TokenType::Dot)
                {
                    parser.ReadToken();
                    Token identifierToken = parser.ReadMatchingToken(TokenType::Identifier);

                    location = location.toField(identifierToken.Content.getBuffer());
                    if (location.isInvalid())
                    {
                        outStream.print("Unable to find field '%s' in '%s'\n", identifierToken.Content.getBuffer(), entry.name.getBuffer());
                        return SLANG_FAIL;
                    }
                }
                else if (token.Type == TokenType::Comma)
                {
                    // Break out
                    break;
                }
                else
                {
                    throw TextFormatException(String("Invalid input syntax at line ") + parser.NextToken().Position.Line);
                }
            }

            auto& srcEntry = layout.entries[entryIndex];

            auto typeLayout = location.getTypeLayout();
            const auto kind = typeLayout->getKind();
            switch (kind)
            {
                case slang::TypeReflection::Kind::Vector:
                case slang::TypeReflection::Kind::Matrix:
                case slang::TypeReflection::Kind::Array:
                case slang::TypeReflection::Kind::Scalar:
                case slang::TypeReflection::Kind::Struct:
                {
                    SLANG_RETURN_ON_FAIL(binding.setInplace(location, srcEntry.bufferData.getBuffer(), srcEntry.bufferData.getCount() * sizeof(unsigned int)));
                    break;
                }
                default:
                    break;
                case slang::TypeReflection::Kind::ConstantBuffer:
                {
                    SLANG_RETURN_ON_FAIL(binding.setBufferContents(location, srcEntry.bufferData.getBuffer(), srcEntry.bufferData.getCount() * sizeof(unsigned int)));
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
                    auto type = typeLayout->getType();
                    auto shape = type->getResourceShape();

                    //auto access = type->getResourceAccess();

                    switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
                    {
                        default:
                            assert(!"unhandled case");
                            break;
                        case SLANG_TEXTURE_2D:
                        {
                            slang::TypeReflection* typeReflection = location.getTypeLayout()->getResourceResultType();

                            int count = 1;
                            if (typeReflection->getKind() == slang::TypeReflection::Kind::Vector)
                            {
                                count = int(typeReflection->getElementCount());
                            }

                            RefPtr<Resource> resource = _newOneTexture2D(count);
                            outContext.m_resources.add(resource);

                            SLANG_RETURN_ON_FAIL(binding.setObject(location, resource->getInterface()));
                            break;
                        }
                        case SLANG_TEXTURE_1D:
                        case SLANG_TEXTURE_3D:
                        case SLANG_TEXTURE_CUBE:
                        case SLANG_TEXTURE_BUFFER:
                        {
                            // Just set to null for now
                            SLANG_RETURN_ON_FAIL(binding.setObject(location, nullptr));
                            break;
                        }
                        case SLANG_BYTE_ADDRESS_BUFFER:
                        case SLANG_STRUCTURED_BUFFER:
                        {
                            CPUMemoryBinding::Buffer buffer;
                            SLANG_RETURN_ON_FAIL(binding.setNewBuffer(location, srcEntry.bufferData.getBuffer(), srcEntry.bufferData.getCount() * sizeof(unsigned int), buffer));
                            buffers[entryIndex] = buffer;
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
    }

    return SLANG_OK;
}

/* static */SlangResult CPUComputeUtil::calcExecuteInfo(ExecuteStyle style, ISlangSharedLibrary* sharedLib, const uint32_t dispatchSize[3], const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout, Context& context, ExecuteInfo& out)
{
    auto request = compilationAndLayout.output.request;
    auto reflection = (slang::ShaderReflection*) spGetReflection(request);

    slang::EntryPointReflection* entryPoint = nullptr;
    auto entryPointCount = reflection->getEntryPointCount();
    SLANG_ASSERT(entryPointCount == 1);

    entryPoint = reflection->getEntryPointByIndex(0);

    const char* entryPointName = entryPoint->getName();

    // Copy dispatch size
    for (int i = 0; i < 3; ++i)
    {
        out.m_dispatchSize[i] = dispatchSize[i];
    }

    out.m_style = style;
    out.m_uniformState = (void*)context.binding.m_rootBuffer.m_data;
    out.m_uniformEntryPointParams = (void*)context.binding.m_entryPointBuffer.m_data;

    switch (style)
    {
        case ExecuteStyle::Group:
        {
            StringBuilder groupEntryPointName;
            groupEntryPointName << entryPointName << "_Group";

            CPPPrelude::ComputeFunc groupFunc = (CPPPrelude::ComputeFunc)sharedLib->findFuncByName(groupEntryPointName.getBuffer());
            if (!groupFunc)
            {
                return SLANG_FAIL;
            }

            out.m_func = (ExecuteInfo::Func)groupFunc;
            break;
        }
        case ExecuteStyle::GroupRange:
        {
            CPPPrelude::ComputeFunc groupRangeFunc = nullptr;
            groupRangeFunc = (CPPPrelude::ComputeFunc)sharedLib->findFuncByName(entryPointName);
            if (!groupRangeFunc)
            {
                return SLANG_FAIL;
            }
            out.m_func = (ExecuteInfo::Func)groupRangeFunc;
            break;
        }
        case ExecuteStyle::Thread:
        {
            StringBuilder threadEntryPointName;
            threadEntryPointName << entryPointName << "_Thread";

            CPPPrelude::ComputeThreadFunc threadFunc = (CPPPrelude::ComputeThreadFunc)sharedLib->findFuncByName(threadEntryPointName.getBuffer());
            if (!threadFunc)
            {
                return SLANG_FAIL;
            }

            SlangUInt numThreadsPerAxis[3];
            entryPoint->getComputeThreadGroupSize(3, numThreadsPerAxis);
            for (int i = 0; i < 3; ++i)
            {
                out.m_numThreadsPerAxis[i] = uint32_t(numThreadsPerAxis[i]);
            }
            out.m_func = (ExecuteInfo::Func)threadFunc;
            break;
        }
        default:
        {
            return SLANG_FAIL;
        }
    }

    return SLANG_OK;
}

/* static */SlangResult CPUComputeUtil::execute(const ExecuteInfo& info)
{
    CPPPrelude::UniformState* uniformState = (CPPPrelude::UniformState*)info.m_uniformState;
    CPPPrelude::UniformEntryPointParams* uniformEntryPointParams = (CPPPrelude::UniformEntryPointParams*)info.m_uniformEntryPointParams;

    switch (info.m_style)
    {
        case ExecuteStyle::Group:
        {
            CPPPrelude::ComputeFunc groupFunc = (CPPPrelude::ComputeFunc)info.m_func;
            CPPPrelude::ComputeVaryingInput varying;

            const uint32_t groupXCount = info.m_dispatchSize[0];
            const uint32_t groupYCount = info.m_dispatchSize[1];
            const uint32_t groupZCount = info.m_dispatchSize[2];

            for (uint32_t groupZ = 0; groupZ < groupZCount; ++groupZ)
            {
                for (uint32_t groupY = 0; groupY < groupYCount; ++groupY)
                {
                    for (uint32_t groupX = 0; groupX < groupXCount; ++groupX)
                    {
                        varying.startGroupID = { groupX, groupY, groupZ };
                        groupFunc(&varying, uniformEntryPointParams, uniformState);
                    }
                }
            }
            break;
        }
        case ExecuteStyle::GroupRange:
        {
            CPPPrelude::ComputeFunc groupRangeFunc = (CPPPrelude::ComputeFunc)info.m_func;
            CPPPrelude::ComputeVaryingInput varying;

            varying.startGroupID = {};
            varying.endGroupID = { info.m_dispatchSize[0], info.m_dispatchSize[1], info.m_dispatchSize[2] };

            groupRangeFunc(&varying, uniformEntryPointParams, uniformState);
            break;
        }
        case ExecuteStyle::Thread:
        {
            CPPPrelude::ComputeThreadFunc threadFunc = (CPPPrelude::ComputeThreadFunc)info.m_func;
            CPPPrelude::ComputeThreadVaryingInput varying;

            const uint32_t groupXCount = info.m_dispatchSize[0];
            const uint32_t groupYCount = info.m_dispatchSize[1];
            const uint32_t groupZCount = info.m_dispatchSize[2];

            const uint32_t threadXCount = uint32_t(info.m_numThreadsPerAxis[0]);
            const uint32_t threadYCount = uint32_t(info.m_numThreadsPerAxis[1]);
            const uint32_t threadZCount = uint32_t(info.m_numThreadsPerAxis[2]);

            for (uint32_t groupZ = 0; groupZ < groupZCount; ++groupZ)
            {
                for (uint32_t groupY = 0; groupY < groupYCount; ++groupY)
                {
                    for (uint32_t groupX = 0; groupX < groupXCount; ++groupX)
                    {
                        varying.groupID = { groupX, groupY, groupZ };

                        for (uint32_t z = 0; z < threadZCount; ++z)
                        {
                            varying.groupThreadID.z = z;
                            for (uint32_t y = 0; y < threadYCount; ++y)
                            {
                                varying.groupThreadID.y = y;
                                for (uint32_t x = 0; x < threadXCount; ++x)
                                {
                                    varying.groupThreadID.x = x;

                                    threadFunc(&varying, uniformEntryPointParams, uniformState);
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
        default: return SLANG_FAIL;
    }

    return SLANG_OK;
}


/* static */ SlangResult CPUComputeUtil::checkStyleConsistency(ISlangSharedLibrary* sharedLib, const uint32_t dispatchSize[3], const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout)
{
    Context context;
    SLANG_RETURN_ON_FAIL(CPUComputeUtil::calcBindings(compilationAndLayout, context));

    // Run the thread style to test against
    {
        ExecuteInfo info;
        SLANG_RETURN_ON_FAIL(calcExecuteInfo(ExecuteStyle::Thread, sharedLib, dispatchSize, compilationAndLayout, context, info));
        SLANG_RETURN_ON_FAIL(execute(info));
    }

    ExecuteStyle styles[] = { ExecuteStyle::Group, ExecuteStyle::GroupRange };
    for (auto style: styles)
    {
        Context checkContext;
        SLANG_RETURN_ON_FAIL(CPUComputeUtil::calcBindings(compilationAndLayout, checkContext));

        ExecuteInfo info;
        SLANG_RETURN_ON_FAIL(calcExecuteInfo(style, sharedLib, dispatchSize, compilationAndLayout, checkContext, info));
        SLANG_RETURN_ON_FAIL(execute(info));

        // Make sure the out buffers are all the same

        const auto& entries = compilationAndLayout.layout.entries;

        for (int i = 0; i < entries.getCount(); ++i)
        {
            const auto& entry = entries[i];
            if (entry.isOutput)
            {
                const auto& buffer = context.buffers[i];
                const auto& checkBuffer = checkContext.buffers[i];

                if (buffer.m_sizeInBytes != checkBuffer.m_sizeInBytes ||
                    memcmp(buffer.m_data, checkBuffer.m_data, buffer.m_sizeInBytes) != 0)
                {
                    return SLANG_FAIL;
                }
            }
        }
    }

    return SLANG_OK;
}


} // renderer_test
