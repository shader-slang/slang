#pragma once

#include "core/slang-platform.h"
#include "core/slang-shared-library.h"

#include <stddef.h>
#include <stdint.h>
#include <type_traits>

namespace Slang
{
namespace TestCUDA
{

typedef int CudaResult;
typedef int CudaDevice;
typedef unsigned long long CudaDevicePtr;
typedef struct CudaContextImpl* CudaContext;
typedef struct CudaModuleImpl* CudaModule;
typedef struct CudaFunctionImpl* CudaFunction;

/// Loads the small CUDA Driver API surface shared by GPU runtime unit tests.
///
/// The declarations intentionally avoid CUDA toolkit headers and import libraries. Tests can
/// therefore build everywhere and classify a missing driver or symbol as an unavailable runtime
/// prerequisite.
struct CudaDriverApi
{
    CudaResult (*cuInit)(unsigned int flags) = nullptr;
    CudaResult (*cuDeviceGetCount)(int* count) = nullptr;
    CudaResult (*cuDeviceGet)(CudaDevice* device, int ordinal) = nullptr;
    CudaResult (*cuDeviceGetAttribute)(int* value, int attribute, CudaDevice device) = nullptr;
    CudaResult (*cuDevicePrimaryCtxRetain)(CudaContext* context, CudaDevice device) = nullptr;
    CudaResult (*cuDevicePrimaryCtxRelease)(CudaDevice device) = nullptr;
    CudaResult (*cuCtxSetCurrent)(CudaContext context) = nullptr;
    CudaResult (*cuCtxSynchronize)() = nullptr;
    CudaResult (*cuModuleLoadData)(CudaModule* module, const void* image) = nullptr;
    CudaResult (*cuModuleUnload)(CudaModule module) = nullptr;
    CudaResult (*cuModuleGetFunction)(CudaFunction* function, CudaModule module, const char* name) =
        nullptr;
    CudaResult (*cuModuleGetGlobal)(
        CudaDevicePtr* devicePtr,
        size_t* bytes,
        CudaModule module,
        const char* name) = nullptr;
    CudaResult (*cuMemAlloc)(CudaDevicePtr* devicePtr, size_t bytes) = nullptr;
    CudaResult (*cuMemFree)(CudaDevicePtr devicePtr) = nullptr;
    CudaResult (*cuMemcpyHtoD)(CudaDevicePtr dst, const void* src, size_t bytes) = nullptr;
    CudaResult (*cuMemcpyDtoH)(void* dst, CudaDevicePtr src, size_t bytes) = nullptr;
    CudaResult (*cuMemsetD8)(CudaDevicePtr dst, unsigned char value, size_t bytes) = nullptr;
    CudaResult (*cuLaunchKernel)(
        CudaFunction function,
        unsigned int gridDimX,
        unsigned int gridDimY,
        unsigned int gridDimZ,
        unsigned int blockDimX,
        unsigned int blockDimY,
        unsigned int blockDimZ,
        unsigned int sharedMemBytes,
        void* stream,
        void** kernelParams,
        void** extra) = nullptr;

    SharedLibrary::Handle library = nullptr;

    bool load()
    {
#if SLANG_WINDOWS_FAMILY
        const char* const libraryNames[] = {"nvcuda.dll"};
#elif SLANG_LINUX_FAMILY
        const char* const libraryNames[] = {"libcuda.so.1", "libcuda.so"};
#else
        return false;
#endif
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
        for (const char* name : libraryNames)
        {
            if (SLANG_SUCCEEDED(SharedLibrary::loadWithPlatformPath(name, library)))
                break;
        }
        if (!library)
            return false;

        bool allFound = true;
        auto resolve = [&](auto& function, const char* symbolName)
        {
            function = reinterpret_cast<std::remove_reference_t<decltype(function)>>(
                SharedLibrary::findSymbolAddressByName(library, symbolName));
            if (!function)
                allFound = false;
        };
        resolve(cuInit, "cuInit");
        resolve(cuDeviceGetCount, "cuDeviceGetCount");
        resolve(cuDeviceGet, "cuDeviceGet");
        resolve(cuDeviceGetAttribute, "cuDeviceGetAttribute");
        resolve(cuDevicePrimaryCtxRetain, "cuDevicePrimaryCtxRetain");
        resolve(cuDevicePrimaryCtxRelease, "cuDevicePrimaryCtxRelease_v2");
        resolve(cuCtxSetCurrent, "cuCtxSetCurrent");
        resolve(cuCtxSynchronize, "cuCtxSynchronize");
        resolve(cuModuleLoadData, "cuModuleLoadData");
        resolve(cuModuleUnload, "cuModuleUnload");
        resolve(cuModuleGetFunction, "cuModuleGetFunction");
        resolve(cuModuleGetGlobal, "cuModuleGetGlobal_v2");
        resolve(cuMemAlloc, "cuMemAlloc_v2");
        resolve(cuMemFree, "cuMemFree_v2");
        resolve(cuMemcpyHtoD, "cuMemcpyHtoD_v2");
        resolve(cuMemcpyDtoH, "cuMemcpyDtoH_v2");
        resolve(cuMemsetD8, "cuMemsetD8_v2");
        resolve(cuLaunchKernel, "cuLaunchKernel");
        return allFound;
#endif
    }

    ~CudaDriverApi()
    {
        if (library)
            SharedLibrary::unload(library);
    }
};

struct CudaBufferGuard
{
    const CudaDriverApi& api;
    CudaDevicePtr ptr;
    ~CudaBufferGuard()
    {
        if (ptr)
            api.cuMemFree(ptr);
    }
};

struct CudaModuleGuard
{
    const CudaDriverApi& api;
    CudaModule module;
    ~CudaModuleGuard()
    {
        if (module)
            api.cuModuleUnload(module);
    }
};

struct CudaPrimaryContextGuard
{
    const CudaDriverApi& api;
    CudaDevice device;
    ~CudaPrimaryContextGuard() { api.cuDevicePrimaryCtxRelease(device); }
};

static const int kCudaDeviceAttributeComputeCapabilityMajor = 75;
static const int kCudaDeviceAttributeComputeCapabilityMinor = 76;

} // namespace TestCUDA
} // namespace Slang
