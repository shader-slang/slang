// slang-ir-extract-value-from-type.h
#pragma once

#include "slang/ir/slang-ir.h"
#include "slang/layout-and-binding/slang-type-layout.h"

namespace Slang
{

// Emit code using builder that yields an `IRInst` representing a value of `size` bytes
// starting at `offset` in `src`. `src` must be a value of `struct`, array, vector or basic type.
// `size` can be 1, 2, 4 or 8. The result is an unsigned integer holding the bytes in its low bits:
// `uint` for a size up to 4 and `uint64_t` for 8. (A smaller value read out of an 8-byte leaf can
// also come back as `uint64_t`, so callers cast the result to the type they need.)
IRInst* extractValueAtOffset(
    IRBuilder& builder,
    TargetProgram* targetReq,
    IRInst* src,
    uint32_t offset,
    uint32_t size);

} // namespace Slang
