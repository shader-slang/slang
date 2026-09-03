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
static const NVVMIntrinsicSemantic kNVVMIntrinsicSemanticAtomicReduceAdd =
    kNVVMIntrinsicSemanticTextureSample + 1;
static const NVVMIntrinsicSemantic kNVVMIntrinsicSemanticAtomicReduceSubtract =
    kNVVMIntrinsicSemanticAtomicReduceAdd + 1;
static const NVVMIntrinsicSemantic kNVVMIntrinsicSemanticAtomicReduceMin =
    kNVVMIntrinsicSemanticAtomicReduceSubtract + 1;
static const NVVMIntrinsicSemantic kNVVMIntrinsicSemanticAtomicReduceMax =
    kNVVMIntrinsicSemanticAtomicReduceMin + 1;
static const NVVMIntrinsicSemantic kNVVMIntrinsicSemanticAtomicReduceBitAnd =
    kNVVMIntrinsicSemanticAtomicReduceMax + 1;
static const NVVMIntrinsicSemantic kNVVMIntrinsicSemanticAtomicReduceBitOr =
    kNVVMIntrinsicSemanticAtomicReduceBitAnd + 1;
static const NVVMIntrinsicSemantic kNVVMIntrinsicSemanticAtomicReduceBitXor =
    kNVVMIntrinsicSemanticAtomicReduceBitOr + 1;
static const NVVMIntrinsicSemantic kNVVMIntrinsicSemanticAtomicReduceIncrement =
    kNVVMIntrinsicSemanticAtomicReduceBitXor + 1;
static const NVVMIntrinsicSemantic kNVVMIntrinsicSemanticAtomicReduceDecrement =
    kNVVMIntrinsicSemanticAtomicReduceIncrement + 1;

} // namespace Slang
