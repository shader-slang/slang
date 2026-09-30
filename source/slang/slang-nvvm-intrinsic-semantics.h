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
// TextureSample + 1 through + 9 are reserved for retired atomic reduction semantics.
static const NVVMIntrinsicSemantic kNVVMIntrinsicSemanticSurfaceLoad =
    kNVVMIntrinsicSemanticTextureSample + 10;
static const NVVMIntrinsicSemantic kNVVMIntrinsicSemanticSurfaceStore =
    kNVVMIntrinsicSemanticSurfaceLoad + 1;

} // namespace Slang
