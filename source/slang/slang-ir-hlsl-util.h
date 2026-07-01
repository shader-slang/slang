// slang-ir-hlsl-util.h
#pragma once

#include "slang-ir.h"

namespace Slang
{

// Returns true for IR ops that materialize HLSL `Barrier` flag enum values.
bool isBarrierFlagGetterOp(IROp op);

// Returns the underlying integer flag value after removing implicit enum-to-integer casts.
IRInst* getBarrierFlagValueInst(IRInst* inst);

// Returns the mask of named HLSL barrier memory flags accepted as bitwise combinations.
uint32_t getKnownBarrierMemoryTypeFlags();

// Returns the mask of named HLSL barrier semantic flags accepted as bitwise combinations.
uint32_t getKnownBarrierSemanticFlags();

// Returns true for valid HLSL barrier memory flag values, including the special `ALL_MEMORY`
// spelling whose value is not decomposed into named bit flags.
bool isValidBarrierMemoryTypeFlags(uint32_t flagVal);

// Returns true for valid HLSL barrier semantic flag values, including the special `REORDER`
// spelling whose value is not decomposed into named bit flags.
bool isValidBarrierSemanticFlags(uint32_t flagVal);

// Returns true for implicit non-pointer casts that feed only HLSL barrier flag getter ops.
// Those casts are value conversions for named flag emission, not l-value pointer casts.
bool isBarrierFlagValueCast(IRInst* castInst, IRType* fromType, IRType* toType);

} // namespace Slang
