// Prelude for PyTorch cpp binding.

// clang-format off
#include <torch/extension.h>
// clang-format on

#include <ATen/cuda/CUDAContext.h>
#include <ATen/cuda/CUDAUtils.h>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef SLANG_LLVM
#include "slang-llvm.h"
#else // SLANG_LLVM
#if SLANG_GCC_FAMILY && __GNUC__ < 6
#include <cmath>
#define SLANG_PRELUDE_STD std::
#else
#include <math.h>
#define SLANG_PRELUDE_STD
#endif

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif // SLANG_LLVM

#include "../source/core/slang-string.h"

#if defined(_MSC_VER)
#define SLANG_PRELUDE_SHARED_LIB_EXPORT __declspec(dllexport)
#else
#define SLANG_PRELUDE_SHARED_LIB_EXPORT __attribute__((__visibility__("default")))
// #   define SLANG_PRELUDE_SHARED_LIB_EXPORT __attribute__ ((dllexport))
// __attribute__((__visibility__("default")))
#endif

#ifdef __cplusplus
#define SLANG_PRELUDE_EXTERN_C extern "C"
#define SLANG_PRELUDE_EXTERN_C_START \
    extern "C"                       \
    {
#define SLANG_PRELUDE_EXTERN_C_END }
#else
#define SLANG_PRELUDE_EXTERN_C
#define SLANG_PRELUDE_EXTERN_C_START
#define SLANG_PRELUDE_EXTERN_C_END
#endif

#define SLANG_PRELUDE_NAMESPACE

#ifndef SLANG_NO_THROW
#define SLANG_NO_THROW
#endif
#ifndef SLANG_STDCALL
#define SLANG_STDCALL
#endif
#ifndef SLANG_MCALL
#define SLANG_MCALL SLANG_STDCALL
#endif
#ifndef SLANG_FORCE_INLINE
#define SLANG_FORCE_INLINE inline
#endif
#include "slang-cpp-scalar-intrinsics.h"
#include "slang-cpp-types-core.h"


static const int kSlangTorchTensorMaxDim = 5;

// NOTE: If you change this struct's layout, also update the hard-coded size/alignment
// in _createTypeLayout() in slang-type-layout.cpp.
template<typename Offset>
struct TensorViewT
{
    using OffsetType = Offset;

    uint8_t* data;
    Offset strides[kSlangTorchTensorMaxDim];
    Offset sizes[kSlangTorchTensorMaxDim];
    uint32_t dimensionCount;
};

// Keep this type distinct so that default kernel signatures retain their C++ mangled names.
struct TensorView : TensorViewT<uint32_t>
{
};

template<typename Offset>
TensorViewT<Offset> make_tensor_view(
    torch::Tensor val,
    const char* name,
    torch::ScalarType targetScalarType,
    bool requireContiguous)
{
    // We're currently not trying to implicitly cast or transfer to device for two reasons:
    // 1. There appears to be a bug with .to() where successive calls after the first one fail.
    // 2. Silent casts like this can cause large memory allocations & unexpected overheads.
    //    It's better to be explicit.

    // Expect tensors to be on CUDA device
    if (!val.device().is_cuda())
        throw std::runtime_error(
            std::string(name).append(": tensor is not on CUDA device.").c_str());

    // Expect tensors to be the right type.
    if (val.dtype() != targetScalarType)
        throw std::runtime_error(
            std::string(name).append(": tensor is not of the expected type.").c_str());

    // Check that the tensor is contiguous
    if (requireContiguous && !val.is_contiguous())
        throw std::runtime_error(std::string(name).append(": tensor is not contiguous.").c_str());

    TensorViewT<Offset> res = {};
    res.dimensionCount = val.dim();
    res.data = nullptr;
    size_t elementSize = 4;

    switch (val.scalar_type())
    {
    case torch::kInt8:
    case torch::kUInt8:
        elementSize = 1;
        res.data = (uint8_t*)val.data_ptr<uint8_t>();
        break;
    case torch::kBFloat16:
        elementSize = 2;
        res.data = (uint8_t*)val.data_ptr<torch::BFloat16>();
        break;
    case torch::kFloat16:
        elementSize = 2;
        res.data = (uint8_t*)val.data_ptr<at::Half>();
        break;
    case torch::kInt16:
        elementSize = 2;
        res.data = (uint8_t*)val.data_ptr<int16_t>();
        break;
    case torch::kFloat32:
        elementSize = 4;
        res.data = (uint8_t*)val.data_ptr<float>();
        break;
    case torch::kInt32:
        elementSize = 4;
        res.data = (uint8_t*)val.data_ptr<int32_t>();
        break;
    case torch::kFloat64:
        elementSize = 8;
        res.data = (uint8_t*)val.data_ptr<double>();
        break;
    case torch::kInt64:
        elementSize = 8;
        res.data = (uint8_t*)val.data_ptr<int64_t>();
        break;
    case torch::kBool:
        elementSize = 1;
        res.data = (uint8_t*)val.data_ptr<bool>();
        break;
    }

    if (val.dim() > kSlangTorchTensorMaxDim)
        throw std::runtime_error(std::string(name)
                                     .append(": number of dimensions exceeds limit (")
                                     .append(std::to_string(kSlangTorchTensorMaxDim))
                                     .append(")")
                                     .c_str());

    // A tensor can have zero elements even if some dimensions are non-zero
    // (e.g. shape (10, 0)). Emptiness must be based on numel().
    bool isEmpty = (val.numel() == 0);
    const uint64_t maxOffset = (std::numeric_limits<Offset>::max)();
    uint64_t maxByteOffset = 0;
    for (int i = 0; i < val.dim(); ++i)
    {
        // Check sizes before narrowing and strides before multiplying so that conversion
        // cannot overflow the selected offset type.
        if (uint64_t(val.size(i)) > maxOffset)
            throw std::runtime_error(
                std::string(name).append(": tensor dimension size exceeds offset type limit."));
        if (uint64_t(val.stride(i)) > maxOffset / elementSize)
            throw std::runtime_error(
                std::string(name).append(": tensor byte stride exceeds offset type limit."));
        res.sizes[i] = val.size(i);
        res.strides[i] = uint64_t(val.stride(i)) * elementSize;
        if (!isEmpty && res.strides[i] == 0)
            throw std::runtime_error(
                std::string(name)
                    .append(": tensors with broadcasted dimensions are not supported (use "
                            "tensor.contiguous() to make tensor whole)")
                    .c_str());
        // Representable strides alone do not guarantee representable addresses. For example,
        // a contiguous float tensor can exceed 4 GiB even though its byte stride is only four.
        if (!isEmpty && res.sizes[i] > 1)
        {
            uint64_t lastIndex = res.sizes[i] - 1;
            if (uint64_t(res.strides[i]) > (maxOffset - maxByteOffset) / lastIndex)
                throw std::runtime_error(
                    std::string(name).append(": tensor byte offset exceeds offset type limit."));
            maxByteOffset += uint64_t(res.strides[i]) * lastIndex;
        }
    }

    if (!res.data && !isEmpty)
        throw std::runtime_error(std::string(name).append(": data pointer is invalid.").c_str());

    return res;
}

TensorView make_tensor_view(
    torch::Tensor val,
    const char* name,
    torch::ScalarType targetScalarType,
    bool requireContiguous)
{
    TensorView res = {};
    static_cast<TensorViewT<uint32_t>&>(res) =
        make_tensor_view<uint32_t>(val, name, targetScalarType, requireContiguous);
    return res;
}

#define SLANG_PRELUDE_EXPORT
