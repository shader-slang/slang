// slang-ir-typeflow-specialize.h
#pragma once
#include "slang-ir.h"

namespace Slang
{

// Lower `UntaggedUnionType` types.
void lowerUntaggedUnionTypes(IRModule* module, DiagnosticSink* sink);

// Lower `SetTaggedUnion` and `CastInterfaceToTaggedUnionPtr` instructions
// May create new `Reinterpret` instructions.
//
bool lowerTaggedUnionTypes(IRModule* module, DiagnosticSink* sink);

// Lower `SetTagType` types
void lowerTagTypes(IRModule* module);

// Lower `GetTagOfElementInSet`,
// `GetTagForSuperSet`, and `GetTagForMappedSet` instructions,
//
void lowerTagInsts(IRModule* module, DiagnosticSink* sink);

// Lower `GetTagFromSequentialID` and `GetSequentialIDFromTag` instructions
void lowerSequentialIDTagCasts(IRModule* module, DiagnosticSink* sink);

// Lower `GetDispatcher` and `GetSpecializedDispatcher` instructions
bool lowerDispatchers(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
