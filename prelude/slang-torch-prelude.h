// Prelude for PyTorch cpp binding.

#include <torch/extension.h>
#include <ATen/cuda/CUDAContext.h>
#include <ATen/cuda/CUDAUtils.h>
#include <vector>
#include <stdexcept>
#include <string>

#ifndef SLANG_NO_THROW
#   define SLANG_NO_THROW
#endif

#ifndef SLANG_STDCALL
#   define SLANG_STDCALL
#endif
#ifndef SLANG_MCALL
#   define SLANG_MCALL SLANG_STDCALL
#endif
#ifndef SLANG_FORCE_INLINE
#    define SLANG_FORCE_INLINE inline
#endif

#ifdef SLANG_LLVM
#include "slang-llvm.h"
#else // SLANG_LLVM
#   if SLANG_GCC_FAMILY && __GNUC__ < 6
#       include <cmath>
#       define SLANG_PRELUDE_STD std::
#   else
#       include <math.h>
#       define SLANG_PRELUDE_STD
#   endif

#   include <assert.h>
#   include <stdlib.h>
#   include <string.h>
#   include <stdint.h>
#endif // SLANG_LLVM

#include "slang-cpp-types-core.h"
#include "slang-cpp-scalar-intrinsics.h"

struct TensorView
{
    uint8_t* data;
    uint32_t* strides;
    uint32_t* sizes;
    uint32_t dimensionCount;
};

struct CudaTaskMemoryAllocator
{
    std::vector<void*> allocations;

    uint32_t* allocUIntArray(uint32_t size)
    {
        void* ptr = nullptr;
        cudaMallocManaged(&ptr, size * sizeof(uint32_t));
        AT_CUDA_CHECK(cudaGetLastError());
        return (uint32_t*)ptr;
    }

    ~CudaTaskMemoryAllocator()
    {
        for (auto ptr : allocations)
            cudaFree(ptr);
    }
};

TensorView make_tensor_view(CudaTaskMemoryAllocator* allocator, torch::Tensor val, const char* name)
{
    if (!val.device().is_cuda())
        throw std::runtime_error(std::string(name).append(" must be a CUDA tensor").c_str());
    if (!val.is_contiguous())
        throw std::runtime_error(std::string(name).append(" must be contiguous").c_str());
    printf("success 1\n");
    TensorView res = {};
    res.dimensionCount = val.dim();
    res.strides = allocator->allocUIntArray(val.dim());
    res.sizes = allocator->allocUIntArray(val.dim());
    res.data = nullptr;
    size_t elementSize = 4;
    printf("success 2\n");

    switch (val.scalar_type())
    {
    case torch::kInt8:
    case torch::kUInt8:
        elementSize = 1;
        res.data = (uint8_t*)val.data<uint8_t>();
        break;
    case torch::kBFloat16:
        elementSize = 2;
        res.data = (uint8_t*)val.data<torch::BFloat16>();
        break;
    case torch::kInt16:
        elementSize = 2;
        res.data = (uint8_t*)val.data<int16_t>();
        break;
    case torch::kFloat32:
        elementSize = 4;
        res.data = (uint8_t*)val.data<float>();
        break;
    case torch::kInt32:
        elementSize = 4;
        res.data = (uint8_t*)val.data<int32_t>();
        break;
    case torch::kFloat64:
        elementSize = 8;
        res.data = (uint8_t*)val.data<double>();
        break;
    case torch::kInt64:
        elementSize = 8;
        res.data = (uint8_t*)val.data<int64_t>();
        break;
    }
    for (int i = 0; i < val.dim(); ++i)
    {
        res.strides[i] = val.stride(i) * elementSize;
        res.sizes[i] = val.size(i);
    }
    if (!res.data)
        throw std::runtime_error(std::string(name).append(": data pointer is invalid.").c_str());
    printf("success 3, data %lld\n", res.data);
    return res;
}

size_t slangGetCudaKernelSharedMemSize(const void* func)
{
    cudaFuncAttributes attr = {};
    cudaFuncGetAttributes(&attr, func);
    AT_CUDA_CHECK(cudaGetLastError());
    return attr.sharedSizeBytes;
}

#define SLANG_PRELUDE_EXPORT
