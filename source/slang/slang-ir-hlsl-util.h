// slang-ir-hlsl-util.h
#pragma once

#include "slang-ir.h"

namespace Slang
{

// Returns true for the IR ops that convert work-graph Barrier enum values to HLSL flag bits.
bool isBarrierFlagGetterOp(IROp op);

// Strips l-value cast wrappers around a Barrier flag getter operand to recover the literal value.
IRInst* getBarrierFlagValueInst(IRInst* inst);

// Returns the bitmask of named BarrierMemoryTypeFlags values known to Slang and HLSL, excluding
// the aggregate AllMemory value. Validation and emission use this so values like
// `UavMemory | NodeInputMemory` share one mask.
uint32_t getKnownBarrierMemoryTypeFlags();

// Returns the bitmask of named BarrierSemanticFlags values known to Slang and HLSL, excluding the
// special Reorder value. Validation and emission use this so values like
// `GroupSync | DeviceScope` share one mask.
uint32_t getKnownBarrierSemanticFlags();

// Returns true when `flagVal` is AllMemory or a non-zero subset of known memory type bits.
bool isValidBarrierMemoryTypeFlags(uint32_t flagVal);

// Returns true when `flagVal` is Reorder or a subset of known semantic bits.
bool isValidBarrierSemanticFlags(uint32_t flagVal);

// Returns true if an l-value cast only wraps a workgraph barrier flag value. For example, HLSL
// barrier diagnostics may leave an enum-to-int cast around `getEnumBarrierSemanticFlags(...)`.
bool isBarrierFlagValueCast(IRInst* castInst, IRType* fromType, IRType* toType);

} // namespace Slang
