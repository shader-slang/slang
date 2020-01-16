#ifndef CUDA_COMPUTE_UTIL_H
#define CUDA_COMPUTE_UTIL_H

#include "../slang-support.h"
#include "../options.h"

#include "../../source/core/slang-smart-pointer.h"

namespace renderer_test {

struct CUDAComputeUtil
{
    struct Context
    {
            /// Holds the binding information
        BindSet m_bindSet;
        CPULikeBindRoot m_bindRoot;
            /// Buffers are held in same order as entries in layout (useful for dumping out bindings)
        List<BindSet::Value*> m_buffers;
    };

    static SlangResult execute(const ShaderCompilerUtil::OutputAndLayout& outputAndLayout, Context& outContext);

    static bool canCreateDevice();
};


} // renderer_test

#endif //CPU_MEMORY_BINDING_H
