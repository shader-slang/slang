#include "slang-ir-lower-dynamic-insts.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{


// Enumeration for different kinds of judgments about IR instructions.
//
// This forms a lattice with
//
// None < Value
// None < ConcreteX < SetOfX < Unbounded
// None < Existential < Unbounded
//
enum class PropagationJudgment
{
    None,          // No judgment (initial value)
    Value,         // Regular value computation (unrelated to dynamic dispatch)
    ConcreteType,  // Concrete type reference
    ConcreteTable, // Concrete witness table reference
    ConcreteFunc,  // Concrete function reference
    SetOfTypes,    // Set of possible types
    SetOfTables,   // Set of possible witness tables
    SetOfFuncs,    // Set of possible functions
    Existential,   // Existential box with a set of possible witness tables
    Unbounded,     // Unknown set of possible types/tables/funcs (e.g. COM interface types)
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

    PropagationInfo()
        : judgment(PropagationJudgment::None), concreteValue(nullptr), dynFuncType(nullptr)
    {
    }

    PropagationInfo(PropagationJudgment j)
        : judgment(j)
    {
    }

    static PropagationInfo makeValue() { return PropagationInfo(PropagationJudgment::Value); }
    static PropagationInfo makeConcrete(PropagationJudgment j, IRInst* value);
    static PropagationInfo makeSet(PropagationJudgment j, const HashSet<IRInst*>& values);
    static PropagationInfo makeSetOfFuncs(const HashSet<IRInst*>& funcs, IRFuncType* dynFuncType);
    static PropagationInfo makeExistential(const HashSet<IRInst*>& tables);
    static PropagationInfo makeUnbounded();
    static PropagationInfo none();

    bool isNone() const { return judgment == PropagationJudgment::None; }

    operator bool() const { return judgment != PropagationJudgment::None; }
};

// Data structures for interprocedural data-flow analysis

// Represents an interprocedural edge between call sites and functions
struct InterproceduralEdge
{
    enum class Direction
    {
        CallToFunc, // From call site to function entry (propagating arguments)
        FuncToCall  // From function return to call site (propagating return value)
    };

    Direction direction;
    IRCall* callInst;   // The call instruction
    IRFunc* targetFunc; // The function being called/returned from

    InterproceduralEdge() = default;
    InterproceduralEdge(Direction dir, IRCall* call, IRFunc* func)
        : direction(dir), callInst(call), targetFunc(func)
    {
    }
};

// Union type representing either an intra-procedural or interprocedural edge
struct WorkItem
{
    enum class Type
    {
        None,      // Invalid
        Block,     // Propagate information within a block
        IntraProc, // Propagate through within-function edge (IREdge)
        InterProc  // Propagate across function call/return (InterproceduralEdge)
    };

    Type type;
    union
    {
        IRBlock* block;
        IREdge intraProcEdge;
        InterproceduralEdge interProcEdge;
    };

    WorkItem()
        : type(Type::None)
    {
    }

    WorkItem(IRBlock* block)
        : type(Type::Block), block(block)
    {
    }

    WorkItem(IREdge edge)
        : type(Type::IntraProc), intraProcEdge(edge)
    {
    }

    WorkItem(InterproceduralEdge edge)
        : type(Type::InterProc), interProcEdge(edge)
    {
    }

    WorkItem(InterproceduralEdge::Direction dir, IRCall* call, IRFunc* func)
        : type(Type::InterProc), interProcEdge(dir, call, func)
    {
    }

    // Copy constructor and assignment needed for union with non-trivial types
    WorkItem(const WorkItem& other)
        : type(other.type)
    {
        if (type == Type::IntraProc)
            intraProcEdge = other.intraProcEdge;
        else if (type == Type::InterProc)
            interProcEdge = other.interProcEdge;
        else
            block = other.block;
    }

    WorkItem& operator=(const WorkItem& other)
    {
        type = other.type;
        if (type == Type::IntraProc)
            intraProcEdge = other.intraProcEdge;
        else if (type == Type::InterProc)
            interProcEdge = other.interProcEdge;
        else
            block = other.block;
        return *this;
    }
};

// PropagationInfo implementation
PropagationInfo PropagationInfo::makeConcrete(PropagationJudgment j, IRInst* value)
{
    auto info = PropagationInfo(j);
    info.concreteValue = value;
    return info;
}

PropagationInfo PropagationInfo::makeSet(PropagationJudgment j, const HashSet<IRInst*>& values)
{
    auto info = PropagationInfo(j);
    SLANG_ASSERT(j != PropagationJudgment::SetOfFuncs);
    info.possibleValues = values;
    return info;
}


PropagationInfo PropagationInfo::makeSetOfFuncs(
    const HashSet<IRInst*>& values,
    IRFuncType* dynFuncType)
{
    auto info = PropagationInfo(PropagationJudgment::SetOfFuncs);
    info.possibleValues = values;
    info.dynFuncType = dynFuncType;
    return info;
}

PropagationInfo PropagationInfo::makeExistential(const HashSet<IRInst*>& tables)
{
    auto info = PropagationInfo(PropagationJudgment::Existential);
    info.possibleValues = tables;
    return info;
}

PropagationInfo PropagationInfo::makeUnbounded()
{
    return PropagationInfo(PropagationJudgment::Unbounded);
}

PropagationInfo PropagationInfo::none()
{
    return PropagationInfo(PropagationJudgment::None);
}

bool areInfosEqual(const PropagationInfo& a, const PropagationInfo& b)
{
    if (a.judgment != b.judgment)
        return false;

    switch (a.judgment)
    {
    case PropagationJudgment::Value:
        return true; // All value judgments are equal

    case PropagationJudgment::ConcreteType:
    case PropagationJudgment::ConcreteTable:
    case PropagationJudgment::ConcreteFunc:
        return a.concreteValue == b.concreteValue;

    case PropagationJudgment::SetOfTypes:
    case PropagationJudgment::SetOfTables:
    case PropagationJudgment::SetOfFuncs:
        if (a.possibleValues.getCount() != b.possibleValues.getCount())
            return false;
        for (auto value : a.possibleValues)
        {
            if (!b.possibleValues.contains(value))
                return false;
        }
        return true;

    case PropagationJudgment::Existential:
        if (a.possibleValues.getCount() != b.possibleValues.getCount())
            return false;
        for (auto table : a.possibleValues)
        {
            if (!b.possibleValues.contains(table))
                return false;
        }
        return true;

    case PropagationJudgment::Unbounded:
        return true; // All unknown sets are considered equal

    default:
        return false;
    }
}

struct DynamicInstLoweringContext
{
    // DynamicInstLoweringContext implementation
    PropagationInfo tryGetInfo(IRInst* inst)
    {
        // If this is a global instruction (parent is module), return concrete info
        if (as<IRModuleInst>(inst->getParent()))
        {
            if (as<IRType>(inst))
            {
                PropagationInfo typeInfo =
                    PropagationInfo::makeConcrete(PropagationJudgment::ConcreteType, nullptr);
                typeInfo.concreteValue = inst;
                return typeInfo;
            }
            else if (as<IRWitnessTable>(inst))
            {
                PropagationInfo tableInfo =
                    PropagationInfo::makeConcrete(PropagationJudgment::ConcreteTable, nullptr);
                tableInfo.concreteValue = inst;
                return tableInfo;
            }
            else if (as<IRFunc>(inst))
            {
                PropagationInfo funcInfo =
                    PropagationInfo::makeConcrete(PropagationJudgment::ConcreteFunc, nullptr);
                funcInfo.concreteValue = inst;
                return funcInfo;
            }
            else
            {
                return PropagationInfo::makeValue();
            }
        }

        // For non-global instructions, look up in the map
        auto found = propagationMap.tryGetValue(inst);
        if (found)
            return *found;
        return PropagationInfo::none();
    }

    PropagationInfo tryGetFuncReturnInfo(IRFunc* func)
    {
        auto found = funcReturnInfo.tryGetValue(func);
        if (found)
            return *found;
        return PropagationInfo::none();
    }

    void processBlock(IRBlock* block, LinkedList<WorkItem>& workQueue)
    {
        bool anyInfoChanged = false;

        HashSet<IRBlock*> affectedBlocks;
        HashSet<IRTerminatorInst*> affectedTerminators;
        for (auto inst : block->getChildren())
        {
            // Skip parameters & terminator
            if (as<IRParam>(inst) || as<IRTerminatorInst>(inst))
                continue;

            auto oldInfo = tryGetInfo(inst);
            processInstForPropagation(inst, workQueue);
            auto newInfo = tryGetInfo(inst);

            // If information has changed, propagate to appropriate blocks/edges
            if (!areInfosEqual(oldInfo, newInfo))
            {
                for (auto use = inst->firstUse; use; use = use->nextUse)
                {
                    auto userBlock = as<IRBlock>(use->getUser());
                    if (userBlock && userBlock != block)
                        affectedBlocks.add(userBlock);

                    if (auto terminator = as<IRTerminatorInst>(use->getUser()))
                        affectedTerminators.add(terminator);
                }
            }
        }

        for (auto block : affectedBlocks)
        {
            workQueue.addLast(WorkItem(block));
        }

        for (auto terminator : affectedTerminators)
        {
            auto successors = as<IRBlock>(terminator->getParent())->getSuccessors();
            for (auto succIter = successors.begin(), succEnd = successors.end();
                 succIter != succEnd;
                 ++succIter)
            {
                workQueue.addLast(WorkItem(succIter.getEdge()));
            }
        }

        if (as<IRReturn>(block->getTerminator()))
        {
            // If the block has a return inst, we need to propagate return values
            propagateReturnValues(block, workQueue);
        }
    };

    void performInformationPropagation()
    {
        // Global worklist for interprocedural analysis
        LinkedList<WorkItem> workQueue;

        // Add all global function entry blocks to worklist.
        for (auto inst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
            {
                initializeFirstBlockParameters(func);

                // Add all blocks to start with. Once the initial
                // sweep is done, propagation will proceed on an on-demand basis
                // depending on affected blocks & edges
                //
                for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
                {
                    workQueue.addLast(WorkItem(block));
                }
            }
        }

        // Process until fixed point
        while (workQueue.getCount() > 0)
        {
            // Pop work item from front
            auto item = workQueue.getFirst();
            workQueue.getFirstNode()->removeAndDelete();

            switch (item.type)
            {
            case WorkItem::Type::Block:
                processBlock(item.block, workQueue);
                break;
            case WorkItem::Type::IntraProc:
                propagateWithinFuncEdge(item.intraProcEdge, workQueue);
                break;
            case WorkItem::Type::InterProc:
                propagateInterproceduralEdge(item.interProcEdge, workQueue);
                break;
            default:
                SLANG_UNEXPECTED("Unhandled work item type");
                return;
            }
        }
    }

    IRInst* maybeReinterpret(IRInst* arg, PropagationInfo destInfo)
    {
        auto argInfo = tryGetInfo(arg);

        if (!argInfo || !destInfo)
            return arg;

        if (argInfo.judgment == PropagationJudgment::Existential &&
            destInfo.judgment == PropagationJudgment::Existential)
        {
            if (argInfo.possibleValues.getCount() != destInfo.possibleValues.getCount())
            {
                // If the sets of witness tables are not equal, reinterpret to the parameter type
                IRBuilder builder(module);
                builder.setInsertAfter(arg);

                // We'll use nulltype for the reinterpret since the type is going to be re-written
                // and if it doesn't, this will help catch it before code-gen.
                //
                auto reinterpret = builder.emitReinterpret(nullptr, arg);
                propagationMap[reinterpret] = destInfo;
                return reinterpret; // Return the reinterpret instruction
            }
        }

        return arg; // Can use as-is.
    }

    void insertReinterprets()
    {
        // Process each function in the module
        for (auto inst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
            {
                // Skip the first block as it contains function parameters, not phi parameters
                for (auto block = func->getFirstBlock()->getNextBlock(); block;
                     block = block->getNextBlock())
                {
                    // Process each parameter in this block (these are phi parameters)
                    for (auto param : block->getParams())
                    {
                        auto paramInfo = tryGetInfo(param);
                        if (!paramInfo)
                            continue;

                        // Check all predecessors and their corresponding arguments
                        Index paramIndex = 0;
                        for (auto p : block->getParams())
                        {
                            if (p == param)
                                break;
                            paramIndex++;
                        }

                        // Find all predecessors of this block
                        for (auto pred : block->getPredecessors())
                        {
                            auto terminator = pred->getTerminator();
                            if (!terminator)
                                continue;

                            if (auto unconditionalBranch = as<IRUnconditionalBranch>(terminator))
                            {
                                // Get the argument at the same index as this parameter
                                if (paramIndex < unconditionalBranch->getArgCount())
                                {
                                    auto arg = unconditionalBranch->getArg(paramIndex);
                                    auto newArg = maybeReinterpret(arg, tryGetInfo(param));

                                    if (newArg != arg)
                                    {
                                        // Replace the argument in the branch instruction
                                        SLANG_ASSERT(!as<IRLoop>(unconditionalBranch));
                                        unconditionalBranch->setOperand(1 + paramIndex, newArg);
                                    }
                                }
                            }
                        }
                    }

                    // Is the terminator a return instruction?
                    if (auto returnInst = as<IRReturn>(block->getTerminator()))
                    {
                        if (!as<IRVoidType>(returnInst->getVal()->getDataType()))
                        {
                            auto funcReturnInfo = tryGetFuncReturnInfo(func);
                            auto newReturnVal =
                                maybeReinterpret(returnInst->getVal(), funcReturnInfo);
                            if (newReturnVal != returnInst->getVal())
                            {
                                // Replace the return value with the reinterpreted value
                                returnInst->setOperand(0, newReturnVal);
                            }
                        }
                    }

                    List<IRCall*> callInsts;
                    // Collect all call instructions in this block
                    for (auto inst : block->getChildren())
                    {
                        if (auto callInst = as<IRCall>(inst))
                            callInsts.add(callInst);
                    }

                    // Look at all the args and reinterpret them if necessary
                    for (auto callInst : callInsts)
                    {
                        if (auto irFunc = as<IRFunc>(callInst->getCallee()))
                        {
                            List<IRInst*> params;
                            List<IRInst*> args;
                            Index i = 0;
                            for (auto param : irFunc->getParams())
                            {
                                auto newArg =
                                    maybeReinterpret(callInst->getArg(i), tryGetInfo(param));
                                if (newArg != callInst->getArg(i))
                                {
                                    // Replace the argument in the call instruction
                                    callInst->setArg(i, newArg);
                                }
                                i++;
                            }
                        }
                    }
                }
            }
        }
    }

    void processInstForPropagation(IRInst* inst, LinkedList<WorkItem>& workQueue)
    {
        PropagationInfo info;

        switch (inst->getOp())
        {
        case kIROp_CreateExistentialObject:
            info = analyzeCreateExistentialObject(as<IRCreateExistentialObject>(inst));
            break;
        case kIROp_MakeExistential:
            info = analyzeMakeExistential(as<IRMakeExistential>(inst));
            break;
        case kIROp_LookupWitnessMethod:
            info = analyzeLookupWitnessMethod(as<IRLookupWitnessMethod>(inst));
            break;
        case kIROp_ExtractExistentialWitnessTable:
            info =
                analyzeExtractExistentialWitnessTable(as<IRExtractExistentialWitnessTable>(inst));
            break;
        case kIROp_ExtractExistentialType:
            info = analyzeExtractExistentialType(as<IRExtractExistentialType>(inst));
            break;
        case kIROp_ExtractExistentialValue:
            info = analyzeExtractExistentialValue(as<IRExtractExistentialValue>(inst));
            break;
        case kIROp_Call:
            info = analyzeCall(as<IRCall>(inst), workQueue);
            break;
        default:
            info = analyzeDefault(inst);
            break;
        }

        propagationMap[inst] = info;
    }

    PropagationInfo analyzeCreateExistentialObject(IRCreateExistentialObject* inst)
    {
        // For now, error out as specified
        SLANG_UNIMPLEMENTED_X("IRCreateExistentialObject lowering not yet implemented");
        return PropagationInfo::makeValue();
    }

    PropagationInfo analyzeMakeExistential(IRMakeExistential* inst)
    {
        auto witnessTable = inst->getWitnessTable();
        auto value = inst->getWrappedValue();
        auto valueType = value->getDataType();

        // Get the witness table info
        auto witnessTableInfo = tryGetInfo(witnessTable);

        if (!witnessTableInfo)
            return PropagationInfo::none();

        if (witnessTableInfo.judgment == PropagationJudgment::Unbounded)
            return PropagationInfo::makeUnbounded();

        HashSet<IRInst*> tables;

        if (witnessTableInfo.judgment == PropagationJudgment::ConcreteTable)
        {
            tables.add(witnessTableInfo.concreteValue);
        }
        else if (witnessTableInfo.judgment == PropagationJudgment::SetOfTables)
        {
            for (auto table : witnessTableInfo.possibleValues)
            {
                tables.add(table);
            }
        }

        return PropagationInfo::makeExistential(tables);
    }

    static IRInst* lookupEntry(IRInst* witnessTable, IRInst* key)
    {
        if (auto concreteTable = as<IRWitnessTable>(witnessTable))
        {
            for (auto entry : concreteTable->getEntries())
            {
                if (entry->getRequirementKey() == key)
                {
                    return entry->getSatisfyingVal();
                }
            }
        }
        return nullptr; // Not found
    }

    PropagationInfo analyzeLookupWitnessMethod(IRLookupWitnessMethod* inst)
    {
        auto witnessTable = inst->getWitnessTable();
        auto key = inst->getRequirementKey();
        auto witnessTableInfo = tryGetInfo(witnessTable);

        if (!witnessTableInfo)
            return PropagationInfo::none();

        if (witnessTableInfo.judgment == PropagationJudgment::Unbounded)
            return PropagationInfo::makeUnbounded();

        HashSet<IRInst*> results;

        if (witnessTableInfo.judgment == PropagationJudgment::ConcreteTable)
        {
            results.add(lookupEntry(witnessTableInfo.concreteValue, key));
        }
        else if (witnessTableInfo.judgment == PropagationJudgment::SetOfTables)
        {
            for (auto table : witnessTableInfo.possibleValues)
            {
                results.add(lookupEntry(table, key));
            }
        }

        if (witnessTableInfo.judgment == PropagationJudgment::ConcreteTable)
        {
            if (as<IRFuncType>(inst->getDataType()))
            {
                return PropagationInfo::makeConcrete(
                    PropagationJudgment::ConcreteFunc,
                    *results.begin());
            }
            else if (as<IRTypeKind>(inst->getDataType()))
            {
                return PropagationInfo::makeConcrete(
                    PropagationJudgment::ConcreteType,
                    *results.begin());
            }
            else if (as<IRWitnessTableType>(inst->getDataType()))
            {
                return PropagationInfo::makeConcrete(
                    PropagationJudgment::ConcreteTable,
                    *results.begin());
            }
            else
            {
                SLANG_UNEXPECTED("Unexpected data type for LookupWitnessMethod");
            }
        }
        else
        {
            if (auto funcType = as<IRFuncType>(inst->getDataType()))
            {
                return PropagationInfo::makeSetOfFuncs(results, funcType);
            }
            else if (as<IRTypeKind>(inst->getDataType()))
            {
                return PropagationInfo::makeSet(PropagationJudgment::SetOfTypes, results);
            }
            else if (as<IRWitnessTableType>(inst->getDataType()))
            {
                return PropagationInfo::makeSet(PropagationJudgment::SetOfTables, results);
            }
            else
            {
                SLANG_UNEXPECTED("Unexpected data type for LookupWitnessMethod");
            }
        }
    }

    PropagationInfo analyzeExtractExistentialWitnessTable(IRExtractExistentialWitnessTable* inst)
    {
        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(operand);

        if (!operandInfo)
            return PropagationInfo::none();

        if (operandInfo.judgment == PropagationJudgment::Unbounded)
            return PropagationInfo::makeUnbounded();

        if (operandInfo.judgment == PropagationJudgment::Existential)
        {
            HashSet<IRInst*> tables;
            for (auto table : operandInfo.possibleValues)
            {
                tables.add(table);
            }

            if (tables.getCount() == 1)
            {
                return PropagationInfo::makeConcrete(
                    PropagationJudgment::ConcreteTable,
                    *tables.begin());
            }
            else
            {
                return PropagationInfo::makeSet(PropagationJudgment::SetOfTables, tables);
            }
        }

        return PropagationInfo::makeConcrete(PropagationJudgment::ConcreteTable, inst);
    }

    PropagationInfo analyzeExtractExistentialType(IRExtractExistentialType* inst)
    {
        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(operand);

        if (!operandInfo)
            return PropagationInfo::none();

        if (operandInfo.judgment == PropagationJudgment::Unbounded)
            return PropagationInfo::makeUnbounded();

        if (operandInfo.judgment == PropagationJudgment::Existential)
        {
            HashSet<IRInst*> types;
            // Extract types from witness tables by looking at the concrete types
            for (auto table : operandInfo.possibleValues)
            {
                // Get the concrete type from the witness table
                if (auto witnessTable = as<IRWitnessTable>(table))
                {
                    if (auto concreteType = witnessTable->getConcreteType())
                    {
                        types.add(concreteType);
                    }
                }
                else
                {
                    SLANG_UNEXPECTED("Expected witness table in existential extraction base type");
                }
            }

            if (types.getCount() == 0)
            {
                // No concrete types found, treat as this instruction
                types.add(inst);
            }

            if (types.getCount() == 1)
            {
                return PropagationInfo::makeConcrete(
                    PropagationJudgment::ConcreteType,
                    *types.begin());
            }
            else
            {
                return PropagationInfo::makeSet(PropagationJudgment::SetOfTypes, types);
            }
        }

        return PropagationInfo::makeConcrete(PropagationJudgment::ConcreteType, inst);
    }

    PropagationInfo analyzeExtractExistentialValue(IRExtractExistentialValue* inst)
    {
        // The value itself is just a regular value
        return PropagationInfo::makeValue();
    }

    PropagationInfo analyzeCall(IRCall* inst, LinkedList<WorkItem>& workQueue)
    {
        auto callee = inst->getCallee();
        auto calleeInfo = tryGetInfo(callee);

        auto funcType = as<IRFuncType>(callee->getDataType());

        //
        // Propagate the input judgments to the call & append a work item
        // for inter-procedural propagation.
        //

        // For now, we'll handle just a concrete func. But the logic for multiple functions
        // is exactly the same (add an edge for each function).
        //
        auto propagateToCallSite = [&](IRFunc* func)
        {
            // Register the call site in the map to allow for the
            // return-edge to be created.
            //
            // We use an explicit map instead of walking the uses of the
            // func, since we might have functions that are called indirectly
            // through lookups.
            //
            this->funcCallSites.addIfNotExists(func, HashSet<IRCall*>());
            if (this->funcCallSites[func].add(inst))
            {
                // If this is a new call site, add a propagation task to the queue (in case there's
                // already information about this function)
                workQueue.addLast(WorkItem(InterproceduralEdge::Direction::FuncToCall, inst, func));
            }
            workQueue.addLast(WorkItem(InterproceduralEdge::Direction::CallToFunc, inst, func));
        };

        if (calleeInfo)
        {
            if (calleeInfo.judgment == PropagationJudgment::ConcreteFunc)
            {
                // If we have a concrete function, register the call site
                propagateToCallSite(as<IRFunc>(calleeInfo.concreteValue));
            }
            else if (calleeInfo.judgment == PropagationJudgment::SetOfFuncs)
            {
                // If we have a set of functions, register each one
                for (auto func : calleeInfo.possibleValues)
                    propagateToCallSite(as<IRFunc>(func));
            }
        }

        if (auto callInfo = tryGetInfo(inst))
            return callInfo;
        else
            return PropagationInfo::none();
    }

    void propagateWithinFuncEdge(IREdge edge, LinkedList<WorkItem>& workQueue)
    {
        // Handle intra-procedural edge (original logic)
        auto predecessorBlock = edge.getPredecessor();
        auto successorBlock = edge.getSuccessor();

        // Get the terminator instruction and extract arguments
        auto terminator = predecessorBlock->getTerminator();
        if (!terminator)
            return;

        // Right now, only unconditional branches can propagate new information
        auto unconditionalBranch = as<IRUnconditionalBranch>(terminator);
        if (!unconditionalBranch)
            return;

        // Find which successor this edge leads to (should be the target)
        if (unconditionalBranch->getTargetBlock() != successorBlock)
            return;

        // Collect propagation info for each argument and update corresponding parameter
        // TODO: Unify this logic with the affectedBlocks logic in the per-inst processing logic.
        HashSet<IRBlock*> affectedBlocks;
        Index paramIndex = 0;
        for (auto param : successorBlock->getParams())
        {
            if (paramIndex < unconditionalBranch->getArgCount())
            {
                auto arg = unconditionalBranch->getArg(paramIndex);
                if (auto argInfo = tryGetInfo(arg))
                {
                    // Union with existing parameter info
                    bool infoChanged = false;
                    if (auto existingInfo = tryGetInfo(param))
                    {
                        propagationMap[param] = unionPropagationInfo(existingInfo, argInfo);
                        if (!infoChanged && !areInfosEqual(existingInfo, propagationMap[param]))
                            infoChanged = true;
                    }
                    else
                    {
                        propagationMap[param] = argInfo;
                        infoChanged = true;
                    }
                    // If any info changed, add all user blocks to the affected set
                    if (infoChanged)
                    {
                        for (auto use = param->firstUse; use; use = use->nextUse)
                        {
                            auto user = use->getUser();
                            if (auto block = as<IRBlock>(user->getParent()))
                                affectedBlocks.add(block);
                        }
                    }
                }
            }
            paramIndex++;
        }

        for (auto block : affectedBlocks)
        {
            workQueue.addLast(WorkItem(block));
        }
    }

    void propagateInterproceduralEdge(InterproceduralEdge edge, LinkedList<WorkItem>& workQueue)
    {
        // Handle interprocedural edge
        auto callInst = edge.callInst;
        auto targetFunc = edge.targetFunc;

        switch (edge.direction)
        {
        case InterproceduralEdge::Direction::CallToFunc:
            {
                // Propagate argument info from call site to function parameters
                auto firstBlock = targetFunc->getFirstBlock();
                if (!firstBlock)
                    return;

                Index argIndex = 1; // Skip callee (operand 0)
                HashSet<IRBlock*> affectedBlocks;
                for (auto param : firstBlock->getParams())
                {
                    if (argIndex < callInst->getOperandCount())
                    {
                        auto arg = callInst->getOperand(argIndex);
                        if (auto argInfo = tryGetInfo(arg))
                        {
                            // Union with existing parameter info
                            auto existingInfo = tryGetInfo(param);
                            auto newInfo = unionPropagationInfo(tryGetInfo(param), argInfo);
                            propagationMap[param] = newInfo;
                            if (!areInfosEqual(existingInfo, newInfo))
                            {
                                for (auto use = param->firstUse; use; use = use->nextUse)
                                {
                                    auto user = use->getUser();
                                    if (auto block = as<IRBlock>(user->getParent()))
                                        affectedBlocks.add(block);
                                }
                            }
                        }
                    }
                    argIndex++;
                }
                // Add the affected block to the work queue if any info changed
                for (auto block : affectedBlocks)
                    workQueue.addLast(WorkItem(block));
                break;
            }
        case InterproceduralEdge::Direction::FuncToCall:
            {
                // Propagate return value info from function to call site
                auto returnInfo = funcReturnInfo.tryGetValue(targetFunc);

                bool anyInfoChanged = false;
                if (returnInfo)
                {
                    // Union with existing call info
                    auto existingCallInfo = tryGetInfo(callInst);
                    auto newInfo = unionPropagationInfo(existingCallInfo, *returnInfo);
                    propagationMap[callInst] = newInfo;
                    if (!areInfosEqual(existingCallInfo, newInfo))
                        anyInfoChanged = true;
                }

                // Add the callInst's parent block to the work queue if any info changed
                if (anyInfoChanged)
                    workQueue.addLast(WorkItem(as<IRBlock>(callInst->getParent())));

                break;
            }
        default:
            SLANG_UNEXPECTED("Unhandled interprocedural edge direction");
            return;
        }
    }

    void initializeFirstBlockParameters(IRFunc* func)
    {
        auto firstBlock = func->getFirstBlock();
        if (!firstBlock)
            return;

        // Initialize parameters based on their types
        for (auto param : firstBlock->getParams())
        {
            auto paramType = param->getDataType();
            auto paramInfo = tryGetInfo(param);
            if (paramInfo)
                continue; // Already has some information

            if (auto interfaceType = as<IRInterfaceType>(paramType))
            {
                if (interfaceType->findDecoration<IRComInterfaceDecoration>())
                    propagationMap[param] = PropagationInfo::makeUnbounded();
                else
                    propagationMap[param] = PropagationInfo::none(); // Initialize to none.
            }
            else if (
                as<IRWitnessTableType>(paramType) || as<IRFuncType>(paramType) ||
                as<IRTypeKind>(paramType) || as<IRTypeType>(paramType))
            {
                propagationMap[param] = PropagationInfo::none();
            }
            else
            {
                propagationMap[param] = PropagationInfo::makeValue();
            }
        }
    }

    bool propagateReturnValues(IRBlock* block, LinkedList<WorkItem>& workQueue)
    {
        auto terminator = block->getTerminator();
        auto returnInst = as<IRReturn>(terminator);
        if (!returnInst)
            return false;

        auto func = as<IRFunc>(block->getParent());
        if (!func)
            return false;

        // Get return value info if there is a return value
        PropagationInfo returnValueInfo;
        if (returnInst->getOperandCount() > 0)
        {
            auto returnValue = returnInst->getOperand(0);
            returnValueInfo = tryGetInfo(returnValue);
        }
        else
        {
            // Void return
            returnValueInfo = PropagationInfo::makeValue();
        }

        // Update function return info by unioning with existing info
        auto existingReturnInfo = funcReturnInfo.tryGetValue(func);
        bool returnInfoChanged = false;

        if (returnValueInfo)
        {
            if (existingReturnInfo)
            {
                auto newReturnInfo = unionPropagationInfo(
                    List<PropagationInfo>({*existingReturnInfo, returnValueInfo}));

                if (!areInfosEqual(*existingReturnInfo, newReturnInfo))
                {
                    funcReturnInfo[func] = newReturnInfo;
                    returnInfoChanged = true;
                }
            }
            else
            {
                funcReturnInfo[func] = returnValueInfo;
                returnInfoChanged = true;
            }
        }

        // If return info changed, add return edges to call sites
        if (returnInfoChanged && this->funcCallSites.containsKey(func))
        {
            for (auto callSite : this->funcCallSites[func])
            {
                workQueue.addLast(
                    WorkItem(InterproceduralEdge::Direction::FuncToCall, callSite, func));
            }
        }

        return returnInfoChanged;
    }

    PropagationInfo unionPropagationInfo(const List<PropagationInfo>& infos)
    {
        if (infos.getCount() == 0)
        {
            return PropagationInfo::makeValue();
        }

        if (infos.getCount() == 1)
        {
            return infos[0];
        }

        // Check if all infos are the same
        bool allSame = true;
        for (Index i = 1; i < infos.getCount(); i++)
        {
            if (!areInfosEqual(infos[0], infos[i]))
            {
                allSame = false;
                break;
            }
        }

        if (allSame)
        {
            return infos[0];
        }

        // Need to create a union - collect all possible values based on judgment types
        HashSet<IRInst*> allValues;
        IRFuncType* dynFuncType = nullptr;
        PropagationJudgment unionJudgment = PropagationJudgment::None;

        // Determine the union judgment type and collect values
        for (auto info : infos)
        {
            switch (info.judgment)
            {
            case PropagationJudgment::ConcreteType:
                unionJudgment = PropagationJudgment::SetOfTypes;
                allValues.add(info.concreteValue);
                break;
            case PropagationJudgment::ConcreteTable:
                unionJudgment = PropagationJudgment::SetOfTables;
                allValues.add(info.concreteValue);
                break;
            case PropagationJudgment::ConcreteFunc:
                unionJudgment = PropagationJudgment::SetOfFuncs;
                allValues.add(info.concreteValue);
                break;
            case PropagationJudgment::SetOfTypes:
                unionJudgment = PropagationJudgment::SetOfTypes;
                for (auto value : info.possibleValues)
                    allValues.add(value);
                break;
            case PropagationJudgment::SetOfTables:
                unionJudgment = PropagationJudgment::SetOfTables;
                for (auto value : info.possibleValues)
                    allValues.add(value);
                break;
            case PropagationJudgment::SetOfFuncs:
                unionJudgment = PropagationJudgment::SetOfFuncs;
                for (auto value : info.possibleValues)
                    allValues.add(value);
                if (!dynFuncType)
                {
                    // If we haven't set a function type yet, use the first one
                    dynFuncType = info.dynFuncType;
                }
                else if (dynFuncType != info.dynFuncType)
                {
                    SLANG_UNEXPECTED(
                        "Mismatched function types in union propagation info for SetOfFuncs");
                }

                break;
            case PropagationJudgment::Value:
                if (unionJudgment == PropagationJudgment::None)
                    unionJudgment = PropagationJudgment::Value;
                else
                {
                    SLANG_ASSERT(unionJudgment == PropagationJudgment::Value);
                }
                break;
            case PropagationJudgment::None:
                // None judgments are basically 'empty'
                break;
            case PropagationJudgment::Existential:
                // For existential union, we need to collect all witness tables
                // For now, we'll handle this properly by creating a new existential with all tables
                {
                    HashSet<IRInst*> allTables;
                    for (auto otherInfo : infos)
                    {
                        if (otherInfo.judgment == PropagationJudgment::Existential)
                        {
                            for (auto table : otherInfo.possibleValues)
                            {
                                allTables.add(table);
                            }
                        }
                    }
                    if (allTables.getCount() > 0)
                    {
                        return PropagationInfo::makeExistential(allTables);
                    }
                }
                return PropagationInfo::none();
            case PropagationJudgment::Unbounded:
                // If any info is unbounded, the union is unbounded
                return PropagationInfo::makeUnbounded();
            }
        }

        // If we collected values, create a set; otherwise return value
        if (allValues.getCount() > 0)
        {
            if (unionJudgment == PropagationJudgment::SetOfFuncs && dynFuncType)
                return PropagationInfo::makeSetOfFuncs(allValues, dynFuncType);

            return PropagationInfo::makeSet(unionJudgment, allValues);
        }
        else
        {
            return PropagationInfo::none();
        }
    }

    PropagationInfo unionPropagationInfo(PropagationInfo info1, PropagationInfo info2)
    {
        // Union the two infos
        List<PropagationInfo> infos;
        infos.add(info1);
        infos.add(info2);
        return unionPropagationInfo(infos);
    }

    PropagationInfo analyzeDefault(IRInst* inst)
    {
        // Check if this is a type, witness table, or function
        if (as<IRType>(inst))
        {
            return PropagationInfo::makeConcrete(PropagationJudgment::ConcreteType, inst);
        }
        else if (as<IRWitnessTable>(inst))
        {
            return PropagationInfo::makeConcrete(PropagationJudgment::ConcreteTable, inst);
        }
        else if (as<IRFunc>(inst))
        {
            return PropagationInfo::makeConcrete(PropagationJudgment::ConcreteFunc, inst);
        }
        else
        {
            return PropagationInfo::makeValue();
        }
    }

    void performDynamicInstLowering()
    {
        // Collect all instructions that need lowering
        List<IRInst*> typeInstsToLower;
        List<IRInst*> valueInstsToLower;
        List<IRInst*> instWithReplacementTypes;
        List<IRFunc*> funcTypesToProcess;

        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(globalInst))
            {
                // Process each function's instructions
                for (auto block : func->getBlocks())
                {
                    for (auto child : block->getChildren())
                    {
                        if (as<IRTerminatorInst>(child))
                            continue; // Skip parameters and terminators

                        switch (child->getOp())
                        {
                        case kIROp_LookupWitnessMethod:
                            {
                                if (child->getDataType()->getOp() == kIROp_TypeKind)
                                    typeInstsToLower.add(child);
                                else
                                    valueInstsToLower.add(child);
                                break;
                            }
                        case kIROp_ExtractExistentialType:
                            typeInstsToLower.add(child);
                            break;
                        case kIROp_ExtractExistentialWitnessTable:
                        case kIROp_ExtractExistentialValue:
                        case kIROp_MakeExistential:
                        case kIROp_CreateExistentialObject:
                            valueInstsToLower.add(child);
                            break;
                        case kIROp_Call:
                            {
                                if (auto info = tryGetInfo(child))
                                    if (info.judgment == PropagationJudgment::Existential)
                                        instWithReplacementTypes.add(child);

                                if (auto calleeInfo = tryGetInfo(as<IRCall>(child)->getCallee()))
                                    if (calleeInfo.judgment == PropagationJudgment::SetOfFuncs)
                                        valueInstsToLower.add(child);
                            }
                            break;
                        default:
                            if (auto info = tryGetInfo(child))
                                if (info.judgment == PropagationJudgment::Existential)
                                    // If this instruction has a set of types, tables, or funcs,
                                    // we need to lower it to a unified type.
                                    instWithReplacementTypes.add(child);
                        }
                    }
                }

                funcTypesToProcess.add(func);
            }
        }

        for (auto inst : typeInstsToLower)
            lowerInst(inst);

        for (auto func : funcTypesToProcess)
            replaceFuncType(func, this->funcReturnInfo[func]);

        for (auto inst : valueInstsToLower)
            lowerInst(inst);

        for (auto inst : instWithReplacementTypes)
            replaceType(inst);
    }

    void replaceFuncType(IRFunc* func, PropagationInfo& returnTypeInfo)
    {
        IRFuncType* origFuncType = as<IRFuncType>(func->getFullType());
        IRType* returnType = origFuncType->getResultType();
        if (returnTypeInfo.judgment == PropagationJudgment::Existential)
        {
            // If the return type is existential, we need to replace it with a tuple type
            returnType = getTypeForExistential(returnTypeInfo);
        }

        List<IRType*> paramTypes;
        for (auto param : func->getFirstBlock()->getParams())
        {
            // Extract the existential type from the parameter if it exists
            auto paramInfo = tryGetInfo(param);
            if (paramInfo && paramInfo.judgment == PropagationJudgment::Existential)
            {
                paramTypes.add(getTypeForExistential(paramInfo));
            }
            else
                paramTypes.add(param->getDataType());
        }

        IRBuilder builder(module);
        builder.setInsertBefore(func);
        func->setFullType(builder.getFuncType(paramTypes, returnType));
    }

    IRType* getTypeForExistential(PropagationInfo info)
    {
        // Replace type with Tuple<UInt, AnyValueType>
        IRBuilder builder(module);
        builder.setInsertInto(module);

        HashSet<IRInst*> types;
        // Extract types from witness tables by looking at the concrete types
        for (auto table : info.possibleValues)
            if (auto witnessTable = as<IRWitnessTable>(table))
                if (auto concreteType = witnessTable->getConcreteType())
                    types.add(concreteType);

        auto anyValueType = createAnyValueTypeFromInsts(types);
        return builder.getTupleType(List<IRType*>({builder.getUIntType(), anyValueType}));
    }

    void replaceType(IRInst* inst)
    {
        auto info = tryGetInfo(inst);
        if (!info || info.judgment != PropagationJudgment::Existential)
            return;

        /* Replace type with Tuple<UInt, AnyValueType>
        IRBuilder builder(module);
        builder.setInsertBefore(inst);

        HashSet<IRInst*> types;
        // Extract types from witness tables by looking at the concrete types
        for (auto table : info.possibleValues)
            if (auto witnessTable = as<IRWitnessTable>(table))
                if (auto concreteType = witnessTable->getConcreteType())
                    types.add(concreteType);

        auto anyValueType = createAnyValueTypeFromInsts(types);
        auto tupleType = builder.getTupleType(List<IRType*>({builder.getUIntType(),
        anyValueType}));*/

        inst->setFullType(getTypeForExistential(info));
    }

    void lowerInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_LookupWitnessMethod:
            lowerLookupWitnessMethod(as<IRLookupWitnessMethod>(inst));
            break;
        case kIROp_ExtractExistentialWitnessTable:
            lowerExtractExistentialWitnessTable(as<IRExtractExistentialWitnessTable>(inst));
            break;
        case kIROp_ExtractExistentialType:
            lowerExtractExistentialType(as<IRExtractExistentialType>(inst));
            break;
        case kIROp_ExtractExistentialValue:
            lowerExtractExistentialValue(as<IRExtractExistentialValue>(inst));
            break;
        case kIROp_Call:
            lowerCall(as<IRCall>(inst));
            break;
        case kIROp_MakeExistential:
            lowerMakeExistential(as<IRMakeExistential>(inst));
            break;
        case kIROp_CreateExistentialObject:
            lowerCreateExistentialObject(as<IRCreateExistentialObject>(inst));
            break;
        }
    }

    void lowerLookupWitnessMethod(IRLookupWitnessMethod* inst)
    {
        auto info = tryGetInfo(inst);
        if (!info)
            return;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Check if this is a TypeKind data type with SetOfTypes judgment
        if (info.judgment == PropagationJudgment::SetOfTypes &&
            inst->getDataType()->getOp() == kIROp_TypeKind)
        {
            // Create an any-value type based on the set of types
            auto anyValueType = createAnyValueTypeFromInsts(info.possibleValues);

            // Store the mapping for later use
            loweredInstToAnyValueType[inst] = anyValueType;

            // Replace the instruction with the any-value type
            inst->replaceUsesWith(anyValueType);
            inst->removeAndDeallocate();
            return;
        }

        if (info.judgment == PropagationJudgment::SetOfTables ||
            info.judgment == PropagationJudgment::SetOfFuncs)
        {
            // Get the witness table operand info
            auto witnessTableInst = inst->getWitnessTable();
            auto witnessTableInfo = tryGetInfo(witnessTableInst);

            if (witnessTableInfo && witnessTableInfo.judgment == PropagationJudgment::SetOfTables)
            {
                // Create a key mapping function
                auto keyMappingFunc = createKeyMappingFunc(
                    inst->getRequirementKey(),
                    witnessTableInfo.possibleValues,
                    info.possibleValues);

                // Replace with call to key mapping function
                auto witnessTableId = builder.emitCallInst(
                    builder.getUIntType(),
                    keyMappingFunc,
                    List<IRInst*>({inst->getWitnessTable()}));
                inst->replaceUsesWith(witnessTableId);
                propagationMap[witnessTableId] = info;
                inst->removeAndDeallocate();
            }
        }
    }

    void lowerExtractExistentialWitnessTable(IRExtractExistentialWitnessTable* inst)
    {
        auto info = tryGetInfo(inst);
        if (!info)
            return;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        if (info.judgment == PropagationJudgment::SetOfTables)
        {
            // Replace with GetElement(loweredInst, 0) -> uint
            auto operand = inst->getOperand(0);
            auto element = builder.emitGetTupleElement(builder.getUIntType(), operand, 0);
            inst->replaceUsesWith(element);
            propagationMap[element] = info;
            inst->removeAndDeallocate();
        }
    }

    void lowerExtractExistentialValue(IRExtractExistentialValue* inst)
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Check if we have a lowered any-value type for the result
        auto resultType = inst->getDataType();
        auto loweredType = loweredInstToAnyValueType.tryGetValue(inst);
        if (loweredType)
        {
            resultType = *loweredType;
        }

        // Replace with GetElement(loweredInst, 1) -> AnyValueType
        auto operand = inst->getOperand(0);
        auto element = builder.emitGetTupleElement(resultType, operand, 1);
        inst->replaceUsesWith(element);
        inst->removeAndDeallocate();
    }

    void lowerExtractExistentialType(IRExtractExistentialType* inst)
    {
        auto info = tryGetInfo(inst);
        if (!info || info.judgment != PropagationJudgment::SetOfTypes)
            return;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Create an any-value type based on the set of types
        auto anyValueType = createAnyValueTypeFromInsts(info.possibleValues);

        // Store the mapping for later use
        loweredInstToAnyValueType[inst] = anyValueType;

        // Replace the instruction with the any-value type
        inst->replaceUsesWith(anyValueType);
        inst->removeAndDeallocate();
    }

    IRFuncType* getExpectedFuncType(IRCall* inst)
    {
        // Translate argument types into expected function type.
        // For now, we handle just 'in' arguments.
        List<IRType*> argTypes;
        for (UInt i = 0; i < inst->getArgCount(); i++)
        {
            auto arg = inst->getArg(i);
            if (auto argInfo = tryGetInfo(arg))
            {
                // If the argument is existential, we need to use the type for existential
                if (argInfo.judgment == PropagationJudgment::Existential)
                {
                    argTypes.add(getTypeForExistential(argInfo));
                    continue;
                }
            }

            argTypes.add(arg->getDataType());
        }

        // Translate result type.
        IRType* resultType = inst->getDataType();
        auto returnInfo = tryGetInfo(inst);
        if (returnInfo && returnInfo.judgment == PropagationJudgment::Existential)
        {
            resultType = getTypeForExistential(returnInfo);
        }

        IRBuilder builder(module);
        builder.setInsertInto(module);
        return builder.getFuncType(argTypes, resultType);
    }

    void lowerCall(IRCall* inst)
    {
        auto callee = inst->getCallee();
        auto calleeInfo = tryGetInfo(callee);

        if (!calleeInfo || calleeInfo.judgment != PropagationJudgment::SetOfFuncs)
            return;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        auto expectedFuncType = getExpectedFuncType(inst);
        // Create dispatch function
        auto dispatchFunc = createDispatchFunc(calleeInfo.possibleValues, expectedFuncType);

        // Replace call with dispatch
        List<IRInst*> newArgs;
        newArgs.add(callee); // Add the lookup as first argument (will get lowered into an uint tag)
        for (UInt i = 1; i < inst->getOperandCount(); i++)
        {
            newArgs.add(inst->getOperand(i));
        }

        auto newCall = builder.emitCallInst(inst->getDataType(), dispatchFunc, newArgs);
        inst->replaceUsesWith(newCall);
        if (auto info = tryGetInfo(inst))
            propagationMap[newCall] = info;
        replaceType(newCall); // "maybe replace type"
        inst->removeAndDeallocate();
    }

    void lowerMakeExistential(IRMakeExistential* inst)
    {
        auto info = tryGetInfo(inst);
        if (!info || info.judgment != PropagationJudgment::Existential)
            return;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Get unique ID for the witness table. TODO: Assert that this is a concrete table..
        auto witnessTable = cast<IRWitnessTable>(inst->getWitnessTable());
        auto tableId = builder.getIntValue(builder.getUIntType(), getUniqueID(witnessTable));

        // Collect types from the witness tables to determine the any-value type
        HashSet<IRType*> types;
        for (auto table : info.possibleValues)
        {
            if (auto witnessTableInst = as<IRWitnessTable>(table))
            {
                if (auto concreteType = witnessTableInst->getConcreteType())
                {
                    types.add(concreteType);
                }
            }
        }

        // Create the appropriate any-value type
        auto anyValueType = createAnyValueType(types);

        // Pack the value
        auto packedValue = builder.emitPackAnyValue(anyValueType, inst->getWrappedValue());

        // Create tuple (table_unique_id, PackAnyValue(val))
        auto tupleType = builder.getTupleType(
            List<IRType*>({builder.getUIntType(), packedValue->getDataType()}));
        IRInst* tupleArgs[] = {tableId, packedValue};
        auto tuple = builder.emitMakeTuple(tupleType, 2, tupleArgs);

        inst->replaceUsesWith(tuple);
        inst->removeAndDeallocate();
    }

    void lowerCreateExistentialObject(IRCreateExistentialObject* inst)
    {
        // Error out for now as specified
        sink->diagnose(
            inst,
            Diagnostics::unimplemented,
            "IRCreateExistentialObject lowering not yet implemented");
    }

    UInt getUniqueID(IRInst* funcOrTable)
    {
        auto existingId = uniqueIds.tryGetValue(funcOrTable);
        if (existingId)
            return *existingId;

        UInt newId = nextUniqueId++;
        uniqueIds[funcOrTable] = newId;
        return newId;
    }

    IRFunc* createKeyMappingFunc(
        IRInst* key,
        const HashSet<IRInst*>& inputTables,
        const HashSet<IRInst*>& outputVals)
    {
        // Create a function that maps input IDs to output IDs
        IRBuilder builder(module);

        auto funcType =
            builder.getFuncType(List<IRType*>({builder.getUIntType()}), builder.getUIntType());
        auto func = builder.createFunc();
        builder.setInsertInto(func);
        func->setFullType(funcType);

        auto entryBlock = builder.emitBlock();
        builder.setInsertInto(entryBlock);

        auto param = builder.emitParam(builder.getUIntType());

        // Create default block that returns 0
        auto defaultBlock = builder.emitBlock();
        builder.setInsertInto(defaultBlock);
        builder.emitReturn(builder.getIntValue(builder.getUIntType(), 0));

        // Go back to entry block and create switch
        builder.setInsertInto(entryBlock);

        // Create case blocks for each input table
        List<IRInst*> caseValues;
        List<IRBlock*> caseBlocks;

        // Build mapping from input tables to output values
        List<IRInst*> inputTableArray;
        List<IRInst*> outputValArray;

        for (auto table : inputTables)
            inputTableArray.add(table);
        for (auto table : outputVals)
            outputValArray.add(table);

        for (Index i = 0; i < inputTableArray.getCount(); i++)
        {
            auto inputTable = inputTableArray[i];
            auto inputId = getUniqueID(inputTable);

            // Find corresponding output table (for now, use simple 1:1 mapping)
            IRInst* outputVal = nullptr;
            if (i < outputValArray.getCount())
            {
                outputVal = outputValArray[i];
            }
            else if (outputValArray.getCount() > 0)
            {
                outputVal = outputValArray[0]; // Fallback to first output
            }

            if (outputVal)
            {
                auto outputId = getUniqueID(outputVal);

                // Create case block
                auto caseBlock = builder.emitBlock();
                builder.setInsertInto(caseBlock);
                builder.emitReturn(builder.getIntValue(builder.getUIntType(), outputId));

                caseValues.add(builder.getIntValue(builder.getUIntType(), inputId));
                caseBlocks.add(caseBlock);
            }
        }

        // Create flattened case arguments array
        List<IRInst*> flattenedCaseArgs;
        for (Index i = 0; i < caseValues.getCount(); i++)
        {
            flattenedCaseArgs.add(caseValues[i]);
            flattenedCaseArgs.add(caseBlocks[i]);
        }

        // Emit an unreachable block for the break block.
        auto unreachableBlock = builder.emitBlock();
        builder.setInsertInto(unreachableBlock);
        builder.emitUnreachable();

        // Go back to entry and emit switch
        builder.setInsertInto(entryBlock);
        builder.emitSwitch(
            param,
            unreachableBlock,
            defaultBlock,
            flattenedCaseArgs.getCount(),
            flattenedCaseArgs.getBuffer());

        return func;
    }

    IRFunc* createDispatchFunc(const HashSet<IRInst*>& funcs, IRFuncType* expectedFuncType)
    {
        // Create a dispatch function with switch-case for each function
        IRBuilder builder(module);

        // Extract parameter types from the first function in the set
        List<IRType*> paramTypes;
        paramTypes.add(builder.getUIntType()); // ID parameter

        // Get parameter types from first function
        List<IRInst*> funcArray;
        for (auto func : funcs)
            funcArray.add(func);

        for (UInt i = 0; i < expectedFuncType->getParamCount(); i++)
        {
            paramTypes.add(expectedFuncType->getParamType(i));
        }

        auto resultType = expectedFuncType->getResultType();
        auto funcType = builder.getFuncType(paramTypes, resultType);
        auto func = builder.createFunc();
        builder.setInsertInto(func);
        func->setFullType(funcType);

        auto entryBlock = builder.emitBlock();
        builder.setInsertInto(entryBlock);

        auto idParam = builder.emitParam(builder.getUIntType());

        // Create parameters for the original function arguments
        List<IRInst*> originalParams;
        for (UInt i = 1; i < paramTypes.getCount(); i++)
        {
            originalParams.add(builder.emitParam(paramTypes[i]));
        }

        // Create default block
        auto defaultBlock = builder.emitBlock();
        builder.setInsertInto(defaultBlock);
        if (resultType->getOp() == kIROp_VoidType)
        {
            builder.emitReturn();
        }
        else
        {
            // Return a default-constructed value
            auto defaultValue = builder.emitDefaultConstruct(resultType);
            builder.emitReturn(defaultValue);
        }

        auto maybePackValue = [&](IRBuilder* builder, IRInst* value, IRType* type) -> IRInst*
        {
            // If the type is AnyValueType, pack the value
            if (as<IRAnyValueType>(type) && !as<IRAnyValueType>(value->getDataType()))
            {
                return builder->emitPackAnyValue(type, value);
            }
            return value; // Otherwise, return as is
        };

        auto maybeUnpackValue = [&](IRBuilder* builder, IRInst* value, IRType* type) -> IRInst*
        {
            // If the type is AnyValueType, unpack the value
            if (as<IRAnyValueType>(value->getDataType()) && !as<IRAnyValueType>(type))
            {
                return builder->emitUnpackAnyValue(type, value);
            }
            return value; // Otherwise, return as is
        };

        // Go back to entry block and create switch
        builder.setInsertInto(entryBlock);

        // Create case blocks for each function
        List<IRInst*> caseValues;
        List<IRBlock*> caseBlocks;

        for (auto funcInst : funcs)
        {
            auto funcId = getUniqueID(funcInst);

            // Create case block
            auto caseBlock = builder.emitBlock();
            builder.setInsertInto(caseBlock);

            List<IRInst*> callArgs;
            auto concreteFuncType = as<IRFuncType>(funcInst->getDataType());
            for (UIndex ii = 0; ii < originalParams.getCount(); ii++)
            {
                callArgs.add(maybeUnpackValue(
                    &builder,
                    originalParams[ii],
                    concreteFuncType->getParamType(ii)));
            }

            // Call the specific function
            auto callResult =
                builder.emitCallInst(concreteFuncType->getResultType(), funcInst, callArgs);

            if (resultType->getOp() == kIROp_VoidType)
            {
                builder.emitReturn();
            }
            else
            {
                builder.emitReturn(maybePackValue(&builder, callResult, resultType));
            }

            caseValues.add(builder.getIntValue(builder.getUIntType(), funcId));
            caseBlocks.add(caseBlock);
        }

        // Create flattened case arguments array
        List<IRInst*> flattenedCaseArgs;
        for (Index i = 0; i < caseValues.getCount(); i++)
        {
            flattenedCaseArgs.add(caseValues[i]);
            flattenedCaseArgs.add(caseBlocks[i]);
        }

        // Create an unreachable block for the break block.
        auto unreachableBlock = builder.emitBlock();
        builder.setInsertInto(unreachableBlock);
        builder.emitUnreachable();

        // Go back to entry and emit switch
        builder.setInsertInto(entryBlock);
        builder.emitSwitch(
            idParam,
            unreachableBlock,
            defaultBlock,
            flattenedCaseArgs.getCount(),
            flattenedCaseArgs.getBuffer());

        return func;
    }

    IRAnyValueType* createAnyValueType(const HashSet<IRType*>& types)
    {
        IRBuilder builder(module);
        auto size = calculateAnyValueSize(types);
        return builder.getAnyValueType(size);
    }

    IRAnyValueType* createAnyValueTypeFromInsts(const HashSet<IRInst*>& typeInsts)
    {
        HashSet<IRType*> types;
        for (auto inst : typeInsts)
        {
            if (auto type = as<IRType>(inst))
            {
                types.add(type);
            }
        }
        return createAnyValueType(types);
    }

    SlangInt calculateAnyValueSize(const HashSet<IRType*>& types)
    {
        SlangInt maxSize = 0;
        for (auto type : types)
        {
            auto size = getAnyValueSize(type);
            if (size > maxSize)
                maxSize = size;
        }
        return maxSize;
    }

    bool needsReinterpret(PropagationInfo sourceInfo, PropagationInfo targetInfo)
    {
        if (!sourceInfo || !targetInfo)
            return false;

        // Check if both are SetOfTypes with different sets
        if (sourceInfo.judgment == PropagationJudgment::SetOfTypes &&
            targetInfo.judgment == PropagationJudgment::SetOfTypes)
        {
            if (sourceInfo.possibleValues.getCount() != targetInfo.possibleValues.getCount())
                return true;
        }

        // Check if both are Existential with different witness table sets
        if (sourceInfo.judgment == PropagationJudgment::Existential &&
            targetInfo.judgment == PropagationJudgment::Existential)
        {
            if (sourceInfo.possibleValues.getCount() != targetInfo.possibleValues.getCount())
                return true;
        }

        return false;
    }

    bool isExistentialType(IRType* type) { return as<IRInterfaceType>(type) != nullptr; }

    bool isInterfaceType(IRType* type) { return as<IRInterfaceType>(type) != nullptr; }

    HashSet<IRInst*> collectExistentialTables(IRInterfaceType* interfaceType)
    {
        HashSet<IRInst*> tables;

        IRWitnessTableType* targetTableType = nullptr;
        // First, find the IRWitnessTableType that wraps the given interfaceType
        for (auto use = interfaceType->firstUse; use; use = use->nextUse)
        {
            if (auto wtType = as<IRWitnessTableType>(use->getUser()))
            {
                if (wtType->getConformanceType() == interfaceType)
                {
                    targetTableType = wtType;
                    break;
                }
            }
        }

        // If the target witness table type was found, gather all witness tables using it
        if (targetTableType)
        {
            for (auto use = targetTableType->firstUse; use; use = use->nextUse)
            {
                if (auto witnessTable = as<IRWitnessTable>(use->getUser()))
                {
                    if (witnessTable->getDataType() == targetTableType)
                    {
                        tables.add(witnessTable);
                    }
                }
            }
        }

        return tables;
    }

    void processModule()
    {
        // Phase 1: Information Propagation
        performInformationPropagation();

        // Phase 1.5: Insert reinterprets for points where sets merge
        // e.g. phi, return, call
        //
        insertReinterprets();

        // Phase 2: Dynamic Instruction Lowering
        performDynamicInstLowering();
    }

    DynamicInstLoweringContext(IRModule* module, DiagnosticSink* sink)
        : module(module), sink(sink)
    {
    }

    // Basic context
    IRModule* module;
    DiagnosticSink* sink;

    // Mapping from instruction to propagation information
    Dictionary<IRInst*, PropagationInfo> propagationMap;

    // Mapping from function to return value propagation information
    Dictionary<IRFunc*, PropagationInfo> funcReturnInfo;

    // Mapping from functions to call-sites.
    Dictionary<IRFunc*, HashSet<IRCall*>> funcCallSites;

    // Unique ID assignment for functions and witness tables
    Dictionary<IRInst*, UInt> uniqueIds;
    UInt nextUniqueId = 1;

    // Mapping from lowered instruction to their any-value types
    Dictionary<IRInst*, IRAnyValueType*> loweredInstToAnyValueType;
};

// Main entry point
void lowerDynamicInsts(IRModule* module, DiagnosticSink* sink)
{
    DynamicInstLoweringContext context(module, sink);
    context.processModule();
}
} // namespace Slang
