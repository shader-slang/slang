#pragma once

#include "compiler-core/slang-nvvm-ir-builder-api.h"

namespace Slang
{

// Producer-owned intrinsic semantics use the provider value-operation IDs for ordinary values.
// Rich operation families start after that fixed range and are resolved through their own typed
// provider interface.
typedef uint32_t NVVMIntrinsicSemantic;
static const NVVMIntrinsicSemantic kNVVMIntrinsicSemanticTextureSample =
    SLANG_NVVM_VALUE_OPERATION_COUNT;

} // namespace Slang
