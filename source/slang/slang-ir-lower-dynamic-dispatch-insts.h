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

// Lower `ExtractExistentialValue`, `ExtractExistentialType`, `ExtractExistentialWitnessTable`,
// `InterfaceType`, `GetSequentialID`, `WitnessTableIDType` and `RTTIHandleType` instructions.
//
bool lowerExistentials(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

void lowerGetSpecializedDispatcher(
    IRModule* module,
    DiagnosticSink* sink,
    IRGetSpecializedDispatcher* dispatcher,
    bool shouldDiagnose,
    List<IRInst*>& newCallees);

void lowerGetDispatcher(
    IRModule* module,
    DiagnosticSink* sink,
    IRGetDispatcher* dispatcher,
    bool shouldDiagnose,
    List<IRInst*>& newCallees);

// Create a dispatch function that switches on a tag (first parameter) to call
// one of the functions in the mapping.
IRFunc* createDispatchFunc(
    IRFuncType* dispatchFuncType,
    Dictionary<IRInst*, std::pair<IRInst*, IRFuncType*>>& mapping);

// Report diagnostic information about a dynamic dispatch site.
void reportDispatchLocation(
    IRModule* module,
    DiagnosticSink* sink,
    IRUse* use,
    IRWitnessTableSet* witnessTableSet);

void reportSpecializedDispatchLocation(
    IRModule* module,
    DiagnosticSink* sink,
    IRUse* use,
    IRWitnessTableSet* witnessTableSet,
    List<IRInst*>& specArgs);

} // namespace Slang
