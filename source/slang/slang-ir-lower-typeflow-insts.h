// slang-ir-typeflow-specialize.h
#pragma once
#include "slang-ir.h"

namespace Slang
{

// Lower `TypeCollection` instructions
void lowerTypeCollections(IRModule* module, DiagnosticSink* sink);

// Lower `FuncCollection`, `GetTagForSuperCollection`, `GetTagForMappedCollection` and
// `GetTagForSpecializedCollection` instructions
//
void lowerTagInsts(IRModule* module, DiagnosticSink* sink);

// Lower `GetTagFromSequentialID` and `GetSequentialIDFromTag` instructions
void lowerSequentialIDTagCasts(IRModule* module, DiagnosticSink* sink);

// Lower `CollectionTagType` instructions
void lowerTagTypes(IRModule* module);

// Lower `CollectionTaggedUnion`and `CastInterfaceToTaggedUnionPtr` instructions
// May create new `Reinterpret` instructions.
//
bool lowerTaggedUnionTypes(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
