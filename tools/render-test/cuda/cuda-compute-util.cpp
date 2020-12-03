
#include "cuda-compute-util.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-std-writers.h"
#include "../../source/core/slang-token-reader.h"
#include "../../source/core/slang-semantic-version.h"

#include "../bind-location.h"

#include <cuda.h>

#include <cuda_runtime_api.h>

// TODO: should conditionalize this on OptiX being present
#ifdef RENDER_TEST_OPTIX

// The `optix_stubs.h` header produces warnings when compiled with MSVC
#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif
#include <optix.h>
#include <optix_function_table_definition.h>
#include <optix_stubs.h>
#endif

namespace renderer_test {
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
    CUDAErrorInfo(const char* filePath, int lineNo, const char* errorName = nullptr, const char* errorString = nullptr):
        m_filePath(filePath),
        m_lineNo(lineNo),
        m_errorName(errorName),
        m_errorString(errorString)
    {
    }
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

        //Slang::signalUnexpectedError(builder.getBuffer());
        return SLANG_FAIL;
    }

    const char* m_filePath;
    int m_lineNo;
    const char* m_errorName;
    const char* m_errorString;
};

#if 1
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

#define SLANG_CUDA_HANDLE_ERROR(x) _handleCUDAError(_res, __FILE__, __LINE__)

#else
// If this code path is enabled, errors are not reported, but can have an assert enabled

static SlangResult _handleCUDAError(CUresult cuResult)
{
    SLANG_UNUSED(cuResult);
    //SLANG_ASSERT(!"Failed CUDA call");
    return SLANG_FAIL;
}

static SlangResult _handleCUDAError(cudaError_t error)
{
    SLANG_UNUSED(error);
    //SLANG_ASSERT(!"Failed CUDA call");
    return SLANG_FAIL;
}

#define SLANG_CUDA_HANDLE_ERROR(x) _handleCUDAError(_res)
#endif

#define SLANG_CUDA_RETURN_ON_FAIL(x) { auto _res = x; if (_isError(_res)) return SLANG_CUDA_HANDLE_ERROR(_res); }
#define SLANG_CUDA_RETURN_WITH_REPORT_ON_FAIL(x, r) \
    { \
        auto _res = x; \
        if (_isError(_res)) \
        { \
            return (r == CUDAReportStyle::Normal) ? SLANG_CUDA_HANDLE_ERROR(_res) : SLANG_FAIL; \
        } \
    } \

#define SLANG_CUDA_ASSERT_ON_FAIL(x) { auto _res = x; if (_isError(_res)) { SLANG_ASSERT(!"Failed CUDA call"); }; }

#ifdef RENDER_TEST_OPTIX

static bool _isError(OptixResult result) { return result != OPTIX_SUCCESS; }

#if 1
static SlangResult _handleOptixError(OptixResult result, char const* file, int line)
{
    fprintf(stderr, "%s(%d): optix: %s (%s)\n",
        file,
        line,
        optixGetErrorString(result),
        optixGetErrorName(result));
    return SLANG_FAIL;
}
#define SLANG_OPTIX_HANDLE_ERROR(RESULT) _handleOptixError(RESULT, __FILE__, __LINE__)
#else
#define SLANG_OPTIX_HANDLE_ERROR(RESULT) SLANG_FAIL
#endif

#define SLANG_OPTIX_RETURN_ON_FAIL(EXPR) do { auto _res = EXPR; if(_isError(_res)) return SLANG_OPTIX_HANDLE_ERROR(_res); } while(0)

void _optixLogCallback(unsigned int level, const char* tag, const char* message, void* userData)
{
    fprintf(stderr, "optix: %s (%s)\n",
        message,
        tag);
}

#endif

class MemoryCUDAResource : public CUDAResource
{
public:
    typedef CUDAResource Super;

        /// Dtor
    ~MemoryCUDAResource()
    {
        if (m_cudaMemory)
        {
            SLANG_CUDA_ASSERT_ON_FAIL(cuMemFree(m_cudaMemory));
        }
    }

    static MemoryCUDAResource* asResource(BindSet::Value* value)
    {
        return value ? dynamic_cast<MemoryCUDAResource*>(value->m_target.Ptr()) : nullptr;
    }
        /// Helper function to get the CUDA memory pointer when given a value
    static CUdeviceptr getCUDAData(BindSet::Value* value)
    {
        auto resource = asResource(value);
        return resource ? resource->m_cudaMemory : CUdeviceptr();
    }

    virtual uint64_t getBindlessHandle() override
    {
        return (uint64_t)m_cudaMemory;
    }

    CUdeviceptr m_cudaMemory = CUdeviceptr();
};

class TextureCUDAResource : public CUDAResource
{
public:
    typedef CUDAResource Super;

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

    static TextureCUDAResource* asResource(BindSet::Value* value)
    {
        return value ? dynamic_cast<TextureCUDAResource*>(value->m_target.Ptr()) : nullptr;
    }

    static CUtexObject getTexObject(BindSet::Value* value)
    {
        auto resource = asResource(value);
        // It's an assumption here that 0 is okay for null. Seems to work...
        return resource ? resource->m_cudaTexObj : CUtexObject(0);
    }

    static CUsurfObject getSurfObject(BindSet::Value* value)
    {
        auto resource = asResource(value);
        return resource ? resource->m_cudaSurfObj : CUsurfObject(0);
    }

    virtual uint64_t getBindlessHandle() override
    {
        return (uint64_t)m_cudaTexObj;
    }

    // The texObject is for reading 'texture' like things. This is an opaque type, that's backed by a long long
    CUtexObject m_cudaTexObj = CUtexObject();

    // The surfObj is for reading/writing 'texture like' things, but not for sampling.
    CUsurfObject m_cudaSurfObj = CUsurfObject();

    CUarray m_cudaArray = CUarray();
    CUmipmappedArray m_cudaMipMappedArray = CUmipmappedArray();
};

class ScopeCUDAModule
{
public:

    operator CUmodule () const { return m_module; }

    ScopeCUDAModule(): m_module(nullptr) {}
    SlangResult load(const void* image)
    {
        release(); 
        SLANG_CUDA_RETURN_ON_FAIL(cuModuleLoadData(&m_module, image));
        return SLANG_OK;
    }
    void release()
    {
        if (m_module)
        {
            cuModuleUnload(m_module);
            m_module = nullptr;
        }
    }

    ~ScopeCUDAModule() { release(); }

    CUmodule m_module;
};

class ScopeCUDAStream
{
public:

    SlangResult init(unsigned int flags)
    {
        release();
        SLANG_ASSERT(m_stream == nullptr);
        SLANG_CUDA_RETURN_ON_FAIL(cuStreamCreate(&m_stream, flags));
        return SLANG_OK;
    }

    SlangResult sync()
    {
        if (m_stream)
        {
            SLANG_CUDA_RETURN_ON_FAIL(cuStreamSynchronize(m_stream));
        }
        else
        {
            SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceSynchronize());
        }
        return SLANG_OK;
    }

    void release()
    {
        if (m_stream)
        {
            sync();
            SLANG_CUDA_ASSERT_ON_FAIL(cuStreamDestroy(m_stream));
            m_stream = nullptr;
        }
    }

    ScopeCUDAStream():m_stream(nullptr) {}

    ~ScopeCUDAStream() { release(); }

    operator CUstream () const { return m_stream; }

    CUstream m_stream;
};

static int _calcSMCountPerMultiProcessor(int major, int minor)
{
    // Defines for GPU Architecture types (using the SM version to determine
    // the # of cores per SM
    struct SMInfo
    {
        int sm;  // 0xMm (hexadecimal notation), M = SM Major version, and m = SM minor version
        int coreCount;
    };

    static const SMInfo infos[] =
    {
        {0x30, 192},
        {0x32, 192},
        {0x35, 192},
        {0x37, 192},
        {0x50, 128},
        {0x52, 128},
        {0x53, 128},
        {0x60,  64},
        {0x61, 128},
        {0x62, 128},
        {0x70,  64},
        {0x72,  64},
        {0x75,  64}
    };

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
        SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&computeMode, cudaDevAttrComputeMode, currentDevice));
        SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&major, cudaDevAttrComputeCapabilityMajor, currentDevice));
        SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&minor, cudaDevAttrComputeCapabilityMinor, currentDevice));

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
            SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&multiProcessorCount, cudaDevAttrMultiProcessorCount, currentDevice));
            SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&clockRate, cudaDevAttrClockRate, currentDevice));
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

class ScopeCUDAContext
{
public:
    ScopeCUDAContext() :
        m_context(nullptr),
        m_device(-1),
        m_deviceIndex(-1)
    {}

    SlangResult init(unsigned int flags, int deviceIndex, CUDAReportStyle reportType = CUDAReportStyle::Normal)
    {
        SLANG_RETURN_ON_FAIL(_initCuda(reportType));

        if (m_context)
        {
            cuCtxDestroy(m_context);
            m_context = nullptr;
        }

        m_deviceIndex = deviceIndex;
        SLANG_CUDA_RETURN_ON_FAIL(cuDeviceGet(&m_device, deviceIndex));

        SLANG_CUDA_RETURN_WITH_REPORT_ON_FAIL(cuCtxCreate(&m_context, flags, m_device), reportType);
        return SLANG_OK;
    }

    SlangResult init(unsigned int flags, CUDAReportStyle reportType = CUDAReportStyle::Normal)
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

        SLANG_CUDA_RETURN_WITH_REPORT_ON_FAIL(cuCtxCreate(&m_context, flags, m_device), reportType);
        return SLANG_OK;
    }

    ~ScopeCUDAContext()
    {
        if (m_context)
        {
            cuCtxDestroy(m_context);
        }
    }
    SLANG_FORCE_INLINE operator CUcontext () const { return m_context; }

    int m_deviceIndex;
    CUdevice m_device;
    CUcontext m_context;
};

/* static */SlangResult CUDAComputeUtil::parseFeature(const Slang::UnownedStringSlice& feature, bool& outResult)
{
    outResult = false;

    if (feature.startsWith("cuda_sm_"))
    {
        const UnownedStringSlice versionSlice = UnownedStringSlice(feature.begin() + 8, feature.end());
        SemanticVersion requiredVersion;
        SLANG_RETURN_ON_FAIL(SemanticVersion::parse(versionSlice, '_', requiredVersion));

        // Need to get the version from the cuda device
        ScopeCUDAContext context;
        SLANG_RETURN_ON_FAIL(context.init(0, CUDAReportStyle::Silent));

        const int deviceIndex = context.m_deviceIndex;

        int computeMode = -1;
        SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&computeMode, cudaDevAttrComputeMode, deviceIndex));

        // If we don't have compute mode availability, we can't execute
        if (computeMode == cudaComputeModeProhibited)
        {
            return SLANG_FAIL;
        }

        int major, minor;
        SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&major,  cudaDevAttrComputeCapabilityMajor, deviceIndex));
        SLANG_CUDA_RETURN_ON_FAIL(cudaDeviceGetAttribute(&minor, cudaDevAttrComputeCapabilityMinor, deviceIndex));

        SemanticVersion actualVersion;
        actualVersion.set(major, minor);

        outResult = actualVersion >= requiredVersion;

        return SLANG_OK;
    }

    return SLANG_FAIL;
}

/* static */bool CUDAComputeUtil::hasFeature(const Slang::UnownedStringSlice& feature)
{
    bool res;
    return SLANG_SUCCEEDED(parseFeature(feature, res)) ? res : false;
}

/* static */bool CUDAComputeUtil::canCreateDevice()
{
    ScopeCUDAContext context;
    return SLANG_SUCCEEDED(context.init(0, CUDAReportStyle::Silent));
}

static bool _hasReadAccess(SlangResourceAccess access)
{
    return access = SLANG_RESOURCE_ACCESS_READ || access == SLANG_RESOURCE_ACCESS_READ_WRITE;
}

static bool _hasWriteAccess(SlangResourceAccess access)
{
    return access == SLANG_RESOURCE_ACCESS_READ_WRITE;
}

/* static */SlangResult CUDAComputeUtil::createTextureResource(const ShaderInputLayoutEntry& srcEntry, slang::TypeLayoutReflection* typeLayout, RefPtr<CUDAResource>& outResource)
{
    SlangResourceAccess access = SLANG_RESOURCE_ACCESS_READ;
    SlangResourceShape baseShape = SLANG_TEXTURE_2D;
    if (typeLayout)
    {
        auto type = typeLayout->getType();
        auto shape = type->getResourceShape();
        access = type->getResourceAccess();

        if (!(access == SLANG_RESOURCE_ACCESS_READ || access == SLANG_RESOURCE_ACCESS_READ_WRITE))
        {
            SLANG_ASSERT(!"Only read or read write currently supported");
            return SLANG_FAIL;
        }
        baseShape = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;
    }
    else
    {
        if (srcEntry.textureDesc.isCube)
        {
            baseShape = SLANG_TEXTURE_CUBE;
        }
        else
        {
            switch (srcEntry.textureDesc.dimension)
            {
            case 1:
                baseShape = SLANG_TEXTURE_1D;
                break;
            case 2:
                baseShape = SLANG_TEXTURE_2D;
                break;
            case 3:
                baseShape = SLANG_TEXTURE_3D;
                break;
            default:
                break;
            }
        }
        if (srcEntry.textureDesc.isRWTexture)
            access = SLANG_RESOURCE_ACCESS_READ_WRITE;
    }
    CUresourcetype resourceType = CU_RESOURCE_TYPE_ARRAY;

    InputTextureDesc textureDesc = srcEntry.textureDesc;

    if (_hasWriteAccess(access))
    {
        textureDesc.mipMapCount = 1;
    }
    
    // CUDA wants the unused dimensions to be 0.
    // Might need to specially handle elsewhere
    int width = textureDesc.size;
    int height = 0;
    int depth = 0;

    switch (baseShape)
    {
        case SLANG_TEXTURE_1D:
        {
            break;
        }
        case SLANG_TEXTURE_2D:
        {
            height = textureDesc.size;
            break;
        }
        case SLANG_TEXTURE_3D:
        {
            height = textureDesc.size;
            depth = textureDesc.size;
            break;
        }
        case SLANG_TEXTURE_CUBE:
        {
            height = width;
            depth = 1;
            break;
        }
        default:
        {
            SLANG_ASSERT(!"Type not supported");
            return SLANG_FAIL;
        }
    }
    
    TextureData texData;
    generateTextureData(texData, textureDesc);

    auto mipLevels = texData.mipLevels;

    RefPtr<TextureCUDAResource> tex = new TextureCUDAResource;

    size_t elementSize = 0;

    {
        CUarray_format format = CU_AD_FORMAT_FLOAT;
        int numChannels = 0;

        switch (textureDesc.format)
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

        if (mipLevels > 1)
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

            if (textureDesc.arrayLength > 1)
            {
                if (baseShape == SLANG_TEXTURE_1D ||
                    baseShape == SLANG_TEXTURE_2D ||
                    baseShape == SLANG_TEXTURE_CUBE)
                {
                    arrayDesc.Flags |= CUDA_ARRAY3D_LAYERED;
                    arrayDesc.Depth = textureDesc.arrayLength;
                }
                else
                {
                    SLANG_ASSERT(!"Arrays only supported for 1D and 2D");
                    return SLANG_FAIL;
                }
            }
            
            if (baseShape == SLANG_TEXTURE_CUBE)
            {
                arrayDesc.Flags |= CUDA_ARRAY3D_CUBEMAP;
                arrayDesc.Depth *= 6;
            }

            SLANG_CUDA_RETURN_ON_FAIL(cuMipmappedArrayCreate(&tex->m_cudaMipMappedArray, &arrayDesc, mipLevels));
        }
        else
        {
            resourceType = CU_RESOURCE_TYPE_ARRAY;

            if (textureDesc.arrayLength > 1)
            {
                if (baseShape == SLANG_TEXTURE_1D || baseShape == SLANG_TEXTURE_2D || baseShape == SLANG_TEXTURE_CUBE)
                {
                    SLANG_ASSERT(!"Only 1D, 2D and Cube arrays supported");
                    return SLANG_FAIL;
                }

                CUDA_ARRAY3D_DESCRIPTOR arrayDesc;
                memset(&arrayDesc, 0, sizeof(arrayDesc));

                // Set the depth as the array length
                arrayDesc.Depth = textureDesc.arrayLength;
                if (baseShape == SLANG_TEXTURE_CUBE)
                {
                    arrayDesc.Depth *= 6;
                }
                
                arrayDesc.Height = height;
                arrayDesc.Width = width;
                arrayDesc.Format = format;
                arrayDesc.NumChannels = numChannels;

                if (baseShape == SLANG_TEXTURE_CUBE)
                {
                    arrayDesc.Flags |= CUDA_ARRAY3D_CUBEMAP;
                }

                SLANG_CUDA_RETURN_ON_FAIL(cuArray3DCreate(&tex->m_cudaArray, &arrayDesc));
            }
            else if (baseShape == SLANG_TEXTURE_3D || baseShape == SLANG_TEXTURE_CUBE)
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
                if (baseShape == SLANG_TEXTURE_CUBE)
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

                arrayDesc.Width = width;
                arrayDesc.Height = height;
                arrayDesc.Format = format;
                arrayDesc.NumChannels = numChannels;

                // Allocate the array, will work for 1D or 2D case
                SLANG_CUDA_RETURN_ON_FAIL(cuArrayCreate(&tex->m_cudaArray, &arrayDesc));
            }
        }
    }

    // Work space for holding data for uploading if it needs to be rearranged
    List<uint8_t> workspace;

    for (int mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
    {
        int mipWidth = width >> mipLevel;
        int mipHeight = height >> mipLevel;
        int mipDepth = depth >> mipLevel;

        mipWidth = (mipWidth == 0) ? 1 : mipWidth;
        mipHeight = (mipHeight == 0) ? 1 : mipHeight;
        mipDepth = (mipDepth == 0) ? 1 : mipDepth;

        // If it's a cubemap then the depth is always 6
        if (baseShape == SLANG_TEXTURE_CUBE)
        {
            mipDepth = 6;
        }

        auto dstArray = tex->m_cudaArray;
        if (tex->m_cudaMipMappedArray)
        {
            // Get the array for the mip level
            SLANG_CUDA_RETURN_ON_FAIL(cuMipmappedArrayGetLevel(&dstArray, tex->m_cudaMipMappedArray, mipLevel));
        }
        SLANG_ASSERT(dstArray);

        // Check using the desc to see if it's plausible
        {
            CUDA_ARRAY_DESCRIPTOR arrayDesc;
            SLANG_CUDA_RETURN_ON_FAIL(cuArrayGetDescriptor(&arrayDesc, dstArray));

            SLANG_ASSERT(mipWidth == arrayDesc.Width);
            SLANG_ASSERT(mipHeight == arrayDesc.Height || (mipHeight == 1 && arrayDesc.Height == 0));
        }

        const void* srcDataPtr = nullptr;

        if (textureDesc.arrayLength > 1)
        {
            SLANG_ASSERT(baseShape == SLANG_TEXTURE_1D || baseShape == SLANG_TEXTURE_2D || baseShape == SLANG_TEXTURE_CUBE);

            // TODO(JS): Here I assume that arrays are just held contiguously within a 'face'
            // This seems reasonable and works with the Copy3D.
            const size_t faceSizeInBytes = elementSize * mipWidth * mipHeight;

            Index faceCount = textureDesc.arrayLength;
            if (baseShape == SLANG_TEXTURE_CUBE)
            {
                faceCount *= 6;
            }

            const size_t mipSizeInBytes = faceSizeInBytes * faceCount;
            workspace.setCount(mipSizeInBytes);

            // We need to add the face data from each mip
            // We iterate over face count so we copy all of the cubemap faces
            for (Index j = 0; j < faceCount; j++)
            {
                const auto& srcData = texData.dataBuffer[mipLevel + j * mipLevels];
                // Copy over to the workspace to make contiguous
                ::memcpy(workspace.begin() + faceSizeInBytes * j, srcData.getBuffer(), faceSizeInBytes);
            }

            srcDataPtr = workspace.getBuffer();
        }
        else
        {
            if (baseShape == SLANG_TEXTURE_CUBE)
            {
                size_t faceSizeInBytes = elementSize * mipWidth * mipHeight;

                workspace.setCount(faceSizeInBytes * 6);

                // Copy the data over to make contiguous
                for (Index j = 0; j < 6; j++)
                {
                    const auto& srcData = texData.dataBuffer[mipLevels * j + mipLevel];
                    SLANG_ASSERT(mipWidth * mipHeight == srcData.getCount());

                    ::memcpy(workspace.getBuffer() + faceSizeInBytes * j, srcData.getBuffer(), faceSizeInBytes);
                }

                srcDataPtr = workspace.getBuffer();
            }
            else
            {
                const auto& srcData = texData.dataBuffer[mipLevel];
                SLANG_ASSERT(mipWidth * mipHeight * mipDepth == srcData.getCount());

                srcDataPtr = srcData.getBuffer();
            }
        }

        if (textureDesc.arrayLength > 1)
        {
            SLANG_ASSERT(baseShape == SLANG_TEXTURE_1D || baseShape == SLANG_TEXTURE_2D || baseShape == SLANG_TEXTURE_CUBE);

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
            copyParam.Depth = textureDesc.arrayLength;

            if (baseShape == SLANG_TEXTURE_CUBE)
            {
                copyParam.Depth *= 6;
            }

            SLANG_CUDA_RETURN_ON_FAIL(cuMemcpy3D(&copyParam));
        }
        else
        {
            switch (baseShape)
            {
                case SLANG_TEXTURE_1D:
                case SLANG_TEXTURE_2D:
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
                case SLANG_TEXTURE_3D:
                case SLANG_TEXTURE_CUBE:
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

        if (_hasWriteAccess(access))
        {
            // If has write access it's effectively UAV, and so doesn't have sampling available
            SLANG_CUDA_RETURN_ON_FAIL(cuSurfObjectCreate(&tex->m_cudaSurfObj, &resDesc));
        }
        else
        {
            // If read only it's a SRV and can sample, but cannot write
            CUDA_TEXTURE_DESC texDesc;
            memset(&texDesc, 0, sizeof(CUDA_TEXTURE_DESC));
            texDesc.addressMode[0] = CU_TR_ADDRESS_MODE_WRAP;
            texDesc.addressMode[1] = CU_TR_ADDRESS_MODE_WRAP;
            texDesc.addressMode[2] = CU_TR_ADDRESS_MODE_WRAP;
            texDesc.filterMode = CU_TR_FILTER_MODE_LINEAR;
            texDesc.flags = CU_TRSF_NORMALIZED_COORDINATES;

            SLANG_CUDA_RETURN_ON_FAIL(cuTexObjectCreate(&tex->m_cudaTexObj, &resDesc, &texDesc, nullptr));
        }

    }

    outResource = tex;
    return SLANG_OK;
}

    /// Load kernel code and invoke a compute program
    ///
    /// Assumes that data for binding the kernel parameters is already
    /// set up in `outContext.`
    ///
static SlangResult _invokeComputeProgram(
    CUcontext cudaContext,
    ScopeCUDAStream& cudaStream,
    ScopeCUDAModule& cudaModule,
    const ShaderCompilerUtil::OutputAndLayout& outputAndLayout,
    const uint32_t dispatchSize[3],
    CUDAComputeUtil::Context& outContext)
{
    auto reflection = slang::ProgramLayout::get(outputAndLayout.output.getRequestForReflection());

    auto& bindSet = outContext.m_bindSet;
    auto& bindRoot = outContext.m_bindRoot;

    // The global-scope shader parameters in the input Slang program
    // will be collected into a single `__constant__` global variable
    // in the output CUDA module.
    //
    // We need to query the address of the `__constant__` variable
    // so that we can copy parameter data into it when invoking
    // a kernel.
    //
    // The Slang compiler always names this symbol `SLANG_globalParams`
    // so that it is easy to look up independent of the module or
    // entry point in question.
    //
    CUdeviceptr globalParamsSymbol = 0;
    size_t globalParamsSymbolSize = 0;
    cuModuleGetGlobal(&globalParamsSymbol, &globalParamsSymbolSize, cudaModule, "SLANG_globalParams");

    slang::EntryPointReflection* entryPoint = nullptr;
    auto entryPointCount = reflection->getEntryPointCount();
    SLANG_ASSERT(entryPointCount == 1);

    entryPoint = reflection->getEntryPointByIndex(0);

    const char* entryPointName = entryPoint->getName();

    // Get the entry point
    CUfunction cudaEntryPoint;
    SLANG_CUDA_RETURN_ON_FAIL(cuModuleGetFunction(&cudaEntryPoint, cudaModule, entryPointName));

    // Get the max threads per block for this function

    int maxTheadsPerBlock;
    SLANG_CUDA_RETURN_ON_FAIL(cuFuncGetAttribute(&maxTheadsPerBlock, CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK, cudaEntryPoint));

    int sharedSizeInBytes;
    SLANG_CUDA_RETURN_ON_FAIL(cuFuncGetAttribute(&sharedSizeInBytes, CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES, cudaEntryPoint));

    // A single CUDA kernel can be invoked with thread groups
    // of different shapes/sizes, but an HLSL/Slang compute
    // kernel always has a fixed thread group shape baked in.
    // We use reflection to query the thread-group size that
    // the kernel expects, so that we can use the right size
    // when invoking the kernel.
    //
    SlangUInt numThreadsPerAxis[3];
    entryPoint->getComputeThreadGroupSize(3, numThreadsPerAxis);

    // The argument data for the kernel has been set up in `bindRoot`,
    // which encapsulates global buffers for both the global and
    // entry-point parameter data.
    //
    // In the case of global parameters, we just need to extract the
    // device address of the parameter data, so we can copy it into
    // the `SLANG_globalParams` symbol.
    //
    {
        CUdeviceptr globalParamsCUDAData = MemoryCUDAResource::getCUDAData(bindRoot.getRootValue());
        cudaMemcpyAsync(
            (void*) globalParamsSymbol,
            (void*) globalParamsCUDAData,
            globalParamsSymbolSize,
            cudaMemcpyDeviceToDevice,
            cudaStream);
    }
    //
    // In the case of the entry-point parameters, we have to deal with
    // two different wrinkles.
    //
    // First, the `bindRoot` will have the entry-point argument data
    // stored in a GPU-memory buffer, but we actually need it to be
    // in host CPU memory. We handle that for now by allocating a
    // temporary host memory buffer (if needed) and copying the data
    // from device to host.
    //
    auto entryPointBindValue = bindRoot.getEntryPointValue();
    CUdeviceptr entryPointCUDAData = MemoryCUDAResource::getCUDAData(entryPointBindValue);
    size_t entryPointDataSize = entryPointBindValue ? entryPointBindValue->m_sizeInBytes : 0;
    void* entryPointHostData = nullptr;
    if(entryPointDataSize)
    {
        entryPointHostData = alloca(entryPointDataSize);
        cudaMemcpy(entryPointHostData, (void*)entryPointCUDAData, entryPointDataSize, cudaMemcpyDeviceToHost);
    }
    //
    // Second, the argument data for the entry-point parameters has
    // been allocated and filled in as a single buffer, but `cuLaunchKernel`
    // defaults to taking pointers to each of the kernel arguments.
    //
    // We could loop over the entry-point parameters using the refleciton
    // information, and set up a pointer to each using the offset stored
    // for it in the reflection data. Such an approach would require
    // us to create and fill in a dynamically-sized array here.
    //
    // Instead, we take advantage of a documented but seldom-used feature
    // of `cuLaunchKernel` that allows the argument data for all of the
    // kernel "launch parameters" to be specified as a single buffer.
    //
    void* extraOptions[] = {
        CU_LAUNCH_PARAM_BUFFER_POINTER, (void*) entryPointHostData,
        CU_LAUNCH_PARAM_BUFFER_SIZE, &entryPointDataSize,
        CU_LAUNCH_PARAM_END,
    };

    // Once we have all the decessary data extracted and/or
    // set up, we can launch the kernel and see what happens.
    //
    auto cudaLaunchResult = cuLaunchKernel(cudaEntryPoint,
        dispatchSize[0], dispatchSize[1], dispatchSize[2], 
        int(numThreadsPerAxis[0]), int(numThreadsPerAxis[1]), int(numThreadsPerAxis[2]),        // Threads per block
        0,              // Shared memory size
        cudaStream,     // Stream. 0 is no stream.
        nullptr,        // Not using traditional argument passing
        extraOptions);  // Instead passing kernel arguments via "extra" options
    SLANG_CUDA_RETURN_ON_FAIL(cudaLaunchResult);

    // Do a sync here. Makes sure any issues are detected early and not on some implicit sync
    SLANG_RETURN_ON_FAIL(cudaStream.sync());

    return SLANG_OK;
}

#ifdef RENDER_TEST_OPTIX
    /// Load kernel code and invoke a ray-tracing program
    ///
    /// Assumes that data for binding the kernel parameters is already
    /// set up in `outContext.`
    ///
    /// Currently only works for programs that have a single
    /// ray generation shader and no other entry points.
    ///
static SlangResult _loadAndInvokeRayTracingProgram(
    CUcontext cudaContext,
    ScopeCUDAStream& cudaStream,
    const ShaderCompilerUtil::OutputAndLayout& outputAndLayout,
    const uint32_t dispatchSize[3],
    CUDAComputeUtil::Context& outContext)
{
    SLANG_OPTIX_RETURN_ON_FAIL(optixInit());

    OptixDeviceContextOptions optixOptions = {};

#if _DEBUG
    optixOptions.logCallbackFunction = &_optixLogCallback;
    optixOptions.logCallbackLevel = 4;
#endif

    OptixDeviceContext optixContext = nullptr;
    SLANG_OPTIX_RETURN_ON_FAIL(optixDeviceContextCreate(cudaContext, &optixOptions, &optixContext));

    enum { kOptixLogSize = 2*1024 };
    char log[kOptixLogSize];
    size_t logSize = sizeof(log);

    OptixPipelineCompileOptions optixPipelineCompileOptions = {};
    optixPipelineCompileOptions.pipelineLaunchParamsVariableName = "SLANG_globalParams";

    // We need to load modules from the PTX code available to us,
    // and then also create program groups from the kernels
    // in those modules.
    //
    // For now we will only support program groups with a single
    // kernel in them, and will create one per entry point.
    //
    Index entryPointCount = outputAndLayout.output.kernelDescs.getCount();
    List<OptixProgramGroup> optixProgramGroups;
    List<String> names;

    OptixShaderBindingTable optixSBT = {};

    for( Index ee = 0; ee < entryPointCount; ++ee )
    {
        auto& kernel = outputAndLayout.output.kernelDescs[ee];

        // TODO: The logic here assumes that each kernel will
        // come from its own independent module, and has no
        // provisiion for loading modules that might contain
        // multiple entry points.
        //
        OptixModuleCompileOptions optixModuleCompileOptions = {};
        OptixModule optixModule;
        SLANG_OPTIX_RETURN_ON_FAIL(optixModuleCreateFromPTX(
            optixContext,
            &optixModuleCompileOptions,
            &optixPipelineCompileOptions,
            (char const*) kernel.codeBegin,
            kernel.getCodeSize(),
            log,
            &logSize,
            &optixModule));

        // TODO: The logic here only handles ray-generation entry points.
        //
        // It would seem simple to extend this to handle other entry
        // point types, by inspecting the stage of the entry points
        // being loaded, and this is indeed true for the subset of
        // stages that map one-to-one with OptiX "program groups."
        //
        // The sticking point is "hit groups" which require a collection
        // of entry points to be specified together (insersection,
        // any hit, and closest hit). A hit group can comprise between
        // zero and three entry points.
        //
        // The catch for us is how to determine which entry points
        // should be grouped to form hit groups. Should this be
        // implied in the input code (either by naming convention
        // or by new Slang language features)? Should this be set
        // up via command-line arguments or something akin to
        // `//TEST_INPUT` lines?

        OptixProgramGroupOptions optixProgramGroupOptions = {};

        OptixProgramGroupDesc optixProgramGroupDesc = {};
        optixProgramGroupDesc.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
        optixProgramGroupDesc.raygen.module = optixModule;

        String name = String("__raygen__") + kernel.entryPointName;
        names.add(name);
        optixProgramGroupDesc.raygen.entryFunctionName = name.begin();

        OptixProgramGroup optixProgramGroup = nullptr;
        SLANG_OPTIX_RETURN_ON_FAIL(optixProgramGroupCreate(
            optixContext,
            &optixProgramGroupDesc,
            1,
            &optixProgramGroupOptions,
            log,
            &logSize,
            &optixProgramGroup));

        optixProgramGroups.add(optixProgramGroup);

        {
            CUdeviceptr rayGenRecordPtr;
            size_t rayGenRecordSize = OPTIX_SBT_RECORD_HEADER_SIZE;

            SLANG_CUDA_RETURN_ON_FAIL(cudaMalloc((void**) &rayGenRecordPtr, rayGenRecordSize));

            struct { char data[OPTIX_SBT_RECORD_HEADER_SIZE]; } rayGenRecordData;
            SLANG_OPTIX_RETURN_ON_FAIL(optixSbtRecordPackHeader(optixProgramGroup, &rayGenRecordData));

            SLANG_CUDA_RETURN_ON_FAIL(cudaMemcpy(
                (void*) rayGenRecordPtr,
                &rayGenRecordData,
                rayGenRecordSize,
                cudaMemcpyHostToDevice));

            optixSBT.raygenRecord = rayGenRecordPtr;
        }
    }

    OptixPipeline optixPipeline = nullptr;

    OptixPipelineLinkOptions optixPipelineLinkOptions = {};
    optixPipelineLinkOptions.maxTraceDepth = 5;
    optixPipelineLinkOptions.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_FULL;
    optixPipelineLinkOptions.overrideUsesMotionBlur = false;
    SLANG_OPTIX_RETURN_ON_FAIL(optixPipelineCreate(
        optixContext,
        &optixPipelineCompileOptions,
        &optixPipelineLinkOptions,
        optixProgramGroups.getBuffer(),
        (unsigned int)optixProgramGroups.getCount(),
        log,
        &logSize,
        &optixPipeline));


    {
        // The OptiX API complains if we don't fill in a miss record
        // in the SBT, so we will create a dummy one here to represent
        // the lack of any miss shaders.
        //
        OptixProgramGroupOptions optixProgramGroupOptions = {};
        OptixProgramGroupDesc missGroupDesc = {};
        missGroupDesc.kind = OPTIX_PROGRAM_GROUP_KIND_MISS;
        OptixProgramGroup missProgramGroup;
        SLANG_OPTIX_RETURN_ON_FAIL(optixProgramGroupCreate(
            optixContext,
            &missGroupDesc,
            1,
            &optixProgramGroupOptions,
            log,
            &logSize,
            &missProgramGroup));


        CUdeviceptr missRecordPtr;
        size_t missRecordSize = OPTIX_SBT_RECORD_HEADER_SIZE;

        SLANG_CUDA_RETURN_ON_FAIL(cudaMalloc((void**) &missRecordPtr, missRecordSize));

        struct { char data[OPTIX_SBT_RECORD_HEADER_SIZE]; } missRecordData;
        SLANG_OPTIX_RETURN_ON_FAIL(optixSbtRecordPackHeader(missProgramGroup, &missRecordData));

        SLANG_CUDA_RETURN_ON_FAIL(cudaMemcpy(
            (void*) missRecordPtr,
            &missRecordData,
            missRecordSize,
            cudaMemcpyHostToDevice));

        optixSBT.missRecordBase = missRecordPtr;
        optixSBT.missRecordCount = 1;
        optixSBT.missRecordStrideInBytes = (unsigned int)missRecordSize;
    }
    {
        // Okay, we also need a dummy hit group.

        OptixProgramGroupOptions optixProgramGroupOptions = {};
        OptixProgramGroupDesc hitGroupDesc = {};
        hitGroupDesc.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;
        OptixProgramGroup programGroup;
        SLANG_OPTIX_RETURN_ON_FAIL(optixProgramGroupCreate(
            optixContext,
            &hitGroupDesc,
            1,
            &optixProgramGroupOptions,
            log,
            &logSize,
            &programGroup));


        CUdeviceptr recordPtr;
        size_t recordSize = OPTIX_SBT_RECORD_HEADER_SIZE;

        SLANG_CUDA_RETURN_ON_FAIL(cudaMalloc((void**) &recordPtr, recordSize));

        struct { char data[OPTIX_SBT_RECORD_HEADER_SIZE]; } recordData;
        SLANG_OPTIX_RETURN_ON_FAIL(optixSbtRecordPackHeader(programGroup, &recordData));

        SLANG_CUDA_RETURN_ON_FAIL(cudaMemcpy(
            (void*) recordPtr,
            &recordData,
            recordSize,
            cudaMemcpyHostToDevice));

        optixSBT.hitgroupRecordBase = recordPtr;
        optixSBT.hitgroupRecordCount = 1;
        optixSBT.hitgroupRecordStrideInBytes = (unsigned int)recordSize;
    }

    // Work out the args

    auto& bindRoot = outContext.m_bindRoot;

    CUdeviceptr globalParams = 0;
    size_t globalParamsSize;

    if( auto globalArg = bindRoot.getRootValue() )
    {
        globalParams = MemoryCUDAResource::getCUDAData(globalArg);
        globalParamsSize = globalArg->m_sizeInBytes;
    }

    // TODO: The data for entry point parameters needs to be stored
    // into the SBT.
    //
    // The simplest solution here would be to copy data from the `bindRoot`
    // into the SBT at the point where we are setting up the SBT, but
    // a more optimized approach (more similar to what a real applicaiton
    // would do) would be to allocate the SBT first and then have the
    // binding logic write directly into its entries.
    //
    // One big complication here is that there need not necessarily be
    // a one-to-one relationship between the entry points (or entry-point
    // groups) in a compiled ray-tracing pipeline and the entries in
    // the SBT. Each SBT entry is conceptually an "instance" of one
    // of the entry-point groups in the program, and there can be
    // zero, one, or many instances of a given group.
    //
    // Modelling this more completely in `render-test` requires that
    // we start having a model for the "scene" that is being rendered,
    // and how entry point groups are associated with the objects in
    // that scene.
    //
    CUdeviceptr entryPointParams = MemoryCUDAResource::getCUDAData(bindRoot.getEntryPointValue());

    SLANG_OPTIX_RETURN_ON_FAIL(optixLaunch(
        optixPipeline,
        cudaStream,
        globalParams,
        globalParamsSize,
        &optixSBT,
        dispatchSize[0],
        dispatchSize[1],
        dispatchSize[2]));

    SLANG_RETURN_ON_FAIL(cudaStream.sync());

    return SLANG_OK;
}
#endif

    // Fill in runtime handles (e.g. RTTI pointers values and bindless resource handles) in input buffers.
static SlangResult _fillRuntimeHandlesInBuffers(
    const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout,
    CUDAComputeUtil::Context& context,
    ScopeCUDAModule& cudaModule)
{
    Slang::ComPtr<slang::ISession> linkage;
    spCompileRequest_getSession(compilationAndLayout.output.getRequestForReflection(), linkage.writeRef());
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
                        slang::ShaderReflection::get(compilationAndLayout.output.getRequestForReflection());
                    auto concreteType = reflection->findTypeByName(rtti.typeName.getBuffer());
                    ComPtr<ISlangBlob> outName;
                    linkage->getTypeRTTIMangledName(concreteType, outName.writeRef());
                    if (!outName)
                        return SLANG_FAIL;
                    SLANG_CUDA_RETURN_ON_FAIL(cuModuleGetGlobal(
                        (CUdeviceptr*)&ptrValue,
                        nullptr,
                        cudaModule.m_module,
                        (char*)outName->getBufferPointer()));
                }
                break;
            case RTTIDataEntryType::WitnessTable:
                {
                    auto reflection =
                        slang::ShaderReflection::get(compilationAndLayout.output.getRequestForReflection());
                    auto concreteType = reflection->findTypeByName(rtti.typeName.getBuffer());
                    if (!concreteType)
                        return SLANG_FAIL;
                    auto interfaceType = reflection->findTypeByName(rtti.interfaceName.getBuffer());
                    if (!interfaceType)
                        return SLANG_FAIL;
                    uint32_t id = 0xFFFFFFFF;
                    linkage->getTypeConformanceWitnessSequentialID(
                        concreteType, interfaceType, &id);
                    ptrValue = id;
                    break;
                }
            default:
                break;
            }
            if (rtti.offset >= 0 &&
                rtti.offset + sizeof(ptrValue) <=
                    entry.bufferData.getCount() * sizeof(decltype(entry.bufferData[0])))
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
            RefPtr<CUDAResource> resource;
            uint64_t handleValue = 0;
            if (context.m_bindlessResources.TryGetValue(handle.name, resource))
            {
                handleValue = resource->getBindlessHandle();
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

static SlangResult _createBindlessResources(
    const ShaderCompilerUtil::OutputAndLayout& outputAndLayout,
    CUDAComputeUtil::Context& outContext)
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
            RefPtr<CUDAResource> resource;
            CUDAComputeUtil::createTextureResource(entry, nullptr, resource);
            outContext.m_bindlessResources.Add(entry.name, resource);
            break;
        }
        default:
            outStream.print("Unsupported bindless resource type.\n");
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

    /// Fill in the binding information for arguments of a CUDA program.
static SlangResult _setUpArguments(
    CUcontext cudaContext,
    ScopeCUDAStream& cudaStream,
    ScopeCUDAModule& cudaModule,
    const ShaderCompilerUtil::OutputAndLayout& outputAndLayout,
    const uint32_t dispatchSize[3],
    CUDAComputeUtil::Context& outContext)
{
    auto reflection = slang::ProgramLayout::get(outputAndLayout.output.getRequestForReflection());

    auto& bindSet = outContext.m_bindSet;
    auto& bindRoot = outContext.m_bindRoot;

    // Okay now we need to set up binding
    bindRoot.init(&bindSet, reflection, 0);

    // Will set up any root buffers
    bindRoot.addDefaultValues();

    // Now set up the Values from the test

    auto outStream = StdWriters::getOut();

    _createBindlessResources(outputAndLayout, outContext);

    // Fill in RTTI pointers and bindless handles in input buffers before copying
    // it to GPU memory.
    // TODO: enable this for Optix path after it is refactored so that context
    // creation and module loading happens before _setUpArguments.
    if (outputAndLayout.output.desc.pipelineType == PipelineType::Compute)
    {
        SLANG_RETURN_ON_FAIL(_fillRuntimeHandlesInBuffers(outputAndLayout, outContext, cudaModule));
    }

    SLANG_RETURN_ON_FAIL(ShaderInputLayout::addBindSetValues(outputAndLayout.layout.entries, outputAndLayout.sourcePath, outStream, bindRoot));

    ShaderInputLayout::getValueBuffers(outputAndLayout.layout.entries, bindSet, outContext.m_buffers);

    // First create all of the resources for the values

    {
        const auto& values = bindSet.getValues();
        const auto& entries = outputAndLayout.layout.entries;

        for (BindSet::Value* value : values)
        {
            auto typeLayout = value->m_type;
               
            // Get the type kind, if typeLayout is not set we'll assume a 'constant buffer' will do
            slang::TypeReflection::Kind kind = typeLayout ? typeLayout->getKind() : slang::TypeReflection::Kind::ConstantBuffer;
               
            switch (kind)
            {
                case slang::TypeReflection::Kind::ConstantBuffer:
                case slang::TypeReflection::Kind::ParameterBlock:
                {
                    // We can construct the buffers. We can't copy into yet, as we need to set all of the bindings first
                    RefPtr<MemoryCUDAResource> resource = new MemoryCUDAResource;
                    SLANG_CUDA_RETURN_ON_FAIL(cuMemAlloc(&resource->m_cudaMemory, value->m_sizeInBytes));
                    value->m_target = resource;
                    break;
                }
                case slang::TypeReflection::Kind::Resource:
                {
                    auto type = typeLayout->getType();
                    auto shape = type->getResourceShape();

                    auto baseShape = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

                    switch (baseShape)
                    {
                        case SLANG_TEXTURE_1D: 
                        case SLANG_TEXTURE_2D:
                        case SLANG_TEXTURE_3D:
                        case SLANG_TEXTURE_CUBE:
                        {
                            RefPtr<CUDAResource> resource;
                            SLANG_RETURN_ON_FAIL(CUDAComputeUtil::createTextureResource(entries[value->m_userIndex], typeLayout, resource));
                            value->m_target = resource;
                            break;
                        }
                        case SLANG_TEXTURE_BUFFER:
                        {
                            // Need a CUDA impl for these...
                            // For now we can just leave as target will just be nullptr
                            break;
                        }

                        case SLANG_BYTE_ADDRESS_BUFFER:
                        case SLANG_STRUCTURED_BUFFER:
                        {
                            // On CPU we just use the memory in the BindSet buffer, so don't need to create anything
                            RefPtr<MemoryCUDAResource> resource = new MemoryCUDAResource;
                            SLANG_CUDA_RETURN_ON_FAIL(cuMemAlloc(&resource->m_cudaMemory, value->m_sizeInBytes));
                            value->m_target = resource;
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
        bindSet.getBindings(locations, values);

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
                        CUDAComputeUtil::Array array = { CUdeviceptr(), 0 };
                        auto resource = MemoryCUDAResource::asResource(value);
                        if (resource)
                        {
                            array.data = resource->m_cudaMemory;
                            array.count = value->m_elementCount;
                        }

                        location.setUniform(&array, sizeof(array));
                    }
                    break;
                }
                case slang::TypeReflection::Kind::ConstantBuffer:
                case slang::TypeReflection::Kind::ParameterBlock:
                {
                    // These map down to just pointers
                    *location.getUniform<CUdeviceptr>() = MemoryCUDAResource::getCUDAData(value);
                    break;
                }
                case slang::TypeReflection::Kind::Resource:
                {
                    auto type = typeLayout->getType();
                    auto shape = type->getResourceShape();

                    auto access = type->getResourceAccess();

                    const auto baseShape = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

                    switch (baseShape)
                    {
                        case SLANG_STRUCTURED_BUFFER:
                        {
                            CUDAComputeUtil::StructuredBuffer buffer = { CUdeviceptr(), 0 };
                            auto resource = MemoryCUDAResource::asResource(value);
                            if (resource)
                            {
                                buffer.data = resource->m_cudaMemory;
                                buffer.count = value->m_elementCount;
                            }

                            location.setUniform(&buffer, sizeof(buffer));
                            break;
                        }
                        case SLANG_BYTE_ADDRESS_BUFFER:
                        {
                            CUDAComputeUtil::ByteAddressBuffer buffer = { CUdeviceptr(), 0 };

                            auto resource = MemoryCUDAResource::asResource(value);
                            if (resource)
                            {
                                buffer.data = resource->m_cudaMemory;
                                buffer.sizeInBytes = value->m_sizeInBytes;
                            }

                            location.setUniform(&buffer, sizeof(buffer));
                            break;
                        }
                        case SLANG_TEXTURE_1D:
                        case SLANG_TEXTURE_2D:
                        case SLANG_TEXTURE_3D:
                        case SLANG_TEXTURE_CUBE:
                        {
                            if (_hasWriteAccess(access))
                            {
                                *location.getUniform<CUsurfObject>() = TextureCUDAResource::getSurfObject(value);
                            }
                            else
                            {
                                *location.getUniform<CUtexObject>() = TextureCUDAResource::getTexObject(value);
                            }
                            break;
                        }

                    }
                    break;
                }
                default: break;
            }
        }
    }

    // Okay now the memory is all set up, we can copy everything over
    {
        const auto& values = bindSet.getValues();
        for (BindSet::Value* value : values)
        {
            CUdeviceptr cudaMem = MemoryCUDAResource::getCUDAData(value);
            if (value && value->m_data && cudaMem)
            {
                // Okay copy the data over...
                SLANG_CUDA_RETURN_ON_FAIL(cuMemcpyHtoD(cudaMem, value->m_data, value->m_sizeInBytes));
            }
        }
    }

    return SLANG_OK;
}

    /// Read back any output arguments from a CUDA program.
static SlangResult _readBackOutputs(
    CUcontext cudaContext,
    ScopeCUDAStream& cudaStream,
    const ShaderCompilerUtil::OutputAndLayout& outputAndLayout,
    const uint32_t dispatchSize[3],
    CUDAComputeUtil::Context& outContext)
{
    const auto& entries = outputAndLayout.layout.entries;

    for (Index i = 0; i < entries.getCount(); ++i)
    {
        const auto& entry = entries[i];
        BindSet::Value* value = outContext.m_buffers[i];

        if (entry.isOutput)
        {
            // Copy back to CPU memory
            CUdeviceptr cudaMem = MemoryCUDAResource::getCUDAData(value);
            if (value && value->m_data && cudaMem)
            {
                // Okay copy the data back...
                SLANG_CUDA_RETURN_ON_FAIL(cuMemcpyDtoH(value->m_data, cudaMem, value->m_sizeInBytes));
            }
        }
    }

    return SLANG_OK;
}

SlangResult _loadCUDAModule(
    const ShaderCompilerUtil::OutputAndLayout& outputAndLayout,
    ScopeCUDAModule& outModule)
{
    const Index index = outputAndLayout.output.findKernelDescIndex(StageType::Compute);
    if (index < 0)
    {
        return SLANG_FAIL;
    }
    const auto& kernelDesc = outputAndLayout.output.kernelDescs[index];
    SLANG_RETURN_ON_FAIL(outModule.load(kernelDesc.codeBegin));
    return SLANG_OK;
}

    /// Load and invoke a CUDA program (either compute or ray-tracing)
SlangResult _loadAndInvokeKernel(
    CUcontext cudaContext,
    ScopeCUDAStream& cudaStream,
    ScopeCUDAModule& cudaModule,
    const ShaderCompilerUtil::OutputAndLayout& outputAndLayout,
    const uint32_t dispatchSize[3],
    CUDAComputeUtil::Context& outContext)
{
    switch( outputAndLayout.output.desc.pipelineType )
    {
    case PipelineType::Compute:
        return _invokeComputeProgram(
            cudaContext, cudaStream, cudaModule, outputAndLayout, dispatchSize, outContext);

    case PipelineType::RayTracing:
#ifdef RENDER_TEST_OPTIX
        return _loadAndInvokeRayTracingProgram(
            cudaContext, cudaStream, outputAndLayout, dispatchSize, outContext);
#endif
        break;
    
    default: break;
    }

    return SLANG_FAIL;
}

    /// Execute a CUDA program (either compute or ray-tracing)
    ///
    /// This function handles loading code and argument data,
    /// invoking the kernel(s), and reading back results.
    ///
/* static */SlangResult CUDAComputeUtil::execute(const ShaderCompilerUtil::OutputAndLayout& outputAndLayout, const uint32_t dispatchSize[3], Context& outContext)
{
    ScopeCUDAContext cudaContext;
    SLANG_RETURN_ON_FAIL(cudaContext.init(0));

    // A default stream, will act as a global stream. Calling sync will globally sync
    ScopeCUDAStream cudaStream;
    //SLANG_CUDA_RETURN_ON_FAIL(cudaStream.init(cudaStreamNonBlocking));

    ScopeCUDAModule cudaModule;

    auto& bindSet = outContext.m_bindSet;
    auto& bindRoot = outContext.m_bindRoot;

    auto request = outputAndLayout.output.getRequestForReflection();
    auto reflection = (slang::ShaderReflection*) spGetReflection(request);

    // Load cuda module first so its symbols may be queried and filled into argument buffers.
    // TODO: refactor optix path to also front-load its context creation and module loading here.
    // For now just front-load compute kernels.
    if (outputAndLayout.output.desc.pipelineType == PipelineType::Compute)
    {
        SLANG_RETURN_ON_FAIL(_loadCUDAModule(outputAndLayout, cudaModule));
    }

    SLANG_RETURN_ON_FAIL(_setUpArguments(
        cudaContext, cudaStream, cudaModule, outputAndLayout, dispatchSize, outContext));

    SLANG_RETURN_ON_FAIL(_loadAndInvokeKernel(
        cudaContext, cudaStream, cudaModule, outputAndLayout, dispatchSize, outContext));

        // Finally we need to copy the data back
    SLANG_RETURN_ON_FAIL(_readBackOutputs(
        cudaContext, cudaStream, outputAndLayout, dispatchSize, outContext));

    // Release all othe CUDA resource/allocations
    bindSet.releaseValueTargets();
    outContext.releaseBindlessResources();

    return SLANG_OK;
}


void CUDAComputeUtil::Context::releaseBindlessResources()
{
    m_bindlessResources = decltype(m_bindlessResources)();
}

} // namespace renderer_test
