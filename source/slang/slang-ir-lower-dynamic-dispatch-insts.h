// slang-ir-lower-dynamic-dispatch-insts.h
#pragma once
#include "slang-ir.h"

namespace Slang
{
class Linkage;
class TargetProgram;

// Lower `UntaggedUnionType` types.
void lowerUntaggedUnionTypes(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

// Lower `SetTaggedUnion` and `CastInterfaceToTaggedUnionPtr` instructions
// May create new `Reinterpret` instructions.
//
bool lowerTaggedUnionTypes(IRModule* module, DiagnosticSink* sink);

// Lower `SetTagType` types
void lowerTagTypes(IRModule* module);

// Lower `GetTagOfElementInSet`, `GetTagForSuperSet`, `GetTagForSubSet` and `GetTagForMappedSet`
// instructions,
//
void lowerTagInsts(IRModule* module, DiagnosticSink* sink);

// Lower `GetTagFromSequentialID` and `GetSequentialIDFromTag` instructions
void lowerSequentialIDTagCasts(IRModule* module, Linkage* linkage, DiagnosticSink* sink);

// Lower `GetDispatcher` and `GetSpecializedDispatcher` instructions
bool lowerDispatchers(IRModule* module, DiagnosticSink* sink, bool reportDispatchLocations = false);

// Lower `ExtractExistentialValue`, `ExtractExistentialType`, `ExtractExistentialWitnessTable`,
// `InterfaceType`, `GetSequentialID`, `WitnessTableIDType` and `RTTIHandleType` instructions.
//
bool lowerExistentials(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

} // namespace Slang
