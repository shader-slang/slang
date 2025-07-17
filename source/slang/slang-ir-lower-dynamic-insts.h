// slang-ir-lower-dynamic-insts.h
#pragma once
#include "../core/slang-linked-list.h"
#include "../core/slang-smart-pointer.h"
#include "slang-ir.h"

namespace Slang
{

// Enumeration for different kinds of judgments about IR instructions
enum class PropagationJudgment
{
    Value,         // Regular value computation (not interface-related)
    ConcreteType,  // Concrete type reference
    ConcreteTable, // Concrete witness table reference
    ConcreteFunc,  // Concrete function reference
    SetOfTypes,    // Set of possible types
    SetOfTables,   // Set of possible witness tables
    SetOfFuncs,    // Set of possible functions
    UnknownSet,    // Unknown set of possible types/tables/funcs (e.g. from COM interface types)
    Existential    // Existential box with a set of possible witness tables
};

// Data structure to hold propagation information for an instruction
struct PropagationInfo : RefObject
{
    PropagationJudgment judgment;

    // For concrete references
    IRInst* concreteValue = nullptr;

    // For sets of types/tables/funcs and existential witness tables
    HashSet<IRInst*> possibleValues;

    // For SetOfFuncs
    IRFuncType* dynFuncType;

    PropagationInfo() = default;
    PropagationInfo(PropagationJudgment j)
        : judgment(j)
    {
    }

    static RefPtr<PropagationInfo> makeValue()
    {
        return new PropagationInfo(PropagationJudgment::Value);
    }
    static RefPtr<PropagationInfo> makeConcrete(PropagationJudgment j, IRInst* value);
    static RefPtr<PropagationInfo> makeSet(PropagationJudgment j, const HashSet<IRInst*>& values);
    static RefPtr<PropagationInfo> makeSetOfFuncs(
        const HashSet<IRInst*>& funcs,
        IRFuncType* dynFuncType);
    static RefPtr<PropagationInfo> makeExistential(const HashSet<IRInst*>& tables);
    static RefPtr<PropagationInfo> makeUnknown();
};

// Context for the abstract interpretation pass
struct DynamicInstLoweringContext
{
    IRModule* module;
    DiagnosticSink* sink;

    // Mapping from instruction to propagation information
    Dictionary<IRInst*, RefPtr<PropagationInfo>> propagationMap;

    // Unique ID assignment for functions and witness tables
    Dictionary<IRInst*, UInt> uniqueIds;
    UInt nextUniqueId = 1;

    // Mapping from lowered instruction to their any-value types
    Dictionary<IRInst*, IRAnyValueType*> loweredInstToAnyValueType;

    DynamicInstLoweringContext(IRModule* inModule, DiagnosticSink* inSink)
        : module(inModule), sink(inSink)
    {
    }

    // Phase 1: Information Propagation
    void performInformationPropagation();
    void processFunction(IRFunc* func);
    void propagateEdge(IREdge edge);
    void processInstForPropagation(IRInst* inst);

    // Helper to get propagation info, handling global insts specially
    RefPtr<PropagationInfo> tryGetInfo(IRInst* inst);

    // Control flow analysis helpers
    RefPtr<PropagationInfo> unionPropagationInfo(const List<RefPtr<PropagationInfo>>& infos);
    void initializeFirstBlockParameters(IRFunc* func);
    void insertReinterpretsForPhiParameters();

    // Analysis of specific instruction types
    RefPtr<PropagationInfo> analyzeCreateExistentialObject(IRCreateExistentialObject* inst);
    RefPtr<PropagationInfo> analyzeMakeExistential(IRMakeExistential* inst);
    RefPtr<PropagationInfo> analyzeLookupWitnessMethod(IRLookupWitnessMethod* inst);
    RefPtr<PropagationInfo> analyzeExtractExistentialWitnessTable(
        IRExtractExistentialWitnessTable* inst);
    RefPtr<PropagationInfo> analyzeExtractExistentialType(IRExtractExistentialType* inst);
    RefPtr<PropagationInfo> analyzeExtractExistentialValue(IRExtractExistentialValue* inst);
    RefPtr<PropagationInfo> analyzeCall(IRCall* inst);
    RefPtr<PropagationInfo> analyzeDefault(IRInst* inst);

    // Phase 2: Dynamic Instruction Lowering
    void performDynamicInstLowering();
    void lowerInst(IRInst* inst);
    void replaceType(IRInst* inst);

    // Lowering of specific instruction types
    void lowerLookupWitnessMethod(IRLookupWitnessMethod* inst);
    void lowerExtractExistentialWitnessTable(IRExtractExistentialWitnessTable* inst);
    void lowerExtractExistentialType(IRExtractExistentialType* inst);
    void lowerExtractExistentialValue(IRExtractExistentialValue* inst);
    void lowerCall(IRCall* inst);
    void lowerMakeExistential(IRMakeExistential* inst);
    void lowerCreateExistentialObject(IRCreateExistentialObject* inst);

    // Helper functions
    UInt getUniqueID(IRInst* funcOrTable);
    IRFunc* createKeyMappingFunc(
        IRInst* key,
        const HashSet<IRInst*>& inputTables,
        const HashSet<IRInst*>& outputTables);
    IRFunc* createDispatchFunc(const HashSet<IRInst*>& funcs, IRFuncType* expectedFuncType);
    IRAnyValueType* createAnyValueType(const HashSet<IRType*>& types);
    IRAnyValueType* createAnyValueTypeFromInsts(const HashSet<IRInst*>& typeInsts);
    SlangInt calculateAnyValueSize(const HashSet<IRType*>& types);
    bool needsReinterpret(RefPtr<PropagationInfo> sourceInfo, RefPtr<PropagationInfo> targetInfo);
    IRInst* maybeReinterpretArg(IRInst* arg, IRInst* param);

    // Utility functions
    bool isExistentialType(IRType* type);
    bool isInterfaceType(IRType* type);
    HashSet<IRInst*> collectExistentialTables(IRInterfaceType* interfaceType);

    // Main entry point
    void processModule();
};

// Main entry point for the pass
void lowerDynamicInsts(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
