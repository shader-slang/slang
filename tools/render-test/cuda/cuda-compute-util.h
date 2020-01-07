#ifndef CUDA_COMPUTE_UTIL_H
#define CUDA_COMPUTE_UTIL_H

#include "../slang-support.h"
#include "../options.h"

#include "../../source/core/slang-smart-pointer.h"

namespace renderer_test {

struct CUDAComputeUtil
{
    static SlangResult execute();

    static bool canCreateDevice();
};


} // renderer_test

#endif //CPU_MEMORY_BINDING_H
