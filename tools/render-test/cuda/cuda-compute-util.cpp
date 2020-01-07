
#include "cuda-compute-util.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-std-writers.h"
#include "../../source/core/slang-token-reader.h"

#include <cuda.h>

namespace renderer_test {
using namespace Slang;

/* static */SlangResult CUDAComputeUtil::execute()
{

    return SLANG_OK;
}

/* static */SlangResult _createDevice()
{
    cuInit(0);

    return SLANG_OK;
}

/* static */bool CUDAComputeUtil::canCreateDevice()
{
    return false;
}

} // renderer_test
