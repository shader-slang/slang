#include "slang-ir-lower-dynamic-insts.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-clone.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize.h"
#include "slang-ir-util.h"
#include "slang-ir-witness-table-wrapper.h"
#include "slang-ir.h"


namespace Slang
{

constexpr IRIntegerValue kDefaultAnyValueSize = 16;
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

// Helper to iterate over collection elements
template<typename F>
void forEachInCollection(IRCollectionBase* info, F func)
{
    for (UInt i = 0; i < info->getOperandCount(); ++i)
        func(info->getOperand(i));
}

template<typename F>
void forEachInCollection(IRCollectionTagType* tagType, F func)
{
    forEachInCollection(as<IRCollectionBase>(tagType->getOperand(0)), func);
}

static IRInst* findEntryInConcreteTable(IRInst* witnessTable, IRInst* key)
{
    if (auto concreteTable = as<IRWitnessTable>(witnessTable))
        for (auto entry : concreteTable->getEntries())
            if (entry->getRequirementKey() == key)
                return entry->getSatisfyingVal();
    return nullptr; // Not found
}

struct DynamicInstLoweringContext
{
    // Helper methods for creating canonical collections
    IRCollectionBase* createCollection(IROp op, const HashSet<IRInst*>& elements)
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

        List<IRInst*> sortedElements;
        for (auto element : elements)
            sortedElements.add(element);

        // Sort elements by their unique IDs to ensure canonical ordering
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
        if (as<IRGeneric>(inst))
            return kIROp_GenericCollection;

        if (as<IRTypeKind>(inst->getDataType()))
            return kIROp_TypeCollection;
        else if (as<IRFuncType>(inst->getDataType()))
            return kIROp_FuncCollection;
        else if (as<IRType>(inst) && !as<IRInterfaceType>(inst))
            return kIROp_TypeCollection;
        else if (as<IRWitnessTableType>(inst->getDataType()))
            return kIROp_TableCollection;
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

    IRCollectionTagType* makeTagType(IRCollectionBase* collection)
    {
        IRInst* collectionInst = collection;
        // Create the tag type from the collection
        IRBuilder builder(module);
        return as<IRCollectionTagType>(
            builder.emitIntrinsicInst(nullptr, kIROp_CollectionTagType, 1, &collectionInst));
    }

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

    UCount getCollectionCount(IRCollectionTagType* tagType)
    {
        auto collection = tagType->getOperand(0);
        return getCollectionCount(as<IRCollectionBase>(collection));
    }

    IRInst* getCollectionElement(IRCollectionBase* collection, UInt index)
    {
        if (!collection || index >= collection->getOperandCount())
            return nullptr;
        return collection->getOperand(index);
    }

    IRInst* getCollectionElement(IRCollectionTagType* collectionTagType, UInt index)
    {
        auto typeCollection = collectionTagType->getOperand(0);
        return getCollectionElement(as<IRCollectionBase>(typeCollection), index);
    }

    IRUnboundedCollection* makeUnbounded()
    {
        IRBuilder builder(module);
        return as<IRUnboundedCollection>(
            builder.emitIntrinsicInst(nullptr, kIROp_UnboundedCollection, 0, nullptr));
    }

    IRTypeFlowData* none() { return nullptr; }

    // Helper to convert collection to HashSet
    HashSet<IRInst*> collectionToHashSet(IRCollectionBase* info)
    {
        HashSet<IRInst*> result;
        forEachInCollection(info, [&](IRInst* element) { result.add(element); });
        return result;
    }

    IRInst* tryGetInfo(Element element)
    {
        // For non-global instructions, look up in the map
        auto found = propagationMap.tryGetValue(element);
        if (found)
            return *found;
        return none();
    }

    IRInst* tryGetInfo(IRInst* context, IRInst* inst)
    {
        if (auto typeFlowData = as<IRTypeFlowData>(inst->getDataType()))
        {
            // If the instruction already has a stablilized type flow data,
            // return it directly.
            //
            return typeFlowData;
        }

        if (!inst->getParent())
            return none();

        // If this is a global instruction (parent is module), return concrete info
        if (as<IRModuleInst>(inst->getParent()))
        {
            if (as<IRType>(inst) || as<IRWitnessTable>(inst) || as<IRFunc>(inst) ||
                as<IRGeneric>(inst))
            {
                // We won't directly handle interface types, but rather treat objects of interface
                // type as objects that can be specialized with collections.
                //
                if (as<IRInterfaceType>(inst))
                    return none();

                if (as<IRGeneric>(inst) && as<IRInterfaceType>(getGenericReturnVal(inst)))
                    return none();

                return makeSingletonSet(inst);
            }
            else
                return none();
        }

        return tryGetInfo(Element(context, inst));
    }

    IRInst* tryGetFuncReturnInfo(IRFunc* func)
    {
        auto found = funcReturnInfo.tryGetValue(func);
        if (found)
            return *found;
        return none();
    }

    // Centralized method to update propagation info and manage work queue
    // Use this when you want to propagate new information to an existing instruction
    // This will union the new info with existing info and add users to work queue if changed
    void updateInfo(IRInst* context, IRInst* inst, IRInst* newInfo, LinkedList<WorkItem>& workQueue)
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
        IRInst* info,
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
    void updateFuncReturnInfo(IRInst* callable, IRInst* returnInfo, LinkedList<WorkItem>& workQueue)
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

    IRIntegerValue getInterfaceAnyValueSize(IRInst* type)
    {
        if (auto decor = type->findDecoration<IRAnyValueSizeDecoration>())
        {
            return decor->getSize();
        }

        // We could conceivably make it an error to have an interface
        // without an `[anyValueSize(...)]` attribute, but then we risk
        // producing error messages even when doing 100% static specialization.
        //
        // It is simpler to use a reasonable default size and treat any
        // type without an explicit attribute as using that size.
        //
        return kDefaultAnyValueSize;
    }

    IRType* lowerInterfaceType(IRInterfaceType* interfaceType)
    {
        IRBuilder builder(module);
        auto anyValueType = builder.getAnyValueType(getInterfaceAnyValueSize(interfaceType));
        auto witnessTableType = builder.getWitnessTableIDType((IRType*)interfaceType);
        auto rttiType = builder.getRTTIHandleType();
        return builder.getTupleType({rttiType, witnessTableType, anyValueType});
    }

    IRInst* upcastCollection(IRInst* context, IRInst* arg, IRType* destInfo)
    {
        auto argInfo = arg->getDataType();
        if (!argInfo || !destInfo)
            return arg;

        if (as<IRTupleType>(argInfo) && as<IRTupleType>(destInfo))
        {
            auto argTupleType = as<IRTupleType>(argInfo);
            auto destTupleType = as<IRTupleType>(destInfo);

            List<IRInst*> upcastedElements;
            bool hasUpcastedElements = false;

            IRBuilder builder(module);
            builder.setInsertAfter(arg);

            // Upcast each element of the tuple
            for (UInt i = 0; i < argTupleType->getOperandCount(); ++i)
            {
                auto argElementType = argTupleType->getOperand(i);
                auto destElementType = destTupleType->getOperand(i);

                // If the element types are different, we need to reinterpret
                if (argElementType != destElementType)
                {
                    hasUpcastedElements = true;
                    upcastedElements.add(upcastCollection(
                        context,
                        builder.emitGetTupleElement((IRType*)argElementType, arg, i),
                        (IRType*)destElementType));
                }
                else
                {
                    upcastedElements.add(
                        builder.emitGetTupleElement((IRType*)argElementType, arg, i));
                }
            }

            if (hasUpcastedElements)
            {
                return builder.emitMakeTuple(upcastedElements);
            }
        }
        else if (as<IRCollectionTagType>(argInfo) && as<IRCollectionTagType>(destInfo))
        {
            if (getCollectionCount(as<IRCollectionTagType>(argInfo)) !=
                getCollectionCount(as<IRCollectionTagType>(destInfo)))
            {
                IRBuilder builder(module);
                builder.setInsertAfter(arg);
                return builder
                    .emitIntrinsicInst((IRType*)destInfo, kIROp_GetTagForSuperCollection, 1, &arg);
            }
        }
        else if (as<IRCollectionBase>(argInfo) && as<IRCollectionBase>(destInfo))
        {
            if (getCollectionCount(as<IRCollectionBase>(argInfo)) !=
                getCollectionCount(as<IRCollectionBase>(destInfo)))
            {
                // If the sets of witness tables are not equal, reinterpret to the parameter type
                IRBuilder builder(module);
                builder.setInsertAfter(arg);
                return builder.emitReinterpret((IRType*)destInfo, arg);
            }
        }
        else if (!as<IRCollectionBase>(argInfo) && as<IRCollectionBase>(destInfo))
        {
            IRBuilder builder(module);
            builder.setInsertAfter(arg);
            return builder.emitPackAnyValue((IRType*)destInfo, arg);
        }
        else if (as<IRInterfaceType>(argInfo) && as<IRCollectionTaggedUnionType>(destInfo))
        {
            auto loweredInterfaceType = lowerInterfaceType(as<IRInterfaceType>(argInfo));
            IRBuilder builder(module);
            builder.setInsertAfter(arg);
            auto witnessTable =
                builder.emitGetTupleElement(builder.getWitnessTableIDType(argInfo), arg, 1);
            auto tableID = builder.emitGetSequentialIDInst(witnessTable);
            auto tableCollection = cast<IRTableCollection>(destInfo->getOperand(1));
            auto typeCollection = cast<IRTypeCollection>(destInfo->getOperand(0));

            List<IRInst*> getTagOperands;
            getTagOperands.add(argInfo);
            getTagOperands.add(tableID);
            auto tableTag = builder.emitIntrinsicInst(
                (IRType*)makeTagType(tableCollection),
                kIROp_GetTagFromSequentialID,
                getTagOperands.getCount(),
                getTagOperands.getBuffer());

            return builder.emitMakeTuple(
                {tableTag,
                 builder.emitReinterpret(
                     (IRType*)typeCollection,
                     builder.emitGetTupleElement(
                         (IRType*)loweredInterfaceType->getOperand(0),
                         arg,
                         2))});
        }

        return arg; // Can use as-is.
    }

    void processInstForPropagation(IRInst* context, IRInst* inst, LinkedList<WorkItem>& workQueue)
    {
        IRInst* info;

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
        case kIROp_RWStructuredBufferLoad:
        case kIROp_StructuredBufferLoad:
            info = analyzeLoad(context, inst);
            break;
        case kIROp_Store:
            info = analyzeStore(context, as<IRStore>(inst), workQueue);
            break;
        case kIROp_GetElementPtr:
            info = analyzeGetElementPtr(context, as<IRGetElementPtr>(inst));
            break;
        default:
            info = analyzeDefault(context, inst);
            break;
        }

        updateInfo(context, inst, info, workQueue);
    }

    IRInst* analyzeCreateExistentialObject(IRInst* context, IRCreateExistentialObject* inst)
    {
        //
        // TODO: Actually use the integer<->type map present in the linkage to
        // extract a set of possible witness tables (if the index is a compile-time constant).
        //

        if (auto interfaceType = as<IRInterfaceType>(inst->getDataType()))
        {
            if (isComInterfaceType(interfaceType) || isBuiltin(interfaceType))
            {
                // If this is a COM interface, we treat it as unbounded
                return makeUnbounded();
            }

            auto tables = collectExistentialTables(interfaceType);
            if (tables.getCount() > 0)
                return makeExistential(
                    as<IRTableCollection>(createCollection(kIROp_TableCollection, tables)));
            else
                return none();
        }

        return none();
    }

    IRInst* analyzeMakeExistential(IRInst* context, IRMakeExistential* inst)
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

        if (as<IRWitnessTable>(witnessTable))
            return makeExistential(as<IRTableCollection>(makeSingletonSet(witnessTable)));

        if (auto collectionTag = as<IRCollectionTagType>(witnessTableInfo))
            return makeExistential(cast<IRTableCollection>(collectionTag->getOperand(0)));

        SLANG_UNEXPECTED("Unexpected witness table info type in analyzeMakeExistential");
    }

    bool isResourcePointer(IRInst* inst)
    {
        return isPointerToResourceType(inst->getDataType()) ||
               inst->getOp() == kIROp_RWStructuredBufferGetElementPtr;
    }

    IRInst* analyzeLoad(IRInst* context, IRInst* inst)
    {
        // Default: Transfer the prop info from the address to the loaded value
        if (auto loadInst = as<IRLoad>(inst))
        {
            if (isResourcePointer(loadInst->getPtr()))
            {
                if (auto interfaceType = as<IRInterfaceType>(loadInst->getDataType()))
                {
                    if (!isComInterfaceType(interfaceType) && !isBuiltin(interfaceType))
                    {
                        auto tables = collectExistentialTables(interfaceType);
                        if (tables.getCount() > 0)
                            return makeExistential(
                                as<IRTableCollection>(
                                    createCollection(kIROp_TableCollection, tables)));
                        else
                            return none();
                    }
                    else
                    {
                        return makeUnbounded();
                    }
                }
            }

            // If the load is from a pointer, we can transfer the info directly
            auto address = as<IRLoad>(loadInst)->getPtr();
            if (auto addrInfo = tryGetInfo(context, address))
                return as<IRPtrTypeBase>(addrInfo)->getValueType();
            else
                return none(); // No info for the address
        }
        else if (as<IRRWStructuredBufferLoad>(inst) || as<IRStructuredBufferLoad>(inst))
        {
            if (auto interfaceType = as<IRInterfaceType>(inst->getDataType()))
            {
                if (!isComInterfaceType(interfaceType) && !isBuiltin(interfaceType))
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
        }

        return none(); // No info for other load types
    }

    IRInst* analyzeStore(IRInst* context, IRStore* storeInst, LinkedList<WorkItem>& workQueue)
    {
        // Transfer the prop info from stored value to the address
        auto address = storeInst->getPtr();
        if (auto valInfo = tryGetInfo(context, storeInst->getVal()))
        {
            IRBuilder builder(module);
            auto ptrInfo = builder.getPtrTypeWithAddressSpace(
                (IRType*)valInfo,
                as<IRPtrTypeBase>(address->getDataType()));

            // Update the base instruction for the entire access chain
            maybeUpdatePtr(context, address, ptrInfo, workQueue);
        }

        return none(); // The store itself doesn't have any info.
    }

    IRInst* analyzeGetElementPtr(IRInst* context, IRGetElementPtr* getElementPtr)
    {
        // The base info should be in Ptr<Array<T>> form, so we just need to unpack and
        // return Ptr<T> as the result.
        //
        IRBuilder builder(module);
        builder.setInsertAfter(getElementPtr);
        auto basePtr = getElementPtr->getBase();
        if (auto ptrType = as<IRPtrTypeBase>(tryGetInfo(context, basePtr)))
        {
            auto arrayType = as<IRArrayType>(ptrType->getValueType());
            SLANG_ASSERT(arrayType);
            return builder.getPtrTypeWithAddressSpace(arrayType->getElementType(), ptrType);
        }

        return none(); // No info for the base pointer
    }

    IRInst* analyzeLookupWitnessMethod(IRInst* context, IRLookupWitnessMethod* inst)
    {
        auto key = inst->getRequirementKey();

        auto witnessTable = inst->getWitnessTable();
        auto witnessTableInfo = tryGetInfo(context, witnessTable);

        if (!witnessTableInfo)
            return none();

        if (as<IRUnboundedCollection>(witnessTableInfo))
            return makeUnbounded();

        if (auto tagType = as<IRCollectionTagType>(witnessTableInfo))
        {
            HashSet<IRInst*> results;
            forEachInCollection(
                cast<IRTableCollection>(tagType->getOperand(0)),
                [&](IRInst* table) { results.add(findEntryInConcreteTable(table, key)); });
            return makeTagType(makeSet(results));
        }

        SLANG_UNEXPECTED("Unexpected witness table info type in analyzeLookupWitnessMethod");
    }

    IRInst* analyzeExtractExistentialWitnessTable(
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
            return makeTagType(cast<IRTableCollection>(taggedUnion->getOperand(1)));

        SLANG_UNEXPECTED("Unhandled info type in analyzeExtractExistentialWitnessTable");
    }

    IRInst* analyzeExtractExistentialType(IRInst* context, IRExtractExistentialType* inst)
    {
        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(context, operand);

        if (!operandInfo)
            return none();

        if (as<IRUnboundedCollection>(operandInfo))
            return makeUnbounded();

        if (auto taggedUnion = as<IRCollectionTaggedUnionType>(operandInfo))
            return makeTagType(cast<IRTypeCollection>(taggedUnion->getOperand(0)));

        SLANG_UNEXPECTED("Unhandled info type in analyzeExtractExistentialType");
    }

    IRInst* analyzeExtractExistentialValue(IRInst* context, IRExtractExistentialValue* inst)
    {
        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(context, operand);

        if (!operandInfo)
            return none();

        if (as<IRUnboundedCollection>(operandInfo))
            return makeUnbounded();

        if (auto taggedUnion = as<IRCollectionTaggedUnionType>(operandInfo))
            return cast<IRTypeCollection>(taggedUnion->getOperand(0));
    }

    IRInst* analyzeSpecialize(IRInst* context, IRSpecialize* inst)
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

        if (as<IRCollectionTagType>(operandInfo) || as<IRCollectionBase>(operandInfo))
        {
            // If any of the specialization arguments need a tag (or the generic itself is a tag),
            // we need the result to also be wrapped in a tag type.
            bool needsTag = false;

            List<IRInst*> specializationArgs;
            for (auto i = 0; i < inst->getArgCount(); ++i)
            {
                // For concrete args, add as-is.
                if (isGlobalInst(inst->getArg(i)))
                {
                    specializationArgs.add(inst->getArg(i));
                    continue;
                }

                // For dynamic args, we need to replace them with
                // their sets (if available)
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

                if (auto argCollectionTag = as<IRCollectionTagType>(argInfo))
                {
                    if (getCollectionCount(argCollectionTag) == 1)
                        specializationArgs.add(getCollectionElement(argCollectionTag, 0));
                    else
                    {
                        needsTag = true;
                        specializationArgs.add(
                            cast<IRCollectionBase>(argCollectionTag->getOperand(0)));
                    }
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
                        if (auto infoCollectionTag = as<IRCollectionTagType>(info))
                        {
                            if (getCollectionCount(infoCollectionTag) == 1)
                                return getCollectionElement(infoCollectionTag, 0);
                            else
                            {
                                return as<IRCollectionBase>(infoCollectionTag->getOperand(0));
                            }
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

            IRCollectionBase* collection = nullptr;
            if (auto _collection = as<IRCollectionBase>(operandInfo))
            {
                collection = _collection;
            }
            else if (auto collectionTagType = as<IRCollectionTagType>(operandInfo))
            {
                needsTag = true;
                collection = cast<IRCollectionBase>(collectionTagType->getOperand(0));
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

            if (needsTag)
                return makeTagType(makeSet(specializedSet));
            else
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
                            updateInfo(context, param, makeTagType(collection), workQueue);
                        }
                        else if (as<IRType>(arg) || as<IRWitnessTable>(arg))
                        {
                            updateInfo(
                                context,
                                param,
                                makeTagType(makeSingletonSet(arg)),
                                workQueue);
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

    IRInst* analyzeCall(IRInst* context, IRCall* inst, LinkedList<WorkItem>& workQueue)
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

        if (auto collectionTag = as<IRCollectionTagType>(calleeInfo))
        {
            // If we have a set of functions, register each one
            forEachInCollection(collectionTag, [&](IRInst* func) { propagateToCallSite(func); });
        }
        else if (auto collection = as<IRFuncCollection>(calleeInfo))
        {
            // If we have a collection of functions, register each one
            forEachInCollection(collection, [&](IRInst* func) { propagateToCallSite(func); });
        }

        if (auto callInfo = tryGetInfo(context, inst))
            return callInfo;
        else
            return none();
    }

    void maybeUpdatePtr(
        IRInst* context,
        IRInst* inst,
        IRInst* info,
        LinkedList<WorkItem>& workQueue)
    {
        if (auto getElementPtr = as<IRGetElementPtr>(inst))
        {
            if (auto thisPtrInfo = as<IRPtrTypeBase>(info))
            {
                auto thisValueType = thisPtrInfo->getValueType();

                IRInst* baseValueType =
                    as<IRPtrTypeBase>(getElementPtr->getBase()->getDataType())->getValueType();
                SLANG_ASSERT(as<IRArrayType>(baseValueType));

                // Propagate 'this' information to the base by wrapping it as a pointer to array.
                IRBuilder builder(module);
                auto baseInfo = builder.getPtrTypeWithAddressSpace(
                    builder.getArrayType(
                        (IRType*)thisValueType,
                        as<IRArrayType>(baseValueType)->getElementCount()),
                    as<IRPtrTypeBase>(getElementPtr->getBase()->getDataType()));
                maybeUpdatePtr(context, getElementPtr->getBase(), baseInfo, workQueue);
            }
        }
        else if (auto var = as<IRVar>(inst))
        {
            updateInfo(context, var, info, workQueue);
        }
        else
        {
            // Do nothing..
            return;
        }
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

    bool isGlobalInst(IRInst* inst) { return inst->getParent()->getOp() == kIROp_ModuleInst; }

    List<IRType*> getParamEffectiveTypes(IRInst* context)
    {
        List<IRType*> effectiveTypes;
        IRFunc* func = nullptr;
        if (as<IRFunc>(context))
        {
            func = as<IRFunc>(context);
        }
        else if (auto specialize = as<IRSpecialize>(context))
        {
            auto generic = specialize->getBase();
            auto innerFunc = getGenericReturnVal(generic);
            func = cast<IRFunc>(innerFunc);
        }
        else
        {
            // If it's not a function or a specialization, we can't get parameter info
            SLANG_UNEXPECTED("Unexpected context type for parameter info retrieval");
        }

        UIndex idx = 0;
        for (auto param : func->getParams())
        {
            /*if (auto newType = tryGetInfo(context, param))
                effectiveTypes.add((IRType*)newType);
            else
            {
                const auto [direction, type] = getParameterDirectionAndType(
                    as<IRFuncType>(context->getDataType())->getParamType(idx));
                SLANG_ASSERT(isGlobalInst(type));
                effectiveTypes.add((IRType*)type);
            }*/
            if (auto newType = tryGetInfo(context, param))
                effectiveTypes.add((IRType*)newType);
            else
                effectiveTypes.add(
                    (IRType*)as<IRFuncType>(context->getDataType())->getParamType(idx));
            idx++;
        }

        return effectiveTypes;
    }

    List<IRInst*> getParamInfos(IRInst* context)
    {
        List<IRInst*> infos;
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
                            const auto [paramDirection, paramType] =
                                getParameterDirectionAndType(param->getDataType());

                            // Only update if the parameter is abstract type.
                            if (isGlobalInst(paramType) && !(as<IRInterfaceType>(paramType)))
                            {
                                argIndex++;
                                continue;
                            }

                            switch (paramDirection)
                            {
                            case kParameterDirection_Out:
                            case kParameterDirection_InOut:
                                {
                                    IRBuilder builder(module);
                                    auto newInfo = fromDirectionAndType(
                                        &builder,
                                        paramDirection,
                                        as<IRPtrTypeBase>(argInfo)->getValueType());
                                    updateInfo(edge.targetContext, param, newInfo, workQueue);
                                    break;
                                }
                            case kParameterDirection_In:
                                {
                                    // Use centralized update method
                                    updateInfo(edge.targetContext, param, argInfo, workQueue);
                                    break;
                                }
                            default:
                                SLANG_UNEXPECTED(
                                    "Unhandled parameter direction in interprocedural edge");
                            }
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
                        auto arg = callInst->getArg(argIndex);
                        auto argPtrType = as<IRPtrTypeBase>(arg->getDataType());

                        IRBuilder builder(module);
                        updateInfo(
                            edge.callerContext,
                            builder.getPtrTypeWithAddressSpace((IRType*)paramInfo, argPtrType),
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

    IRInst* getFuncReturnInfo(IRInst* callee)
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
                if (isComInterfaceType(interfaceType) || isBuiltin(interfaceType))
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

    template<typename T>
    T* unionCollection(T* collection1, T* collection2)
    {
        SLANG_ASSERT(as<IRCollectionBase>(collection1) && as<IRCollectionBase>(collection2));
        SLANG_ASSERT(collection1->getOp() == collection2->getOp());

        if (!collection1)
            return collection2;
        if (!collection2)
            return collection1;
        if (collection1 == collection2)
            return collection1;

        HashSet<IRInst*> allValues;
        // Collect all values from both collections
        forEachInCollection(collection1, [&](IRInst* value) { allValues.add(value); });
        forEachInCollection(collection2, [&](IRInst* value) { allValues.add(value); });

        return as<T>(createCollection(
            collection1->getOp(),
            allValues)); // Create a new collection with the union of values
    }

    IRInst* unionPropagationInfo(IRInst* info1, IRInst* info2)
    {
        if (!info1)
            return info2;
        if (!info2)
            return info1;

        if (as<IRArrayType>(info1) && as<IRArrayType>(info2))
        {
            SLANG_ASSERT(info1->getOperand(1) == info2->getOperand(1));
            // If both are array types, union their element types
            IRBuilder builder(module);
            builder.setInsertInto(module);
            return builder.getArrayType(
                (IRType*)unionPropagationInfo(info1->getOperand(0), info2->getOperand(0)),
                info1->getOperand(1)); // Keep the same size
        }

        if (as<IRPtrTypeBase>(info1) && as<IRPtrTypeBase>(info2))
        {
            SLANG_ASSERT(info1->getOp() == info2->getOp());

            // If both are array types, union their element types
            IRBuilder builder(module);
            builder.setInsertInto(module);
            return builder.getPtrTypeWithAddressSpace(
                (IRType*)unionPropagationInfo(info1->getOperand(0), info2->getOperand(0)),
                as<IRPtrTypeBase>(info1));
        }

        if (as<IRUnboundedCollection>(info1) && as<IRUnboundedCollection>(info2))
        {
            // If either info is unbounded, the union is unbounded
            return makeUnbounded();
        }

        if (as<IRCollectionTaggedUnionType>(info1) && as<IRCollectionTaggedUnionType>(info2))
        {
            return makeExistential(
                unionCollection<IRTableCollection>(
                    cast<IRTableCollection>(info1->getOperand(1)),
                    cast<IRTableCollection>(info2->getOperand(1))));
        }

        if (as<IRCollectionTagType>(info1) && as<IRCollectionTagType>(info2))
        {
            return makeTagType(
                unionCollection<IRCollectionBase>(
                    cast<IRCollectionBase>(info1->getOperand(0)),
                    cast<IRCollectionBase>(info2->getOperand(0))));
        }

        if (as<IRCollectionBase>(info1) && as<IRCollectionBase>(info2))
        {
            return unionCollection<IRCollectionBase>(
                cast<IRCollectionBase>(info1),
                cast<IRCollectionBase>(info2));
        }

        SLANG_UNEXPECTED("Unhandled propagation info types in unionPropagationInfo");
    }

    IRTypeFlowData* analyzeDefault(IRInst* context, IRInst* inst)
    {
        // Check if this is a global concrete type, witness table, or function.
        // If so, it's a concrete element. We'll create a singleton set for it.
        if (isGlobalInst(inst) &&
            (!as<IRInterfaceType>(inst) &&
             (as<IRType>(inst) || as<IRWitnessTable>(inst) || as<IRFunc>(inst))))
            return makeSingletonSet(inst);

        auto instType = inst->getDataType();
        if (isGlobalInst(inst))
        {
            if (as<IRType>(instType) && !(as<IRInterfaceType>(instType)))
                return none(); // We'll avoid storing propagation info for concrete insts. (can just
                               // use the inst directly)

            if (as<IRInterfaceType>(instType))
            {
                // As a general rule, if none of the non-default cases handled this inst that is
                // producing an existential type, then we assume that we can't constrain it
                //
                return makeUnbounded();
            }
        }

        return none(); // Default case, no propagation info
    }

    bool lowerInstsInBlock(IRInst* context, IRBlock* block)
    {
        List<IRInst*> instsToLower;
        bool hasChanges = false;
        for (auto inst : block->getChildren())
            instsToLower.add(inst);

        UIndex paramIndex = 0;
        for (auto inst : instsToLower)
            hasChanges |= lowerInst(context, inst);

        return hasChanges;
    }

    bool lowerFunc(IRFunc* func)
    {
        bool hasChanges = false;
        for (auto block : func->getBlocks())
            hasChanges |= lowerInstsInBlock(func, block);

        for (auto block : func->getBlocks())
        {
            UIndex paramIndex = 0;
            // Process each parameter in this block (these are phi parameters)
            for (auto param : block->getParams())
            {
                auto paramInfo = tryGetInfo(param);
                if (!paramInfo)
                {
                    paramIndex++;
                    continue;
                }

                // Find all predecessors of this block
                for (auto pred : block->getPredecessors())
                {
                    auto terminator = pred->getTerminator();
                    if (auto unconditionalBranch = as<IRUnconditionalBranch>(terminator))
                    {
                        auto arg = unconditionalBranch->getArg(paramIndex);
                        auto newArg = upcastCollection(func, arg, param->getDataType());

                        if (newArg != arg)
                        {
                            hasChanges = true;
                            // Replace the argument in the branch instruction
                            SLANG_ASSERT(!as<IRLoop>(unconditionalBranch));
                            unconditionalBranch->setOperand(1 + paramIndex, newArg);
                        }
                    }
                }

                paramIndex++;
            }

            // Is the terminator a return instruction?
            if (auto returnInst = as<IRReturn>(block->getTerminator()))
            {
                if (!as<IRVoidType>(returnInst->getVal()->getDataType()))
                {
                    if (auto loweredType = getLoweredType(getFuncReturnInfo(func)))
                    {
                        auto newReturnVal =
                            upcastCollection(func, returnInst->getVal(), loweredType);
                        if (newReturnVal != returnInst->getVal())
                        {
                            // Replace the return value with the reinterpreted value
                            hasChanges = true;
                            returnInst->setOperand(0, newReturnVal);
                        }
                    }
                }
            }
        }

        auto effectiveFuncType = getEffectiveFuncType(func);
        if (effectiveFuncType != func->getFullType())
        {
            hasChanges = true;
            func->setFullType(effectiveFuncType);
        }

        return hasChanges;
    }

    bool performDynamicInstLowering()
    {
        List<IRFunc*> funcsToProcess;

        for (auto globalInst : module->getGlobalInsts())
            if (auto func = as<IRFunc>(globalInst))
            {
                funcsToProcess.add(func);
            }

        bool hasChanges = false;
        do
        {
            while (funcsToProcess.getCount() > 0)
            {
                auto func = funcsToProcess.getLast();
                funcsToProcess.removeLast();

                // Lower the instructions in the function
                hasChanges |= lowerFunc(func);
            }
        } while (funcsToProcess.getCount() > 0);

        return hasChanges;
    }

    IRType* getTypeForExistential(IRCollectionTaggedUnionType* taggedUnion)
    {
        // Replace type with Tuple<CollectionTagType(collection), TypeCollection>
        IRBuilder builder(module);
        builder.setInsertInto(module);

        auto typeCollection = cast<IRTypeCollection>(taggedUnion->getOperand(0));
        auto tableCollection = cast<IRTableCollection>(taggedUnion->getOperand(1));

        if (getCollectionCount(typeCollection) == 1)
            return builder.getTupleType(
                List<IRType*>(
                    {(IRType*)makeTagType(tableCollection),
                     (IRType*)getCollectionElement(typeCollection, 0)}));

        return builder.getTupleType(
            List<IRType*>({(IRType*)makeTagType(tableCollection), (IRType*)typeCollection}));
    }

    IRType* getLoweredType(IRInst* info)
    {
        if (!info)
            return nullptr;

        if (as<IRUnboundedCollection>(info))
            return nullptr;

        if (auto ptrType = as<IRPtrTypeBase>(info))
        {
            IRBuilder builder(module);
            return builder.getPtrTypeWithAddressSpace(
                (IRType*)getLoweredType(ptrType->getValueType()),
                ptrType);
        }

        if (auto arrayType = as<IRArrayType>(info))
        {
            IRBuilder builder(module);
            return builder.getArrayType(
                (IRType*)getLoweredType(arrayType->getElementType()),
                arrayType->getElementCount());
        }

        if (auto taggedUnion = as<IRCollectionTaggedUnionType>(info))
        {
            // If this is a tagged union, we need to create a tuple type
            return getTypeForExistential(taggedUnion);
        }

        if (auto collectionTag = as<IRCollectionTagType>(info))
        {
            // If this is a collection tag, we can return the collection type
            return (IRType*)collectionTag;
        }

        if (auto collection = as<IRTypeCollection>(info))
        {
            if (getCollectionCount(collection) == 1)
            {
                // If there's only one type in the collection, return it directly
                return (IRType*)getCollectionElement(collection, 0);
            }

            // If this is a concrete collection, return it directly
            return (IRType*)collection;
        }

        if (as<IRFuncCollection>(info) || as<IRTableCollection>(info))
        {
            // Don't lower these collections.. they should be used through
            // tag types, or be processed out during lowering.
            //
            return nullptr;
        }

        return (IRType*)info;
        // SLANG_UNEXPECTED("Unhandled IRTypeFlowData type in getLoweredType");
    }

    bool replaceType(IRInst* context, IRInst* inst)
    {
        if (auto info = tryGetInfo(context, inst))
        {
            if (auto loweredType = getLoweredType(info))
            {
                if (loweredType == inst->getDataType())
                    return false; // No change
                inst->setFullType(loweredType);
                return true;
            }
        }
        return false;
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
        case kIROp_RWStructuredBufferLoad:
        case kIROp_StructuredBufferLoad:
            return lowerStructuredBufferLoad(context, inst);
        case kIROp_Load:
            return lowerLoad(context, inst);
        case kIROp_Store:
            return lowerStore(context, as<IRStore>(inst));
        default:
            {
                if (auto info = tryGetInfo(context, inst))
                    return replaceType(context, inst);
                return false;
            }
        }
    }

    bool lowerLookupWitnessMethod(IRInst* context, IRLookupWitnessMethod* inst)
    {
        // Handle trivial case.
        if (auto witnessTable = as<IRWitnessTable>(inst->getWitnessTable()))
        {
            inst->replaceUsesWith(
                findEntryInConcreteTable(witnessTable, inst->getRequirementKey()));
            inst->removeAndDeallocate();
            return true;
        }

        auto info = tryGetInfo(context, inst);
        if (!info)
            return false;

        auto collectionTagType = as<IRCollectionTagType>(info);
        if (!collectionTagType)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        if (getCollectionCount(collectionTagType) == 1)
        {
            // Found a single possible type. Simple replacement.
            inst->replaceUsesWith(getCollectionElement(collectionTagType, 0));
            inst->removeAndDeallocate();
            return true;
        }

        if (auto typeCollection = as<IRTypeCollection>(collectionTagType->getOperand(0)))
        {
            // If this is a type collection, we can replace it with the collection type
            // We don't currently care about the tag of a type.
            //
            inst->replaceUsesWith(typeCollection);
            inst->removeAndDeallocate();
            return true;
        }

        // Get the witness table operand info
        auto witnessTableInst = inst->getWitnessTable();
        auto witnessTableInfo = tryGetInfo(context, witnessTableInst);

        SLANG_ASSERT(as<IRCollectionTagType>(witnessTableInfo));
        List<IRInst*> operands = {witnessTableInst, inst->getRequirementKey()};

        auto newInst = builder.emitIntrinsicInst(
            (IRType*)info,
            kIROp_GetTagForMappedCollection,
            operands.getCount(),
            operands.getBuffer());
        inst->replaceUsesWith(newInst);
        propagationMap[Element(context, newInst)] = info;
        inst->removeAndDeallocate();

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

        auto collectionTagType = as<IRCollectionTagType>(info);
        if (!collectionTagType)
            return false;

        if (getCollectionCount(collectionTagType) == 1)
        {
            // Found a single possible type. Simple replacement.
            inst->replaceUsesWith(getCollectionElement(collectionTagType, 0));
            inst->removeAndDeallocate();
            return true;
        }
        else
        {
            // Replace with GetElement(loweredInst, 0) -> uint
            auto operand = inst->getOperand(0);
            auto element = builder.emitGetTupleElement((IRType*)collectionTagType, operand, 0);
            inst->replaceUsesWith(element);
            // propagationMap[Element(context, element)] = info;
            inst->removeAndDeallocate();
            return true;
        }
    }

    bool lowerExtractExistentialValue(IRInst* context, IRExtractExistentialValue* inst)
    {
        auto existential = inst->getOperand(0);
        auto existentialInfo = existential->getDataType();
        if (isTaggedUnionType(existentialInfo))
        {
            auto valType = existentialInfo->getOperand(1);
            IRBuilder builder(inst);
            builder.setInsertAfter(inst);

            auto val = builder.emitGetTupleElement((IRType*)valType, existential, 1);
            inst->replaceUsesWith(val);
            inst->removeAndDeallocate();
            return true;
        }

        return false;
        /*
        auto operandInfo = tryGetInfo(context, inst->getOperand(0));
        auto taggedUnion = as<IRCollectionTaggedUnionType>(operandInfo);
        if (!taggedUnion)
            return false;

        auto info = tryGetInfo(context, inst);
        auto typeCollection = as<IRTypeCollection>(info);
        if (!typeCollection)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Replace with GetElement(loweredInst, 1) : TypeCollection
        auto operand = inst->getOperand(0);
        auto element = builder.emitGetTupleElement((IRType*)info, operand, 1);
        inst->replaceUsesWith(element);
        inst->removeAndDeallocate();
        return true;*/
    }

    bool lowerExtractExistentialType(IRInst* context, IRExtractExistentialType* inst)
    {
        auto info = tryGetInfo(context, inst);
        auto collectionTagType = as<IRCollectionTagType>(info);
        if (!collectionTagType)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        if (getCollectionCount(collectionTagType) == 1)
        {
            // Found a single possible type. Simple replacement.
            auto singletonValue = getCollectionElement(collectionTagType, 0);
            inst->replaceUsesWith(singletonValue);
            inst->removeAndDeallocate();
            return true;
        }

        // Replace the instruction with the collection type.
        inst->replaceUsesWith(collectionTagType->getOperand(0));
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

    IRType* fromDirectionAndType(IRBuilder* builder, ParameterDirection direction, IRType* type)
    {
        switch (direction)
        {
        case ParameterDirection::kParameterDirection_In:
            return type;
        case ParameterDirection::kParameterDirection_Out:
            return builder->getOutType(type);
        case ParameterDirection::kParameterDirection_InOut:
            return builder->getInOutType(type);
        case ParameterDirection::kParameterDirection_ConstRef:
            return builder->getConstRefType(type);
        default:
            SLANG_UNEXPECTED("Unhandled parameter direction in fromDirectionAndType");
            return nullptr;
        }
    }

    bool isTaggedUnionType(IRInst* type)
    {
        if (auto tupleType = as<IRTupleType>(type))
            return as<IRCollectionTagType>(tupleType->getOperand(0)) != nullptr;

        return false;
    }

    IRType* updateType(IRType* currentType, IRType* newType)
    {
        // TODO: This is feeling very similar to the unionCollection logic.
        // Maybe unify?
        if (auto collection = as<IRCollectionBase>(currentType))
        {
            HashSet<IRInst*> collectionElements;
            forEachInCollection(
                collection,
                [&](IRInst* element) { collectionElements.add(element); });

            if (auto newCollection = as<IRCollectionBase>(newType))
            {
                // If the new type is also a collection, merge the two collections
                forEachInCollection(
                    newCollection,
                    [&](IRInst* element) { collectionElements.add(element); });
            }
            else
            {
                // Otherwise, just add the new type to the collection
                collectionElements.add(newType);
            }

            // If this is a collection, we need to create a new collection with the new type
            auto newCollection = createCollection(collection->getOp(), collectionElements);
            return (IRType*)newCollection;
        }
        else if (currentType == newType)
        {
            return currentType;
        }
        else if (currentType == nullptr)
        {
            return newType;
        }
        else if (isTaggedUnionType(currentType) && isTaggedUnionType(newType))
        {
            IRBuilder builder(module);
            // Merge the elements of both tagged unions into a new tuple type
            return builder.getTupleType(
                List<IRType*>(
                    {(IRType*)makeTagType(
                         as<IRCollectionBase>(updateType(
                             (IRType*)currentType->getOperand(0)->getOperand(0),
                             (IRType*)newType->getOperand(0)->getOperand(0)))),
                     (IRType*)updateType(
                         (IRType*)currentType->getOperand(1),
                         (IRType*)newType->getOperand(1))}));
        }
        else // Need to create a new collection.
        {
            HashSet<IRInst*> collectionElements;

            SLANG_ASSERT(!as<IRCollectionBase>(currentType) && !as<IRCollectionBase>(newType));

            collectionElements.add(currentType);
            collectionElements.add(newType);

            // If this is a collection, we need to create a new collection with the new type
            auto newCollection = createCollection(kIROp_TypeCollection, collectionElements);
            return (IRType*)newCollection;
        }
    }

    IRFuncType* getEffectiveFuncType(IRInst* callee)
    {
        IRBuilder builder(module);

        List<IRType*> paramTypes;
        IRType* resultType = nullptr;

        auto updateParamType = [&](UInt index, IRType* paramType) -> IRType*
        {
            if (paramTypes.getCount() <= index)
            {
                // If we don't have enough types, just add the new type
                paramTypes.add(paramType);
                return paramType;
            }
            else
            {
                // Otherwise, update the existing type
                auto [currentDirection, currentType] =
                    getParameterDirectionAndType(paramTypes[index]);
                auto [newDirection, newType] = getParameterDirectionAndType(paramType);
                auto updatedType = updateType(currentType, newType);
                SLANG_ASSERT(currentDirection == newDirection);
                paramTypes[index] = fromDirectionAndType(&builder, currentDirection, updatedType);
                return updatedType;
            }
        };

        List<IRInst*> contextsToProcess;
        if (auto collection = as<IRFuncCollection>(callee))
        {
            forEachInCollection(collection, [&](IRInst* func) { contextsToProcess.add(func); });
        }
        else if (auto collectionTagType = as<IRCollectionTagType>(callee->getDataType()))
        {
            forEachInCollection(
                collectionTagType,
                [&](IRInst* func) { contextsToProcess.add(func); });
        }
        else
        {
            // Otherwise, just process the single function
            contextsToProcess.add(callee);
        }

        for (auto context : contextsToProcess)
        {
            auto paramEffectiveTypes = getParamEffectiveTypes(context);
            auto paramDirections = getParamDirections(context);
            for (UInt i = 0; i < paramEffectiveTypes.getCount(); i++)
            {
                updateParamType(i, getLoweredType(paramEffectiveTypes[i]));
                /*if (auto collectionType = as<IRTypeFlowData>(paramEffectiveTypes[i]))

                else if (paramEffectiveTypes[i] != nullptr)
                    updateParamType(
                        i,
                        fromDirectionAndType(
                            &builder,
                            paramDirections[i],
                            (IRType*)paramEffectiveTypes[i]));*/
            }

            auto returnType = getFuncReturnInfo(context);
            if (auto newResultType = getLoweredType(returnType))
            {
                resultType = updateType(resultType, newResultType);
            }
            else if (auto funcType = as<IRFuncType>(context->getDataType()))
            {
                SLANG_ASSERT(isGlobalInst(funcType->getResultType()));
                resultType = updateType(resultType, funcType->getResultType());
            }
            else
            {
                SLANG_UNEXPECTED("Cannot determine result type for context");
            }
        }

        //
        // Add in extra parameter types for a call to the callee.
        //

        List<IRType*> extraParamTypes;
        // If the callee is a collection, then we need a tag as input.
        if (auto funcCollection = as<IRFuncCollection>(callee))
        {
            // If this is a non-trivial collection, we need to add a tag type for the collection
            // as the first parameter.
            if (getCollectionCount(funcCollection) > 1)
                extraParamTypes.add((IRType*)makeTagType(funcCollection));
            // extraParamTypes.add((IRType*)makeTagType(funcCollection));
        }

        // If the any of the elements in the callee (or the callee itself in case
        // of a singleton) is a dynamic specialization, each non-singleton TableCollection,
        // requries a corresponding tag input.
        //
        auto calleeToCheck = as<IRCollectionBase>(callee)
                                 ? getCollectionElement(as<IRCollectionBase>(callee), 0)
                                 : callee;
        if (isDynamicGeneric(calleeToCheck))
        {
            auto specializeInst = as<IRSpecialize>(calleeToCheck);

            // If this is a dynamic generic, we need to add a tag type for each
            // TableCollection in the callee.
            for (UIndex i = 0; i < specializeInst->getArgCount(); i++)
                if (auto tableCollection = as<IRTableCollection>(specializeInst->getArg(i)))
                    extraParamTypes.add((IRType*)makeTagType(tableCollection));
        }

        List<IRType*> allParamTypes;
        allParamTypes.addRange(extraParamTypes);
        allParamTypes.addRange(paramTypes);

        return builder.getFuncType(allParamTypes, resultType);
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

    List<IRInst*> getArgsForDynamicSpecialization(IRSpecialize* specializedCallee)
    {
        List<IRInst*> callArgs;
        for (auto ii = 0; ii < specializedCallee->getArgCount(); ii++)
        {
            auto specArg = specializedCallee->getArg(ii);
            auto argInfo = specArg->getDataType();

            // Pull all tag-type arguments from the specialization arguments
            // and add them to the call arguments.
            //
            if (as<IRCollectionTagType>(argInfo))
                callArgs.add(specArg);
        }

        return callArgs;
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
        IRInst* calleeTagInst = nullptr;

        // If we're calling using a tag, place a call to the collection,
        // with the tag as the first argument. So the callee is
        // the collection itself.
        //
        if (auto collectionTag = as<IRCollectionTagType>(callee->getDataType()))
        {
            if (getCollectionCount(collectionTag) > 1)
            {
                calleeTagInst = callee; // Only keep the tag if there are multiple elements.

                // If we're placing a specialized call, use the base tag since the
                // specialization arguments will also become arguments to the call.
                //
                if (auto specializedTag = as<IRSpecialize>(calleeTagInst))
                    calleeTagInst = specializedTag->getBase();
            }
            callee = collectionTag->getOperand(0);
        }

        // If by this point, we haven't resolved our callee into a global inst (
        // either a collection or a single function), then we can't lower it (likely unbounded)
        //
        if (!isGlobalInst(callee))
            return false;

        auto expectedFuncType = getEffectiveFuncType(callee);

        List<IRInst*> newArgs;
        IRInst* newCallee = nullptr;

        // Determine a new callee.
        auto calleeCollection = as<IRFuncCollection>(callee);
        if (!calleeCollection)
            newCallee = callee; // Not a collection, no need to lower
        else if (getCollectionCount(calleeCollection) == 1)
        {
            auto singletonValue = getCollectionElement(calleeCollection, 0);
            if (singletonValue == callee)
            {
                newCallee = callee;
            }
            else
            {
                if (isDynamicGeneric(singletonValue))
                    newArgs.addRange(
                        getArgsForDynamicSpecialization(cast<IRSpecialize>(inst->getCallee())));

                newCallee = singletonValue;
            }
        }
        else
        {
            // Multiple elements in the collection.
            if (calleeTagInst)
                newArgs.add(calleeTagInst);
            auto funcCollection = cast<IRFuncCollection>(calleeCollection);

            // Check if the first element is a dynamic generic (this should imply that all
            // elements are similar dynamic generics, but we might want to check for that..)
            //
            if (isDynamicGeneric(getCollectionElement(funcCollection, 0)))
            {
                auto dynamicSpecArgs =
                    getArgsForDynamicSpecialization(cast<IRSpecialize>(inst->getCallee()));
                for (auto& arg : dynamicSpecArgs)
                    newArgs.add(arg);
            }

            if (!as<IRFuncType>(funcCollection->getDataType()))
            {
                auto typeForCollection = getEffectiveFuncType(funcCollection);
                funcCollection->setFullType(typeForCollection);
            }

            newCallee = funcCollection;
        }

        // First, we'll legalize all operands by upcasting if necessary.
        // This needs to be done even if the callee is not a collection.
        //
        // List<IRTypeFlowData*> paramTypeFlows = getParamInfos(callee);
        // List<ParameterDirection> paramDirections = getParamDirections(callee);
        UCount extraArgCount = newArgs.getCount();
        for (UInt i = 0; i < inst->getArgCount(); i++)
        {
            auto arg = inst->getArg(i);
            const auto [paramDirection, paramType] =
                getParameterDirectionAndType(expectedFuncType->getParamType(i + extraArgCount));

            IRInst* newArg = nullptr;
            switch (paramDirection)
            {
            case kParameterDirection_In:
                newArgs.add(upcastCollection(context, arg, paramType));
                break;
            case kParameterDirection_Out:
            case kParameterDirection_InOut:
                {
                    auto argValueType = as<IRPtrTypeBase>(arg->getDataType())->getValueType();
                    if (argValueType != paramType)
                    {
                        SLANG_UNEXPECTED("ptr-typed parameters should have matching types");
                    }
                    else
                    {
                        newArgs.add(arg);
                    }
                    break;
                }
            default:
                SLANG_UNEXPECTED("Unhandled parameter direction in lowerCall");
            }
        }

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        bool changed = false;
        for (UInt i = 0; i < newArgs.getCount(); i++)
        {
            if (newArgs[i] != inst->getArg(i))
            {
                changed = true;
                break;
            }
        }

        if (newCallee != inst->getCallee())
        {
            changed = true;
        }

        if (changed)
        {
            auto newCall =
                builder.emitCallInst(expectedFuncType->getResultType(), newCallee, newArgs);
            inst->replaceUsesWith(newCall);
            inst->removeAndDeallocate();
            return true;
        }
        else if (expectedFuncType->getResultType() != inst->getDataType())
        {
            // If we didn't change the callee or the arguments, we still might
            // need to update the result type.
            //
            inst->setFullType(expectedFuncType->getResultType());
            return true;
        }
        else
        {
            // Nothing changed.
            return false;
        }
    }

    bool lowerMakeExistential(IRInst* context, IRMakeExistential* inst)
    {
        auto info = tryGetInfo(context, inst);
        auto taggedUnion = as<IRCollectionTaggedUnionType>(info);
        if (!taggedUnion)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Collect types from the witness tables to determine the any-value type
        auto tableCollection = as<IRTableCollection>(taggedUnion->getOperand(1));
        auto typeCollection = as<IRTypeCollection>(taggedUnion->getOperand(0));

        IRInst* witnessTableID = nullptr;
        if (auto witnessTable = as<IRWitnessTable>(inst->getWitnessTable()))
        {
            auto singletonTagType = makeTagType(makeSingletonSet(witnessTable));
            auto zeroValueOfTagType = builder.getIntValue((IRType*)singletonTagType, 0);
            witnessTableID = builder.emitIntrinsicInst(
                (IRType*)makeTagType(tableCollection),
                kIROp_GetTagForSuperCollection,
                1,
                &zeroValueOfTagType);
        }
        else if (
            auto witnessTableTag = as<IRCollectionTagType>(inst->getWitnessTable()->getDataType()))
        {
            // Dynamic. Use the witness table inst as a tag
            witnessTableID = inst->getWitnessTable();
        }

        // Create the appropriate any-value type
        auto collectionType = getCollectionCount(typeCollection) == 1
                                  ? (IRType*)typeCollection->getOperand(0)
                                  : (IRType*)typeCollection;

        // Pack the value
        auto packedValue = as<IRTypeCollection>(collectionType)
                               ? builder.emitPackAnyValue(collectionType, inst->getWrappedValue())
                               : inst->getWrappedValue();

        auto taggedUnionTupleType = getLoweredType(taggedUnion);

        // Create tuple (table_unique_id, PackAnyValue(val))
        IRInst* tupleArgs[] = {witnessTableID, packedValue};
        auto tuple = builder.emitMakeTuple(taggedUnionTupleType, 2, tupleArgs);

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

        auto taggedUnionTupleType = getLoweredType(taggedUnion);

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        List<IRInst*> args;
        args.add(inst->getDataType());
        args.add(inst->getTypeID());
        auto translatedTag = builder.emitIntrinsicInst(
            (IRType*)taggedUnionTupleType->getOperand(0),
            kIROp_GetTagFromSequentialID,
            args.getCount(),
            args.getBuffer());

        IRInst* packedValue = nullptr;
        if (auto collection = as<IRTypeCollection>(taggedUnionTupleType->getOperand(1)))
        {
            packedValue = builder.emitPackAnyValue((IRType*)collection, inst->getValue());
        }
        else
        {
            packedValue = builder.emitReinterpret(
                (IRType*)taggedUnionTupleType->getOperand(1),
                inst->getValue());
        }

        auto newInst = builder.emitMakeTuple(
            taggedUnionTupleType,
            List<IRInst*>({translatedTag, packedValue}));

        inst->replaceUsesWith(newInst);
        inst->removeAndDeallocate();
        return true;
    }

    bool lowerStructuredBufferLoad(IRInst* context, IRInst* inst)
    {
        auto valInfo = tryGetInfo(context, inst);

        if (!valInfo)
            return false;

        auto bufferType = (IRType*)inst->getOperand(0)->getDataType();
        auto bufferBaseType = (IRType*)bufferType->getOperand(0);

        if (bufferBaseType != (IRType*)getLoweredType(valInfo))
        {
            IRBuilder builder(inst);
            builder.setInsertAfter(inst);

            IRCloneEnv cloneEnv;
            auto newLoad = cloneInst(&cloneEnv, &builder, inst);

            auto loweredVal = upcastCollection(context, newLoad, (IRType*)valInfo);

            // TODO: this is a hack. Encode this in the type-flow-data.
            if (as<IRInterfaceType>(bufferBaseType) && !isComInterfaceType(inst->getDataType()) &&
                !isBuiltin(inst->getDataType()))
            {
                newLoad->setFullType(lowerInterfaceType(as<IRInterfaceType>(bufferBaseType)));
            }

            inst->replaceUsesWith(loweredVal);
            inst->removeAndDeallocate();
            return true;
        }
        else if (inst->getDataType() != bufferBaseType)
        {
            // If the data type is not the same, we need to update it.
            inst->setFullType((IRType*)getLoweredType(valInfo));
            return true;
        }
        else
        {
            // No change needed.
            return false;
        }
    }

    bool lowerLoad(IRInst* context, IRInst* inst)
    {
        auto valInfo = tryGetInfo(context, inst);

        if (!valInfo)
            return false;

        IRType* ptrValType = nullptr;
        ptrValType = as<IRPtrTypeBase>(as<IRLoad>(inst)->getPtr()->getDataType())->getValueType();

        if (ptrValType != (IRType*)getLoweredType(valInfo))
        {
            SLANG_ASSERT(!as<IRParam>(inst));
            IRBuilder builder(inst);
            builder.setInsertAfter(inst);

            IRCloneEnv cloneEnv;
            auto newLoad = cloneInst(&cloneEnv, &builder, inst);

            auto loweredVal = upcastCollection(context, newLoad, (IRType*)valInfo);

            // TODO: this is a hack. Encode this in the type-flow-data.
            if (as<IRInterfaceType>(ptrValType) && !isComInterfaceType(inst->getDataType()) &&
                !isBuiltin(inst->getDataType()))
            {
                newLoad->setFullType(lowerInterfaceType(as<IRInterfaceType>(ptrValType)));
            }

            inst->replaceUsesWith(loweredVal);
            inst->removeAndDeallocate();

            return true;
        }
        else if (inst->getDataType() != ptrValType)
        {
            inst->setFullType((IRType*)getLoweredType(valInfo));
            return true;
        }

        return false;
    }

    bool lowerStore(IRInst* context, IRStore* inst)
    {
        auto ptr = inst->getPtr();
        auto ptrInfo = as<IRPtrTypeBase>(ptr->getDataType())->getValueType();

        auto valInfo = inst->getVal()->getDataType();

        auto loweredVal = upcastCollection(context, inst->getVal(), ptrInfo);

        if (loweredVal != inst->getVal())
        {
            // If the value was changed, we need to update the store instruction.
            IRBuilder builder(inst);
            builder.replaceOperand(inst->getValUse(), loweredVal);
            return true;
        }

        return false;
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
        // hasChanges |= insertReinterprets();

        // Phase 2: Dynamic Instruction Lowering
        hasChanges |= performDynamicInstLowering();

        // Phase 3: Lower collection types.
        // if (hasChanges)
        //    lowerTypeCollections();

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
    Dictionary<Element, IRInst*> propagationMap;

    // Mapping from function to return value propagation information
    Dictionary<IRInst*, IRInst*> funcReturnInfo;

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

IRAnyValueType* createAnyValueType(IRBuilder* builder, const HashSet<IRType*>& types)
{
    auto size = calculateAnyValueSize(types);
    return builder->getAnyValueType(size);
}

IRFunc* createDispatchFunc(IRFuncCollection* collection)
{
    // An effective func type should have been set during the dynamic-inst-lowering
    // pass.
    //
    IRFuncType* dispatchFuncType = cast<IRFuncType>(collection->getFullType());

    // Create a dispatch function with switch-case for each function
    IRBuilder builder(collection->getModule());

    // Consume the first parameter of the expected function type
    List<IRType*> innerParamTypes;
    for (auto paramType : dispatchFuncType->getParamTypes())
        innerParamTypes.add(paramType);
    innerParamTypes.removeAt(0); // Remove the first parameter (ID)

    auto resultType = dispatchFuncType->getResultType();
    auto innerFuncType = builder.getFuncType(innerParamTypes, resultType);

    auto func = builder.createFunc();
    builder.setInsertInto(func);
    func->setFullType(dispatchFuncType);

    auto entryBlock = builder.emitBlock();
    builder.setInsertInto(entryBlock);

    auto idParam = builder.emitParam(builder.getUIntType());

    // Create parameters for the original function arguments
    List<IRInst*> originalParams;
    for (UInt i = 0; i < innerParamTypes.getCount(); i++)
    {
        originalParams.add(builder.emitParam(innerParamTypes[i]));
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

    UIndex funcSeqID = 0;
    forEachInCollection(
        collection,
        [&](IRInst* funcInst)
        {
            auto funcId = funcSeqID++;
            auto wrapperFunc =
                emitWitnessTableWrapper(funcInst->getModule(), funcInst, innerFuncType);

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
        });

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


IRFunc* createIntegerMappingFunc(IRModule* module, Dictionary<UInt, UInt>& mapping)
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

// This context lowers `IRGetTagFromSequentialID`,
// `IRGetTagForSuperCollection`, and `IRGetTagForMappedCollection` instructions,
//

struct TagOpsLoweringContext : public InstPassBase
{
    TagOpsLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    void lowerGetTagForSuperCollection(IRGetTagForSuperCollection* inst)
    {
        auto srcCollection = cast<IRCollectionBase>(
            cast<IRCollectionTagType>(inst->getOperand(0)->getDataType())->getOperand(0));
        auto destCollection =
            cast<IRCollectionBase>(cast<IRCollectionTagType>(inst->getDataType())->getOperand(0));

        IRBuilder builder(inst->getModule());
        builder.setInsertAfter(inst);

        List<IRInst*> indices;
        for (UInt i = 0; i < srcCollection->getOperandCount(); i++)
        {
            // Find in destCollection
            auto srcElement = srcCollection->getOperand(i);

            bool found = false;
            for (UInt j = 0; j < destCollection->getOperandCount(); j++)
            {
                auto destElement = destCollection->getOperand(j);
                if (srcElement == destElement)
                {
                    found = true;
                    indices.add(builder.getIntValue(builder.getUIntType(), j));
                    break; // Found the index
                }
            }

            if (!found)
            {
                // destCollection must be a super-set
                SLANG_UNEXPECTED("Element not found in destination collection");
            }
        }

        // Create an array for the lookup
        auto lookupArrayType = builder.getArrayType(
            builder.getUIntType(),
            builder.getIntValue(builder.getUIntType(), indices.getCount()));
        auto lookupArray =
            builder.emitMakeArray(lookupArrayType, indices.getCount(), indices.getBuffer());
        auto resultID =
            builder.emitElementExtract(inst->getDataType(), lookupArray, inst->getOperand(0));
        inst->replaceUsesWith(resultID);
        inst->removeAndDeallocate();
    }

    void lowerGetTagForMappedCollection(IRGetTagForMappedCollection* inst)
    {
        auto srcCollection = cast<IRTableCollection>(
            cast<IRCollectionTagType>(inst->getOperand(0)->getDataType())->getOperand(0));
        auto destCollection =
            cast<IRCollectionBase>(cast<IRCollectionTagType>(inst->getDataType())->getOperand(0));
        auto key = cast<IRStructKey>(inst->getOperand(1));

        IRBuilder builder(inst->getModule());
        builder.setInsertAfter(inst);

        List<IRInst*> indices;
        for (UInt i = 0; i < srcCollection->getOperandCount(); i++)
        {
            // Find in destCollection
            bool found = false;
            auto srcElement = findEntryInConcreteTable(srcCollection->getOperand(i), key);
            for (UInt j = 0; j < destCollection->getOperandCount(); j++)
            {
                auto destElement = destCollection->getOperand(j);
                if (srcElement == destElement)
                {
                    found = true;
                    indices.add(builder.getIntValue(builder.getUIntType(), j));
                    break; // Found the index
                }
            }

            if (!found)
            {
                // destCollection must be a super-set
                SLANG_UNEXPECTED("Element not found in destination collection");
            }
        }

        // Create an array for the lookup
        auto lookupArrayType = builder.getArrayType(
            builder.getUIntType(),
            builder.getIntValue(builder.getUIntType(), indices.getCount()));
        auto lookupArray =
            builder.emitMakeArray(lookupArrayType, indices.getCount(), indices.getBuffer());
        auto resultID =
            builder.emitElementExtract(inst->getDataType(), lookupArray, inst->getOperand(0));
        inst->replaceUsesWith(resultID);
        inst->removeAndDeallocate();
    }

    void processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_GetTagForSuperCollection:
            lowerGetTagForSuperCollection(as<IRGetTagForSuperCollection>(inst));
            break;
        case kIROp_GetTagForMappedCollection:
            lowerGetTagForMappedCollection(as<IRGetTagForMappedCollection>(inst));
            break;
        default:
            break;
        }
    }

    void lowerFuncCollection(IRFuncCollection* collection)
    {
        IRBuilder builder(collection->getModule());
        if (collection->hasUses() && collection->getDataType() != nullptr)
        {
            auto dispatchFunc = createDispatchFunc(collection);
            traverseUses(
                collection,
                [&](IRUse* use)
                {
                    if (auto callInst = as<IRCall>(use->getUser()))
                    {
                        // If the call is a collection call, replace it with the dispatch function
                        if (callInst->getCallee() == collection)
                        {
                            IRBuilder callBuilder(callInst);
                            callBuilder.setInsertBefore(callInst);
                            callBuilder.replaceOperand(callInst->getCalleeUse(), dispatchFunc);
                        }
                    }
                });
        }
    }

    void processModule()
    {
        processInstsOfType<IRFuncCollection>(
            kIROp_FuncCollection,
            [&](IRFuncCollection* inst) { return lowerFuncCollection(inst); });

        processAllInsts([&](IRInst* inst) { return processInst(inst); });
    }
};

// This context lowers `IRTypeCollection` and `IRFuncCollection` instructions
struct CollectionLoweringContext : public InstPassBase
{
    CollectionLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    void lowerTypeCollection(IRTypeCollection* collection)
    {
        HashSet<IRType*> types;
        for (UInt i = 0; i < collection->getOperandCount(); i++)
        {
            if (auto type = as<IRType>(collection->getOperand(i)))
            {
                types.add(type);
            }
        }

        IRBuilder builder(collection->getModule());
        auto anyValueType = createAnyValueType(&builder, types);
        collection->replaceUsesWith(anyValueType);
    }

    void processModule()
    {
        processInstsOfType<IRTypeCollection>(
            kIROp_TypeCollection,
            [&](IRTypeCollection* inst) { return lowerTypeCollection(inst); });
    }
};

void lowerTypeCollections(IRModule* module, DiagnosticSink* sink)
{
    CollectionLoweringContext context(module);
    context.processModule();
}

struct SequentialIDTagLoweringContext : public InstPassBase
{
    SequentialIDTagLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }
    void lowerGetTagFromSequentialID(IRGetTagFromSequentialID* inst)
    {
        auto srcInterfaceType = cast<IRInterfaceType>(inst->getOperand(0));
        auto srcSeqID = inst->getOperand(1);

        Dictionary<UInt, UInt> mapping;

        // Map from sequential ID to unique ID
        auto destCollection =
            cast<IRCollectionBase>(cast<IRCollectionTagType>(inst->getDataType())->getOperand(0));

        UIndex dstSeqID = 0;
        forEachInCollection(
            destCollection,
            [&](IRInst* table)
            {
                // Get unique ID for the witness table
                auto witnessTable = cast<IRWitnessTable>(table);
                auto outputId = dstSeqID++;
                auto seqDecoration = table->findDecoration<IRSequentialIDDecoration>();
                if (seqDecoration)
                {
                    auto inputId = seqDecoration->getSequentialID();
                    mapping[inputId] = outputId; // Map ID to itself for now
                }
            });

        IRBuilder builder(inst);
        builder.setInsertAfter(inst);
        auto translatedID = builder.emitCallInst(
            inst->getDataType(),
            createIntegerMappingFunc(builder.getModule(), mapping),
            List<IRInst*>({srcSeqID}));

        inst->replaceUsesWith(translatedID);
        inst->removeAndDeallocate();
    }

    void processModule()
    {
        processInstsOfType<IRGetTagFromSequentialID>(
            kIROp_GetTagFromSequentialID,
            [&](IRGetTagFromSequentialID* inst) { return lowerGetTagFromSequentialID(inst); });
    }
};

void lowerSequentialIDTagCasts(IRModule* module, DiagnosticSink* sink)
{
    SequentialIDTagLoweringContext context(module);
    context.processModule();
}

void lowerTagInsts(IRModule* module, DiagnosticSink* sink)
{
    TagOpsLoweringContext tagContext(module);
    tagContext.processModule();
}

struct TagTypeLoweringContext : public InstPassBase
{
    TagTypeLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    void processModule()
    {
        processInstsOfType<IRCollectionTagType>(
            kIROp_CollectionTagType,
            [&](IRCollectionTagType* inst)
            {
                IRBuilder builder(inst->getModule());
                inst->replaceUsesWith(builder.getUIntType());
            });
    }
};

void lowerTagTypes(IRModule* module)
{
    TagTypeLoweringContext context(module);
    context.processModule();
}

// Main entry point
bool lowerDynamicInsts(IRModule* module, DiagnosticSink* sink)
{
    DynamicInstLoweringContext context(module, sink);
    return context.processModule();
}

} // namespace Slang
