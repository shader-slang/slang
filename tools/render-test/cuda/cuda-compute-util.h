#ifndef CUDA_COMPUTE_UTIL_H
#define CUDA_COMPUTE_UTIL_H

#include "../slang-support.h"
#include "../options.h"

#include "../../source/core/slang-smart-pointer.h"

namespace renderer_test {


struct CUDAComputeUtil
{
    // Define here, so we don't need to include the cude header
    typedef size_t CUdeviceptr;

        /// NOTE! MUST match up to definitions in the CUDA prelude
    struct ByteAddressBuffer
    {
        CUdeviceptr data;
        size_t sizeInBytes;
    };
    struct StructuredBuffer
    {
        CUdeviceptr data;
        size_t count;
    };
    struct Array
    {
        CUdeviceptr data;
        size_t count;
    };

    struct Context
    {
            /// Holds the binding information
        BindSet m_bindSet;
        CPULikeBindRoot m_bindRoot;
            /// Buffers are held in same order as entries in layout (useful for dumping out bindings)
        List<BindSet::Value*> m_buffers;
    };

    class ResourceBase : public RefObject
    {
    public:
    };

    static SlangResult createTextureResource(const ShaderInputLayoutEntry& srcEntry, slang::TypeLayoutReflection* typeLayout, RefPtr<ResourceBase>& outResource);

    static SlangResult execute(const ShaderCompilerUtil::OutputAndLayout& outputAndLayout, const uint32_t dispatchSize[3], Context& outContext);

    static bool canCreateDevice();
};


} // renderer_test

#endif //CPU_MEMORY_BINDING_H
