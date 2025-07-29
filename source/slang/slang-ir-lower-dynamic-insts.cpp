#include "slang-ir-lower-dynamic-insts.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-witness-table-wrapper.h"
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

bool areInfosEqual(IRInst* a, IRInst* b)
{
    return a == b;
}

struct DynamicInstLoweringContext
{
    // Helper methods for creating canonical collections
    IRCollectionBase* createCollection(IROp op, const HashSet<IRInst*>& elements)
    {
        List<IRInst*> sortedElements;
        for (auto element : elements)
            sortedElements.add(element);

        return createCollection(op, sortedElements);
    }

    IRCollectionBase* createCollection(IROp op, const List<IRInst*>& elements)
    {
        SLANG_ASSERT(
            op == kIROp_TypeCollection || op == kIROp_FuncCollection ||
            op == kIROp_TableCollection || op == kIROp_GenericCollection);

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
        return as<IRCollectionBase>(builder.emitIntrinsicInst(
            nullptr,
            op,
            sortedElements.getCount(),
            sortedElements.getBuffer()));
    }

    IROp getCollectionTypeForInst(IRInst* inst)
    {
        if (as<IRTypeKind>(inst->getDataType()))
            return kIROp_TypeCollection;
        else if (as<IRType>(inst) && !as<IRInterfaceType>(inst))
            return kIROp_TypeCollection;
        else if (as<IRFuncType>(inst->getDataType()))
            return kIROp_FuncCollection;
        else if (as<IRWitnessTableType>(inst->getDataType()))
            return kIROp_TableCollection;
        else if (as<IRGeneric>(inst->getDataType()))
            return kIROp_GenericCollection;
        else
            SLANG_UNEXPECTED("Unsupported collection type for instruction");
    }

    // Factory methods for PropagationInfo
    IRCollectionBase* makeSingletonSet(IRInst* value)
    {
        HashSet<IRInst*> singleSet;
        singleSet.add(value);
        return createCollection(getCollectionTypeForInst(value), singleSet);
    }

    IRCollectionBase* makeSet(const HashSet<IRInst*>& values)
    {
        SLANG_ASSERT(values.getCount() > 0);
        return createCollection(getCollectionTypeForInst(*values.begin()), values);
    }

    IRCollectionTaggedUnionType* makeExistential(IRTableCollection* tableCollection)
    {
        HashSet<IRInst*> typeSet;
        // Collect all types from the witness tables
        forEachInCollection(
            tableCollection,
            [&](IRInst* witnessTable)
            {
                if (auto table = as<IRWitnessTable>(witnessTable))
                    typeSet.add(table->getConcreteType());
            });

        auto typeCollection = createCollection(kIROp_TypeCollection, typeSet);

        // Create the tagged union type
        IRBuilder builder(module);
        List<IRInst*> elements = {typeCollection, tableCollection};
        return as<IRCollectionTaggedUnionType>(builder.emitIntrinsicInst(
            nullptr,
            kIROp_CollectionTaggedUnionType,
            elements.getCount(),
            elements.getBuffer()));
    }

    /*IRCollectionTaggedUnionType* makeExistential(const HashSet<IRInst*>& tables)
    {
        SLANG_ASSERT(tables.getCount() > 0);
        auto tableCollection = createCollection(kIROp_TableCollection, tables);
        return makeExistential(tableCollection);
    }*/

    UCount getCollectionCount(IRCollectionBase* collection)
    {
        if (!collection)
            return 0;
        return collection->getOperandCount();
    }

    UCount getCollectionCount(IRCollectionTaggedUnionType* taggedUnion)
    {
        auto typeCollection = taggedUnion->getOperand(0);
        return getCollectionCount(as<IRCollectionBase>(typeCollection));
    }

    IRInst* getCollectionElement(IRCollectionBase* collection, UInt index)
    {
        if (!collection || index >= collection->getOperandCount())
            return nullptr;
        return collection->getOperand(index);
    }

    IRUnboundedCollection* makeUnbounded()
    {
        IRBuilder builder(module);
        return as<IRUnboundedCollection>(
            builder.emitIntrinsicInst(nullptr, kIROp_UnboundedCollection, 0, nullptr));
    }

    IRTypeFlowData* none() { return nullptr; }

    // Helper to iterate over collection elements
    template<typename F>
    void forEachInCollection(IRCollectionBase* info, F func)
    {
        for (UInt i = 0; i < info->getOperandCount(); ++i)
            func(info->getOperand(i));
    }

    // Helper to convert collection to HashSet
    HashSet<IRInst*> collectionToHashSet(IRCollectionBase* info)
    {
        HashSet<IRInst*> result;
        forEachInCollection(info, [&](IRInst* element) { result.add(element); });
        return result;
    }

    IRTypeFlowData* tryGetInfo(Element element)
    {
        // For non-global instructions, look up in the map
        auto found = propagationMap.tryGetValue(element);
        if (found)
            return *found;
        return none();
    }

    IRTypeFlowData* tryGetInfo(IRInst* context, IRInst* inst)
    {
        if (!inst->getParent())
            return none();

        // If this is a global instruction (parent is module), return concrete info
        if (as<IRModuleInst>(inst->getParent()))
            if (as<IRType>(inst) || as<IRWitnessTable>(inst) || as<IRFunc>(inst) ||
                as<IRGeneric>(inst))
                return makeSingletonSet(inst);
            else
                return none();

        return tryGetInfo(Element(context, inst));
    }

    IRTypeFlowData* tryGetFuncReturnInfo(IRFunc* func)
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
        IRTypeFlowData* newInfo,
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
        IRTypeFlowData* info,
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

            // If the user is a top-level inout/out parameter, we need to handle it
            // like we would a func-return.
            //
            if (auto param = as<IRParam>(user))
            {
                auto paramBlock = as<IRBlock>(param->getParent());
                auto paramFunc = as<IRFunc>(paramBlock->getParent());
                if (paramFunc && paramFunc->getFirstBlock() == paramBlock)
                {
                    if (this->funcCallSites.containsKey(context))
                        for (auto callSite : this->funcCallSites[context])
                        {
                            workQueue.addLast(WorkItem(
                                InterproceduralEdge::Direction::FuncToCall,
                                callSite.context,
                                as<IRCall>(callSite.inst),
                                context));
                        }
                }
            }
        }
    }

    // Helper method to update function return info and propagate to call sites
    void updateFuncReturnInfo(
        IRInst* callable,
        IRTypeFlowData* returnInfo,
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

    IRInst* maybeReinterpret(IRInst* context, IRInst* arg, IRTypeFlowData* destInfo)
    {
        auto argInfo = tryGetInfo(context, arg);

        if (!argInfo || !destInfo)
            return arg;

        if (as<IRCollectionTaggedUnionType>(argInfo) && as<IRCollectionTaggedUnionType>(destInfo))
        {
            if (getCollectionCount(as<IRCollectionTaggedUnionType>(argInfo)) !=
                getCollectionCount(as<IRCollectionTaggedUnionType>(destInfo)))
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
                    List<IRStore*> storeInsts;
                    // Collect all call instructions in this block
                    for (auto inst : block->getChildren())
                    {
                        if (auto callInst = as<IRCall>(inst))
                            callInsts.add(callInst);
                        else if (auto storeInst = as<IRStore>(inst))
                            storeInsts.add(storeInst);
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

                    // Look at all the stores and reinterpret them if necessary
                    for (auto storeInst : storeInsts)
                    {
                        auto newValToStore = maybeReinterpret(
                            context,
                            storeInst->getVal(),
                            tryGetInfo(storeInst->getPtr()));
                        if (newValToStore != storeInst->getVal())
                        {
                            // Replace the value in the store instruction
                            changed = true;
                            storeInst->setOperand(1, newValToStore);
                        }
                    }
                }
            }
        }

        return changed;
    }

    void processInstForPropagation(IRInst* context, IRInst* inst, LinkedList<WorkItem>& workQueue)
    {
        IRTypeFlowData* info;

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
        case kIROp_Load:
            info = analyzeLoad(context, as<IRLoad>(inst));
            break;
        case kIROp_Store:
            info = analyzeStore(context, as<IRStore>(inst), workQueue);
            break;
        default:
            info = analyzeDefault(context, inst);
            break;
        }

        updateInfo(context, inst, info, workQueue);
    }

    IRTypeFlowData* analyzeCreateExistentialObject(IRInst* context, IRCreateExistentialObject* inst)
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
                    return makeExistential(
                        as<IRTableCollection>(createCollection(kIROp_TableCollection, tables)));
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

    IRTypeFlowData* analyzeMakeExistential(IRInst* context, IRMakeExistential* inst)
    {
        auto witnessTable = inst->getWitnessTable();
        auto value = inst->getWrappedValue();
        auto valueType = value->getDataType();

        // Get the witness table info
        auto witnessTableInfo = tryGetInfo(context, witnessTable);

        if (!witnessTableInfo)
            return none();

        if (as<IRUnboundedCollection>(witnessTableInfo))
            return makeUnbounded();

        HashSet<IRInst*> tables;
        if (auto collection = as<IRTableCollection>(witnessTableInfo))
            return makeExistential(collection);

        SLANG_UNEXPECTED("Unexpected witness table info type in analyzeMakeExistential");
    }

    static IRInst* findEntryInConcreteTable(IRInst* witnessTable, IRInst* key)
    {
        if (auto concreteTable = as<IRWitnessTable>(witnessTable))
            for (auto entry : concreteTable->getEntries())
                if (entry->getRequirementKey() == key)
                    return entry->getSatisfyingVal();
        return nullptr; // Not found
    }

    IRTypeFlowData* analyzeLoad(IRInst* context, IRLoad* loadInst)
    {
        // Transfer the prop info from the address to the loaded value
        auto address = loadInst->getPtr();
        return tryGetInfo(context, address);
    }

    IRTypeFlowData* analyzeStore(
        IRInst* context,
        IRStore* storeInst,
        LinkedList<WorkItem>& workQueue)
    {
        // Transfer the prop info from stored value to the address
        auto address = storeInst->getPtr();
        updateInfo(context, address, tryGetInfo(context, storeInst->getVal()), workQueue);
        return none(); // The store itself doesn't have any info.
    }

    IRTypeFlowData* analyzeLookupWitnessMethod(IRInst* context, IRLookupWitnessMethod* inst)
    {
        auto key = inst->getRequirementKey();

        auto witnessTable = inst->getWitnessTable();
        auto witnessTableInfo = tryGetInfo(context, witnessTable);

        if (!witnessTableInfo)
            return none();

        if (as<IRUnboundedCollection>(witnessTableInfo))
            return makeUnbounded();

        if (auto collection = as<IRCollectionBase>(witnessTableInfo))
        {
            HashSet<IRInst*> results;
            forEachInCollection(
                collection,
                [&](IRInst* table) { results.add(findEntryInConcreteTable(table, key)); });
            return makeSet(results);
        }

        SLANG_UNEXPECTED("Unexpected witness table info type in analyzeLookupWitnessMethod");
    }

    IRTypeFlowData* analyzeExtractExistentialWitnessTable(
        IRInst* context,
        IRExtractExistentialWitnessTable* inst)
    {
        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(context, operand);

        if (!operandInfo)
            return none();

        if (as<IRUnboundedCollection>(operandInfo))
            return makeUnbounded();

        if (auto taggedUnion = as<IRCollectionTaggedUnionType>(operandInfo))
            return as<IRTableCollection>(taggedUnion->getOperand(1));

        SLANG_UNEXPECTED("Unhandled info type in analyzeExtractExistentialWitnessTable");
    }

    IRTypeFlowData* analyzeExtractExistentialType(IRInst* context, IRExtractExistentialType* inst)
    {
        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(context, operand);

        if (!operandInfo)
            return none();

        if (as<IRUnboundedCollection>(operandInfo))
            return makeUnbounded();

        if (auto taggedUnion = as<IRCollectionTaggedUnionType>(operandInfo))
            return as<IRTypeCollection>(taggedUnion->getOperand(0));

        SLANG_UNEXPECTED("Unhandled info type in analyzeExtractExistentialType");
    }

    IRTypeFlowData* analyzeExtractExistentialValue(IRInst* context, IRExtractExistentialValue* inst)
    {
        // We don't care about the value itself.
        // (We rely on the propagation info for the type)
        //
        return none();
    }

    IRTypeFlowData* analyzeSpecialize(IRInst* context, IRSpecialize* inst)
    {
        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(context, operand);

        if (!operandInfo)
            return none();

        if (as<IRUnboundedCollection>(operandInfo))
            return makeUnbounded();

        if (as<IRCollectionTaggedUnionType>(operandInfo))
        {
            SLANG_UNEXPECTED(
                "Unexpected ExtractExistentialWitnessTable on Set (should be Existential)");
        }

        if (auto collection = as<IRCollectionBase>(operandInfo))
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
                if (!argInfo)
                    return none(); // Can't determine the result just yet.

                if (as<IRUnboundedCollection>(argInfo) || as<IRCollectionTaggedUnionType>(argInfo))
                {
                    SLANG_UNEXPECTED(
                        "Unexpected Existential operand in specialization argument. Should be "
                        "set");
                }

                if (auto argCollection = as<IRCollectionBase>(argInfo))
                {
                    if (getCollectionCount(argCollection) == 1)
                        specializationArgs.add(getCollectionElement(argCollection, 0));
                    else
                        specializationArgs.add(argCollection);
                }
                else
                {
                    SLANG_UNEXPECTED("Unhandled PropagationJudgment in analyzeSpecialize");
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
                        if (auto infoCollection = as<IRCollectionBase>(info))
                        {
                            if (getCollectionCount(infoCollection) == 1)
                                return getCollectionElement(infoCollection, 0);
                            else
                                return infoCollection;
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
                collection,
                [&](IRInst* arg)
                {
                    // Create a new specialized instruction for each argument
                    IRBuilder builder(module);
                    builder.setInsertInto(module);
                    specializedSet.add(
                        builder.emitSpecializeInst(typeOfSpecialization, arg, specializationArgs));
                });
            return makeSet(specializedSet);
        }

        SLANG_UNEXPECTED("Unhandled PropagationJudgment in analyzeExtractExistentialWitnessTable");
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

                        if (auto collection = as<IRCollectionBase>(arg))
                        {
                            updateInfo(context, param, collection, workQueue);
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

    IRTypeFlowData* analyzeCall(IRInst* context, IRCall* inst, LinkedList<WorkItem>& workQueue)
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
            if (this->funcCallSites[callee].add(Element(context, inst)))
            {
                // If this is a new call site, add a propagation task to the queue (in case there's
                // already information about this function)
                workQueue.addLast(
                    WorkItem(InterproceduralEdge::Direction::FuncToCall, context, inst, callee));
            }
            workQueue.addLast(
                WorkItem(InterproceduralEdge::Direction::CallToFunc, context, inst, callee));
        };

        if (auto collection = as<IRCollectionBase>(calleeInfo))
        {
            // If we have a set of functions, register each one
            forEachInCollection(collection, [&](IRInst* func) { propagateToCallSite(func); });
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

    List<IRTypeFlowData*> getParamInfos(IRInst* context)
    {
        List<IRTypeFlowData*> infos;
        if (as<IRFunc>(context))
        {
            for (auto param : as<IRFunc>(context)->getParams())
                infos.add(tryGetInfo(context, param));
        }
        else if (auto specialize = as<IRSpecialize>(context))
        {
            auto generic = specialize->getBase();
            auto innerFunc = getGenericReturnVal(generic);
            for (auto param : as<IRFunc>(innerFunc)->getParams())
                infos.add(tryGetInfo(context, param));
        }
        else
        {
            // If it's not a function or a specialization, we can't get parameter info
            SLANG_UNEXPECTED("Unexpected context type for parameter info retrieval");
        }

        return infos;
    }

    List<ParameterDirection> getParamDirections(IRInst* context)
    {
        List<ParameterDirection> directions;
        if (as<IRFunc>(context))
        {
            for (auto param : as<IRFunc>(context)->getParams())
            {
                const auto [direction, type] = getParameterDirectionAndType(param->getDataType());
                directions.add(direction);
            }
        }
        else if (auto specialize = as<IRSpecialize>(context))
        {
            auto generic = specialize->getBase();
            auto innerFunc = getGenericReturnVal(generic);
            for (auto param : as<IRFunc>(innerFunc)->getParams())
            {
                const auto [direction, type] = getParameterDirectionAndType(param->getDataType());
                directions.add(direction);
            }
        }
        else
        {
            // If it's not a function or a specialization, we can't get parameter info
            SLANG_UNEXPECTED("Unexpected context type for parameter info retrieval");
        }

        return directions;
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

                // Also update infos of any out parameters
                auto paramInfos = getParamInfos(edge.targetContext);
                auto paramDirections = getParamDirections(edge.targetContext);
                UIndex argIndex = 0;
                for (auto paramInfo : paramInfos)
                {
                    if (paramDirections[argIndex] == kParameterDirection_Out ||
                        paramDirections[argIndex] == kParameterDirection_InOut)
                    {
                        updateInfo(
                            edge.callerContext,
                            callInst->getArg(argIndex),
                            paramInfo,
                            workQueue);
                    }
                    argIndex++;
                }

                break;
            }
        default:
            SLANG_UNEXPECTED("Unhandled interprocedural edge direction");
            return;
        }
    }

    IRTypeFlowData* getFuncReturnInfo(IRInst* callee)
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

    IRTypeFlowData* unionPropagationInfo(const List<IRTypeFlowData*>& infos)
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

        // Need to create a union - collect all possible values based on IR instruction types
        HashSet<IRInst*> allValues;

        // Determine the union type and collect values
        bool hasUnbounded = false;
        bool hasExistential = false;

        for (auto info : infos)
        {
            if (!info)
                continue;

            if (as<IRUnboundedCollection>(info))
            {
                // If any info is unbounded, the union is unbounded
                return makeUnbounded();
            }
            else if (auto taggedUnion = as<IRCollectionTaggedUnionType>(info))
            {
                hasExistential = true;
                auto tableCollection = as<IRCollectionBase>(taggedUnion->getOperand(1));
                forEachInCollection(tableCollection, [&](IRInst* value) { allValues.add(value); });
            }
            else if (auto collection = as<IRCollectionBase>(info))
            {
                forEachInCollection(collection, [&](IRInst* value) { allValues.add(value); });
            }
        }

        if (hasExistential && allValues.getCount() > 0)
            return makeExistential(
                as<IRTableCollection>(createCollection(kIROp_TableCollection, allValues)));

        if (allValues.getCount() > 0)
            return makeSet(allValues);
        else
            return none();
    }

    IRTypeFlowData* unionPropagationInfo(IRTypeFlowData* info1, IRTypeFlowData* info2)
    {
        // Union the two infos
        List<IRTypeFlowData*> infos;
        infos.add(info1);
        infos.add(info2);
        return unionPropagationInfo(infos);
    }

    IRTypeFlowData* analyzeDefault(IRInst* context, IRInst* inst)
    {
        // Check if this is a global type, witness table, or function.
        // If so, it's a concrete element. We'll create a singleton set for it.
        if (inst->getParent()->getOp() == kIROp_ModuleInst &&
            (as<IRType>(inst) || as<IRWitnessTable>(inst) || as<IRFunc>(inst)))
            return makeSingletonSet(inst);
        else
            return none(); // Default case, no propagation info
    }

    bool lowerInstsInFunc(IRFunc* func)
    {
        // Collect all instructions that need lowering
        List<Element> typeInstsToLower;
        List<Element> valueInstsToLower;
        List<Element> instWithReplacementTypes;
        List<IRFunc*> funcTypesToProcess;

        bool hasChanges = false;
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
                        auto callee = as<IRCall>(child)->getCallee();
                        if (auto info = tryGetInfo(context, child))
                            if (as<IRCollectionTaggedUnionType>(info))
                                instWithReplacementTypes.add(Element(context, child));

                        if (auto calleeInfo = tryGetInfo(context, callee))
                            if (as<IRCollectionBase>(calleeInfo))
                                valueInstsToLower.add(Element(context, child));

                        if (as<IRSpecialize>(callee))
                            valueInstsToLower.add(Element(context, child));
                    }
                    break;
                default:
                    if (auto info = tryGetInfo(context, child))
                        if (as<IRCollectionTaggedUnionType>(info))
                            // If this instruction has a set of types, tables, or funcs,
                            // we need to lower it to a unified type.
                            instWithReplacementTypes.add(Element(context, child));
                }
            }
        }

        for (auto instWithCtx : typeInstsToLower)
        {
            if (instWithCtx.inst->getParent() == nullptr)
                continue;
            hasChanges |= lowerInst(instWithCtx.context, instWithCtx.inst);
        }

        for (auto instWithCtx : valueInstsToLower)
        {
            if (instWithCtx.inst->getParent() == nullptr)
                continue;
            hasChanges |= lowerInst(instWithCtx.context, instWithCtx.inst);
        }

        for (auto instWithCtx : instWithReplacementTypes)
        {
            if (instWithCtx.inst->getParent() == nullptr)
                continue;
            hasChanges |= replaceType(instWithCtx.context, instWithCtx.inst);
        }

        return hasChanges;
    }

    bool performDynamicInstLowering()
    {
        List<IRFunc*> funcsForTypeReplacement;
        List<IRFunc*> funcsToProcess;

        for (auto globalInst : module->getGlobalInsts())
            if (auto func = as<IRFunc>(globalInst))
            {
                funcsForTypeReplacement.add(func);
                funcsToProcess.add(func);
            }

        bool hasChanges = false;
        do
        {
            while (funcsForTypeReplacement.getCount() > 0)
            {
                auto func = funcsForTypeReplacement.getLast();
                funcsForTypeReplacement.removeLast();

                // Replace the function type with a concrete type if it has existential return types
                hasChanges |= replaceFuncType(func, this->funcReturnInfo[func]);
            }

            while (funcsToProcess.getCount() > 0)
            {
                auto func = funcsToProcess.getLast();
                funcsToProcess.removeLast();

                // Lower the instructions in the function
                hasChanges |= lowerInstsInFunc(func);
            }

            // The above loops might have added new contexts to lower.
            for (auto context : this->contextsToLower)
            {
                hasChanges |= lowerContext(context);
                auto newFunc = cast<IRFunc>(this->loweredContexts[context]);
                funcsForTypeReplacement.add(newFunc);
                funcsToProcess.add(newFunc);
            }
            this->contextsToLower.clear();

        } while (funcsForTypeReplacement.getCount() > 0 || funcsToProcess.getCount() > 0);

        return hasChanges;
    }

    bool replaceFuncType(IRFunc* func, IRTypeFlowData* returnTypeInfo)
    {
        IRFuncType* origFuncType = as<IRFuncType>(func->getFullType());
        IRType* returnType = origFuncType->getResultType();
        if (auto taggedUnion = as<IRCollectionTaggedUnionType>(returnTypeInfo))
        {
            // If the return type is existential, we need to replace it with a tuple type
            returnType = getTypeForExistential(taggedUnion);
        }

        List<IRType*> paramTypes;
        for (auto param : func->getFirstBlock()->getParams())
        {
            // Extract the existential type from the parameter if it exists
            auto paramInfo = tryGetInfo(param);
            if (auto paramTaggedUnion = as<IRCollectionTaggedUnionType>(paramInfo))
            {
                paramTypes.add(getTypeForExistential(paramTaggedUnion));
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

    IRType* getTypeForExistential(IRCollectionTaggedUnionType* taggedUnion)
    {
        // Replace type with Tuple<UInt, AnyValueType>
        IRBuilder builder(module);
        builder.setInsertInto(module);

        HashSet<IRInst*> types;
        auto tableCollection = as<IRCollectionBase>(taggedUnion->getOperand(1));
        forEachInCollection(
            tableCollection,
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
        auto taggedUnion = as<IRCollectionTaggedUnionType>(info);
        if (!taggedUnion)
            return false;

        if (auto ptrType = as<IRPtrTypeBase>(inst->getDataType()))
        {
            IRBuilder builder(module);
            inst->setFullType(
                builder.getPtrTypeWithAddressSpace(getTypeForExistential(taggedUnion), ptrType));
        }
        else
        {
            inst->setFullType(getTypeForExistential(taggedUnion));
        }
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

        if (auto collection = as<IRCollectionBase>(info))
        {
            if (getCollectionCount(collection) == 1)
            {
                // Found a single possible type. Simple replacement.
                inst->replaceUsesWith(getCollectionElement(collection, 0));
                inst->removeAndDeallocate();
                return true;
            }
            else
            {
                // Set of types.
                if (inst->getDataType()->getOp() == kIROp_TypeKind)
                {
                    // Create an any-value type based on the set of types
                    auto typeSet = collectionToHashSet(collection);
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

                    if (auto witnessTableCollection = as<IRCollectionBase>(witnessTableInfo))
                    {
                        // Create a key mapping function
                        auto keyMappingFunc = createKeyMappingFunc(
                            inst->getRequirementKey(),
                            collectionToHashSet(witnessTableCollection),
                            collectionToHashSet(collection));

                        // Replace with call to key mapping function
                        auto witnessTableId = builder.emitCallInst(
                            builder.getUIntType(),
                            keyMappingFunc,
                            List<IRInst*>({inst->getWitnessTable()}));
                        inst->replaceUsesWith(witnessTableId);
                        propagationMap[Element(context, witnessTableId)] = info;
                        inst->removeAndDeallocate();
                        return true;
                    }
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

        if (auto collection = as<IRCollectionBase>(info))
        {
            if (getCollectionCount(collection) == 1)
            {
                // Found a single possible type. Simple replacement.
                inst->replaceUsesWith(getCollectionElement(collection, 0));
                inst->removeAndDeallocate();
                return true;
            }
            else
            {
                // Replace with GetElement(loweredInst, 0) -> uint
                auto operand = inst->getOperand(0);
                auto element = builder.emitGetTupleElement(builder.getUIntType(), operand, 0);
                inst->replaceUsesWith(element);
                propagationMap[Element(context, element)] = info;
                inst->removeAndDeallocate();
                return true;
            }
        }
        return false;
    }

    bool lowerExtractExistentialValue(IRInst* context, IRExtractExistentialValue* inst)
    {
        auto operandInfo = tryGetInfo(context, inst->getOperand(0));
        auto taggedUnion = as<IRCollectionTaggedUnionType>(operandInfo);
        if (!taggedUnion)
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
        auto collection = as<IRCollectionBase>(info);
        if (!collection)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        if (getCollectionCount(collection) == 1)
        {
            // Found a single possible type. Simple replacement.
            auto singletonValue = getCollectionElement(collection, 0);
            inst->replaceUsesWith(singletonValue);
            inst->removeAndDeallocate();
            loweredInstToAnyValueType[inst] = singletonValue;
            return true;
        }

        // Create an any-value type based on the set of types
        auto anyValueType = createAnyValueTypeFromInsts(collectionToHashSet(collection));

        // Store the mapping for later use
        loweredInstToAnyValueType[inst] = anyValueType;

        // Replace the instruction with the any-value type
        inst->replaceUsesWith(anyValueType);
        inst->removeAndDeallocate();
        return true;
    }

    // Split into direction and type
    std::tuple<ParameterDirection, IRType*> getParameterDirectionAndType(IRType* paramType)
    {
        if (as<IROutType>(paramType))
            return {
                ParameterDirection::kParameterDirection_Out,
                as<IROutType>(paramType)->getValueType()};
        else if (as<IRInOutType>(paramType))
            return {
                ParameterDirection::kParameterDirection_InOut,
                as<IRInOutType>(paramType)->getValueType()};
        else if (as<IRRefType>(paramType))
            return {
                ParameterDirection::kParameterDirection_Ref,
                as<IRRefType>(paramType)->getValueType()};
        else if (as<IRConstRefType>(paramType))
            return {
                ParameterDirection::kParameterDirection_ConstRef,
                as<IRConstRefType>(paramType)->getValueType()};
        else
            return {ParameterDirection::kParameterDirection_In, paramType};
    }

    IRFuncType* getExpectedFuncType(IRInst* context, IRCall* inst)
    {
        IRBuilder builder(module);
        builder.setInsertInto(module);

        // We'll retreive just the parameter directions from the callee's func-type,
        // since that can't be different before & after the type-flow lowering.
        //
        List<ParameterDirection> paramDirections;
        auto calleeInfo = tryGetInfo(context, inst->getCallee());
        auto calleeCollection = as<IRCollectionBase>(calleeInfo);
        if (!calleeCollection)
            return nullptr;

        auto funcType = as<IRFuncType>(getCollectionElement(calleeCollection, 0)->getDataType());
        for (auto paramType : funcType->getParamTypes())
        {
            auto [direction, type] = getParameterDirectionAndType(paramType);
            paramDirections.add(direction);
        }

        // Translate argument types into expected function type.
        List<IRType*> paramTypes;
        for (UInt i = 0; i < inst->getArgCount(); i++)
        {
            auto arg = inst->getArg(i);

            switch (paramDirections[i])
            {
            case ParameterDirection::kParameterDirection_In:
                {
                    auto argInfo = tryGetInfo(context, arg);
                    if (auto argTaggedUnion = as<IRCollectionTaggedUnionType>(argInfo))
                        paramTypes.add(getTypeForExistential(argTaggedUnion));
                    else
                        paramTypes.add(arg->getDataType());
                    break;
                }
            case ParameterDirection::kParameterDirection_Out:
                {
                    auto argInfo = tryGetInfo(context, arg);
                    if (auto argTaggedUnion = as<IRCollectionTaggedUnionType>(argInfo))
                        paramTypes.add(builder.getOutType(getTypeForExistential(argTaggedUnion)));
                    else
                        paramTypes.add(builder.getOutType(
                            as<IRPtrTypeBase>(arg->getDataType())->getValueType()));
                    break;
                }
            case ParameterDirection::kParameterDirection_InOut:
                {
                    auto argInfo = tryGetInfo(context, arg);
                    if (auto argTaggedUnion = as<IRCollectionTaggedUnionType>(argInfo))
                        paramTypes.add(builder.getInOutType(getTypeForExistential(argTaggedUnion)));
                    else
                        paramTypes.add(builder.getInOutType(
                            as<IRPtrTypeBase>(arg->getDataType())->getValueType()));
                    break;
                }
            default:
                SLANG_UNEXPECTED("Unhandled parameter direction in getExpectedFuncType");
            }
        }

        // Translate result type.
        IRType* resultType = inst->getDataType();
        auto returnInfo = tryGetInfo(context, inst);
        if (auto returnTaggedUnion = as<IRCollectionTaggedUnionType>(returnInfo))
        {
            resultType = getTypeForExistential(returnTaggedUnion);
        }

        return builder.getFuncType(paramTypes, resultType);
    }

    bool isDynamicGeneric(IRInst* callee)
    {
        // If the callee is a specialization, and at least one of its arguments
        // is a type-flow-collection, then it is a dynamic generic.
        //
        if (auto specialize = as<IRSpecialize>(callee))
        {
            for (UInt i = 0; i < specialize->getArgCount(); i++)
            {
                auto arg = specialize->getArg(i);
                if (as<IRCollectionBase>(arg))
                    return true; // Found a type-flow-collection argument
            }
            return false; // No type-flow-collection arguments found
        }

        return false;
    }

    bool lowerContext(IRInst* context)
    {
        auto specializeInst = cast<IRSpecialize>(context);
        auto generic = cast<IRGeneric>(specializeInst->getBase());
        auto genericReturnVal = findGenericReturnVal(generic);

        IRBuilder builder(module);
        builder.setInsertInto(module);

        // Let's start by creating the function itself.
        auto loweredFunc = builder.createFunc();
        builder.setInsertInto(loweredFunc);
        builder.setInsertInto(builder.emitBlock());
        // loweredFunc->setFullType(context->getFullType());

        IRCloneEnv cloneEnv;
        Index argIndex = 0;
        UCount extraIndices = 0;
        // Map the generic's parameters to the specialized arguments.
        for (auto param : generic->getFirstBlock()->getParams())
        {
            auto specArg = specializeInst->getArg(argIndex++);
            if (as<IRCollectionBase>(specArg))
            {
                // We're dealing with a set of types.
                if (as<IRTypeType>(param->getDataType()))
                {
                    HashSet<IRInst*> collectionSet;
                    for (auto index = 0; index < specArg->getOperandCount(); index++)
                    {
                        auto operand = specArg->getOperand(index);
                        collectionSet.add(operand);
                    }

                    auto unionType = createAnyValueTypeFromInsts(collectionSet);
                    cloneEnv.mapOldValToNew[param] = unionType;
                }
                else if (as<IRWitnessTableType>(param->getDataType()))
                {
                    // Add an integer param to the func.
                    cloneEnv.mapOldValToNew[param] = builder.emitParam(builder.getUIntType());
                    extraIndices++;
                }
            }
            else
            {
                // For everything else, just set the parameter type to the argument;
                SLANG_ASSERT(specArg->getParent()->getOp() == kIROp_ModuleInst);
                cloneEnv.mapOldValToNew[param] = specArg;
            }
        }

        // Clone in the rest of the generic's body including the blocks of the returned func.
        for (auto inst = generic->getFirstBlock()->getFirstOrdinaryInst(); inst;
             inst = inst->getNextInst())
        {
            if (inst == genericReturnVal)
            {
                auto returnedFunc = cast<IRFunc>(inst);
                auto funcFirstBlock = returnedFunc->getFirstBlock();

                // cloneEnv.mapOldValToNew[funcFirstBlock] = loweredFunc->getFirstBlock();
                builder.setInsertInto(loweredFunc);
                for (auto block : returnedFunc->getBlocks())
                {
                    // Merge the first block of the generic with the first block of the
                    // returned function to merge the parameter lists.
                    //
                    // if (block != funcFirstBlock)
                    //{
                    cloneEnv.mapOldValToNew[block] =
                        cloneInstAndOperands(&cloneEnv, &builder, block);
                    //}
                }

                builder.setInsertInto(loweredFunc->getFirstBlock());
                builder.emitBranch(as<IRBlock>(cloneEnv.mapOldValToNew[funcFirstBlock]));

                for (auto param : funcFirstBlock->getParams())
                {
                    // Clone the parameters of the first block.
                    builder.setInsertAfter(loweredFunc->getFirstBlock()->getLastParam());
                    cloneInst(&cloneEnv, &builder, param);
                }

                builder.setInsertInto(as<IRBlock>(cloneEnv.mapOldValToNew[funcFirstBlock]));
                for (auto inst = funcFirstBlock->getFirstOrdinaryInst(); inst;
                     inst = inst->getNextInst())
                {
                    // Clone the instructions in the first block.
                    cloneInst(&cloneEnv, &builder, inst);
                }

                for (auto block : returnedFunc->getBlocks())
                {
                    if (block == funcFirstBlock)
                        continue; // Already cloned the first block
                    cloneInstDecorationsAndChildren(
                        &cloneEnv,
                        builder.getModule(),
                        block,
                        cloneEnv.mapOldValToNew[block]);
                }

                builder.setInsertInto(builder.getModule());
                auto loweredFuncType = as<IRFuncType>(
                    cloneInst(&cloneEnv, &builder, as<IRFuncType>(returnedFunc->getFullType())));

                // Add extra indices to the func-type parameters
                List<IRType*> funcTypeParams;
                for (Index i = 0; i < extraIndices; i++)
                    funcTypeParams.add(builder.getUIntType());

                for (auto paramType : loweredFuncType->getParamTypes())
                    funcTypeParams.add(paramType);

                // Set the new function type with the extra indices
                loweredFunc->setFullType(
                    builder.getFuncType(funcTypeParams, loweredFuncType->getResultType()));
            }
            else if (!as<IRReturn>(inst))
            {
                // Keep cloning insts in the generic
                cloneInst(&cloneEnv, &builder, inst);
            }
        }

        // Transfer propagation info.
        for (auto& [oldVal, newVal] : cloneEnv.mapOldValToNew)
        {
            if (propagationMap.containsKey(Element(context, oldVal)))
            {
                // If we have propagation info for the old value, transfer it to the new value
                if (auto info = propagationMap[Element(context, oldVal)])
                {
                    if (newVal->getParent()->getOp() != kIROp_ModuleInst)
                        propagationMap[Element(loweredFunc, newVal)] = info;
                }
            }
        }

        // Transfer func-return value info.
        if (this->funcReturnInfo.containsKey(context))
        {
            this->funcReturnInfo[loweredFunc] = this->funcReturnInfo[context];
        }

        context->replaceUsesWith(loweredFunc);
        // context->removeAndDeallocate();
        this->loweredContexts[context] = loweredFunc;
        return true;
    }

    IRInst* getCalleeForContext(IRInst* context)
    {
        if (this->contextsToLower.contains(context))
            return context; // Not lowered yet.

        if (this->loweredContexts.containsKey(context))
            return this->loweredContexts[context];
        else
            this->contextsToLower.add(context);

        return context;
    }

    bool lowerCallToDynamicGeneric(IRInst* context, IRCall* inst)
    {
        auto specializedCallee = as<IRSpecialize>(inst->getCallee());
        auto calleeInfo = tryGetInfo(context, specializedCallee);
        auto calleeCollection = as<IRCollectionBase>(calleeInfo);
        if (!calleeCollection || getCollectionCount(calleeCollection) != 1)
            return false;

        auto targetContext = getCollectionElement(calleeCollection, 0);

        List<IRInst*> callArgs;
        for (auto ii = 0; ii < specializedCallee->getArgCount(); ii++)
        {
            auto specArg = specializedCallee->getArg(ii);
            auto argInfo = tryGetInfo(context, specArg);
            if (auto argCollection = as<IRCollectionBase>(argInfo))
            {
                if (as<IRWitnessTable>(getCollectionElement(argCollection, 0)))
                {
                    // Needs an index (spec-arg will carry an index, we'll
                    // just need to append it to the call)
                    //
                    callArgs.add(specArg);
                }
                else if (as<IRType>(getCollectionElement(argCollection, 0)))
                {
                    // Needs no dynamic information. Skip.
                }
                else
                {
                    // If it's a witness table, we need to handle it differently
                    // For now, we will not lower this case.
                    SLANG_UNEXPECTED("Unhandled type-flow-collection in dynamic generic call");
                }
            }
        }

        for (auto ii = 0; ii < inst->getArgCount(); ii++)
            callArgs.add(inst->getArg(ii));

        IRBuilder builder(inst->getModule());
        // builder.replaceOperand(inst->getCalleeUse(), specializedCallee);
        builder.setInsertBefore(inst);
        auto newCallInst = builder.emitCallInst(
            as<IRFuncType>(targetContext->getDataType())->getResultType(),
            getCalleeForContext(targetContext),
            callArgs);
        inst->replaceUsesWith(newCallInst);
        inst->removeAndDeallocate();
        return true;
    }

    bool lowerCall(IRInst* context, IRCall* inst)
    {
        auto callee = inst->getCallee();
        auto calleeInfo = tryGetInfo(context, callee);

        auto calleeCollection = as<IRCollectionBase>(calleeInfo);
        if (!calleeCollection)
            return false;

        if (getCollectionCount(calleeCollection) == 1)
        {
            auto singletonValue = getCollectionElement(calleeCollection, 0);
            if (isDynamicGeneric(singletonValue))
                return lowerCallToDynamicGeneric(context, inst);

            if (singletonValue == callee)
                return false;

            IRBuilder builder(inst->getModule());
            builder.replaceOperand(inst->getCalleeUse(), singletonValue);
            return true; // Replaced with a single function
        }

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        auto expectedFuncType = getExpectedFuncType(context, inst);
        // Create dispatch function
        auto dispatchFunc =
            createDispatchFunc(collectionToHashSet(calleeCollection), expectedFuncType);

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
            propagationMap[Element(context, newCall)] = info;
        replaceType(context, newCall); // "maybe replace type"
        inst->removeAndDeallocate();
        return true;
    }

    bool lowerMakeExistential(IRInst* context, IRMakeExistential* inst)
    {
        auto info = tryGetInfo(context, inst);
        auto taggedUnion = as<IRCollectionTaggedUnionType>(info);
        if (!taggedUnion)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        auto witnessTableInfo = tryGetInfo(context, inst->getWitnessTable());
        auto witnessTableCollection = as<IRCollectionBase>(witnessTableInfo);
        if (!witnessTableCollection)
            return false; // Witness table must be a set of tables

        IRInst* witnessTableID = nullptr;
        if (getCollectionCount(witnessTableCollection) == 1)
        {
            // Get unique ID for the witness table.
            witnessTableID = builder.getIntValue(
                builder.getUIntType(),
                getUniqueID(getCollectionElement(witnessTableCollection, 0)));
        }
        else
        {
            // Dynamic. Use the witness table inst as an integer key.
            witnessTableID = inst->getWitnessTable();
        }

        // Collect types from the witness tables to determine the any-value type
        HashSet<IRType*> types;
        auto tableCollection = as<IRCollectionBase>(taggedUnion->getOperand(1));
        forEachInCollection(
            tableCollection,
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
        IRInst* tupleArgs[] = {witnessTableID, packedValue};
        auto tuple = builder.emitMakeTuple(tupleType, 2, tupleArgs);

        if (auto info = tryGetInfo(context, inst))
            propagationMap[Element(context, tuple)] = info;

        inst->replaceUsesWith(tuple);
        inst->removeAndDeallocate();
        return true;
    }

    bool lowerCreateExistentialObject(IRInst* context, IRCreateExistentialObject* inst)
    {
        auto info = tryGetInfo(context, inst);
        auto taggedUnion = as<IRCollectionTaggedUnionType>(info);
        if (!taggedUnion)
            return false;

        Dictionary<UInt, UInt> mapping;
        auto tableCollection = as<IRCollectionBase>(taggedUnion->getOperand(1));
        forEachInCollection(
            tableCollection,
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

        auto existentialTupleType = as<IRTupleType>(getTypeForExistential(taggedUnion));
        auto existentialTuple = builder.emitMakeTuple(
            existentialTupleType,
            List<IRInst*>(
                {translatedID,
                 builder.emitReinterpret(existentialTupleType->getOperand(1), inst->getValue())}));

        if (auto info = tryGetInfo(context, inst))
            propagationMap[Element(context, existentialTuple)] = info;

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

        // Go back to entry block and create switch
        builder.setInsertInto(entryBlock);

        // Create case blocks for each function
        List<IRInst*> caseValues;
        List<IRBlock*> caseBlocks;

        for (auto funcInst : funcs)
        {
            auto funcId = getUniqueID(funcInst);

            auto wrapperFunc =
                emitWitnessTableWrapper(funcInst->getModule(), funcInst, expectedFuncType);

            // Create case block
            auto caseBlock = builder.emitBlock();
            builder.setInsertInto(caseBlock);

            List<IRInst*> callArgs;
            auto wrappedFuncType = as<IRFuncType>(wrapperFunc->getDataType());
            for (UIndex ii = 0; ii < originalParams.getCount(); ii++)
            {
                callArgs.add(originalParams[ii]);
            }

            // Call the specific function
            auto callResult =
                builder.emitCallInst(wrappedFuncType->getResultType(), wrapperFunc, callArgs);

            if (resultType->getOp() == kIROp_VoidType)
            {
                builder.emitReturn();
            }
            else
            {
                builder.emitReturn(callResult);
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

    bool lowerCollectionTypes()
    {
        bool hasChanges = false;

        // Lower all global scope ``IRCollectionBase`` objects that
        // are made up of types.
        //
        for (auto inst : module->getGlobalInsts())
        {
            if (auto collection = as<IRCollectionBase>(inst))
            {
                if (collection->getOp() == kIROp_TypeCollection)
                {
                    HashSet<IRType*> types;
                    for (UInt i = 0; i < collection->getOperandCount(); i++)
                    {
                        if (auto type = as<IRType>(collection->getOperand(i)))
                        {
                            types.add(type);
                        }
                    }
                    auto anyValueType = createAnyValueType(types);
                    collection->replaceUsesWith(anyValueType);
                    hasChanges = true;
                }
            }
        }

        return hasChanges;
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

        // Phase 3: Lower collection types.
        if (hasChanges)
            lowerCollectionTypes();

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
    Dictionary<Element, IRTypeFlowData*> propagationMap;

    // Mapping from function to return value propagation information
    Dictionary<IRInst*, IRTypeFlowData*> funcReturnInfo;

    // Mapping from functions to call-sites.
    Dictionary<IRInst*, HashSet<Element>> funcCallSites;

    // Unique ID assignment for functions and witness tables
    Dictionary<IRInst*, UInt> uniqueIds;
    UInt nextUniqueId = 1;

    // Mapping from lowered instruction to their any-value types
    Dictionary<IRInst*, IRInst*> loweredInstToAnyValueType;

    // Set of open contexts
    HashSet<IRInst*> availableContexts;

    // Contexts requiring lowering
    HashSet<IRInst*> contextsToLower;

    // Lowered contexts.
    Dictionary<IRInst*, IRInst*> loweredContexts;
};

// Main entry point
bool lowerDynamicInsts(IRModule* module, DiagnosticSink* sink)
{
    DynamicInstLoweringContext context(module, sink);
    return context.processModule();
}

} // namespace Slang
