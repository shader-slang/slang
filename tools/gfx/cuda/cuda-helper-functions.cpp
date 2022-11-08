// cuda-helper-functions.cpp
#include "cuda-helper-functions.h"

#include "cuda-device.h"

namespace gfx
{
#ifdef GFX_ENABLE_CUDA
using namespace Slang;

namespace cuda
{
SlangResult CUDAErrorInfo::handle() const
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

    getDebugCallback()->handleMessage(DebugMessageType::Error, DebugMessageSource::Driver,
        builder.getUnownedSlice().begin());

    // Slang::signalUnexpectedError(builder.getBuffer());
    return SLANG_FAIL;
}

SlangResult _handleCUDAError(CUresult cuResult, const char* file, int line)
{
    CUDAErrorInfo info(file, line);
    cuGetErrorString(cuResult, &info.m_errorString);
    cuGetErrorName(cuResult, &info.m_errorName);
    return info.handle();
}

SlangResult _handleCUDAError(cudaError_t error, const char* file, int line)
{
    return CUDAErrorInfo(file, line, cudaGetErrorName(error), cudaGetErrorString(error)).handle();
}

#    ifdef RENDER_TEST_OPTIX

static bool _isError(OptixResult result)
{
    return result != OPTIX_SUCCESS;
}

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

void _optixLogCallback(unsigned int level, const char* tag, const char* message, void* userData)
{
    fprintf(stderr, "optix: %s (%s)\n", message, tag);
}
#       endif
#    endif

AdapterLUID getAdapterLUID(int device)
{
    AdapterLUID luid = {};
#if SLANG_WIN32 || SLANG_WIN64
    // LUID reported by CUDA is undefined i not on windows platform.
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, device);
    SLANG_ASSERT(sizeof(AdapterLUID) >= sizeof(cudaDeviceProp::luid));
    memcpy(&luid, prop.luid, sizeof(cudaDeviceProp::luid));
#else
    SLANG_ASSERT(sizeof(AdapterLUID) >= sizeof(int));
    memcpy(&luid, &device, sizeof(int));
#endif
    return luid;
}

} // namespace cuda

Result SLANG_MCALL getCUDAAdapters(List<AdapterInfo>& outAdapters)
{
    int deviceCount;
    cudaGetDeviceCount(&deviceCount);
    for (int device = 0; device < deviceCount; device++)
    {
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, device);
        AdapterInfo info = {};
        memcpy(info.name, prop.name, Math::Min(strlen(prop.name), sizeof(AdapterInfo::name) - 1));
        info.luid = cuda::getAdapterLUID(device);
        outAdapters.add(info);
    }

    return SLANG_OK;
}

Result SLANG_MCALL createCUDADevice(const IDevice::Desc* desc, IDevice** outDevice)
{
    RefPtr<cuda::DeviceImpl> result = new cuda::DeviceImpl();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    returnComPtr(outDevice, result);
    return SLANG_OK;
}
#else

Result SLANG_MCALL getCUDAAdapters(List<AdapterInfo>& outAdapters)
{
    SLANG_UNUSED(outAdapters);
    return SLANG_FAIL;
}

Result SLANG_MCALL createCUDADevice(const IDevice::Desc* desc, IDevice** outDevice)
{
    SLANG_UNUSED(desc);
    *outDevice = nullptr;
    return SLANG_FAIL;
}
#endif

} // namespace gfx
