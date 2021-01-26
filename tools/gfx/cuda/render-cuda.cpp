#include "render-cuda.h"
#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"
#include "core/slang-basic.h"

#include "../renderer-shared.h"
#include "../render-graphics-common.h"

#ifdef GFX_ENABLE_CUDA
#include <cuda.h>
#include <cuda_runtime_api.h>
#include "core/slang-std-writers.h"

#endif

namespace gfx
{
#ifdef GFX_ENABLE_CUDA
using namespace Slang;

SLANG_FORCE_INLINE static bool _isError(CUresult result) { return result != 0; }
SLANG_FORCE_INLINE static bool _isError(cudaError_t result) { return result != 0; }

// A enum used to control if errors are reported on failure of CUDA call.
enum class CUDAReportStyle
{
    Normal,
    Silent,
};

struct CUDAErrorInfo
{
    CUDAErrorInfo(
        const char* filePath,
        int lineNo,
        const char* errorName = nullptr,
        const char* errorString = nullptr)
        : m_filePath(filePath)
        , m_lineNo(lineNo)
        , m_errorName(errorName)
        , m_errorString(errorString)
    {}
    SlangResult handle() const
    {
        StringBuilder builder;
        builder << "Error: " << m_filePath << " (" << m_lineNo << ") :";

        if (m_errorName)
        {
            builder << m_errorName << " : ";
        }
        if (m_errorString)
        {
            builder << m_errorString;
        }

        StdWriters::getError().put(builder.getUnownedSlice());

        // Slang::signalUnexpectedError(builder.getBuffer());
        return SLANG_FAIL;
    }

    const char* m_filePath;
    int m_lineNo;
    const char* m_errorName;
    const char* m_errorString;
};

#    if 1
// If this code path is enabled, CUDA errors will be reported directly to StdWriter::out stream.

static SlangResult _handleCUDAError(CUresult cuResult, const char* file, int line)
{
    CUDAErrorInfo info(file, line);
    cuGetErrorString(cuResult, &info.m_errorString);
    cuGetErrorName(cuResult, &info.m_errorName);
    return info.handle();
}

static SlangResult _handleCUDAError(cudaError_t error, const char* file, int line)
{
    return CUDAErrorInfo(file, line, cudaGetErrorName(error), cudaGetErrorString(error)).handle();
}

#        define SLANG_CUDA_HANDLE_ERROR(x) _handleCUDAError(_res, __FILE__, __LINE__)

#    else
// If this code path is enabled, errors are not reported, but can have an assert enabled

static SlangResult _handleCUDAError(CUresult cuResult)
{
    SLANG_UNUSED(cuResult);
    // SLANG_ASSERT(!"Failed CUDA call");
    return SLANG_FAIL;
}

static SlangResult _handleCUDAError(cudaError_t error)
{
    SLANG_UNUSED(error);
    // SLANG_ASSERT(!"Failed CUDA call");
    return SLANG_FAIL;
}

#        define SLANG_CUDA_HANDLE_ERROR(x) _handleCUDAError(_res)
#    endif

#    define SLANG_CUDA_RETURN_ON_FAIL(x)              \
        {                                             \
            auto _res = x;                            \
            if (_isError(_res))                       \
                return SLANG_CUDA_HANDLE_ERROR(_res); \
        }
#    define SLANG_CUDA_RETURN_WITH_REPORT_ON_FAIL(x, r)                               \
        {                                                                             \
            auto _res = x;                                                            \
            if (_isError(_res))                                                       \
            {                                                                         \
                return (r == CUDAReportStyle::Normal) ? SLANG_CUDA_HANDLE_ERROR(_res) \
                                                      : SLANG_FAIL;                   \
            }                                                                         \
        }

#    define SLANG_CUDA_ASSERT_ON_FAIL(x)           \
        {                                          \
            auto _res = x;                         \
            if (_isError(_res))                    \
            {                                      \
                SLANG_ASSERT(!"Failed CUDA call"); \
            };                                     \
        }

#    ifdef RENDER_TEST_OPTIX

static bool _isError(OptixResult result) { return result != OPTIX_SUCCESS; }

#        if 1
static SlangResult _handleOptixError(OptixResult result, char const* file, int line)
{
    fprintf(
        stderr,
        "%s(%d): optix: %s (%s)\n",
        file,
        line,
        optixGetErrorString(result),
        optixGetErrorName(result));
    return SLANG_FAIL;
}
#            define SLANG_OPTIX_HANDLE_ERROR(RESULT) _handleOptixError(RESULT, __FILE__, __LINE__)
#        else
#            define SLANG_OPTIX_HANDLE_ERROR(RESULT) SLANG_FAIL
#        endif

#        define SLANG_OPTIX_RETURN_ON_FAIL(EXPR)           \
            do                                             \
            {                                              \
                auto _res = EXPR;                          \
                if (_isError(_res))                        \
                    return SLANG_OPTIX_HANDLE_ERROR(_res); \
            } while (0)

void _optixLogCallback(unsigned int level, const char* tag, const char* message, void* userData)
{
    fprintf(stderr, "optix: %s (%s)\n", message, tag);
}

#    endif

class MemoryCUDAResource : public BufferResource
{
public:
    MemoryCUDAResource(const Desc& _desc)
        : BufferResource(_desc)
    {}

    ~MemoryCUDAResource()
    {
        if (m_cudaMemory)
        {
            SLANG_CUDA_ASSERT_ON_FAIL(cudaFree(m_cudaMemory));
        }
    }

    uint64_t getBindlessHandle() { return (uint64_t)m_cudaMemory; }

    void* m_cudaMemory = nullptr;
};

class TextureCUDAResource : public TextureResource
{
public:
    TextureCUDAResource(const TextureResource::Desc& desc)
        : TextureResource(desc)
    {}
    ~TextureCUDAResource()
    {
        if (m_cudaSurfObj)
        {
            SLANG_CUDA_ASSERT_ON_FAIL(cuSurfObjectDestroy(m_cudaSurfObj));
        }
        if (m_cudaTexObj)
        {
            SLANG_CUDA_ASSERT_ON_FAIL(cuTexObjectDestroy(m_cudaTexObj));
        }
        if (m_cudaArray)
        {
            SLANG_CUDA_ASSERT_ON_FAIL(cuArrayDestroy(m_cudaArray));
        }
        if (m_cudaMipMappedArray)
        {
            SLANG_CUDA_ASSERT_ON_FAIL(cuMipmappedArrayDestroy(m_cudaMipMappedArray));
        }
    }

    uint64_t getBindlessHandle() { return (uint64_t)m_cudaTexObj; }

    // The texObject is for reading 'texture' like things. This is an opaque type, that's backed by
    // a long long
    CUtexObject m_cudaTexObj = CUtexObject();

    // The surfObj is for reading/writing 'texture like' things, but not for sampling.
    CUsurfObject m_cudaSurfObj = CUsurfObject();

    CUarray m_cudaArray = CUarray();
    CUmipmappedArray m_cudaMipMappedArray = CUmipmappedArray();
};

class CUDAResourceView : public IResourceView, public RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    IResourceView* getInterface(const Guid& guid)
    {
        if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IResourceView)
            return static_cast<IResourceView*>(this);
        return nullptr;
    }
public:
    Desc desc;
    RefPtr<MemoryCUDAResource> memoryResource = nullptr;
    RefPtr<TextureCUDAResource> textureResource = nullptr;
};

class CUDAProgramLayout;

class CUDAShaderProgram : public IShaderProgram, public RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    IShaderProgram* getInterface(const Guid& guid)
    {
        if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IShaderProgram)
            return static_cast<IShaderProgram*>(this);
        return nullptr;
    }
public:
    CUmodule cudaModule = nullptr;
    CUfunction cudaKernel;
    String kernelName;
    ComPtr<slang::IComponentType> slangProgram;
    RefPtr<CUDAProgramLayout> layout;

    ~CUDAShaderProgram()
    {
        if (cudaModule)
            cuModuleUnload(cudaModule);
    }
};

class CUDAPipelineState : public IPipelineState, public RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    IPipelineState* getInterface(const Guid& guid)
    {
        if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IPipelineState)
            return static_cast<IPipelineState*>(this);
        return nullptr;
    }
public:
    RefPtr<CUDAShaderProgram> shaderProgram;
};

class CUDAShaderObjectLayout : public IShaderObjectLayout, public RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    IShaderObjectLayout* getInterface(const Guid& guid)
    {
        if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IShaderObjectLayout)
            return static_cast<IShaderObjectLayout*>(this);
        return nullptr;
    }
public:
    slang::TypeLayoutReflection* typeLayout = nullptr;

    struct BindingRangeInfo
    {
        slang::BindingType bindingType;
        Index count;
        Index baseIndex; // Flat index for sub-ojects
        Index uniformOffset; // Uniform offset for a resource typed field.
    };

    struct SubObjectRangeInfo
    {
        RefPtr<CUDAShaderObjectLayout> layout;
        Index bindingRangeIndex;
    };

    List<SubObjectRangeInfo> subObjectRanges;
    List<BindingRangeInfo> m_bindingRanges;

    slang::TypeLayoutReflection* unwrapParameterGroups(slang::TypeLayoutReflection* typeLayout)
    {
        for (;;)
        {
            if (!typeLayout->getType())
            {
                if (auto elementTypeLayout = typeLayout->getElementTypeLayout())
                    typeLayout = elementTypeLayout;
            }

            switch (typeLayout->getKind())
            {
            default:
                return typeLayout;

            case slang::TypeReflection::Kind::ConstantBuffer:
            case slang::TypeReflection::Kind::ParameterBlock:
                typeLayout = typeLayout->getElementTypeLayout();
                continue;
            }
        }
    }

    CUDAShaderObjectLayout(slang::TypeLayoutReflection* layout)
    {
        Index subObjectCount = 0;

        typeLayout = unwrapParameterGroups(layout);

        // Compute the binding ranges that are used to store
        // the logical contents of the object in memory. These will relate
        // to the descriptor ranges in the various sets, but not always
        // in a one-to-one fashion.

        SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();
        for (SlangInt r = 0; r < bindingRangeCount; ++r)
        {
            slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
            SlangInt count = typeLayout->getBindingRangeBindingCount(r);
            slang::TypeLayoutReflection* slangLeafTypeLayout =
                typeLayout->getBindingRangeLeafTypeLayout(r);

            SlangInt descriptorSetIndex = typeLayout->getBindingRangeDescriptorSetIndex(r);
            SlangInt rangeIndexInDescriptorSet =
                typeLayout->getBindingRangeFirstDescriptorRangeIndex(r);

            auto uniformOffset = typeLayout->getDescriptorSetDescriptorRangeIndexOffset(
                descriptorSetIndex, rangeIndexInDescriptorSet);

            Index baseIndex = 0;
            switch (slangBindingType)
            {
            case slang::BindingType::ConstantBuffer:
            case slang::BindingType::ParameterBlock:
            case slang::BindingType::ExistentialValue:
                baseIndex = subObjectCount;
                subObjectCount += count;
                break;

            default:
                break;
            }

            BindingRangeInfo bindingRangeInfo;
            bindingRangeInfo.bindingType = slangBindingType;
            bindingRangeInfo.count = count;
            bindingRangeInfo.baseIndex = baseIndex;
            bindingRangeInfo.uniformOffset = uniformOffset;
            m_bindingRanges.add(bindingRangeInfo);
        }

        SlangInt subObjectRangeCount = typeLayout->getSubObjectRangeCount();
        for (SlangInt r = 0; r < subObjectRangeCount; ++r)
        {
            SlangInt bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(r);
            auto slangBindingType = typeLayout->getBindingRangeType(bindingRangeIndex);
            slang::TypeLayoutReflection* slangLeafTypeLayout =
                typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);

            // A sub-object range can either represent a sub-object of a known
            // type, like a `ConstantBuffer<Foo>` or `ParameterBlock<Foo>`
            // (in which case we can pre-compute a layout to use, based on
            // the type `Foo`) *or* it can represent a sub-object of some
            // existential type (e.g., `IBar`) in which case we cannot
            // know the appropraite type/layout of sub-object to allocate.
            //
            RefPtr<CUDAShaderObjectLayout> subObjectLayout;
            if (slangBindingType != slang::BindingType::ExistentialValue)
            {
                subObjectLayout =
                    new CUDAShaderObjectLayout(slangLeafTypeLayout->getElementTypeLayout());
            }

            SubObjectRangeInfo subObjectRange;
            subObjectRange.bindingRangeIndex = bindingRangeIndex;
            subObjectRange.layout = subObjectLayout;
            subObjectRanges.add(subObjectRange);
        }
    }
};

class CUDAProgramLayout : public CUDAShaderObjectLayout
{
public:
    slang::ProgramLayout* programLayout = nullptr;
    List<RefPtr<CUDAShaderObjectLayout>> entryPointLayouts;
    CUDAProgramLayout(slang::ProgramLayout* inProgramLayout)
        : CUDAShaderObjectLayout(inProgramLayout->getGlobalParamsTypeLayout())
        , programLayout(inProgramLayout)
    {
        for (UInt i =0; i< programLayout->getEntryPointCount(); i++)
        {
            entryPointLayouts.add(new CUDAShaderObjectLayout(
                programLayout->getEntryPointByIndex(i)->getTypeLayout()));
        }

    }

    int getKernelIndex(UnownedStringSlice kernelName)
    {
        for (int i = 0; i < (int)programLayout->getEntryPointCount(); i++)
        {
            auto entryPoint = programLayout->getEntryPointByIndex(i);
            if (kernelName == entryPoint->getName())
            {
                return i;
            }
        }
        return -1;
    }

    void getKernelThreadGroupSize(int kernelIndex, UInt* threadGroupSizes)
    {
        auto entryPoint = programLayout->getEntryPointByIndex(kernelIndex);
        entryPoint->getComputeThreadGroupSize(3, threadGroupSizes);
    }
};

class CUDAShaderObject : public IShaderObject, public RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    IShaderObject* getInterface(const Guid& guid)
    {
        if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IShaderObject)
            return static_cast<IShaderObject*>(this);
        return nullptr;
    }

public:
    RefPtr<MemoryCUDAResource> bufferResource;
    RefPtr<CUDAShaderObjectLayout> layout;
    List<RefPtr<CUDAShaderObject>> objects;
    List<RefPtr<CUDAResourceView>> resources;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        init(IRenderer* renderer, CUDAShaderObjectLayout* typeLayout);

    virtual SLANG_NO_THROW Result SLANG_MCALL initBuffer(IRenderer* renderer, size_t bufferSize)
    {
        BufferResource::Desc bufferDesc;
        bufferDesc.init(bufferSize);
        bufferDesc.cpuAccessFlags |= IResource::AccessFlag::Write;
        ComPtr<IBufferResource> constantBuffer;
        SLANG_RETURN_ON_FAIL(renderer->createBufferResource(
            IResource::Usage::ConstantBuffer, bufferDesc, nullptr, constantBuffer.writeRef()));
        bufferResource = dynamic_cast<MemoryCUDAResource*>(constantBuffer.get());
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL getBuffer()
    {
        return bufferResource ? bufferResource->m_cudaMemory : nullptr;
    }

    virtual SLANG_NO_THROW size_t SLANG_MCALL getBufferSize()
    {
        return bufferResource ? bufferResource->getDesc()->sizeInBytes : 0;
    }

    virtual SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout() override
    {
        return layout->typeLayout;
    }

    virtual SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() override { return 0; }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(UInt index, IShaderObject** outEntryPoint) override
    {
        *outEntryPoint = nullptr;
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setData(ShaderOffset const& offset, void const* data, size_t size)
    {
        size = Math::Min(size, bufferResource->getDesc()->sizeInBytes - offset.uniformOffset);
        SLANG_CUDA_RETURN_ON_FAIL(cudaMemcpy(
            (uint8_t*)bufferResource->m_cudaMemory + offset.uniformOffset,
            data,
            size,
            cudaMemcpyHostToDevice));
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getObject(ShaderOffset const& offset, IShaderObject** object)
    {
        auto subObjectIndex =
            layout->m_bindingRanges[offset.bindingRangeIndex].baseIndex + offset.bindingArrayIndex;
        if (subObjectIndex >= objects.getCount())
        {
            *object = nullptr;
            return SLANG_OK;
        }
        objects[subObjectIndex]->addRef();
        *object = objects[subObjectIndex].Ptr();
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setObject(ShaderOffset const& offset, IShaderObject* object)
    {
        auto subObjectIndex =
            layout->m_bindingRanges[offset.bindingRangeIndex].baseIndex + offset.bindingArrayIndex;
        SLANG_ASSERT(
            offset.uniformOffset ==
            layout->m_bindingRanges[offset.bindingRangeIndex].uniformOffset +
                offset.bindingArrayIndex * sizeof(void*));
        auto cudaObject = dynamic_cast<CUDAShaderObject*>(object);
        if (subObjectIndex >= objects.getCount())
            objects.setCount(subObjectIndex + 1);
        objects[subObjectIndex] = cudaObject;
        return setData(offset, &cudaObject->bufferResource->m_cudaMemory, sizeof(void*));
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setResource(ShaderOffset const& offset, IResourceView* resourceView)
    {
        auto cudaView = dynamic_cast<CUDAResourceView*>(resourceView);
        if (offset.bindingRangeIndex >= resources.getCount())
            resources.setCount(offset.bindingRangeIndex + 1);
        resources[offset.bindingRangeIndex] = cudaView;
        if (cudaView->textureResource)
        {
            if (cudaView->desc.type == IResourceView::Type::UnorderedAccess)
            {
                auto handle = cudaView->textureResource->m_cudaSurfObj;
                setData(offset, &handle, sizeof(uint64_t));
            }
            else
            {
                auto handle = cudaView->textureResource->getBindlessHandle();
                setData(offset, &handle, sizeof(uint64_t));
            }
        }
        else
        {
            auto handle = cudaView->memoryResource->getBindlessHandle();
            setData(offset, &handle, sizeof(handle));
            auto sizeOffset = offset;
            sizeOffset.uniformOffset += sizeof(handle);
            auto& desc = *cudaView->memoryResource->getDesc();
            size_t size = desc.sizeInBytes;
            if (desc.elementSize > 1)
                size /= desc.elementSize;
            setData(sizeOffset, &size, sizeof(size));

        }
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setSampler(ShaderOffset const& offset, ISamplerState* sampler)
    {
        SLANG_UNUSED(sampler);
        SLANG_UNUSED(offset);
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(
        ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler)
    {
        SLANG_UNUSED(sampler);
        setResource(offset, textureView);
        return SLANG_OK;
    }
};

class CUDAEntryPointShaderObject : public CUDAShaderObject
{
public:
    void* hostBuffer = nullptr;
    size_t uniformBufferSize = 0;
    // Override buffer allocation so we store all uniform data on host memory instead of device memory.
    virtual SLANG_NO_THROW Result SLANG_MCALL initBuffer(IRenderer* renderer, size_t bufferSize) override
    {
        uniformBufferSize = bufferSize;
        hostBuffer = malloc(bufferSize);
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        setData(ShaderOffset const& offset, void const* data, size_t size) override
    {
        size = Math::Min(size, uniformBufferSize - offset.uniformOffset);
        memcpy(
            (uint8_t*)hostBuffer + offset.uniformOffset,
            data,
            size);
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL getBuffer() override
    {
        return hostBuffer;
    }

    virtual SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() override
    {
        return uniformBufferSize;
    }

    ~CUDAEntryPointShaderObject()
    {
        free(hostBuffer);
    }
};

class CUDARootShaderObject : public CUDAShaderObject
{
public:
    List<RefPtr<CUDAEntryPointShaderObject>> entryPointObjects;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        init(IRenderer* renderer, CUDAShaderObjectLayout* typeLayout) override;
    virtual SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() override { return entryPointObjects.getCount(); }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(UInt index, IShaderObject** outEntryPoint) override
    {
        *outEntryPoint = entryPointObjects[index].Ptr();
        entryPointObjects[index]->addRef();
        return SLANG_OK;
    }

};

class CUDARenderer : public IRenderer, public RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    IRenderer* getInterface(const Guid& guid)
    {
        return (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IRenderer)
                   ? static_cast<IRenderer*>(this)
                   : nullptr;
    }

private:
    static const CUDAReportStyle reportType = CUDAReportStyle::Normal;
    static int _calcSMCountPerMultiProcessor(int major, int minor)
    {
        // Defines for GPU Architecture types (using the SM version to determine
        // the # of cores per SM
        struct SMInfo
        {
            int sm; // 0xMm (hexadecimal notation), M = SM Major version, and m = SM minor version
            int coreCount;
        };

        static const SMInfo infos[] = {
            {0x30, 192},
            {0x32, 192},
            {0x35, 192},
            {0x37, 192},
            {0x50, 128},
            {0x52, 128},
            {0x53, 128},
            {0x60, 64},
            {0x61, 128},
            {0x62, 128},
            {0x70, 64},
            {0x72, 64},
            {0x75, 64}};

        const int sm = ((major << 4) + minor);
        for (Index i = 0; i < SLANG_COUNT_OF(infos); ++i)
        {
            if (infos[i].sm == sm)
            {
                return infos[i].coreCount;
            }
        }

        const auto& last = infos[SLANG_COUNT_OF(infos) - 1];

        // It must be newer presumably
        SLANG_ASSERT(sm > last.sm);

        // Default to the last entry
        return last.coreCount;
    }

    static SlangResult _findMaxFlopsDeviceIndex(int* outDeviceIndex)
    {
        int smPerMultiproc = 0;
        int maxPerfDevice = -1;
        int deviceCount = 0;
        int devicesProhibited = 0;

        uint64_t maxComputePerf = 0;
        SLANG_CUDA_RETURN_ON_FAIL(cudaGetDeviceCount(&deviceCount));

        // Find the best CUDA capable GPU device
        for (int currentDevice = 0; currentDevice < deviceCount; ++currentDevice)
        {
            int computeMode = -1, major = 0, minor = 0;
            SLANG_CUDA_RETURN_ON_FAIL(
                cudaDeviceGetAttribute(&computeMode, cudaDevAttrComputeMode, currentDevice));
            SLANG_CUDA_RETURN_ON_FAIL(
                cudaDeviceGetAttribute(&major, cudaDevAttrComputeCapabilityMajor, currentDevice));
            SLANG_CUDA_RETURN_ON_FAIL(
                cudaDeviceGetAttribute(&minor, cudaDevAttrComputeCapabilityMinor, currentDevice));

            // If this GPU is not running on Compute Mode prohibited,
            // then we can add it to the list
            if (computeMode != cudaComputeModeProhibited)
            {
                if (major == 9999 && minor == 9999)
                {
                    smPerMultiproc = 1;
                }
                else
                {
                    smPerMultiproc = _calcSMCountPerMultiProcessor(major, minor);
                }

                int multiProcessorCount = 0, clockRate = 0;
                SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(
                    &multiProcessorCount, cudaDevAttrMultiProcessorCount, currentDevice));
                SLANG_CUDA_RETURN_ON_FAIL(
                    cudaDeviceGetAttribute(&clockRate, cudaDevAttrClockRate, currentDevice));
                uint64_t compute_perf = uint64_t(multiProcessorCount) * smPerMultiproc * clockRate;

                if (compute_perf > maxComputePerf)
                {
                    maxComputePerf = compute_perf;
                    maxPerfDevice = currentDevice;
                }
            }
            else
            {
                devicesProhibited++;
            }
        }

        if (maxPerfDevice < 0)
        {
            return SLANG_FAIL;
        }

        *outDeviceIndex = maxPerfDevice;
        return SLANG_OK;
    }

    static SlangResult _initCuda(CUDAReportStyle reportType = CUDAReportStyle::Normal)
    {
        static CUresult res = cuInit(0);
        SLANG_CUDA_RETURN_WITH_REPORT_ON_FAIL(res, reportType);
        return SLANG_OK;
    }

private:
    int m_deviceIndex = -1;
    CUdevice m_device = 0;
    CUcontext m_context = nullptr;
    CUDAPipelineState* currentPipeline = nullptr;
    CUDARootShaderObject* currentRootObject = nullptr;
 public:
    ~CUDARenderer()
    {
        if (m_context)
        {
            cuCtxDestroy(m_context);
        }
    }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc, void* inWindowHandle) override
    {
        SLANG_RETURN_ON_FAIL(_initCuda(reportType));

        SLANG_RETURN_ON_FAIL(_findMaxFlopsDeviceIndex(&m_deviceIndex));
        SLANG_CUDA_RETURN_WITH_REPORT_ON_FAIL(cudaSetDevice(m_deviceIndex), reportType);

        if (m_context)
        {
            cuCtxDestroy(m_context);
            m_context = nullptr;
        }

        SLANG_CUDA_RETURN_ON_FAIL(cuDeviceGet(&m_device, m_deviceIndex));

        SLANG_CUDA_RETURN_WITH_REPORT_ON_FAIL(cuCtxCreate(&m_context, 0, m_device), reportType);
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        IResource::Usage initialUsage,
        const ITextureResource::Desc& desc,
        const ITextureResource::Data* initData,
        ITextureResource** outResource) override
    {
        RefPtr<TextureCUDAResource> tex = new TextureCUDAResource(desc);
        CUresourcetype resourceType;
        size_t elementSize = 0;

        // Our `ITextureResource::Desc` uses an enumeration to specify
        // the "shape"/rank of a texture (1D, 2D, 3D, Cube), but CUDA's
        // `cuMipmappedArrayCreate` seemingly relies on a policy where
        // the extents of the array in dimenions above the rank are
        // specified as zero (e.g., a 1D texture requires `height==0`).
        //
        // We will start by massaging the extents as specified by the
        // user into a form that CUDA wants/expects, based on the
        // texture shape as specified in the `desc`.
        //
        int width = desc.size.width;
        int height = desc.size.height;
        int depth = desc.size.depth;
        switch (desc.type)
        {
        case IResource::Type::Texture1D:
            height = 0;
            depth = 0;
            break;

        case IResource::Type::Texture2D:
            depth = 0;
            break;

        case IResource::Type::Texture3D:
            break;

        case IResource::Type::TextureCube:
            depth = 1;
            break;
        }

        {
            CUarray_format format = CU_AD_FORMAT_FLOAT;
            int numChannels = 0;

            switch (desc.format)
            {
            case Format::R_Float32:
                {
                    format = CU_AD_FORMAT_FLOAT;
                    numChannels = 1;
                    elementSize = sizeof(float);
                    break;
                }
            case Format::RGBA_Unorm_UInt8:
                {
                    format = CU_AD_FORMAT_UNSIGNED_INT8;
                    numChannels = 4;
                    elementSize = sizeof(uint32_t);
                    break;
                }
            default:
                {
                    SLANG_ASSERT(!"Only support R_Float32/RGBA_Unorm_UInt8 formats for now");
                    return SLANG_FAIL;
                }
            }

            if (desc.numMipLevels > 1)
            {
                resourceType = CU_RESOURCE_TYPE_MIPMAPPED_ARRAY;

                CUDA_ARRAY3D_DESCRIPTOR arrayDesc;
                memset(&arrayDesc, 0, sizeof(arrayDesc));

                arrayDesc.Width = width;
                arrayDesc.Height = height;
                arrayDesc.Depth = depth;
                arrayDesc.Format = format;
                arrayDesc.NumChannels = numChannels;
                arrayDesc.Flags = 0;

                if (desc.arraySize > 1)
                {
                    if (desc.type == IResource::Type::Texture1D ||
                        desc.type == IResource::Type::Texture2D ||
                        desc.type == IResource::Type::TextureCube)
                    {
                        arrayDesc.Flags |= CUDA_ARRAY3D_LAYERED;
                        arrayDesc.Depth = desc.arraySize;
                    }
                    else
                    {
                        SLANG_ASSERT(!"Arrays only supported for 1D and 2D");
                        return SLANG_FAIL;
                    }
                }

                if (desc.type == IResource::Type::TextureCube)
                {
                    arrayDesc.Flags |= CUDA_ARRAY3D_CUBEMAP;
                    arrayDesc.Depth *= 6;
                }

                SLANG_CUDA_RETURN_ON_FAIL(
                    cuMipmappedArrayCreate(&tex->m_cudaMipMappedArray, &arrayDesc, desc.numMipLevels));
            }
            else
            {
                resourceType = CU_RESOURCE_TYPE_ARRAY;

                if (desc.arraySize > 1)
                {
                    if (desc.type == IResource::Type::Texture1D ||
                        desc.type == IResource::Type::Texture2D ||
                        desc.type == IResource::Type::TextureCube)
                    {
                        SLANG_ASSERT(!"Only 1D, 2D and Cube arrays supported");
                        return SLANG_FAIL;
                    }

                    CUDA_ARRAY3D_DESCRIPTOR arrayDesc;
                    memset(&arrayDesc, 0, sizeof(arrayDesc));

                    // Set the depth as the array length
                    arrayDesc.Depth = desc.arraySize;
                    if (desc.type == IResource::Type::TextureCube)
                    {
                        arrayDesc.Depth *= 6;
                    }

                    arrayDesc.Height = height;
                    arrayDesc.Width = width;
                    arrayDesc.Format = format;
                    arrayDesc.NumChannels = numChannels;

                    if (desc.type == IResource::Type::TextureCube)
                    {
                        arrayDesc.Flags |= CUDA_ARRAY3D_CUBEMAP;
                    }

                    SLANG_CUDA_RETURN_ON_FAIL(cuArray3DCreate(&tex->m_cudaArray, &arrayDesc));
                }
                else if (desc.type == IResource::Type::Texture3D ||
                    desc.type == IResource::Type::TextureCube)
                {
                    CUDA_ARRAY3D_DESCRIPTOR arrayDesc;
                    memset(&arrayDesc, 0, sizeof(arrayDesc));

                    arrayDesc.Depth = depth;
                    arrayDesc.Height = height;
                    arrayDesc.Width = width;
                    arrayDesc.Format = format;
                    arrayDesc.NumChannels = numChannels;

                    arrayDesc.Flags = 0;

                    // Handle cube texture
                    if (desc.type == IResource::Type::TextureCube)
                    {
                        arrayDesc.Depth = 6;
                        arrayDesc.Flags |= CUDA_ARRAY3D_CUBEMAP;
                    }

                    SLANG_CUDA_RETURN_ON_FAIL(cuArray3DCreate(&tex->m_cudaArray, &arrayDesc));
                }
                else
                {
                    CUDA_ARRAY_DESCRIPTOR arrayDesc;
                    memset(&arrayDesc, 0, sizeof(arrayDesc));

                    arrayDesc.Height = height;
                    arrayDesc.Width = width;
                    arrayDesc.Format = format;
                    arrayDesc.NumChannels = numChannels;

                    // Allocate the array, will work for 1D or 2D case
                    SLANG_CUDA_RETURN_ON_FAIL(cuArrayCreate(&tex->m_cudaArray, &arrayDesc));
                }
            }
        }

        // Work space for holding data for uploading if it needs to be rearranged
        List<uint8_t> workspace;
        for (int mipLevel = 0; mipLevel < desc.numMipLevels; ++mipLevel)
        {
            int mipWidth = width >> mipLevel;
            int mipHeight = height >> mipLevel;
            int mipDepth = depth >> mipLevel;

            mipWidth = (mipWidth == 0) ? 1 : mipWidth;
            mipHeight = (mipHeight == 0) ? 1 : mipHeight;
            mipDepth = (mipDepth == 0) ? 1 : mipDepth;

            // If it's a cubemap then the depth is always 6
            if (desc.type == IResource::Type::TextureCube)
            {
                mipDepth = 6;
            }

            auto dstArray = tex->m_cudaArray;
            if (tex->m_cudaMipMappedArray)
            {
                // Get the array for the mip level
                SLANG_CUDA_RETURN_ON_FAIL(
                    cuMipmappedArrayGetLevel(&dstArray, tex->m_cudaMipMappedArray, mipLevel));
            }
            SLANG_ASSERT(dstArray);

            // Check using the desc to see if it's plausible
            {
                CUDA_ARRAY_DESCRIPTOR arrayDesc;
                SLANG_CUDA_RETURN_ON_FAIL(cuArrayGetDescriptor(&arrayDesc, dstArray));

                SLANG_ASSERT(mipWidth == arrayDesc.Width);
                SLANG_ASSERT(
                    mipHeight == arrayDesc.Height || (mipHeight == 1 && arrayDesc.Height == 0));
            }

            const void* srcDataPtr = nullptr;

            if (desc.arraySize > 1)
            {
                SLANG_ASSERT(
                    desc.type == IResource::Type::Texture1D ||
                    desc.type == IResource::Type::Texture2D ||
                    desc.type == IResource::Type::TextureCube);

                // TODO(JS): Here I assume that arrays are just held contiguously within a 'face'
                // This seems reasonable and works with the Copy3D.
                const size_t faceSizeInBytes = elementSize * mipWidth * mipHeight;

                Index faceCount = desc.arraySize;
                if (desc.type == IResource::Type::TextureCube)
                {
                    faceCount *= 6;
                }

                const size_t mipSizeInBytes = faceSizeInBytes * faceCount;
                workspace.setCount(mipSizeInBytes);

                // We need to add the face data from each mip
                // We iterate over face count so we copy all of the cubemap faces
                if (initData)
                {
                    for (Index j = 0; j < faceCount; j++)
                    {
                        const auto srcData = initData->subResources[mipLevel + j * desc.numMipLevels];
                        // Copy over to the workspace to make contiguous
                        ::memcpy(
                            workspace.begin() + faceSizeInBytes * j, srcData,
                            faceSizeInBytes);
                    }
                }

                srcDataPtr = workspace.getBuffer();
            }
            else
            {
                if (desc.type == IResource::Type::TextureCube)
                {
                    size_t faceSizeInBytes = elementSize * mipWidth * mipHeight;

                    workspace.setCount(faceSizeInBytes * 6);

                    // Copy the data over to make contiguous
                    for (Index j = 0; j < 6; j++)
                    {
                        const auto srcData =
                            initData->subResources[mipLevel + j * desc.numMipLevels];
                        ::memcpy(
                            workspace.getBuffer() + faceSizeInBytes * j, srcData,
                            faceSizeInBytes);
                    }

                    srcDataPtr = workspace.getBuffer();
                }
                else
                {
                    const auto srcData = initData->subResources[mipLevel];
                    srcDataPtr = srcData;
                }
            }

            if (desc.arraySize > 1)
            {
                SLANG_ASSERT(
                    desc.type == IResource::Type::Texture1D ||
                    desc.type == IResource::Type::Texture2D ||
                    desc.type == IResource::Type::TextureCube);

                CUDA_MEMCPY3D copyParam;
                memset(&copyParam, 0, sizeof(copyParam));

                copyParam.dstMemoryType = CU_MEMORYTYPE_ARRAY;
                copyParam.dstArray = dstArray;

                copyParam.srcMemoryType = CU_MEMORYTYPE_HOST;
                copyParam.srcHost = srcDataPtr;
                copyParam.srcPitch = mipWidth * elementSize;
                copyParam.WidthInBytes = copyParam.srcPitch;
                copyParam.Height = mipHeight;
                // Set the depth to the array length
                copyParam.Depth = desc.arraySize;

                if (desc.type == IResource::Type::TextureCube)
                {
                    copyParam.Depth *= 6;
                }

                SLANG_CUDA_RETURN_ON_FAIL(cuMemcpy3D(&copyParam));
            }
            else
            {
                switch (desc.type)
                {
                case IResource::Type::Texture1D:
                case IResource::Type::Texture2D:
                    {
                        CUDA_MEMCPY2D copyParam;
                        memset(&copyParam, 0, sizeof(copyParam));
                        copyParam.dstMemoryType = CU_MEMORYTYPE_ARRAY;
                        copyParam.dstArray = dstArray;
                        copyParam.srcMemoryType = CU_MEMORYTYPE_HOST;
                        copyParam.srcHost = srcDataPtr;
                        copyParam.srcPitch = mipWidth * elementSize;
                        copyParam.WidthInBytes = copyParam.srcPitch;
                        copyParam.Height = mipHeight;
                        SLANG_CUDA_RETURN_ON_FAIL(cuMemcpy2D(&copyParam));
                        break;
                    }
                case IResource::Type::Texture3D:
                case IResource::Type::TextureCube:
                    {
                        CUDA_MEMCPY3D copyParam;
                        memset(&copyParam, 0, sizeof(copyParam));

                        copyParam.dstMemoryType = CU_MEMORYTYPE_ARRAY;
                        copyParam.dstArray = dstArray;

                        copyParam.srcMemoryType = CU_MEMORYTYPE_HOST;
                        copyParam.srcHost = srcDataPtr;
                        copyParam.srcPitch = mipWidth * elementSize;
                        copyParam.WidthInBytes = copyParam.srcPitch;
                        copyParam.Height = mipHeight;
                        copyParam.Depth = mipDepth;

                        SLANG_CUDA_RETURN_ON_FAIL(cuMemcpy3D(&copyParam));
                        break;
                    }

                default:
                    {
                        SLANG_ASSERT(!"Not implemented");
                        break;
                    }
                }
            }
        }

        // Set up texture sampling parameters, and create final texture obj

        {
            CUDA_RESOURCE_DESC resDesc;
            memset(&resDesc, 0, sizeof(CUDA_RESOURCE_DESC));
            resDesc.resType = resourceType;

            if (tex->m_cudaArray)
            {
                resDesc.res.array.hArray = tex->m_cudaArray;
            }
            if (tex->m_cudaMipMappedArray)
            {
                resDesc.res.mipmap.hMipmappedArray = tex->m_cudaMipMappedArray;
            }

            // If the texture might be used as a UAV, then we need to allocate
            // a CUDA "surface" for it.
            //
            // Note: We cannot do this unconditionally, because it will fail
            // on surfaces that are not usable as UAVs (e.g., those with
            // mipmaps).
            //
            // TODO: We should really only be allocating the array at the
            // time we create a resource, and then allocate the surface or
            // texture objects as part of view creation.
            //
            if( desc.bindFlags & IResource::BindFlag::UnorderedAccess )
            {
                SLANG_CUDA_RETURN_ON_FAIL(cuSurfObjectCreate(&tex->m_cudaSurfObj, &resDesc));
            }

            
            // Create handle for sampling.
            CUDA_TEXTURE_DESC texDesc;
            memset(&texDesc, 0, sizeof(CUDA_TEXTURE_DESC));
            texDesc.addressMode[0] = CU_TR_ADDRESS_MODE_WRAP;
            texDesc.addressMode[1] = CU_TR_ADDRESS_MODE_WRAP;
            texDesc.addressMode[2] = CU_TR_ADDRESS_MODE_WRAP;
            texDesc.filterMode = CU_TR_FILTER_MODE_LINEAR;
            texDesc.flags = CU_TRSF_NORMALIZED_COORDINATES;

            SLANG_CUDA_RETURN_ON_FAIL(
                cuTexObjectCreate(&tex->m_cudaTexObj, &resDesc, &texDesc, nullptr));
        }

        *outResource = tex.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        IResource::Usage initialUsage,
        const IBufferResource::Desc& desc,
        const void* initData,
        IBufferResource** outResource) override
    {
        RefPtr<MemoryCUDAResource> resource = new MemoryCUDAResource(desc);
        SLANG_CUDA_RETURN_ON_FAIL(cudaMallocManaged(&resource->m_cudaMemory, desc.sizeInBytes));
        if (initData)
        {
            SLANG_CUDA_RETURN_ON_FAIL(cudaMemcpy(resource->m_cudaMemory, initData, desc.sizeInBytes, cudaMemcpyHostToDevice));
        }
        *outResource = resource.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureView(
        ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView) override
    {
        RefPtr<CUDAResourceView> view = new CUDAResourceView();
        view->desc = desc;
        view->textureResource = dynamic_cast<TextureCUDAResource*>(texture);
        *outView = view.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView) override
    {
        RefPtr<CUDAResourceView> view = new CUDAResourceView();
        view->desc = desc;
        view->memoryResource = dynamic_cast<MemoryCUDAResource*>(buffer);
        *outView = view.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout, IShaderObjectLayout** outLayout) override
    {
        RefPtr<CUDAShaderObjectLayout> cudaLayout;
        cudaLayout = new CUDAShaderObjectLayout(typeLayout);
        *outLayout = cudaLayout.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createShaderObject(IShaderObjectLayout* layout, IShaderObject** outObject) override
    {
        RefPtr<CUDAShaderObject> result = new CUDAShaderObject();
        SLANG_RETURN_ON_FAIL(result->init(this, dynamic_cast<CUDAShaderObjectLayout*>(layout)));
        *outObject = result.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createRootShaderObject(IShaderProgram* program, IShaderObject** outObject) override
    {
        auto cudaProgram = dynamic_cast<CUDAShaderProgram*>(program);
        auto cudaLayout = cudaProgram->layout;

        RefPtr<CUDARootShaderObject> result = new CUDARootShaderObject();
        SLANG_RETURN_ON_FAIL(result->init(this, cudaLayout));
        *outObject = result.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindRootShaderObject(PipelineType pipelineType, IShaderObject* object) override
    {
        currentRootObject = dynamic_cast<CUDARootShaderObject*>(object);
        if (currentRootObject)
            return SLANG_OK;
        return SLANG_E_INVALID_ARG;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override
    {
        if( desc.kernelCount == 0 )
        {
            return createProgramFromSlang(this, desc, outProgram);
        }

        if (desc.kernelCount != 1)
            return SLANG_E_INVALID_ARG;
        RefPtr<CUDAShaderProgram> cudaProgram = new CUDAShaderProgram();
        SLANG_CUDA_RETURN_ON_FAIL(cuModuleLoadData(&cudaProgram->cudaModule, desc.kernels[0].codeBegin));
        SLANG_CUDA_RETURN_ON_FAIL(
            cuModuleGetFunction(&cudaProgram->cudaKernel, cudaProgram->cudaModule, desc.kernels[0].entryPointName));
        cudaProgram->kernelName = desc.kernels[0].entryPointName;

        auto slangProgram = desc.slangProgram;
        if( slangProgram )
        {
            cudaProgram->slangProgram = slangProgram;

            auto slangProgramLayout = slangProgram->getLayout();
            if(!slangProgramLayout)
                return SLANG_FAIL;

            RefPtr<CUDAProgramLayout> cudaLayout;
            cudaLayout = new CUDAProgramLayout(slangProgramLayout);
            cudaLayout->programLayout = slangProgramLayout;
            cudaProgram->layout = cudaLayout;
        }

        *outProgram = cudaProgram.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, IPipelineState** outState) override
    {
        RefPtr<CUDAPipelineState> state = new CUDAPipelineState();
        state->shaderProgram = dynamic_cast<CUDAShaderProgram*>(desc.program);
        *outState = state.detach();
        return Result();
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL map(IBufferResource* buffer, MapFlavor flavor) override
    {
        return dynamic_cast<MemoryCUDAResource*>(buffer)->m_cudaMemory;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL unmap(IBufferResource* buffer) override
    {
        SLANG_UNUSED(buffer);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        setPipelineState(PipelineType pipelineType, IPipelineState* state) override
    {
        SLANG_ASSERT(pipelineType == PipelineType::Compute);
        currentPipeline = dynamic_cast<CUDAPipelineState*>(state);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override
    {
        // Find out thread group size from program reflection.
        auto& kernelName = currentPipeline->shaderProgram->kernelName;
        auto programLayout = dynamic_cast<CUDAProgramLayout*>(currentRootObject->layout.Ptr());
        int kernelId = programLayout->getKernelIndex(kernelName.getUnownedSlice());
        SLANG_ASSERT(kernelId != -1);
        UInt threadGroupSize[3];
        programLayout->getKernelThreadGroupSize(kernelId, threadGroupSize);

        int sharedSizeInBytes;
        cuFuncGetAttribute(
            &sharedSizeInBytes,
            CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES,
            currentPipeline->shaderProgram->cudaKernel);

        // Copy global parameter data to the `SLANG_globalParams` symbol.
        {
            CUdeviceptr globalParamsSymbol = 0;
            size_t globalParamsSymbolSize = 0;
            cuModuleGetGlobal(
                &globalParamsSymbol,
                &globalParamsSymbolSize,
                currentPipeline->shaderProgram->cudaModule,
                "SLANG_globalParams");

            CUdeviceptr globalParamsCUDAData =
                currentRootObject->bufferResource
                    ? (CUdeviceptr)currentRootObject->bufferResource->getBindlessHandle()
                    : 0;
            cudaMemcpyAsync(
                (void*)globalParamsSymbol,
                (void*)globalParamsCUDAData,
                globalParamsSymbolSize,
                cudaMemcpyDeviceToDevice,
                0);
        }
        //
        // The argument data for the entry-point parameters are already
        // stored in host memory in a CUDAEntryPointShaderObject, as expected by cuLaunchKernel.
        //
        auto entryPointBuffer = currentRootObject->entryPointObjects[kernelId]->getBuffer();
        auto entryPointDataSize = currentRootObject->entryPointObjects[kernelId]->getBufferSize();
        
        void* extraOptions[] = {
            CU_LAUNCH_PARAM_BUFFER_POINTER,
            entryPointBuffer,
            CU_LAUNCH_PARAM_BUFFER_SIZE,
            &entryPointDataSize,
            CU_LAUNCH_PARAM_END,
        };

        // Once we have all the decessary data extracted and/or
        // set up, we can launch the kernel and see what happens.
        //
        auto cudaLaunchResult = cuLaunchKernel(
            currentPipeline->shaderProgram->cudaKernel,
            x,
            y,
            z,
            int(threadGroupSize[0]),
            int(threadGroupSize[1]),
            int(threadGroupSize[2]),
            sharedSizeInBytes,
            0,
            nullptr,
            extraOptions);

        SLANG_ASSERT(cudaLaunchResult == CUDA_SUCCESS);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL submitGpuWork() override {}

    virtual SLANG_NO_THROW void SLANG_MCALL waitForGpu() override
    {
        auto result = cudaDeviceSynchronize();
        SLANG_ASSERT(result == CUDA_SUCCESS);
    }

    virtual SLANG_NO_THROW RendererType SLANG_MCALL getRendererType() const override
    {
        return RendererType::CUDA;
    }

public:
    // Unused public interfaces. These functions are not supported on CUDA.
    SLANG_NO_THROW Result SLANG_MCALL getFeatures(
        const char** outFeatures, UInt bufferSize, UInt* outFeatureCount)
    {
        if (outFeatureCount)
            *outFeatureCount = 0;
        return SLANG_OK;
    }

    SLANG_NO_THROW bool SLANG_MCALL hasFeature(const char* featureName)
    {
        return false;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setClearColor(const float color[4]) override
    {
        SLANG_UNUSED(color);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL clearFrame() override {}
    virtual SLANG_NO_THROW void SLANG_MCALL presentFrame() override {}
    virtual SLANG_NO_THROW TextureResource::Desc SLANG_MCALL getSwapChainTextureDesc() override
    {
        return TextureResource::Desc();
    }
    
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler) override
    {
        SLANG_UNUSED(desc);
        *outSampler = nullptr;
        return SLANG_OK;
    }
    
    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        const InputElementDesc* inputElements,
        UInt inputElementCount,
        IInputLayout** outLayout) override
    {
        SLANG_UNUSED(inputElements);
        SLANG_UNUSED(inputElementCount);
        SLANG_UNUSED(outLayout);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSetLayout(
        const IDescriptorSetLayout::Desc& desc, IDescriptorSetLayout** outLayout) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outLayout);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createPipelineLayout(const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outLayout);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createDescriptorSet(IDescriptorSetLayout* layout, IDescriptorSet** outDescriptorSet) override
    {
        SLANG_UNUSED(layout);
        SLANG_UNUSED(outDescriptorSet);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc, IPipelineState** outState) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outState);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL captureScreenSurface(
        void* buffer, size_t* inOutBufferSize, size_t* outRowPitch, size_t* outPixelSize) override
    {
        SLANG_UNUSED(buffer);
        SLANG_UNUSED(inOutBufferSize);
        SLANG_UNUSED(outRowPitch);
        SLANG_UNUSED(outPixelSize);

        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SLANG_NO_THROW void SLANG_MCALL
        setPrimitiveTopology(PrimitiveTopology topology) override
    {
        SLANG_UNUSED(topology);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSet(
        PipelineType pipelineType,
        IPipelineLayout* layout,
        UInt index,
        IDescriptorSet* descriptorSet) override
    {
        SLANG_UNUSED(pipelineType);
        SLANG_UNUSED(layout);
        SLANG_UNUSED(index);
        SLANG_UNUSED(descriptorSet);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        IBufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) override
    {
        SLANG_UNUSED(startSlot);
        SLANG_UNUSED(slotCount);
        SLANG_UNUSED(buffers);
        SLANG_UNUSED(strides);
        SLANG_UNUSED(offsets);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset = 0) override
    {
        SLANG_UNUSED(buffer);
        SLANG_UNUSED(indexFormat);
        SLANG_UNUSED(offset);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL
        setDepthStencilTarget(IResourceView* depthStencilView) override
    {
        SLANG_UNUSED(depthStencilView);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL
        setViewports(UInt count, Viewport const* viewports) override
    {
        SLANG_UNUSED(count);
        SLANG_UNUSED(viewports);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL
        setScissorRects(UInt count, ScissorRect const* rects) override
    {
        SLANG_UNUSED(count);
        SLANG_UNUSED(rects);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL draw(UInt vertexCount, UInt startVertex) override
    {
        SLANG_UNUSED(vertexCount);
        SLANG_UNUSED(startVertex);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL
        drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex) override
    {
        SLANG_UNUSED(indexCount);
        SLANG_UNUSED(startIndex);
        SLANG_UNUSED(baseVertex);
    }
};

SlangResult CUDAShaderObject::init(IRenderer* renderer, CUDAShaderObjectLayout* typeLayout)
{
    this->layout = typeLayout;
    
    // If the layout tells us that there is any uniform data,
    // then we need to allocate a constant buffer to hold that data.
    //
    // TODO: Do we need to allocate a shadow copy for use from
    // the CPU?
    //
    // TODO: When/where do we bind this constant buffer into
    // a descriptor set for later use?
    //
    auto slangLayout = layout->typeLayout;
    size_t uniformSize = layout->typeLayout->getSize();
    if (uniformSize)
    {
        initBuffer(renderer, uniformSize);
    }

    // If the layout specifies that we have any sub-objects, then
    // we need to size the array to account for them.
    //
    Index subObjectCount = slangLayout->getSubObjectRangeCount();
    objects.setCount(subObjectCount);

    for (auto subObjectRange : layout->subObjectRanges)
    {
        RefPtr<CUDAShaderObjectLayout> subObjectLayout = subObjectRange.layout;

        // In the case where the sub-object range represents an
        // existential-type leaf field (e.g., an `IBar`), we
        // cannot pre-allocate the object(s) to go into that
        // range, since we can't possibly know what to allocate
        // at this point.
        //
        if (!subObjectLayout)
            continue;
        //
        // Otherwise, we will allocate a sub-object to fill
        // in each entry in this range, based on the layout
        // information we already have.

        auto& bindingRangeInfo = layout->m_bindingRanges[subObjectRange.bindingRangeIndex];
        for (Index i = 0; i < bindingRangeInfo.count; ++i)
        {
            RefPtr<CUDAShaderObject> subObject = new CUDAShaderObject();
            SLANG_RETURN_ON_FAIL(subObject->init(renderer, subObjectLayout));
            objects[bindingRangeInfo.baseIndex + i] = subObject;
            ShaderOffset offset;
            offset.uniformOffset = bindingRangeInfo.uniformOffset + sizeof(void*) * i;
            if (subObject->bufferResource)
                SLANG_RETURN_ON_FAIL(setData(offset, &subObject->bufferResource->m_cudaMemory, sizeof(void*)));
        }
    }
    return SLANG_OK;
}

SlangResult CUDARootShaderObject::init(IRenderer* renderer, CUDAShaderObjectLayout* typeLayout)
{
    SLANG_RETURN_ON_FAIL(CUDAShaderObject::init(renderer, typeLayout));
    auto programLayout = dynamic_cast<CUDAProgramLayout*>(typeLayout);
    for (auto& entryPoint : programLayout->entryPointLayouts)
    {
        RefPtr<CUDAEntryPointShaderObject> object = new CUDAEntryPointShaderObject();
        SLANG_RETURN_ON_FAIL(object->init(renderer, entryPoint));
        entryPointObjects.add(object);
    }
    return SLANG_OK;
}

SlangResult SLANG_MCALL createCUDARenderer(IRenderer** outRenderer)
{
    *outRenderer = new CUDARenderer();
    (*outRenderer)->addRef();
    return SLANG_OK;
}
#else
SlangResult SLANG_MCALL createCUDARenderer(IRenderer** outRenderer)
{
    *outRenderer = nullptr;
    return SLANG_OK;
}
#endif

}
