#include "slang-ir-lower-dynamic-insts.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// Elements for which we keep track of propagation information.
struct Element
{
    IRInst* context;
    IRInst* inst;

    Element()
        : context(nullptr), inst(nullptr)
    {
    }

    Element(IRInst* context, IRInst* inst)
        : context(context), inst(inst)
    {
        validateElement();
    }

    bool validateElement() const
    {
        switch (context->getOp())
        {
        case kIROp_Func:
            {
                SLANG_ASSERT(inst->getParent()->getParent() == context);
                break;
            }
        case kIROp_Specialize:
            {
                auto generic = as<IRSpecialize>(context)->getBase();
                // The base should be a parent of the inst.
                bool foundParent = false;
                for (auto parent = inst->getParent(); parent; parent = parent->getParent())
                {
                    if (parent == generic)
                    {
                        foundParent = true;
                        break;
                    }
                }
                SLANG_ASSERT(foundParent);
            }
            break;
        default:
            {
                SLANG_UNEXPECTED("Invalid context for Element");
            }
        }
    }

    // Create element from an instruction that has a
    // concrete parent (i.e. global IRFunc)
    //
    Element(IRInst* inst)
        : inst(inst)
    {
        auto block = cast<IRBlock>(inst->getParent());
        auto func = cast<IRFunc>(block->getParent());

        // If parent func is not a global, then it is not a direct
        // reference. An explicit IRSpecialize instruction must be provided as
        // context.
        //
        SLANG_ASSERT(func->getParent()->getOp() == kIROp_ModuleInst);

        context = func;
    }

    Element(const Element& other)
        : context(other.context), inst(other.inst)
    {
    }

    bool operator==(const Element& other) const
    {
        return context == other.context && inst == other.inst;
    }

    // getHashCode()
    HashCode64 getHashCode() const { return combineHash(HashCode(context), HashCode(inst)); }
};
// Enumeration for different kinds of judgments about IR instructions.
//
// This forms a lattice with
//
// None < Set < Unbounded
// None < Existential < Unbounded
//
enum class PropagationJudgment
{
    None,        // Either uninitialized or irrelevant
    Set,         // Set of possible types/tables/funcs
    Existential, // Existential box with a set of possible witness tables
    Unbounded,   // Unknown set of possible types/tables/funcs (e.g. COM interface types)
};

// Data structure to hold propagation information for an instruction
struct PropagationInfo : RefObject
{
    PropagationJudgment judgment;

    // For sets of types/tables/funcs and existential witness tables
    // Instead of HashSet, we use an IRCollection instruction with sorted operands
    IRInst* collection;

    PropagationInfo()
        : judgment(PropagationJudgment::None), collection(nullptr)
    {
    }

    PropagationInfo(PropagationJudgment j)
        : judgment(j), collection(nullptr)
    {
    }

    PropagationInfo(PropagationJudgment j, IRInst* coll)
        : judgment(j), collection(coll)
    {
    }

    // NOTE: Factory methods moved to DynamicInstLoweringContext to access collection creation

    bool isNone() const { return judgment == PropagationJudgment::None; }
    bool isSingleton() const
    {
        return judgment == PropagationJudgment::Set && getCollectionCount() == 1;
    }

    IRInst* getSingletonValue() const
    {
        if (judgment == PropagationJudgment::Set && getCollectionCount() == 1)
            return getCollectionElement(0);

        SLANG_UNEXPECTED("getSingletonValue called on non-singleton PropagationInfo");
    }

    // Helper functions to access collection elements
    UInt getCollectionCount() const
    {
        if (!collection)
            return 0;
        return collection->getOperandCount();
    }

    IRInst* getCollectionElement(UInt index) const
    {
        if (!collection || index >= collection->getOperandCount())
            return nullptr;
        return collection->getOperand(index);
    }

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
    IRInst* callerContext; // The context of the call (e.g. function or specialized generic)
    IRCall* callInst;      // The call instruction
    IRInst* targetContext; // The function/specialized-generic being called/returned from

    InterproceduralEdge() = default;
    InterproceduralEdge(Direction dir, IRInst* callerContext, IRCall* call, IRInst* func)
        : direction(dir), callerContext(callerContext), callInst(call), targetContext(func)
    {
    }
};


// Union type representing either an intra-procedural or interprocedural edge
struct WorkItem
{
    enum class Type
    {
        None,      // Invalid
        Inst,      // Propagate through a single instruction
        Block,     // Propagate information within a block
        IntraProc, // Propagate through within-function edge (IREdge)
        InterProc  // Propagate across function call/return (InterproceduralEdge)
    };

    Type type;
    IRInst* context; // The context of the work item.
    union
    {
        IRInst* inst;                      // Type::Inst
        IRBlock* block;                    // Type::Block
        IREdge intraProcEdge;              // Type::IntraProc
        InterproceduralEdge interProcEdge; // Type::InterProc
    };

    WorkItem()
        : type(Type::None)
    {
    }

    WorkItem(IRInst* context, IRInst* inst)
        : type(Type::Inst), inst(inst), context(context)
    {
        SLANG_ASSERT(context != nullptr && inst != nullptr);
        // Validate that the context is appropriate for the instruction
        Element(context, inst).validateElement();
    }

    WorkItem(IRInst* context, IRBlock* block)
        : type(Type::Block), block(block), context(context)
    {
        SLANG_ASSERT(context != nullptr && block != nullptr);
        // Validate that the context is appropriate for the block
        Element(context, block->getFirstChild()).validateElement();
    }

    WorkItem(IRInst* context, IREdge edge)
        : type(Type::IntraProc), intraProcEdge(edge), context(context)
    {
        SLANG_ASSERT(context != nullptr);
    }

    WorkItem(InterproceduralEdge edge)
        : type(Type::InterProc), interProcEdge(edge), context(nullptr)
    {
    }

    WorkItem(InterproceduralEdge::Direction dir, IRInst* callerCtx, IRCall* call, IRInst* callee)
        : type(Type::InterProc), interProcEdge(dir, callerCtx, call, callee), context(nullptr)
    {
    }

    // Copy constructor and assignment needed for union with non-trivial types
    WorkItem(const WorkItem& other)
        : type(other.type), context(other.context)
    {
        if (type == Type::IntraProc)
            intraProcEdge = other.intraProcEdge;
        else if (type == Type::InterProc)
            interProcEdge = other.interProcEdge;
        else if (type == Type::Inst)
            inst = other.inst;
        else
            block = other.block;
    }

    WorkItem& operator=(const WorkItem& other)
    {
        type = other.type;
        context = other.context;
        if (type == Type::IntraProc)
            intraProcEdge = other.intraProcEdge;
        else if (type == Type::InterProc)
            interProcEdge = other.interProcEdge;
        else if (type == Type::Inst)
            inst = other.inst;
        else
            block = other.block;
        return *this;
    }
};

bool areInfosEqual(const PropagationInfo& a, const PropagationInfo& b)
{
    if (a.judgment != b.judgment)
        return false;

    switch (a.judgment)
    {
    case PropagationJudgment::None:
    case PropagationJudgment::Unbounded:
        return true;
    case PropagationJudgment::Set:
    case PropagationJudgment::Existential:
        return a.collection == b.collection;
    default:
        return false;
    }
}

struct DynamicInstLoweringContext
{
    // Helper methods for creating canonical collections
    IRInst* createCollection(const HashSet<IRInst*>& elements)
    {
        List<IRInst*> sortedElements;
        for (auto element : elements)
            sortedElements.add(element);

        return createCollection(sortedElements);
    }

    IRInst* createCollection(const List<IRInst*>& elements)
    {
        if (elements.getCount() == 0)
            return nullptr;

        // Verify that all operands are global instructions
        for (auto element : elements)
            if (element->getParent()->getOp() != kIROp_ModuleInst)
                SLANG_ASSERT_FAILURE("createCollection called with non-global operands");

        // Sort elements by their unique IDs to ensure canonical ordering
        List<IRInst*> sortedElements = elements;
        sortedElements.sort(
            [&](IRInst* a, IRInst* b) -> bool { return getUniqueID(a) < getUniqueID(b); });

        // Create the collection instruction
        IRBuilder builder(module);
        builder.setInsertInto(module);

        // Use makeTuple as a temporary implementation until IRCollection is available
        return builder.emitIntrinsicInst(
            nullptr,
            kIROp_TypeFlowCollection,
            sortedElements.getCount(),
            sortedElements.getBuffer());
    }

    // Factory methods for PropagationInfo
    PropagationInfo makeSingletonSet(IRInst* value)
    {
        HashSet<IRInst*> singleSet;
        singleSet.add(value);
        auto collection = createCollection(singleSet);
        return PropagationInfo(PropagationJudgment::Set, collection);
    }

    PropagationInfo makeSet(const HashSet<IRInst*>& values)
    {
        SLANG_ASSERT(values.getCount() > 0);
        auto collection = createCollection(values);
        return PropagationInfo(PropagationJudgment::Set, collection);
    }

    PropagationInfo makeExistential(const HashSet<IRInst*>& tables)
    {
        SLANG_ASSERT(tables.getCount() > 0);
        auto collection = createCollection(tables);
        return PropagationInfo(PropagationJudgment::Existential, collection);
    }

    PropagationInfo makeUnbounded()
    {
        return PropagationInfo(PropagationJudgment::Unbounded, nullptr);
    }

    PropagationInfo none() { return PropagationInfo(PropagationJudgment::None, nullptr); }

    // Helper to iterate over collection elements
    template<typename F>
    void forEachInCollection(const PropagationInfo& info, F func)
    {
        for (UInt i = 0; i < info.getCollectionCount(); ++i)
            func(info.getCollectionElement(i));
    }

    // Helper to convert collection to HashSet
    HashSet<IRInst*> collectionToHashSet(const PropagationInfo& info)
    {
        HashSet<IRInst*> result;
        forEachInCollection(info, [&](IRInst* element) { result.add(element); });
        return result;
    }

    /*PropagationInfo tryGetInfo(IRInst* inst)
    {
        // If this is a global instruction (parent is module), return concrete info
        if (as<IRModuleInst>(inst->getParent()))
            if (as<IRType>(inst) || as<IRWitnessTable>(inst) || as<IRFunc>(inst))
                return makeSingletonSet(inst);
            else
                return none();

        return tryGetInfo(Element(inst));
    }*/

    PropagationInfo tryGetInfo(Element element)
    {
        // For non-global instructions, look up in the map
        auto found = propagationMap.tryGetValue(element);
        if (found)
            return *found;
        return none();
    }

    PropagationInfo tryGetInfo(IRInst* context, IRInst* inst)
    {
        // If this is a global instruction (parent is module), return concrete info
        if (as<IRModuleInst>(inst->getParent()))
            if (as<IRType>(inst) || as<IRWitnessTable>(inst) || as<IRFunc>(inst))
                return makeSingletonSet(inst);
            else
                return none();

        return tryGetInfo(Element(context, inst));
    }

    PropagationInfo tryGetFuncReturnInfo(IRFunc* func)
    {
        auto found = funcReturnInfo.tryGetValue(func);
        if (found)
            return *found;
        return none();
    }

    // Centralized method to update propagation info and manage work queue
    // Use this when you want to propagate new information to an existing instruction
    // This will union the new info with existing info and add users to work queue if changed
    void updateInfo(
        IRInst* context,
        IRInst* inst,
        PropagationInfo newInfo,
        LinkedList<WorkItem>& workQueue)
    {
        auto existingInfo = tryGetInfo(context, inst);
        auto unionedInfo = unionPropagationInfo(existingInfo, newInfo);

        // Only proceed if info actually changed
        if (areInfosEqual(existingInfo, unionedInfo))
            return;

        // Update the propagation map
        propagationMap[Element(context, inst)] = unionedInfo;

        // Add all users to appropriate work items
        addUsersToWorkQueue(context, inst, unionedInfo, workQueue);
    }

    // Helper to add users of an instruction to the work queue based on how they use it
    // This handles intra-procedural edges, inter-procedural edges, and return value propagation
    void addUsersToWorkQueue(
        IRInst* context,
        IRInst* inst,
        PropagationInfo info,
        LinkedList<WorkItem>& workQueue)
    {
        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();

            // If user is in a different block (or the inst is a param), add that block to work
            // queue.
            //
            workQueue.addLast(WorkItem(context, user));

            // If user is a terminator, add intra-procedural edges
            if (auto terminator = as<IRTerminatorInst>(user))
            {
                auto parentBlock = as<IRBlock>(terminator->getParent());
                if (parentBlock)
                {
                    auto successors = parentBlock->getSuccessors();
                    for (auto succIter = successors.begin(); succIter != successors.end();
                         ++succIter)
                    {
                        workQueue.addLast(WorkItem(context, succIter.getEdge()));
                    }
                }
            }

            // If user is a return instruction, handle function return propagation
            if (auto returnInst = as<IRReturn>(user))
            {
                updateFuncReturnInfo(context, info, workQueue);
            }
        }
    }

    // Helper method to update function return info and propagate to call sites
    void updateFuncReturnInfo(
        IRInst* callable,
        PropagationInfo returnInfo,
        LinkedList<WorkItem>& workQueue)
    {
        auto existingReturnInfo = getFuncReturnInfo(callable);
        auto newReturnInfo = unionPropagationInfo(existingReturnInfo, returnInfo);

        if (!areInfosEqual(existingReturnInfo, newReturnInfo))
        {
            funcReturnInfo[callable] = newReturnInfo;

            // Add interprocedural edges to all call sites
            if (funcCallSites.containsKey(callable))
            {
                for (auto callSite : funcCallSites[callable])
                {
                    workQueue.addLast(WorkItem(
                        InterproceduralEdge::Direction::FuncToCall,
                        callSite.context,
                        as<IRCall>(callSite.inst),
                        callable));
                }
            }
        }
    }

    void processBlock(IRInst* context, IRBlock* block, LinkedList<WorkItem>& workQueue)
    {
        for (auto inst : block->getChildren())
        {
            // Skip parameters & terminator
            if (as<IRParam>(inst) || as<IRTerminatorInst>(inst))
                continue;
            processInstForPropagation(context, inst, workQueue);
        }

        if (auto returnInfo = as<IRReturn>(block->getTerminator()))
        {
            auto valInfo = returnInfo->getVal();
            updateFuncReturnInfo(context, tryGetInfo(context, valInfo), workQueue);
        }
    };

    void performInformationPropagation()
    {
        // Global worklist for interprocedural analysis
        LinkedList<WorkItem> workQueue;

        // Add all global functions to worklist
        for (auto inst : module->getGlobalInsts())
            if (auto func = as<IRFunc>(inst))
                discoverContext(func, workQueue);

        // Process until fixed point
        while (workQueue.getCount() > 0)
        {
            // Pop work item from front
            auto item = workQueue.getFirst();
            workQueue.getFirstNode()->removeAndDelete();

            switch (item.type)
            {
            case WorkItem::Type::Inst:
                processInstForPropagation(item.context, item.inst, workQueue);
                break;
            case WorkItem::Type::Block:
                processBlock(item.context, item.block, workQueue);
                break;
            case WorkItem::Type::IntraProc:
                propagateWithinFuncEdge(item.context, item.intraProcEdge, workQueue);
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

    IRInst* maybeReinterpret(IRInst* context, IRInst* arg, PropagationInfo destInfo)
    {
        auto argInfo = tryGetInfo(context, arg);

        if (!argInfo || !destInfo)
            return arg;

        if (argInfo.judgment == PropagationJudgment::Existential &&
            destInfo.judgment == PropagationJudgment::Existential)
        {
            if (argInfo.getCollectionCount() != destInfo.getCollectionCount())
            {
                // If the sets of witness tables are not equal, reinterpret to the parameter type
                IRBuilder builder(module);
                builder.setInsertAfter(arg);

                // We'll use nulltype for the reinterpret since the type is going to be re-written
                // and if it doesn't, this will help catch it before code-gen.
                //
                auto reinterpret = builder.emitReinterpret(nullptr, arg);
                propagationMap[Element(reinterpret)] = destInfo;
                return reinterpret; // Return the reinterpret instruction
            }
        }

        return arg; // Can use as-is.
    }

    bool insertReinterprets()
    {
        bool changed = false;
        // Process each function in the module
        for (auto inst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
            {
                auto context = func;
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
                                    auto newArg = maybeReinterpret(context, arg, tryGetInfo(param));

                                    if (newArg != arg)
                                    {
                                        changed = true;
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
                                maybeReinterpret(context, returnInst->getVal(), funcReturnInfo);
                            if (newReturnVal != returnInst->getVal())
                            {
                                // Replace the return value with the reinterpreted value
                                changed = true;
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
                                auto newArg = maybeReinterpret(
                                    context,
                                    callInst->getArg(i),
                                    tryGetInfo(param));
                                if (newArg != callInst->getArg(i))
                                {
                                    // Replace the argument in the call instruction
                                    changed = true;
                                    callInst->setArg(i, newArg);
                                }
                                i++;
                            }
                        }
                    }
                }
            }
        }

        return changed;
    }

    void processInstForPropagation(IRInst* context, IRInst* inst, LinkedList<WorkItem>& workQueue)
    {
        PropagationInfo info;

        switch (inst->getOp())
        {
        case kIROp_CreateExistentialObject:
            info = analyzeCreateExistentialObject(context, as<IRCreateExistentialObject>(inst));
            break;
        case kIROp_MakeExistential:
            info = analyzeMakeExistential(context, as<IRMakeExistential>(inst));
            break;
        case kIROp_LookupWitnessMethod:
            info = analyzeLookupWitnessMethod(context, as<IRLookupWitnessMethod>(inst));
            break;
        case kIROp_ExtractExistentialWitnessTable:
            info = analyzeExtractExistentialWitnessTable(
                context,
                as<IRExtractExistentialWitnessTable>(inst));
            break;
        case kIROp_ExtractExistentialType:
            info = analyzeExtractExistentialType(context, as<IRExtractExistentialType>(inst));
            break;
        case kIROp_ExtractExistentialValue:
            info = analyzeExtractExistentialValue(context, as<IRExtractExistentialValue>(inst));
            break;
        case kIROp_Call:
            info = analyzeCall(context, as<IRCall>(inst), workQueue);
            break;
        case kIROp_Specialize:
            info = analyzeSpecialize(context, as<IRSpecialize>(inst));
            break;
        default:
            info = analyzeDefault(context, inst);
            break;
        }

        updateInfo(context, inst, info, workQueue);
    }

    PropagationInfo analyzeCreateExistentialObject(IRInst* context, IRCreateExistentialObject* inst)
    {
        //
        // TODO: Actually use the integer<->type map present in the linkage to
        // extract a set of possible witness tables (if the index is a compile-time constant).
        //

        if (auto interfaceType = as<IRInterfaceType>(inst->getDataType()))
        {
            if (!interfaceType->findDecoration<IRComInterfaceDecoration>())
            {
                auto tables = collectExistentialTables(interfaceType);
                if (tables.getCount() > 0)
                    return makeExistential(tables);
                else
                    return none();
            }
            else
            {
                return makeUnbounded();
            }
        }

        return none();
    }

    PropagationInfo analyzeMakeExistential(IRInst* context, IRMakeExistential* inst)
    {
        auto witnessTable = inst->getWitnessTable();
        auto value = inst->getWrappedValue();
        auto valueType = value->getDataType();

        // Get the witness table info
        auto witnessTableInfo = tryGetInfo(context, witnessTable);

        if (!witnessTableInfo)
            return none();

        if (witnessTableInfo.judgment == PropagationJudgment::Unbounded)
            return makeUnbounded();

        HashSet<IRInst*> tables;
        if (witnessTableInfo.judgment == PropagationJudgment::Set)
            forEachInCollection(witnessTableInfo, [&](IRInst* table) { tables.add(table); });

        return makeExistential(tables);
    }

    static IRInst* findEntryInConcreteTable(IRInst* witnessTable, IRInst* key)
    {
        if (auto concreteTable = as<IRWitnessTable>(witnessTable))
            for (auto entry : concreteTable->getEntries())
                if (entry->getRequirementKey() == key)
                    return entry->getSatisfyingVal();
        return nullptr; // Not found
    }

    PropagationInfo analyzeLookupWitnessMethod(IRInst* context, IRLookupWitnessMethod* inst)
    {
        auto key = inst->getRequirementKey();

        auto witnessTable = inst->getWitnessTable();
        auto witnessTableInfo = tryGetInfo(context, witnessTable);

        switch (witnessTableInfo.judgment)
        {
        case PropagationJudgment::None:
        case PropagationJudgment::Unbounded:
            return witnessTableInfo.judgment;
        case PropagationJudgment::Set:
            {
                HashSet<IRInst*> results;
                forEachInCollection(
                    witnessTableInfo,
                    [&](IRInst* table) { results.add(findEntryInConcreteTable(table, key)); });
                return makeSet(results);
            }
        case PropagationJudgment::Existential:
            SLANG_UNEXPECTED("Unexpected LookupWitnessMethod on Existential");
            break;
        default:
            SLANG_UNEXPECTED("Unhandled PropagationJudgment in analyzeLookupWitnessMethod");
            break;
        }
    }

    PropagationInfo analyzeExtractExistentialWitnessTable(
        IRInst* context,
        IRExtractExistentialWitnessTable* inst)
    {
        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(context, operand);

        switch (operandInfo.judgment)
        {
        case PropagationJudgment::None:
            return none();
        case PropagationJudgment::Unbounded:
            return makeUnbounded();
        case PropagationJudgment::Existential:
            {
                // Convert collection to HashSet and create Set PropagationInfo
                HashSet<IRInst*> tables;
                forEachInCollection(operandInfo, [&](IRInst* table) { tables.add(table); });
                return makeSet(tables);
            }
        case PropagationJudgment::Set:
            SLANG_UNEXPECTED(
                "Unexpected ExtractExistentialWitnessTable on Set (should be Existential)");
            break;
        default:
            SLANG_UNEXPECTED(
                "Unhandled PropagationJudgment in analyzeExtractExistentialWitnessTable");
            break;
        }
    }

    PropagationInfo analyzeExtractExistentialType(IRInst* context, IRExtractExistentialType* inst)
    {
        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(context, operand);

        switch (operandInfo.judgment)
        {
        case PropagationJudgment::None:
            return none();
        case PropagationJudgment::Unbounded:
            return makeUnbounded();
        case PropagationJudgment::Existential:
            {
                HashSet<IRInst*> types;
                forEachInCollection(
                    operandInfo,
                    [&](IRInst* table)
                    {
                        if (auto witnessTable = cast<IRWitnessTable>(table)) // Expect witness table
                            if (auto concreteType = witnessTable->getConcreteType())
                                types.add(concreteType);
                    });
                return makeSet(types);
            }
        case PropagationJudgment::Set:
            SLANG_UNEXPECTED(
                "Unexpected ExtractExistentialWitnessTable on Set (should be Existential)");
            break;
        default:
            SLANG_UNEXPECTED("Unhandled PropagationJudgment in analyzeExtractExistentialType");
            break;
        }
    }

    PropagationInfo analyzeExtractExistentialValue(IRInst* context, IRExtractExistentialValue* inst)
    {
        // We don't care about the value itself.
        // (We rely on the propagation info for the type)
        //
        return none();
    }


    PropagationInfo analyzeSpecialize(IRInst* context, IRSpecialize* inst)
    {
        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(context, operand);

        switch (operandInfo.judgment)
        {
        case PropagationJudgment::None:
            return none();
        case PropagationJudgment::Unbounded:
            return makeUnbounded();
        case PropagationJudgment::Existential:
            {
                SLANG_UNEXPECTED(
                    "Unexpected ExtractExistentialWitnessTable on Set (should be Existential)");
            }
        case PropagationJudgment::Set:
            {
                List<IRInst*> specializationArgs;
                for (auto i = 0; i < inst->getArgCount(); ++i)
                {
                    // For integer args, add as is (also applies to any value args)
                    if (as<IRIntLit>(inst->getArg(i)))
                    {
                        specializationArgs.add(inst->getArg(i));
                        continue;
                    }

                    // For type args, we need to replace any dynamic args with
                    // their sets.
                    //
                    auto argInfo = tryGetInfo(context, inst->getArg(i));
                    switch (argInfo.judgment)
                    {
                    case PropagationJudgment::None:
                    case PropagationJudgment::Unbounded:
                        SLANG_UNEXPECTED(
                            "Unexpected PropagationJudgment for specialization argument");
                    case PropagationJudgment::Existential:
                        SLANG_UNEXPECTED(
                            "Unexpected Existential operand in specialization argument. Should be "
                            "set");
                    case PropagationJudgment::Set:
                        {
                            if (argInfo.getCollectionCount() == 1)
                                specializationArgs.add(argInfo.getSingletonValue());
                            else
                                specializationArgs.add(argInfo.collection);
                            break;
                        }
                    default:
                        SLANG_UNEXPECTED("Unhandled PropagationJudgment in analyzeSpecialize");
                        break;
                    }
                }

                IRType* typeOfSpecialization = nullptr;
                if (inst->getDataType()->getParent()->getOp() == kIROp_ModuleInst)
                    typeOfSpecialization = inst->getDataType();
                else if (auto funcType = as<IRFuncType>(inst->getDataType()))
                {
                    auto substituteSets = [&](IRInst* type) -> IRInst*
                    {
                        if (auto info = tryGetInfo(context, type))
                        {
                            if (info.judgment == PropagationJudgment::Set)
                            {
                                if (info.getCollectionCount() == 1)
                                    return info.getSingletonValue();
                                else
                                    return info.collection;
                            }
                            else
                                return type;
                        }
                        else
                            return type;
                    };

                    List<IRType*> newParamTypes;
                    for (auto paramType : funcType->getParamTypes())
                        newParamTypes.add((IRType*)substituteSets(paramType));
                    IRBuilder builder(module);
                    builder.setInsertInto(module);
                    typeOfSpecialization = builder.getFuncType(
                        newParamTypes.getCount(),
                        newParamTypes.getBuffer(),
                        (IRType*)substituteSets(funcType->getResultType()));
                }
                else
                {
                    SLANG_ASSERT_FAILURE("Unexpected data type for specialization instruction");
                }

                // Specialize each element in the set
                HashSet<IRInst*> specializedSet;
                forEachInCollection(
                    operandInfo,
                    [&](IRInst* arg)
                    {
                        // Create a new specialized instruction for each argument
                        IRBuilder builder(module);
                        builder.setInsertInto(module);
                        specializedSet.add(builder.emitSpecializeInst(
                            typeOfSpecialization,
                            arg,
                            specializationArgs));
                    });
                return makeSet(specializedSet);
            }
            break;
        default:
            SLANG_UNEXPECTED(
                "Unhandled PropagationJudgment in analyzeExtractExistentialWitnessTable");
            break;
        }
    }

    void discoverContext(IRInst* context, LinkedList<WorkItem>& workQueue)
    {
        if (this->availableContexts.add(context))
        {
            IRFunc* func = nullptr;

            // Newly discovered context. Initialize it.
            switch (context->getOp())
            {
            case kIROp_Func:
                {
                    func = cast<IRFunc>(context);

                    // Initialize the first block parameters
                    initializeFirstBlockParameters(context, func);

                    // Add all blocks to the work queue
                    for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
                        workQueue.addLast(WorkItem(context, block));
                    break;
                }
            case kIROp_Specialize:
                {
                    auto specialize = cast<IRSpecialize>(context);
                    auto generic = cast<IRGeneric>(specialize->getBase());
                    func = cast<IRFunc>(findGenericReturnVal(generic));

                    // Transfer information from specialization arguments to the params in the
                    // first generic block.
                    //
                    IRParam* param = generic->getFirstBlock()->getFirstParam();
                    for (auto index = 0; index < specialize->getArgCount() && param;
                         ++index, param = param->getNextParam())
                    {
                        // Map the specialization argument to the corresponding parameter
                        auto arg = specialize->getArg(index);
                        if (as<IRIntLit>(arg))
                            continue;

                        if (auto collection = as<IRTypeFlowCollection>(arg))
                        {
                            updateInfo(
                                context,
                                param,
                                PropagationInfo(PropagationJudgment::Set, collection),
                                workQueue);
                        }
                        else if (as<IRType>(arg) || as<IRWitnessTable>(arg))
                        {
                            updateInfo(context, param, makeSingletonSet(arg), workQueue);
                        }
                        else
                        {
                            SLANG_UNEXPECTED("Unexpected argument type in specialization");
                        }
                    }

                    // Initialize the first block parameters
                    initializeFirstBlockParameters(context, func);

                    // Add all blocks to the work queue for an initial sweep
                    for (auto block = generic->getFirstBlock(); block;
                         block = block->getNextBlock())
                        workQueue.addLast(WorkItem(context, block));

                    for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
                        workQueue.addLast(WorkItem(context, block));
                }
            }
        }
    }

    PropagationInfo analyzeCall(IRInst* context, IRCall* inst, LinkedList<WorkItem>& workQueue)
    {
        auto callee = inst->getCallee();
        auto calleeInfo = tryGetInfo(context, callee);

        auto funcType = as<IRFuncType>(callee->getDataType());

        //
        // Propagate the input judgments to the call & append a work item
        // for inter-procedural propagation.
        //

        // For now, we'll handle just a concrete func. But the logic for multiple functions
        // is exactly the same (add an edge for each function).
        //
        auto propagateToCallSite = [&](IRInst* callee)
        {
            // Register the call site in the map to allow for the
            // return-edge to be created.
            //
            // We use an explicit map instead of walking the uses of the
            // func, since we might have functions that are called indirectly
            // through lookups.
            //
            discoverContext(callee, workQueue);

            this->funcCallSites.addIfNotExists(callee, HashSet<Element>());
            if (this->funcCallSites[callee].add(inst))
            {
                // If this is a new call site, add a propagation task to the queue (in case there's
                // already information about this function)
                workQueue.addLast(
                    WorkItem(InterproceduralEdge::Direction::FuncToCall, context, inst, callee));
            }
            workQueue.addLast(
                WorkItem(InterproceduralEdge::Direction::CallToFunc, context, inst, callee));
        };

        if (calleeInfo.judgment == PropagationJudgment::Set)
        {
            // If we have a set of functions, register each one
            forEachInCollection(calleeInfo, [&](IRInst* func) { propagateToCallSite(func); });
        }

        if (auto callInfo = tryGetInfo(context, inst))
            return callInfo;
        else
            return none();
    }

    void propagateWithinFuncEdge(IRInst* context, IREdge edge, LinkedList<WorkItem>& workQueue)
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
        Index paramIndex = 0;
        for (auto param : successorBlock->getParams())
        {
            if (paramIndex < unconditionalBranch->getArgCount())
            {
                auto arg = unconditionalBranch->getArg(paramIndex);
                if (auto argInfo = tryGetInfo(context, arg))
                {
                    // Use centralized update method
                    updateInfo(context, param, argInfo, workQueue);
                }
            }
            paramIndex++;
        }
    }

    void propagateInterproceduralEdge(InterproceduralEdge edge, LinkedList<WorkItem>& workQueue)
    {
        // Handle interprocedural edge
        auto callInst = edge.callInst;
        auto targetCallee = edge.targetContext;

        switch (edge.direction)
        {
        case InterproceduralEdge::Direction::CallToFunc:
            {
                // Propagate argument info from call site to function parameters
                auto firstBlock = targetCallee->getFirstBlock();
                if (!firstBlock)
                    return;

                Index argIndex = 1; // Skip callee (operand 0)
                for (auto param : firstBlock->getParams())
                {
                    if (argIndex < callInst->getOperandCount())
                    {
                        auto arg = callInst->getOperand(argIndex);
                        if (auto argInfo = tryGetInfo(edge.callerContext, arg))
                        {
                            // Use centralized update method
                            updateInfo(edge.targetContext, param, argInfo, workQueue);
                        }
                    }
                    argIndex++;
                }
                break;
            }
        case InterproceduralEdge::Direction::FuncToCall:
            {
                // Propagate return value info from function to call site
                auto returnInfo = funcReturnInfo.tryGetValue(targetCallee);
                if (returnInfo)
                {
                    // Use centralized update method
                    updateInfo(edge.callerContext, callInst, *returnInfo, workQueue);
                }
                break;
            }
        default:
            SLANG_UNEXPECTED("Unhandled interprocedural edge direction");
            return;
        }
    }

    PropagationInfo getFuncReturnInfo(IRInst* callee)
    {
        funcReturnInfo.addIfNotExists(callee, none());
        return funcReturnInfo[callee];
    }

    void initializeFirstBlockParameters(IRInst* context, IRFunc* func)
    {
        auto firstBlock = func->getFirstBlock();
        if (!firstBlock)
            return;

        // Initialize parameters based on their types
        for (auto param : firstBlock->getParams())
        {
            auto paramType = param->getDataType();
            auto paramInfo = tryGetInfo(context, param);
            if (paramInfo)
                continue; // Already has some information

            if (auto interfaceType = as<IRInterfaceType>(paramType))
            {
                if (interfaceType->findDecoration<IRComInterfaceDecoration>())
                    propagationMap[Element(context, param)] = makeUnbounded();
                else
                    propagationMap[Element(context, param)] = none(); // Initialize to none.
            }
            else
            {
                propagationMap[Element(context, param)] = none();
            }
        }
    }

    PropagationInfo unionPropagationInfo(const List<PropagationInfo>& infos)
    {
        if (infos.getCount() == 0)
        {
            return none();
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
            case PropagationJudgment::None:
                break;
            case PropagationJudgment::Set:
                unionJudgment = PropagationJudgment::Set;
                forEachInCollection(info, [&](IRInst* value) { allValues.add(value); });
                break;
            case PropagationJudgment::Existential:
                // For existential union, we need to collect all witness tables
                // For now, we'll handle this properly by creating a new existential with all tables
                unionJudgment = PropagationJudgment::Existential;
                forEachInCollection(info, [&](IRInst* value) { allValues.add(value); });
                break;
            case PropagationJudgment::Unbounded:
                // If any info is unbounded, the union is unbounded
                return makeUnbounded();
            }
        }

        if (unionJudgment == PropagationJudgment::Existential)
            if (allValues.getCount() > 0)
                return makeExistential(allValues);
            else
                return none();

        if (unionJudgment == PropagationJudgment::Set)
            if (allValues.getCount() > 0)
                return makeSet(allValues);
            else
                return none();

        // If we reach here, crash instead of returning none (which could make the analysis go into
        // an infinite loop)
        //
        SLANG_UNEXPECTED("Unhandled prop-info union");
    }

    PropagationInfo unionPropagationInfo(PropagationInfo info1, PropagationInfo info2)
    {
        // Union the two infos
        List<PropagationInfo> infos;
        infos.add(info1);
        infos.add(info2);
        return unionPropagationInfo(infos);
    }

    PropagationInfo analyzeDefault(IRInst* context, IRInst* inst)
    {
        // Check if this is a global type, witness table, or function.
        // If so, it's a concrete element. We'll create a singleton set for it.
        if (inst->getParent()->getOp() == kIROp_ModuleInst &&
            (as<IRType>(inst) || as<IRWitnessTable>(inst) || as<IRFunc>(inst)))
            return makeSingletonSet(inst);
        else
            return none(); // Default case, no propagation info
    }

    bool performDynamicInstLowering()
    {
        // Collect all instructions that need lowering
        List<Element> typeInstsToLower;
        List<Element> valueInstsToLower;
        List<Element> instWithReplacementTypes;
        List<IRFunc*> funcTypesToProcess;

        bool hasChanges = false;
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(globalInst))
            {
                auto context = func;
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
                                    typeInstsToLower.add(Element(context, child));
                                else
                                    valueInstsToLower.add(Element(context, child));
                                break;
                            }
                        case kIROp_ExtractExistentialType:
                            typeInstsToLower.add(Element(context, child));
                            break;
                        case kIROp_ExtractExistentialWitnessTable:
                        case kIROp_ExtractExistentialValue:
                        case kIROp_MakeExistential:
                        case kIROp_CreateExistentialObject:
                            valueInstsToLower.add(Element(context, child));
                            break;
                        case kIROp_Call:
                            {
                                if (auto info = tryGetInfo(context, child))
                                    if (info.judgment == PropagationJudgment::Existential)
                                        instWithReplacementTypes.add(Element(context, child));

                                if (auto calleeInfo =
                                        tryGetInfo(context, as<IRCall>(child)->getCallee()))
                                    if (calleeInfo.judgment == PropagationJudgment::Set)
                                        valueInstsToLower.add(Element(context, child));
                            }
                            break;
                        default:
                            if (auto info = tryGetInfo(context, child))
                                if (info.judgment == PropagationJudgment::Existential)
                                    // If this instruction has a set of types, tables, or funcs,
                                    // we need to lower it to a unified type.
                                    instWithReplacementTypes.add(Element(context, child));
                        }
                    }
                }

                funcTypesToProcess.add(func);
            }
        }

        for (auto instWithCtx : typeInstsToLower)
            hasChanges |= lowerInst(instWithCtx.context, instWithCtx.inst);

        for (auto func : funcTypesToProcess)
            hasChanges |= replaceFuncType(func, this->funcReturnInfo[func]);

        for (auto instWithCtx : valueInstsToLower)
            hasChanges |= lowerInst(instWithCtx.context, instWithCtx.inst);

        for (auto instWithCtx : instWithReplacementTypes)
        {
            if (instWithCtx.inst->getParent() == nullptr)
                continue;
            hasChanges |= replaceType(instWithCtx.context, instWithCtx.inst);
        }

        return hasChanges;
    }

    bool replaceFuncType(IRFunc* func, PropagationInfo& returnTypeInfo)
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

        auto newFuncType = builder.getFuncType(paramTypes, returnType);
        if (newFuncType == func->getFullType())
            return false; // No change

        func->setFullType(newFuncType);
        return true;
    }

    IRType* getTypeForExistential(PropagationInfo info)
    {
        // Replace type with Tuple<UInt, AnyValueType>
        IRBuilder builder(module);
        builder.setInsertInto(module);

        HashSet<IRInst*> types;
        forEachInCollection(
            info,
            [&](IRInst* table)
            {
                if (auto witnessTable = as<IRWitnessTable>(table))
                    if (auto concreteType = witnessTable->getConcreteType())
                        types.add(concreteType);
            });

        SLANG_ASSERT(types.getCount() > 0);
        auto unionType = types.getCount() > 1 ? createAnyValueTypeFromInsts(types) : *types.begin();
        return builder.getTupleType(List<IRType*>({builder.getUIntType(), (IRType*)unionType}));
    }

    bool replaceType(IRInst* context, IRInst* inst)
    {
        auto info = tryGetInfo(context, inst);
        if (!info || info.judgment != PropagationJudgment::Existential)
            return false;

        inst->setFullType(getTypeForExistential(info));
        return true;
    }

    bool lowerInst(IRInst* context, IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_LookupWitnessMethod:
            return lowerLookupWitnessMethod(context, as<IRLookupWitnessMethod>(inst));
        case kIROp_ExtractExistentialWitnessTable:
            return lowerExtractExistentialWitnessTable(
                context,
                as<IRExtractExistentialWitnessTable>(inst));
        case kIROp_ExtractExistentialType:
            return lowerExtractExistentialType(context, as<IRExtractExistentialType>(inst));
        case kIROp_ExtractExistentialValue:
            return lowerExtractExistentialValue(context, as<IRExtractExistentialValue>(inst));
        case kIROp_Call:
            return lowerCall(context, as<IRCall>(inst));
        case kIROp_MakeExistential:
            return lowerMakeExistential(context, as<IRMakeExistential>(inst));
        case kIROp_CreateExistentialObject:
            return lowerCreateExistentialObject(context, as<IRCreateExistentialObject>(inst));
        default:
            return false;
        }
    }

    bool lowerLookupWitnessMethod(IRInst* context, IRLookupWitnessMethod* inst)
    {
        auto info = tryGetInfo(context, inst);
        if (!info)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        if (info.isSingleton())
        {
            // Found a single possible type. Simple replacement.
            inst->replaceUsesWith(info.getSingletonValue());
            inst->removeAndDeallocate();
            return true;
        }
        else if (info.judgment == PropagationJudgment::Set)
        {
            // Set of types.
            if (inst->getDataType()->getOp() == kIROp_TypeKind)
            {
                // Create an any-value type based on the set of types
                auto typeSet = collectionToHashSet(info);
                auto unionType = typeSet.getCount() > 1 ? createAnyValueTypeFromInsts(typeSet)
                                                        : *typeSet.begin();

                // Store the mapping for later use
                loweredInstToAnyValueType[inst] = unionType;

                // Replace the instruction with the any-value type
                inst->replaceUsesWith(unionType);
                inst->removeAndDeallocate();
                return true;
            }
            else
            {
                // Get the witness table operand info
                auto witnessTableInst = inst->getWitnessTable();
                auto witnessTableInfo = tryGetInfo(context, witnessTableInst);

                if (witnessTableInfo.judgment == PropagationJudgment::Set)
                {
                    // Create a key mapping function
                    auto keyMappingFunc = createKeyMappingFunc(
                        inst->getRequirementKey(),
                        collectionToHashSet(witnessTableInfo),
                        collectionToHashSet(info));

                    // Replace with call to key mapping function
                    auto witnessTableId = builder.emitCallInst(
                        builder.getUIntType(),
                        keyMappingFunc,
                        List<IRInst*>({inst->getWitnessTable()}));
                    inst->replaceUsesWith(witnessTableId);
                    propagationMap[Element(witnessTableId)] = info;
                    inst->removeAndDeallocate();
                    return true;
                }
            }
        }

        return false;
    }

    bool lowerExtractExistentialWitnessTable(
        IRInst* context,
        IRExtractExistentialWitnessTable* inst)
    {
        auto info = tryGetInfo(context, inst);
        if (!info)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        if (info.isSingleton())
        {
            // Found a single possible type. Simple replacement.
            inst->replaceUsesWith(info.getSingletonValue());
            inst->removeAndDeallocate();
            return true;
        }
        else if (info.judgment == PropagationJudgment::Set)
        {
            // Replace with GetElement(loweredInst, 0) -> uint
            auto operand = inst->getOperand(0);
            auto element = builder.emitGetTupleElement(builder.getUIntType(), operand, 0);
            inst->replaceUsesWith(element);
            propagationMap[Element(element)] = info;
            inst->removeAndDeallocate();
            return true;
        }
        return false;
    }

    bool lowerExtractExistentialValue(IRInst* context, IRExtractExistentialValue* inst)
    {
        auto operandInfo = tryGetInfo(context, inst->getOperand(0));
        if (!operandInfo || operandInfo.judgment != PropagationJudgment::Existential)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Check if we have a lowered any-value type for the result
        auto resultType = inst->getDataType();
        auto loweredType = loweredInstToAnyValueType.tryGetValue(inst);
        if (loweredType)
        {
            resultType = (IRType*)*loweredType;
        }

        // Replace with GetElement(loweredInst, 1) -> AnyValueType
        auto operand = inst->getOperand(0);
        auto element = builder.emitGetTupleElement(resultType, operand, 1);
        inst->replaceUsesWith(element);
        inst->removeAndDeallocate();
        return true;
    }

    bool lowerExtractExistentialType(IRInst* context, IRExtractExistentialType* inst)
    {
        auto info = tryGetInfo(context, inst);
        if (!info || info.judgment != PropagationJudgment::Set)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        if (info.isSingleton())
        {
            // Found a single possible type. Simple replacement.
            inst->replaceUsesWith(info.getSingletonValue());
            inst->removeAndDeallocate();
            loweredInstToAnyValueType[inst] = info.getSingletonValue();
            return true;
        }

        // Create an any-value type based on the set of types
        auto anyValueType = createAnyValueTypeFromInsts(collectionToHashSet(info));

        // Store the mapping for later use
        loweredInstToAnyValueType[inst] = anyValueType;

        // Replace the instruction with the any-value type
        inst->replaceUsesWith(anyValueType);
        inst->removeAndDeallocate();
        return true;
    }

    IRFuncType* getExpectedFuncType(IRInst* context, IRCall* inst)
    {
        // Translate argument types into expected function type.
        // For now, we handle just 'in' arguments.
        List<IRType*> argTypes;
        for (UInt i = 0; i < inst->getArgCount(); i++)
        {
            auto arg = inst->getArg(i);
            if (auto argInfo = tryGetInfo(context, arg))
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
        auto returnInfo = tryGetInfo(context, inst);
        if (returnInfo && returnInfo.judgment == PropagationJudgment::Existential)
        {
            resultType = getTypeForExistential(returnInfo);
        }

        IRBuilder builder(module);
        builder.setInsertInto(module);
        return builder.getFuncType(argTypes, resultType);
    }

    bool lowerCall(IRInst* context, IRCall* inst)
    {
        auto callee = inst->getCallee();
        auto calleeInfo = tryGetInfo(context, callee);

        if (!calleeInfo || calleeInfo.judgment != PropagationJudgment::Set)
            return false;

        if (calleeInfo.isSingleton())
        {
            if (calleeInfo.getSingletonValue() == callee)
                return false;

            IRBuilder builder(inst->getModule());
            builder.replaceOperand(inst->getCalleeUse(), calleeInfo.getSingletonValue());
            return true; // Replaced with a single function
        }

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        auto expectedFuncType = getExpectedFuncType(context, inst);
        // Create dispatch function
        auto dispatchFunc = createDispatchFunc(collectionToHashSet(calleeInfo), expectedFuncType);

        // Replace call with dispatch
        List<IRInst*> newArgs;
        newArgs.add(callee); // Add the lookup as first argument (will get lowered into an uint tag)
        for (UInt i = 1; i < inst->getOperandCount(); i++)
        {
            newArgs.add(inst->getOperand(i));
        }

        auto newCall = builder.emitCallInst(inst->getDataType(), dispatchFunc, newArgs);
        inst->replaceUsesWith(newCall);
        if (auto info = tryGetInfo(context, inst))
            propagationMap[Element(newCall)] = info;
        replaceType(context, newCall); // "maybe replace type"
        inst->removeAndDeallocate();
        return true;
    }

    bool lowerMakeExistential(IRInst* context, IRMakeExistential* inst)
    {
        auto info = tryGetInfo(context, inst);
        if (!info || info.judgment != PropagationJudgment::Existential)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Get unique ID for the witness table.
        auto witnessTable = cast<IRWitnessTable>(inst->getWitnessTable());
        auto tableId = builder.getIntValue(builder.getUIntType(), getUniqueID(witnessTable));

        // Collect types from the witness tables to determine the any-value type
        HashSet<IRType*> types;
        forEachInCollection(
            info,
            [&](IRInst* table)
            {
                if (auto witnessTableInst = as<IRWitnessTable>(table))
                {
                    if (auto concreteType = witnessTableInst->getConcreteType())
                    {
                        types.add(concreteType);
                    }
                }
            });

        // Create the appropriate any-value type
        SLANG_ASSERT(types.getCount() > 0);
        auto unionType = types.getCount() > 1 ? createAnyValueType(types) : *types.begin();

        // Pack the value
        auto packedValue = builder.emitPackAnyValue(unionType, inst->getWrappedValue());

        // Create tuple (table_unique_id, PackAnyValue(val))
        auto tupleType = builder.getTupleType(
            List<IRType*>({builder.getUIntType(), packedValue->getDataType()}));
        IRInst* tupleArgs[] = {tableId, packedValue};
        auto tuple = builder.emitMakeTuple(tupleType, 2, tupleArgs);

        if (auto info = tryGetInfo(context, inst))
            propagationMap[Element(tuple)] = info;

        inst->replaceUsesWith(tuple);
        inst->removeAndDeallocate();
        return true;
    }

    bool lowerCreateExistentialObject(IRInst* context, IRCreateExistentialObject* inst)
    {
        auto info = tryGetInfo(context, inst);
        if (!info || info.judgment != PropagationJudgment::Existential)
            return false;

        Dictionary<UInt, UInt> mapping;
        forEachInCollection(
            info,
            [&](IRInst* table)
            {
                // Get unique ID for the witness table
                auto witnessTable = cast<IRWitnessTable>(table);
                auto outputId = getUniqueID(witnessTable);
                auto seqDecoration = table->findDecoration<IRSequentialIDDecoration>();
                if (seqDecoration)
                {
                    auto inputId = seqDecoration->getSequentialID();
                    mapping[inputId] = outputId; // Map ID to itself for now
                }
            });

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        auto translatedID = builder.emitCallInst(
            builder.getUIntType(),
            createIntegerMappingFunc(mapping),
            List<IRInst*>({inst->getTypeID()}));

        auto existentialTupleType = as<IRTupleType>(getTypeForExistential(info));
        auto existentialTuple = builder.emitMakeTuple(
            existentialTupleType,
            List<IRInst*>(
                {translatedID,
                 builder.emitReinterpret(existentialTupleType->getOperand(1), inst->getValue())}));

        if (auto info = tryGetInfo(context, inst))
            propagationMap[Element(existentialTuple)] = info;

        inst->replaceUsesWith(existentialTuple);
        inst->removeAndDeallocate();
        return true;
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

    IRFunc* createIntegerMappingFunc(Dictionary<UInt, UInt>& mapping)
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

        for (auto item : mapping)
        {
            // Create case block
            auto caseBlock = builder.emitBlock();
            builder.setInsertInto(caseBlock);
            builder.emitReturn(builder.getIntValue(builder.getUIntType(), item.second));

            caseValues.add(builder.getIntValue(builder.getUIntType(), item.first));
            caseBlocks.add(caseBlock);
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

    IRFunc* createKeyMappingFunc(
        IRInst* key,
        const HashSet<IRInst*>& inputTables,
        const HashSet<IRInst*>& outputVals)
    {
        Dictionary<UInt, UInt> mapping;

        // Create a mapping.
        for (auto table : inputTables)
        {
            auto inputId = getUniqueID(table);
            auto outputId = getUniqueID(findEntryInConcreteTable(table, key));
            mapping[inputId] = outputId;
        }

        return createIntegerMappingFunc(mapping);
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

        auto maybeReinterpret = [&](IRBuilder* builder, IRInst* value, IRType* type) -> IRInst*
        {
            if (as<IRAnyValueType>(type) && !as<IRAnyValueType>(value->getDataType()))
            {
                return builder->emitPackAnyValue(type, value);
            }
            else if (as<IRAnyValueType>(value->getDataType()) && !as<IRAnyValueType>(type))
            {
                return builder->emitUnpackAnyValue(type, value);
            }
            else if (value->getDataType() != type)
            {
                // If the value's type is different from the expected type, reinterpret it
                return builder->emitReinterpret(type, value);
            }
            else
            {
                return value; // Otherwise, return as is
            }
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
                callArgs.add(maybeReinterpret(
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
                builder.emitReturn(maybeReinterpret(&builder, callResult, resultType));
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

    bool processModule()
    {
        bool hasChanges = false;

        // Phase 1: Information Propagation
        performInformationPropagation();

        // Phase 1.5: Insert reinterprets for points where sets merge
        // e.g. phi, return, call
        //
        hasChanges |= insertReinterprets();

        // Phase 2: Dynamic Instruction Lowering
        hasChanges |= performDynamicInstLowering();

        return hasChanges;
    }

    DynamicInstLoweringContext(IRModule* module, DiagnosticSink* sink)
        : module(module), sink(sink)
    {
    }

    // Basic context
    IRModule* module;
    DiagnosticSink* sink;

    // Mapping from instruction to propagation information
    Dictionary<Element, PropagationInfo> propagationMap;

    // Mapping from function to return value propagation information
    Dictionary<IRInst*, PropagationInfo> funcReturnInfo;

    // Mapping from functions to call-sites.
    Dictionary<IRInst*, HashSet<Element>> funcCallSites;

    // Unique ID assignment for functions and witness tables
    Dictionary<IRInst*, UInt> uniqueIds;
    UInt nextUniqueId = 1;

    // Mapping from lowered instruction to their any-value types
    Dictionary<IRInst*, IRInst*> loweredInstToAnyValueType;

    // Set of open contexts
    HashSet<IRInst*> availableContexts;
};

// Main entry point
bool lowerDynamicInsts(IRModule* module, DiagnosticSink* sink)
{
    DynamicInstLoweringContext context(module, sink);
    return context.processModule();
}

} // namespace Slang
