#include "render-cuda.h"

#ifdef GFX_ENABLE_CUDA
#include "../render.h"
#include <cuda.h>
#include <cuda_runtime_api.h>
#include "../../source/core/slang-std-writers.h"
#include "slang.h"
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

class CUDAResourceView : public ResourceView
{
public:
    Desc desc;
    RefPtr<MemoryCUDAResource> memoryResource = nullptr;
    RefPtr<TextureCUDAResource> textureResource = nullptr;
};

class CUDAShaderProgram : public ShaderProgram
{
public:
    CUmodule cudaModule = nullptr;
    CUfunction cudaKernel;
    String kernelName;
    ~CUDAShaderProgram()
    {
        if (cudaModule)
            cuModuleUnload(cudaModule);
    }
};

class CUDAPipelineState : public PipelineState
{
public:
    RefPtr<CUDAShaderProgram> shaderProgram;
};

class CUDAShaderObjectLayout : public ShaderObjectLayout
{
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

class CUDAShaderObject : public ShaderObject
{
public:
    RefPtr<MemoryCUDAResource> bufferResource;
    RefPtr<CUDAShaderObjectLayout> layout;
    List<RefPtr<CUDAShaderObject>> objects;
    List<RefPtr<CUDAResourceView>> resources;

    virtual SlangResult init(Renderer* renderer, CUDAShaderObjectLayout* typeLayout);

    virtual SlangResult initBuffer(Renderer* renderer, size_t bufferSize)
    {
        BufferResource::Desc bufferDesc;
        bufferDesc.init(bufferSize);
        bufferDesc.cpuAccessFlags |= Resource::AccessFlag::Write;
        RefPtr<BufferResource> constantBuffer;
        SLANG_RETURN_ON_FAIL(renderer->createBufferResource(
            Resource::Usage::ConstantBuffer, bufferDesc, nullptr, constantBuffer.writeRef()));
        bufferResource = dynamic_cast<MemoryCUDAResource*>(constantBuffer.Ptr());
        return SLANG_OK;
    }

    virtual void* getBuffer()
    {
        return bufferResource ? bufferResource->m_cudaMemory : nullptr;
    }

    virtual size_t getBufferSize()
    {
        return bufferResource ? bufferResource->getDesc().sizeInBytes : 0;
    }

    virtual slang::TypeLayoutReflection* getElementTypeLayout() override
    {
        return layout->typeLayout;
    }

    virtual Slang::Index getEntryPointCount() override { return 0; }
    virtual ShaderObject* getEntryPoint(Slang::Index index) override { return nullptr; }
    virtual SlangResult setData(ShaderOffset const& offset, void const* data, size_t size)
    {
        size = Math::Min(size, bufferResource->getDesc().sizeInBytes - offset.uniformOffset);
        SLANG_CUDA_RETURN_ON_FAIL(cudaMemcpy(
            (uint8_t*)bufferResource->m_cudaMemory + offset.uniformOffset,
            data,
            size,
            cudaMemcpyHostToDevice));
        return SLANG_OK;
    }
    virtual SlangResult getObject(ShaderOffset const& offset, ShaderObject** object)
    {
        auto subObjectIndex =
            layout->m_bindingRanges[offset.bindingRangeIndex].baseIndex + offset.bindingArrayIndex;
        if (subObjectIndex >= objects.getCount())
        {
            *object = nullptr;
            return SLANG_OK;
        }
        *object = objects[subObjectIndex].Ptr();
        return SLANG_OK;
    }
    virtual SlangResult setObject(ShaderOffset const& offset, ShaderObject* object)
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
    virtual SlangResult setResource(ShaderOffset const& offset, ResourceView* resourceView)
    {
        auto cudaView = dynamic_cast<CUDAResourceView*>(resourceView);
        if (offset.bindingRangeIndex >= resources.getCount())
            resources.setCount(offset.bindingRangeIndex + 1);
        resources[offset.bindingRangeIndex] = cudaView;
        if (cudaView->textureResource)
        {
            if (cudaView->desc.type == ResourceView::Type::UnorderedAccess)
            {
                auto handle = cudaView->textureResource->getBindlessHandle();
                setData(offset, &handle, sizeof(uint64_t));
            }
            else
            {
                auto handle = cudaView->textureResource->m_cudaSurfObj;
                setData(offset, &handle, sizeof(uint64_t));
            }
        }
        else
        {
            auto handle = cudaView->memoryResource->getBindlessHandle();
            setData(offset, &handle, sizeof(handle));
            auto sizeOffset = offset;
            sizeOffset.uniformOffset += sizeof(handle);
            auto& desc = cudaView->memoryResource->getDesc();
            size_t size = desc.sizeInBytes;
            if (desc.elementSize > 1)
                size /= desc.elementSize;
            setData(sizeOffset, &size, sizeof(size));

        }
        return SLANG_OK;
    }
    virtual SlangResult setSampler(ShaderOffset const& offset, SamplerState* sampler)
    {
        SLANG_UNUSED(sampler);
        SLANG_UNUSED(offset);
        return SLANG_OK;
    }
    virtual SlangResult setCombinedTextureSampler(
        ShaderOffset const& offset, ResourceView* textureView, SamplerState* sampler)
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
    virtual SlangResult initBuffer(Renderer* renderer, size_t bufferSize) override
    {
        uniformBufferSize = bufferSize;
        hostBuffer = malloc(bufferSize);
        return SLANG_OK;
    }

    virtual SlangResult setData(ShaderOffset const& offset, void const* data, size_t size) override
    {
        size = Math::Min(size, uniformBufferSize - offset.uniformOffset);
        memcpy(
            (uint8_t*)hostBuffer + offset.uniformOffset,
            data,
            size);
        return SLANG_OK;
    }

    virtual void* getBuffer() override
    {
        return hostBuffer;
    }

    virtual size_t getBufferSize() override
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
    virtual SlangResult init(Renderer* renderer, CUDAShaderObjectLayout* typeLayout) override;
    virtual Slang::Index getEntryPointCount() override { return entryPointObjects.getCount(); }
    virtual ShaderObject* getEntryPoint(Slang::Index index) override { return entryPointObjects[index].Ptr(); }

};

class CUDARenderer : public Renderer
{
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
    virtual SlangResult initialize(const Desc& desc, void* inWindowHandle) override
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

    virtual Result createTextureResource(
        Resource::Usage initialUsage,
        const TextureResource::Desc& desc,
        const TextureResource::Data* initData,
        TextureResource** outResource) override
    {
        RefPtr<TextureCUDAResource> tex = new TextureCUDAResource(desc);
        CUresourcetype resourceType;
        size_t elementSize = 0;

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

                arrayDesc.Width = desc.size.width;
                arrayDesc.Height = desc.size.height;
                arrayDesc.Depth = desc.size.depth;
                arrayDesc.Format = format;
                arrayDesc.NumChannels = numChannels;
                arrayDesc.Flags = 0;

                if (desc.arraySize > 1)
                {
                    if (desc.type == Resource::Type::Texture1D ||
                        desc.type == Resource::Type::Texture2D ||
                        desc.type == Resource::Type::TextureCube)
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

                if (desc.type == Resource::Type::TextureCube)
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
                    if (desc.type == Resource::Type::Texture1D ||
                        desc.type == Resource::Type::Texture2D ||
                        desc.type == Resource::Type::TextureCube)
                    {
                        SLANG_ASSERT(!"Only 1D, 2D and Cube arrays supported");
                        return SLANG_FAIL;
                    }

                    CUDA_ARRAY3D_DESCRIPTOR arrayDesc;
                    memset(&arrayDesc, 0, sizeof(arrayDesc));

                    // Set the depth as the array length
                    arrayDesc.Depth = desc.arraySize;
                    if (desc.type == Resource::Type::TextureCube)
                    {
                        arrayDesc.Depth *= 6;
                    }

                    arrayDesc.Height = desc.size.height;
                    arrayDesc.Width = desc.size.width;
                    arrayDesc.Format = format;
                    arrayDesc.NumChannels = numChannels;

                    if (desc.type == Resource::Type::TextureCube)
                    {
                        arrayDesc.Flags |= CUDA_ARRAY3D_CUBEMAP;
                    }

                    SLANG_CUDA_RETURN_ON_FAIL(cuArray3DCreate(&tex->m_cudaArray, &arrayDesc));
                }
                else if (desc.type == Resource::Type::Texture3D ||
                    desc.type == Resource::Type::TextureCube)
                {
                    CUDA_ARRAY3D_DESCRIPTOR arrayDesc;
                    memset(&arrayDesc, 0, sizeof(arrayDesc));

                    arrayDesc.Depth = desc.size.depth;
                    arrayDesc.Height = desc.size.height;
                    arrayDesc.Width = desc.size.width;
                    arrayDesc.Format = format;
                    arrayDesc.NumChannels = numChannels;

                    arrayDesc.Flags = 0;

                    // Handle cube texture
                    if (desc.type == Resource::Type::TextureCube)
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

                    arrayDesc.Height = desc.size.height;
                    arrayDesc.Width = desc.size.width;
                    arrayDesc.Format = format;
                    arrayDesc.NumChannels = numChannels;

                    // Allocate the array, will work for 1D or 2D case
                    SLANG_CUDA_RETURN_ON_FAIL(cuArrayCreate(&tex->m_cudaArray, &arrayDesc));
                }
            }
        }

        // Work space for holding data for uploading if it needs to be rearranged
        List<uint8_t> workspace;
        auto width = desc.size.width;
        auto height = desc.size.height;
        auto depth = desc.size.depth;
        for (int mipLevel = 0; mipLevel < desc.numMipLevels; ++mipLevel)
        {
            int mipWidth = width >> mipLevel;
            int mipHeight = height >> mipLevel;
            int mipDepth = depth >> mipLevel;

            mipWidth = (mipWidth == 0) ? 1 : mipWidth;
            mipHeight = (mipHeight == 0) ? 1 : mipHeight;
            mipDepth = (mipDepth == 0) ? 1 : mipDepth;

            // If it's a cubemap then the depth is always 6
            if (desc.type == Resource::Type::TextureCube)
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
                    desc.type == Resource::Type::Texture1D ||
                    desc.type == Resource::Type::Texture2D ||
                    desc.type == Resource::Type::TextureCube);

                // TODO(JS): Here I assume that arrays are just held contiguously within a 'face'
                // This seems reasonable and works with the Copy3D.
                const size_t faceSizeInBytes = elementSize * mipWidth * mipHeight;

                Index faceCount = desc.arraySize;
                if (desc.type == Resource::Type::TextureCube)
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
                if (desc.type == Resource::Type::TextureCube)
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
                    desc.type == Resource::Type::Texture1D ||
                    desc.type == Resource::Type::Texture2D ||
                    desc.type == Resource::Type::TextureCube);

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

                if (desc.type == Resource::Type::TextureCube)
                {
                    copyParam.Depth *= 6;
                }

                SLANG_CUDA_RETURN_ON_FAIL(cuMemcpy3D(&copyParam));
            }
            else
            {
                switch (desc.type)
                {
                case Resource::Type::Texture1D:
                case Resource::Type::Texture2D:
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
                case Resource::Type::Texture3D:
                case Resource::Type::TextureCube:
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

            // Create handle for uav.
            SLANG_CUDA_RETURN_ON_FAIL(cuSurfObjectCreate(&tex->m_cudaSurfObj, &resDesc));
            
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

    virtual Result createBufferResource(
        Resource::Usage initialUsage,
        const BufferResource::Desc& desc,
        const void* initData,
        BufferResource** outResource) override
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

    virtual Result createTextureView(
        TextureResource* texture, ResourceView::Desc const& desc, ResourceView** outView) override
    {
        RefPtr<CUDAResourceView> view = new CUDAResourceView();
        view->desc = desc;
        view->textureResource = dynamic_cast<TextureCUDAResource*>(texture);
        *outView = view.detach();
        return SLANG_OK;
    }

    virtual Result createBufferView(
        BufferResource* buffer, ResourceView::Desc const& desc, ResourceView** outView) override
    {
        RefPtr<CUDAResourceView> view = new CUDAResourceView();
        view->desc = desc;
        view->memoryResource = dynamic_cast<MemoryCUDAResource*>(buffer);
        *outView = view.detach();
        return SLANG_OK;
    }

    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout, ShaderObjectLayout** outLayout) override
    {
        RefPtr<CUDAShaderObjectLayout> cudaLayout;
        cudaLayout = new CUDAShaderObjectLayout(typeLayout);
        *outLayout = cudaLayout.detach();
        return SLANG_OK;
    }

    virtual Result createRootShaderObjectLayout(
        slang::ProgramLayout* layout, ShaderObjectLayout** outLayout) override
    {
        RefPtr<CUDAProgramLayout> cudaLayout;
        cudaLayout = new CUDAProgramLayout(layout);
        cudaLayout->programLayout = layout;
        *outLayout = cudaLayout.detach();
        return SLANG_OK;
    }

    virtual Result createShaderObject(ShaderObjectLayout* layout, ShaderObject** outObject) override
    {
        RefPtr<CUDAShaderObject> result = new CUDAShaderObject();
        SLANG_RETURN_ON_FAIL(result->init(this, dynamic_cast<CUDAShaderObjectLayout*>(layout)));
        *outObject = result.detach();
        return SLANG_OK;
    }

    virtual Result createRootShaderObject(ShaderObjectLayout* layout, ShaderObject** outObject) override
    {
        RefPtr<CUDARootShaderObject> result = new CUDARootShaderObject();
        SLANG_RETURN_ON_FAIL(result->init(this, dynamic_cast<CUDAShaderObjectLayout*>(layout)));
        *outObject = result.detach();
        return SLANG_OK;
    }

    virtual Result bindRootShaderObject(PipelineType pipelineType, ShaderObject* object) override
    {
        currentRootObject = dynamic_cast<CUDARootShaderObject*>(object);
        if (currentRootObject)
            return SLANG_OK;
        return SLANG_E_INVALID_ARG;
    }

    virtual Result createProgram(const ShaderProgram::Desc& desc, ShaderProgram** outProgram) override
    {
        if (desc.kernelCount != 1)
            return SLANG_E_INVALID_ARG;
        RefPtr<CUDAShaderProgram> cudaProgram = new CUDAShaderProgram();
        SLANG_CUDA_RETURN_ON_FAIL(cuModuleLoadData(&cudaProgram->cudaModule, desc.kernels[0].codeBegin));
        SLANG_CUDA_RETURN_ON_FAIL(
            cuModuleGetFunction(&cudaProgram->cudaKernel, cudaProgram->cudaModule, desc.kernels[0].entryPointName));
        cudaProgram->kernelName = desc.kernels[0].entryPointName;
        *outProgram = cudaProgram.detach();
        return SLANG_OK;
    }

    virtual Result createComputePipelineState(const ComputePipelineStateDesc& desc, PipelineState** outState) override
    {
        RefPtr<CUDAPipelineState> state = new CUDAPipelineState();
        state->shaderProgram = dynamic_cast<CUDAShaderProgram*>(desc.program);
        *outState = state.detach();
        return Result();
    }

    virtual void* map(BufferResource* buffer, MapFlavor flavor) override
    {
        return dynamic_cast<MemoryCUDAResource*>(buffer)->m_cudaMemory;
    }

    virtual void unmap(BufferResource* buffer) override
    {
        SLANG_UNUSED(buffer);
    }

    virtual void setPipelineState(PipelineType pipelineType, PipelineState* state) override
    {
        SLANG_ASSERT(pipelineType == PipelineType::Compute);
        currentPipeline = dynamic_cast<CUDAPipelineState*>(state);
    }

    virtual void dispatchCompute(int x, int y, int z) override
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
                    ? currentRootObject->bufferResource->getBindlessHandle()
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

    virtual void submitGpuWork() override {}

    virtual void waitForGpu() override
    {
        auto result = cudaDeviceSynchronize();
        SLANG_ASSERT(result == CUDA_SUCCESS);
    }

    virtual RendererType getRendererType() const override { return RendererType::CUDA; }

public:
    // Unused public interfaces. These functions are not supported on CUDA.
    virtual const Slang::List<Slang::String>& getFeatures() override
    {
        static Slang::List<Slang::String> featureSet;
        return featureSet;
    }
    virtual void setClearColor(const float color[4]) override
    {
        SLANG_UNUSED(color);
    }
    virtual void clearFrame() override {}
    virtual void presentFrame() override {}
    virtual TextureResource::Desc getSwapChainTextureDesc() override
    {
        return TextureResource::Desc();
    }
    
    virtual Result createSamplerState(SamplerState::Desc const& desc, SamplerState** outSampler) override
    {
        SLANG_UNUSED(desc);
        *outSampler = nullptr;
        return SLANG_OK;
    }
    
    virtual Result createInputLayout(
        const InputElementDesc* inputElements,
        UInt inputElementCount,
        InputLayout** outLayout) override
    {
        SLANG_UNUSED(inputElements);
        SLANG_UNUSED(inputElementCount);
        SLANG_UNUSED(outLayout);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual Result createDescriptorSetLayout(
        const DescriptorSetLayout::Desc& desc, DescriptorSetLayout** outLayout) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outLayout);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual Result createPipelineLayout(const PipelineLayout::Desc& desc, PipelineLayout** outLayout) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outLayout);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual Result createDescriptorSet(DescriptorSetLayout* layout, DescriptorSet** outDescriptorSet) override
    {
        SLANG_UNUSED(layout);
        SLANG_UNUSED(outDescriptorSet);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual Result createGraphicsPipelineState(const GraphicsPipelineStateDesc& desc, PipelineState** outState) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outState);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SlangResult captureScreenSurface(Surface& surfaceOut) override
    {
        SLANG_UNUSED(surfaceOut);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual void setPrimitiveTopology(PrimitiveTopology topology) override
    {
        SLANG_UNUSED(topology);
    }
    virtual void setDescriptorSet(
        PipelineType pipelineType,
        PipelineLayout* layout,
        UInt index,
        DescriptorSet* descriptorSet) override
    {
        SLANG_UNUSED(pipelineType);
        SLANG_UNUSED(layout);
        SLANG_UNUSED(index);
        SLANG_UNUSED(descriptorSet);
    }
    virtual void setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        BufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) override
    {
        SLANG_UNUSED(startSlot);
        SLANG_UNUSED(slotCount);
        SLANG_UNUSED(buffers);
        SLANG_UNUSED(strides);
        SLANG_UNUSED(offsets);
    }
    virtual void setIndexBuffer(BufferResource* buffer, Format indexFormat, UInt offset = 0) override
    {
        SLANG_UNUSED(buffer);
        SLANG_UNUSED(indexFormat);
        SLANG_UNUSED(offset);
    }
    virtual void setDepthStencilTarget(ResourceView* depthStencilView) override
    {
        SLANG_UNUSED(depthStencilView);
    }
    virtual void setViewports(UInt count, Viewport const* viewports) override
    {
        SLANG_UNUSED(count);
        SLANG_UNUSED(viewports);
    }
    virtual void setScissorRects(UInt count, ScissorRect const* rects) override
    {
        SLANG_UNUSED(count);
        SLANG_UNUSED(rects);
    }
    virtual void draw(UInt vertexCount, UInt startVertex) override
    {
        SLANG_UNUSED(vertexCount);
        SLANG_UNUSED(startVertex);
    }
    virtual void drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex) override
    {
        SLANG_UNUSED(indexCount);
        SLANG_UNUSED(startIndex);
        SLANG_UNUSED(baseVertex);
    }
};

SlangResult CUDAShaderObject::init(Renderer* renderer, CUDAShaderObjectLayout* typeLayout)
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

SlangResult CUDARootShaderObject::init(Renderer* renderer, CUDAShaderObjectLayout* typeLayout)
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

Renderer* createCUDARenderer() { return new CUDARenderer(); }
#else
Renderer* createCUDARenderer() { return nullptr; }
#endif

}
