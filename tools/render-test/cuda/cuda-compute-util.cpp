
#include "cuda-compute-util.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-std-writers.h"
#include "../../source/core/slang-token-reader.h"

#include <cuda.h>
#include <cuda_runtime_api.h>

namespace renderer_test {
using namespace Slang;

#define SLANG_CUDA_RETURN_ON_FAIL(x) { int _res = (int)(x); if (_res != 0) return SLANG_FAIL; }

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
    SLANG_ASSERT(sm > last.coreCount );

    // Default to the last entry
    return last.coreCount;
}

static SlangResult _findMaxFlopsDeviceId(int* outDevice)
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

    *outDevice = maxPerfDevice;
    return SLANG_OK;
}

static SlangResult _initCuda()
{
    static CUresult res = cuInit(0);
    SLANG_CUDA_RETURN_ON_FAIL(res);

    return SLANG_OK;
}



/* static */SlangResult _createDevice(CUcontext* outContext)
{
    SLANG_RETURN_ON_FAIL(_initCuda());

    int deviceId;
    SLANG_RETURN_ON_FAIL(_findMaxFlopsDeviceId(&deviceId));
    SLANG_CUDA_RETURN_ON_FAIL(cudaSetDevice(deviceId));

    CUcontext context;

    // Create context
    SLANG_CUDA_RETURN_ON_FAIL(cuCtxCreate(&context, 0, deviceId));

    *outContext = context;
    return SLANG_OK;
}

/* static */bool CUDAComputeUtil::canCreateDevice()
{
    CUcontext context;
    if (SLANG_SUCCEEDED(_createDevice(&context)))
    {
        cuCtxDestroy(context);
        return true;
    }

    return false;
}

static SlangResult _compute(CUcontext context, CUmodule module, const ShaderCompilerUtil::OutputAndLayout& outputAndLayout)
{
    auto request = outputAndLayout.output.request;
    auto reflection = (slang::ShaderReflection*) spGetReflection(request);

    slang::EntryPointReflection* entryPoint = nullptr;
    auto entryPointCount = reflection->getEntryPointCount();
    SLANG_ASSERT(entryPointCount == 1);

    entryPoint = reflection->getEntryPointByIndex(0);

    const char* entryPointName = entryPoint->getName();

    // Get the entry point
    CUfunction kernel;

    SLANG_CUDA_RETURN_ON_FAIL(cuModuleGetFunction(&kernel, module, entryPointName));


    return SLANG_OK;
}

/* static */SlangResult CUDAComputeUtil::execute(const ShaderCompilerUtil::OutputAndLayout& outputAndLayout)
{
    CUcontext context;
    SLANG_RETURN_ON_FAIL(_createDevice(&context));

    const Index index = outputAndLayout.output.findKernelDescIndex(StageType::Compute);
    if (index < 0)
    {
        return SLANG_FAIL;
    }

    const auto& kernel = outputAndLayout.output.kernelDescs[index];

    CUmodule module = 0;
    SLANG_CUDA_RETURN_ON_FAIL(cuModuleLoadData(&module, kernel.codeBegin));

    SLANG_RETURN_ON_FAIL(_compute(context, module, outputAndLayout));

    SLANG_CUDA_RETURN_ON_FAIL(cuModuleUnload(module));

    cuCtxDestroy(context);

    return SLANG_OK;
}


} // renderer_test
