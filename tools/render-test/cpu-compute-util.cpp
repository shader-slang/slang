#define _CRT_SECURE_NO_WARNINGS 1

#include "cpu-compute-util.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-std-writers.h"
#include "../../source/core/slang-token-reader.h"

#include "bind-location.h"

#define SLANG_PRELUDE_NAMESPACE CPPPrelude
#include "../../prelude/slang-cpp-types.h"

struct UniformState;

namespace renderer_test {
using namespace Slang;

static void _fixMipSize(uint32_t& ioDim, int mipLevel)
{
    uint32_t dim = ioDim;
    if (dim > 0)
    {
        dim >>= mipLevel;
        dim = (dim == 0) ? 1 : dim;
        ioDim = dim;
    }
}

CPPPrelude::TextureDimensions _calcMipDims(int mipLevel, const CPPPrelude::TextureDimensions& inDims)
{
    if (mipLevel > 0 && mipLevel < int(inDims.numberOfLevels))
    {
        CPPPrelude::TextureDimensions dims(inDims);
        _fixMipSize(dims.width, mipLevel);
        _fixMipSize(dims.height, mipLevel);
        _fixMipSize(dims.depth, mipLevel);
        return dims;
    }
    else
    {
        return inDims;
    }
}

template <int COUNT>
struct ValueTexture : public CPUComputeUtil::Resource, public CPPPrelude::ITexture
{
    // ITexture interface
    virtual CPPPrelude::TextureDimensions GetDimensions(int mipLevel) SLANG_OVERRIDE
    {
        return _calcMipDims(mipLevel, m_dims);
    }
    virtual void Load(const int32_t* loc, void* out, size_t dataSize) SLANG_OVERRIDE
    {
        _set(out);
    }
    virtual void Sample(CPPPrelude::SamplerState samplerState, const float* loc, void* out, size_t dataSize) SLANG_OVERRIDE
    {
        _set(out);
    }
    virtual void SampleLevel(CPPPrelude::SamplerState samplerState, const float* loc, float level, void* out, size_t dataSize) SLANG_OVERRIDE
    {
        _set(out);
    }

    ValueTexture(const CPPPrelude::TextureDimensions& dims, float value) :
        m_value(value),
        m_dims(dims)
    {
        m_interface = static_cast<CPPPrelude::ITexture*>(this);
    }

    void _set(void* out)
    {
        float* dst = (float*)out;
        for (int i = 0; i < COUNT; ++i)
        {
            dst[i] = m_value;
        }
    }

    float m_value;
    CPPPrelude::TextureDimensions m_dims;
};

class FloatTextureData
{
public:
    FloatTextureData() {}
    FloatTextureData(int elementCount, int dimCount, const uint32_t* dims)
    {
        init(elementCount, dimCount, dims);
    }

    void init(int elementCount, int dimCount, const uint32_t* dims)
    {
        SLANG_ASSERT(elementCount >= 1 && elementCount <= 4);
        SLANG_ASSERT(dimCount >= 1 && dimCount < 4);

        Index totalSize = 1;

        for (Index i = 0; i < Index(dimCount); ++i)
        {
            m_dims[i] = (dims[i] <= 0) ? 1 : dims[i];
            totalSize *= m_dims[i];
        }

        m_dimCount = uint8_t(dimCount);
        m_elementCount = uint8_t(elementCount);

        // Set the array to hold the total capacity needed
        m_values.setCount(totalSize);
    }

    void setValue(float value)
    {
        const Index count = m_values.getCount();
        float* dst = m_values.getBuffer();

        for (Index i = 0; i < count; ++i)
        {
            dst[i] = value;
        }
    }

    void setAt(const uint32_t* location, const float* value)
    {
        const Index index = _getIndex(location);
        float* dst = &m_values[index];
        switch (m_elementCount)
        {
            case 1: dst[0] = value[0]; break;
            case 2: dst[0] = value[0]; dst[1] = value[1]; break; 
            case 3: dst[0] = value[0]; dst[1] = value[1]; dst[2] = value[2]; break;
            case 4: dst[0] = value[0]; dst[1] = value[1]; dst[2] = value[2]; dst[3] = value[3]; break;
        }
    }

    float* getAt(const uint32_t* location)
    {
        const Index index = _getIndex(location);
        return &m_values[index];
    }

    void getAt(const uint32_t* location, float* dst)
    {
        const Index index = _getIndex(location);
        float* value = &m_values[index];
        switch (m_elementCount)
        {
            case 1: dst[0] = value[0]; break;
            case 2: dst[0] = value[0]; dst[1] = value[1]; break;
            case 3: dst[0] = value[0]; dst[1] = value[1]; dst[2] = value[2]; break;
            case 4: dst[0] = value[0]; dst[1] = value[1]; dst[2] = value[2]; dst[3] = value[3]; break;
        }
    }

    bool isLocationValid(const uint32_t* location) const
    {
        for (Index i = 0; i < m_dimCount; ++i)
        {
            const auto v = location[i];
            if (v >= m_dims[i])
            {
                return false;
            }
        }
        return true;
    }

    Index _getIndex(const uint32_t* location)
    {
        const auto style = (m_dimCount << 2) | m_elementCount;
        SLANG_ASSERT(isLocationValid(location));
        switch (m_dimCount)
        {
            default:    return 0;
            case 1:     return (location[0] )* m_elementCount;
            case 2:     return (location[0] + location[1] * m_dims[0]) * m_elementCount;
            case 3:     return (location[0] + (location[1] + location[2] * m_dims[1]) * m_dims[0]) * m_elementCount;
            case 4:     return (location[0] + (location[1] + (location[2] + location[3] * m_dims[2]) * m_dims[1]) * m_dims[0]) * m_elementCount;
        }
    }

    uint8_t m_style;
    uint8_t m_elementCount;             ///< Number of elements in each value

    uint8_t m_dimCount;
    uint32_t m_dims[4];                  ///< Sizes in each dimension
    
    List<float> m_values;               ///< Holds the contained data
};

// For a RWTexture we will define it to have memory, and that it can only be accessed via 
struct FloatRWTexture : public CPUComputeUtil::Resource, public CPPPrelude::IRWTexture
{
    // IRWTexture
    virtual CPPPrelude::TextureDimensions GetDimensions(int mipLevel) SLANG_OVERRIDE
    {
        return _calcMipDims(mipLevel, m_dims);
    }
    virtual void Load(const int32_t* loc, void* out, size_t dataSize) SLANG_OVERRIDE { m_data.getAt((const uint32_t*)loc, (float*)out); }
    virtual void* refAt(const uint32_t* loc) SLANG_OVERRIDE { return m_data.getAt(loc); }

    virtual void Sample(CPPPrelude::SamplerState samplerState, const float* loc, void* out, size_t dataSize) SLANG_OVERRIDE
    {}

    virtual void SampleLevel(CPPPrelude::SamplerState samplerState, const float* loc, float level, void* out, size_t dataSize) SLANG_OVERRIDE
    {}

    FloatRWTexture(int elementCount, const CPPPrelude::TextureDimensions& inDims, float initialValue):
        m_dims(inDims)
    {
        uint32_t dimSizes[4];
        int dimSizesCount = inDims.getDimSizes(dimSizes);

        m_data.init(elementCount, dimSizesCount, dimSizes);
        m_data.setValue(initialValue);
        m_interface = static_cast<CPPPrelude::IRWTexture*>(this);
    }

    FloatTextureData m_data;
    CPPPrelude::TextureDimensions m_dims;
};

static int _calcDims(const InputTextureDesc& desc, slang::TypeLayoutReflection* typeLayout, CPPPrelude::TextureDimensions& outDims)
{
    outDims.reset();
    SlangResourceShape shape = SLANG_TEXTURE_2D;
    if (typeLayout)
    {
        const auto kind = typeLayout->getKind();
        SLANG_ASSERT(kind == slang::TypeReflection::Kind::Resource);
        auto type = typeLayout->getType();
        shape = type->getResourceShape();
    }
    else
    {
        if (desc.isCube)
        {
            shape = SLANG_TEXTURE_CUBE;
        }
        else
        {
            switch (desc.dimension)
            {
            case 1:
                shape = SLANG_TEXTURE_1D;
                break;
            case 2:
                shape = SLANG_TEXTURE_2D;
                break;
            case 3:
                shape = SLANG_TEXTURE_3D;
                break;
            default:
                break;
            }
        }
    }
    
    outDims.shape = shape;

    const uint32_t size = uint32_t(desc.size);
    const auto baseShape = (shape & SLANG_RESOURCE_BASE_SHAPE_MASK);

    int dimsCount = 0;

    switch (baseShape)
    {
        case SLANG_TEXTURE_1D:
        {
            outDims.width = size;
            break;
        }
        case SLANG_TEXTURE_2D:
        {
            outDims.width = size;
            outDims.height = size;
            break;
        }
        case SLANG_TEXTURE_3D:
        {
            outDims.width = size;
            outDims.height = size;
            outDims.depth = size;
            break;
        }
        case SLANG_TEXTURE_CUBE:
        {
            outDims.width = size;
            outDims.height = size;
            break;
        }
    }

    if (shape & SLANG_TEXTURE_ARRAY_FLAG)
    {
        outDims.arrayElementCount = uint32_t(desc.arrayLength);
    }

    int maxMipCount = outDims.calcMaxMIPLevels();
    SLANG_ASSERT(desc.mipMapCount <= maxMipCount);

    outDims.numberOfLevels = (desc.mipMapCount == 0) ? uint32_t(maxMipCount) : uint32_t(desc.mipMapCount);
   
    return dimsCount;
}


static CPUComputeUtil::Resource* _newReadTexture(int elemCount, const CPPPrelude::TextureDimensions& dims, float initialValue)
{
    switch (elemCount)
    {
        case 1: return new ValueTexture<1>(dims, initialValue);
        case 2: return new ValueTexture<2>(dims, initialValue);
        case 3: return new ValueTexture<3>(dims, initialValue);
        case 4: return new ValueTexture<4>(dims, initialValue);
        default: break;
    }
    return nullptr;
}

static SlangResult _newTexture(const InputTextureDesc& desc, slang::TypeLayoutReflection* typeLayout, RefPtr<CPUComputeUtil::Resource>& outResource)
{
    SlangResourceAccess access = SLANG_RESOURCE_ACCESS_READ;
    SlangResourceShape shape = SLANG_TEXTURE_2D;
    int elemCount = 1;
    if (typeLayout)
    {
        const auto kind = typeLayout->getKind();
        SLANG_ASSERT(kind == slang::TypeReflection::Kind::Resource);

        auto type = typeLayout->getType();
        shape = type->getResourceShape();

        access = type->getResourceAccess();
        slang::TypeReflection* typeReflection = typeLayout->getResourceResultType();
        if (typeReflection->getKind() == slang::TypeReflection::Kind::Vector)
        {
            elemCount = int(typeReflection->getElementCount());
        }
    }
    else
    {
        if (desc.isCube)
        {
            shape = SLANG_TEXTURE_CUBE;
        }
        else
        {
            switch (desc.dimension)
            {
            case 1:
                shape = SLANG_TEXTURE_1D;
                break;
            case 2:
                shape = SLANG_TEXTURE_2D;
                break;
            case 3:
                shape = SLANG_TEXTURE_3D;
                break;
            default:
                break;
            }
        }
        if (desc.isRWTexture)
            access = SLANG_RESOURCE_ACCESS_READ_WRITE;
        elemCount = 4;
    }

    // TODO(JS): Currently we support only textures who's content is either
    // 0 or 1. This is because this is easy to implement.
    // Will need to do something better in the future..

    float initialValue = 0.0f;

    switch (desc.content)
    {
        case InputTextureContent::One:  initialValue = 1.0f; break;
        case InputTextureContent::Zero: initialValue = 0.0f; break;
        default: break;
    }

    CPPPrelude::TextureDimensions dims;
    _calcDims(desc, typeLayout, dims);

    // These need a different style of texture if can be written to
    if (access == SLANG_RESOURCE_ACCESS_READ_WRITE)
    {
        
        switch (shape)
        {
            case SLANG_TEXTURE_1D:
            case SLANG_TEXTURE_2D:
            case SLANG_TEXTURE_3D:
            case SLANG_TEXTURE_CUBE:
            case SLANG_TEXTURE_1D_ARRAY:
            case SLANG_TEXTURE_2D_ARRAY:
            {
                outResource = new FloatRWTexture(elemCount, dims, initialValue);
                return SLANG_OK;
            }
        }
    }
    else
    {
        outResource = _newReadTexture(elemCount, dims, initialValue);
        return outResource ? SLANG_OK : SLANG_FAIL;
    }

    return SLANG_FAIL;
}

/* static */bool CPUComputeUtil::hasFeature(const UnownedStringSlice& feature)
{
    SLANG_UNUSED(feature);
    // CPU has no specific support requirements
    return false;
}

SlangResult CPUComputeUtil::fillRuntimeHandleInBuffers(
    ShaderCompilerUtil::OutputAndLayout& compilationAndLayout,
    Context& context,
    ISlangSharedLibrary* sharedLib)
{
    auto request = compilationAndLayout.output.getRequestForReflection();
    Slang::ComPtr<slang::ISession> linkage;
    spCompileRequest_getSession(request, linkage.writeRef());
    auto& inputLayout = compilationAndLayout.layout;
    for (auto& entry : inputLayout.entries)
    {
        for (auto& rtti : entry.rttiEntries)
        {
            uint64_t ptrValue = 0;
            switch (rtti.type)
            {
            case RTTIDataEntryType::RTTIObject:
                {
                    auto reflection =
                        slang::ShaderReflection::get(request);
                    auto concreteType = reflection->findTypeByName(rtti.typeName.getBuffer());
                    ComPtr<ISlangBlob> outName;
                    linkage->getTypeRTTIMangledName(concreteType, outName.writeRef());
                    if (!outName)
                        return SLANG_FAIL;
                    ptrValue = (uint64_t)sharedLib->findSymbolAddressByName((char*)outName->getBufferPointer());
                }
                break;
            case RTTIDataEntryType::WitnessTable:
            {
                auto reflection = slang::ShaderReflection::get(request);
                auto concreteType = reflection->findTypeByName(rtti.typeName.getBuffer());
                if (!concreteType)
                    return SLANG_FAIL;
                auto interfaceType = reflection->findTypeByName(rtti.interfaceName.getBuffer());
                if (!interfaceType)
                    return SLANG_FAIL;
                uint32_t id = -1;
                linkage->getTypeConformanceWitnessSequentialID(concreteType, interfaceType, &id);
                ptrValue = id;
                break;
            }
            default:
                break;
            }
            if (rtti.offset >= 0 && rtti.offset + sizeof(ptrValue) <= entry.bufferData.getCount() * sizeof(decltype(entry.bufferData[0])))
            {
                memcpy(
                    ((char*)entry.bufferData.getBuffer()) + rtti.offset,
                    &ptrValue,
                    sizeof(ptrValue));
            }
            else
            {
                return SLANG_FAIL;
            }
        }
        for (auto& handle : entry.bindlessHandleEntry)
        {
            RefPtr<Resource> resource;
            uint64_t handleValue = 0;
            if (context.m_bindlessResources.TryGetValue(handle.name, resource))
            {
                handleValue = (uint64_t)resource->getInterface();
            }
            else
            {
                return SLANG_FAIL;
            }
            if (handle.offset >= 0 &&
                handle.offset + sizeof(uint64_t) <=
                    entry.bufferData.getCount() * sizeof(decltype(entry.bufferData[0])))
            {
                memcpy(
                    ((char*)entry.bufferData.getBuffer()) + handle.offset,
                    &handleValue,
                    sizeof(handleValue));
            }
            else
            {
                return SLANG_FAIL;
            }
        }
    }
    return SLANG_OK;
}

/* static */SlangResult CPUComputeUtil::calcBindings(const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout, Context& outContext)
{
    auto request = compilationAndLayout.output.getRequestForReflection();
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

                        auto access = type->getResourceAccess();

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

                                RefPtr<CPUComputeUtil::Resource> resource;
                                SLANG_RETURN_ON_FAIL(_newTexture(srcEntry.textureDesc, typeLayout, resource));
                                value->m_target = resource;
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
    auto request = compilationAndLayout.output.getRequestForReflection();
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
    void* uniformState = info.m_uniformState;
    void* uniformEntryPointParams = info.m_uniformEntryPointParams;

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

SlangResult renderer_test::CPUComputeUtil::createBindlessResources(
    ShaderCompilerUtil::OutputAndLayout& outputAndLayout, Context& context)
{
    auto outStream = StdWriters::getOut();
    for (auto& entry : outputAndLayout.layout.entries)
    {
        if (!entry.isBindlessObject)
            continue;
        switch (entry.type)
        {
        case ShaderInputType::Texture:
            {
                RefPtr<Resource> resource;
                _newTexture(entry.textureDesc, nullptr, resource);
                context.m_bindlessResources.Add(entry.name, resource);
                break;
            }
        default:
            outStream.print("Unsupported bindless resource type.\n");
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}


} // renderer_test
