#define _CRT_SECURE_NO_WARNINGS 1

#include "cpu-compute-util.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-std-writers.h"
#include "../../source/core/slang-token-reader.h"

#include "bind-location.h"

#define SLANG_PRELUDE_NAMESPACE CPPPrelude
#include "../../prelude/slang-cpp-types.h"

namespace renderer_test {
using namespace Slang;

template <int COUNT>
struct ValueTextureCube : public CPUComputeUtil::Resource, public CPPPrelude::ITextureCube
{
    void set(void* out)
    {
        float* dst = (float*)out;
        for (int i = 0; i < COUNT; ++i)
        {
            dst[i] = m_value;
        }
    }

    virtual void Sample(CPPPrelude::SamplerState samplerState, const CPPPrelude::float3& loc, void* out) SLANG_OVERRIDE
    {
        set(out);
    }
    virtual void SampleLevel(CPPPrelude::SamplerState samplerState, const CPPPrelude::float3& loc, float level, void* out) SLANG_OVERRIDE
    {
        set(out);
    }

    ValueTextureCube(float value) :
        m_value(value)
    {
        m_interface = static_cast<CPPPrelude::ITextureCube*>(this);
    }

    float m_value;
};

template <int COUNT>
struct ValueTexture3D : public CPUComputeUtil::Resource, public CPPPrelude::ITexture3D
{
    void set(void* out)
    {
        float* dst = (float*)out;
        for (int i = 0; i < COUNT; ++i)
        {
            dst[i] = m_value;
        }
    }

    virtual void Load(const CPPPrelude::int4& v, void* out) SLANG_OVERRIDE
    {
        set(out);
    }
    virtual void Sample(CPPPrelude::SamplerState samplerState, const CPPPrelude::float3& loc, void* out) SLANG_OVERRIDE
    {
        set(out);
    }
    virtual void SampleLevel(CPPPrelude::SamplerState samplerState, const CPPPrelude::float3& loc, float level, void* out) SLANG_OVERRIDE
    {
        set(out);
    }

    ValueTexture3D(float value) :
        m_value(value)
    {
        m_interface = static_cast<CPPPrelude::ITexture3D*>(this);
    }

    float m_value;
};

template <int COUNT>
struct ValueTexture2D : public CPUComputeUtil::Resource, public CPPPrelude::ITexture2D
{
    void set(void* out)
    {
        float* dst = (float*)out;
        for (int i = 0; i < COUNT; ++i)
        {
            dst[i] = m_value;
        }
    }

    virtual void Load(const CPPPrelude::int3& v, void* out) SLANG_OVERRIDE
    {
        set(out);
    }
    virtual void Sample(CPPPrelude::SamplerState samplerState, const CPPPrelude::float2& loc, void* out) SLANG_OVERRIDE
    {
        set(out);
    }
    virtual void SampleLevel(CPPPrelude::SamplerState samplerState, const CPPPrelude::float2& loc, float level, void* out) SLANG_OVERRIDE
    {
        set(out);
    }

    ValueTexture2D(float value):
        m_value(value)
    {
        m_interface = static_cast<CPPPrelude::ITexture2D*>(this);
    }

    float m_value;
};

template <int COUNT>
struct ValueTexture1D : public CPUComputeUtil::Resource, public CPPPrelude::ITexture1D
{
    void set(void* out)
    {
        float* dst = (float*)out;
        for (int i = 0; i < COUNT; ++i)
        {
            dst[i] = m_value;
        }
    }

    virtual void Load(const CPPPrelude::int2& v, void* out) SLANG_OVERRIDE
    {
        set(out);
    }
    virtual void Sample(CPPPrelude::SamplerState samplerState, float loc, void* out) SLANG_OVERRIDE
    {
        set(out);
    }
    virtual void SampleLevel(CPPPrelude::SamplerState samplerState, float loc, float level, void* out) SLANG_OVERRIDE
    {
        set(out);
    }

    ValueTexture1D(float value) :
        m_value(value)
    {
        m_interface = static_cast<CPPPrelude::ITexture1D*>(this);
    }

    float m_value;
};



static CPUComputeUtil::Resource* _newValueTexture(SlangResourceShape baseShape, int elemCount, float value)
{
    switch (baseShape)
    {
        case SLANG_TEXTURE_1D:
        {
            switch (elemCount)
            {
                case 1: return new ValueTexture1D<1>(value);
                case 2: return new ValueTexture1D<2>(value);
                case 3: return new ValueTexture1D<3>(value);
                case 4: return new ValueTexture1D<4>(value);
                default: break;
            }
            break;
        }
        case SLANG_TEXTURE_2D:
        {
            switch (elemCount)
            {
                case 1: return new ValueTexture2D<1>(value);
                case 2: return new ValueTexture2D<2>(value);
                case 3: return new ValueTexture2D<3>(value);
                case 4: return new ValueTexture2D<4>(value);
                default: break;
            }
        }
        case SLANG_TEXTURE_3D:
        {
            switch (elemCount)
            {
                case 1: return new ValueTexture3D<1>(value);
                case 2: return new ValueTexture3D<2>(value);
                case 3: return new ValueTexture3D<3>(value);
                case 4: return new ValueTexture3D<4>(value);
                default: break;
            }
        }
        case SLANG_TEXTURE_CUBE:
        {
            switch (elemCount)
            {
                case 1: return new ValueTextureCube<1>(value);
                case 2: return new ValueTextureCube<2>(value);
                case 3: return new ValueTextureCube<3>(value);
                case 4: return new ValueTextureCube<4>(value);
                default: break;
            }
        }
        default: break;
    }
    return nullptr;
}

/* static */SlangResult CPUComputeUtil::calcBindings(const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout, Context& outContext)
{
    auto request = compilationAndLayout.output.request;
    auto reflection = (slang::ShaderReflection*) spGetReflection(request);

    const auto& sourcePath = compilationAndLayout.sourcePath;

    outContext.m_bindRoot.init(&outContext.m_bindSet, reflection, 0);

    // This will set up constant buffer that are contained from the roots
    outContext.m_bindRoot.addDefaultValues();

    // Okay lets iterate adding buffers
    auto outStream = StdWriters::getOut();
    SLANG_RETURN_ON_FAIL(ShaderInputLayout::addBindSetValues(compilationAndLayout.layout.entries, compilationAndLayout.sourcePath, outStream, outContext.m_bindRoot));
    ShaderInputLayout::getValueBuffers(compilationAndLayout.layout.entries, outContext.m_bindSet, outContext.m_buffers);
    
    // Okay we need to find all of the bindings and match up to those in the layout
    const ShaderInputLayout& layout = compilationAndLayout.layout;

    // The final stage is to actual set up the CPU based variables

    {
        // First create all of the resources for the values
        // We don't need to create anything backed by a buffer on CPU, as the memory buffer as provided
        // by BindSet::Resource can just be used
        {
            const auto& values = outContext.m_bindSet.getValues();

            for (BindSet::Value* value : values)
            {
                auto typeLayout = value->m_type;
                if (typeLayout == nullptr)
                {
                    // We need type layout here to create anything
                    continue;
                }

                // TODO(JS):
                // Here we should be using information about what textures hold to create appropriate
                // textures. For now we only support 2d textures that always return 1.
                const auto kind = typeLayout->getKind();
                switch (kind)
                {
                    case slang::TypeReflection::Kind::Resource:
                    {
                        auto type = typeLayout->getType();
                        auto shape = type->getResourceShape();

                        //auto access = type->getResourceAccess();

                        auto baseShape = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;
                        switch (baseShape)
                        {
                            case SLANG_TEXTURE_1D:
                            case SLANG_TEXTURE_2D:
                            case SLANG_TEXTURE_3D:
                            case SLANG_TEXTURE_CUBE:
                            {
                                SLANG_ASSERT(value->m_userIndex >= 0);
                                auto& srcEntry = layout.entries[value->m_userIndex];

                                // TODO(JS): Currently we support only textures who's content is either
                                // 0 or 1. This is because this is easy to implement.
                                // Will need to do something better in the future..

                                slang::TypeReflection* typeReflection = typeLayout->getResourceResultType();

                                int count = 1;
                                if (typeReflection->getKind() == slang::TypeReflection::Kind::Vector)
                                {
                                    count = int(typeReflection->getElementCount());
                                }

                                switch (srcEntry.textureDesc.content)
                                {
                                    case InputTextureContent::One:
                                    {
                                        value->m_target = _newValueTexture(baseShape, count, 1.0f);
                                        break;                                        
                                    }
                                    case InputTextureContent::Zero:
                                    {
                                        value->m_target = _newValueTexture(baseShape, count, 0.0f);
                                        break;
                                    }
                                    default: break;
                                }

                                if (value->m_target == nullptr)
                                {
                                    SLANG_ASSERT(!"Couldn't construct resource type");
                                    return SLANG_FAIL;
                                }

                                break;
                            }
                            case SLANG_TEXTURE_BUFFER:
                            {
                                // Need a CPU impl for these...
                                // For now we can just leave as target will just be nullptr
                                break;
                            }

                            case SLANG_BYTE_ADDRESS_BUFFER:
                            case SLANG_STRUCTURED_BUFFER:
                            {
                                // On CPU we just use the memory in the BindSet buffer, so don't need to create anything
                                break;
                            }

                        }
                    }
                    default: break;
                }
            }
        }

        // Now we need to go through all of the bindings and set the appropriate data
        {
            List<BindLocation> locations;
            List<BindSet::Value*> values;
            outContext.m_bindSet.getBindings(locations, values);

            for (Index i = 0; i < locations.getCount(); ++i)
            {
                const auto& location = locations[i];
                BindSet::Value* value = values[i];

                // Okay now we need to set up the actual handles that CPU will follow.
                auto typeLayout = location.getTypeLayout();

                const auto kind = typeLayout->getKind();
                switch (kind)
                {
                    case slang::TypeReflection::Kind::Array:
                    {
                        auto elementCount = int(typeLayout->getElementCount());
                        if (elementCount == 0)
                        {
                            CPPPrelude::Array<uint8_t>* array = location.getUniform<CPPPrelude::Array<uint8_t> >();

                            // If set, we setup the data needed for array on CPU side
                            if (value && array)
                            {
                                array->data = value->m_data;
                                array->count = value->m_elementCount;
                            }
                        }
                        break;
                    }
                    case slang::TypeReflection::Kind::ConstantBuffer:
                    case slang::TypeReflection::Kind::ParameterBlock:
                    {
                        // These map down to pointers. In our case the contents of the resource
                        void* data = value ? value->m_data : nullptr;
                        location.setUniform(&data, sizeof(data));
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
                            case SLANG_TEXTURE_1D:
                            case SLANG_TEXTURE_2D:
                            case SLANG_TEXTURE_3D:
                            case SLANG_TEXTURE_CUBE:
                            case SLANG_TEXTURE_BUFFER:
                            {
                                Resource* targetResource = value ? static_cast<Resource*>(value->m_target.Ptr()) : nullptr;
                                void* intf = targetResource ? targetResource->getInterface() : nullptr;
                                *location.getUniform<void*>() = intf;
                                break;
                            }
                            case SLANG_STRUCTURED_BUFFER:
                            {
                                if (value)
                                {
                                    auto& dstBuf = *location.getUniform<CPPPrelude::StructuredBuffer<uint8_t> >();
                                    dstBuf.data = (uint8_t*)value->m_data;
                                    dstBuf.count = value->m_elementCount;
                                }
                                break;
                            }
                            case SLANG_BYTE_ADDRESS_BUFFER:
                            {
                                if (value)
                                {
                                    auto& dstBuf = *location.getUniform<CPPPrelude::ByteAddressBuffer>();
                                    dstBuf.data = (uint32_t*)value->m_data;
                                    dstBuf.sizeInBytes = value->m_sizeInBytes;
                                }
                                break;
                            }
                        }
                    }
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
    out.m_uniformState = (void*)context.m_bindRoot.getRootData();
    out.m_uniformEntryPointParams = (void*)context.m_bindRoot.getEntryPointData();

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
                BindSet::Value* buffer = context.m_buffers[i];
                BindSet::Value* checkBuffer = checkContext.m_buffers[i];

                if (buffer->m_sizeInBytes != checkBuffer->m_sizeInBytes ||
                    ::memcmp(buffer->m_data, checkBuffer->m_data, buffer->m_sizeInBytes) != 0)
                {
                    return SLANG_FAIL;
                }
            }
        }
    }

    return SLANG_OK;
}


} // renderer_test
