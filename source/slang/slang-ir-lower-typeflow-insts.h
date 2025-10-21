// slang-ir-typeflow-specialize.h
#pragma once
#include "slang-ir.h"

namespace Slang
{

// Lower `ValueOfCollectionType` types.
void lowerTypeCollections(IRModule* module, DiagnosticSink* sink);

// Lower `CollectionTaggedUnion` and `CastInterfaceToTaggedUnionPtr` instructions
// May create new `Reinterpret` instructions.
//
bool lowerTaggedUnionTypes(IRModule* module, DiagnosticSink* sink);

// Lower `CollectionTagType` types
void lowerTagTypes(IRModule* module);

// Lower `GetTagOfElementInCollection`,
// `GetTagForSuperCollection`, and `GetTagForMappedCollection` instructions,
//
void lowerTagInsts(IRModule* module, DiagnosticSink* sink);

// Lower `GetTagFromSequentialID` and `GetSequentialIDFromTag` instructions
void lowerSequentialIDTagCasts(IRModule* module, DiagnosticSink* sink);

// Lower `GetDispatcher` and `GetSpecializedDispatcher` instructions
bool lowerDispatchers(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
