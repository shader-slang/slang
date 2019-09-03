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


struct CPUResource : public RefObject
{
    void* getInterface() const { return m_interface; }
    void* m_interface;
};

template <int COUNT>
struct OneTexture2D : public CPUResource, public CPPPrelude::ITexture2D
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

static CPUResource* _newOneTexture2D(int elemCount)
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

    List<RefPtr<CPUResource> > resources;

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
                    throw TextFormatException("Invalid input syntax at line " + parser.NextToken().Position.Line);
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

                            RefPtr<CPUResource> resource = _newOneTexture2D(count);
                            resources.add(resource);

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

/* static */SlangResult CPUComputeUtil::execute(const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout, Context& context)
{
    auto request = compilationAndLayout.output.request;
    auto reflection = (slang::ShaderReflection*) spGetReflection(request);

    ComPtr<ISlangSharedLibrary> sharedLibrary;
    SLANG_RETURN_ON_FAIL(spGetEntryPointHostCallable(request, 0, 0, sharedLibrary.writeRef()));

    // Use reflection to find the entry point name
    
    struct UniformState;
    typedef void(*Func)(CPPPrelude::ComputeVaryingInput* varyingInput, CPPPrelude::UniformEntryPointParams* uniformEntryPointParams, UniformState* uniformState);

    slang::EntryPointReflection* entryPoint = nullptr;
    Func func = nullptr;
    {
        auto entryPointCount = reflection->getEntryPointCount();
        SLANG_ASSERT(entryPointCount == 1);

        entryPoint = reflection->getEntryPointByIndex(0);

        const char* entryPointName = entryPoint->getName();
        func = (Func)sharedLibrary->findFuncByName(entryPointName);

        if (!func)
        {
            return SLANG_FAIL;
        }
    }

    SlangUInt numThreadsPerAxis[3];
    entryPoint->getComputeThreadGroupSize(3, numThreadsPerAxis);

    {
        UniformState* uniformState = (UniformState*)context.binding.m_rootBuffer.m_data;
        CPPPrelude::UniformEntryPointParams* uniformEntryPointParams = (CPPPrelude::UniformEntryPointParams*)context.binding.m_entryPointBuffer.m_data;

        CPPPrelude::ComputeVaryingInput varying;
        varying.groupID = {};

        for (int z = 0; z < int(numThreadsPerAxis[2]); ++z)
        {
            varying.groupThreadID.z = z;
            for (int y = 0; y < int(numThreadsPerAxis[1]); ++y)
            {
                varying.groupThreadID.y = y;
                for (int x = 0; x < int(numThreadsPerAxis[0]); ++x)
                {
                    varying.groupThreadID.x = x;

                    func(&varying, uniformEntryPointParams, uniformState);
                }
            }
        }
    }

    return SLANG_OK;
}



} // renderer_test
