#include "slang-ir-typeflow-specialize.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-clone.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize.h"
#include "slang-ir-translate.h"
#include "slang-ir-typeflow-collection.h"
#include "slang-ir-util.h"
#include "slang-ir-witness-table-wrapper.h"
#include "slang-ir.h"


namespace Slang
{

// Forward-declare.. (TODO: Just include this from the header instead)
IRInst* specializeGeneric(IRSpecialize* specializeInst);

// Is the inst an 'executable'/'interpretable' thing, or a global const
// value?
//
bool isFuncOrGenericChild(IRInst* inst)
{
    // Right now, we go off the fact that all such insts are
    // children of blocks (the block could be in a Func or a Generic)
    //
    // If there are any other block-based insts, we might need a
    // more refined check.
    //
    if (as<IRBlock>(inst->getParent()))
        return true;
    else
        return false;
}

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

    void validateElement() const
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
        Invoke, // From call site to function entry (propagating arguments)
        Return  // From function return to call site (propagating return value)
    };

    Direction direction;
    IRInst* callerContext; // The context of the call (e.g. function or specialized generic)
    IRInst* callInst;      // The call/specialize instruction
    IRInst* targetContext; // The function/specialized-generic being called/returned from

    InterproceduralEdge() = default;
    InterproceduralEdge(Direction dir, IRInst* callerContext, IRInst* call, IRInst* func)
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
        Inst,      // Propagate through a single instruction.
        Block,     // Propagate through each instruction in a block.
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

    WorkItem(
        InterproceduralEdge::Direction dir,
        IRInst* callerCtx,
        IRInst* invokeInst,
        IRInst* callee)
        : type(Type::InterProc), interProcEdge(dir, callerCtx, invokeInst, callee), context(nullptr)
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

struct WorkQueue
{
    List<WorkItem> enqueueList;
    List<WorkItem> dequeueList;
    Index dequeueIndex = 0;

    void enqueue(const WorkItem& item) { enqueueList.add(item); }

    WorkItem dequeue()
    {
        if (dequeueList.getCount() <= dequeueIndex)
        {
            dequeueList.swapWith(enqueueList);
            enqueueList.clear();
            dequeueIndex = 0;
        }

        SLANG_ASSERT(dequeueIndex < dequeueList.getCount());
        return dequeueList[dequeueIndex++];
    }

    bool hasItems() const
    {
        return (dequeueIndex < dequeueList.getCount()) || (enqueueList.getCount() > 0);
    }
};

struct TypeFlowSpecializationContext
{
    IRInst* resolve(IRInst* inst)
    {
        auto resolved = getResolvedInst(inst);
        if (resolved != inst)
            inst->replaceUsesWith(resolved);

        IRBuilder builder(module);
        if (as<IRWitnessTable>(resolved))
            builder.replaceOperand(resolved->getOperands() + 0, resolve(resolved->getOperand(0)));

        return resolved;
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

        auto typeCollection = cBuilder.createCollection(kIROp_TypeCollection, typeSet);

        // Create the tagged union type
        IRBuilder builder(module);
        List<IRInst*> elements = {typeCollection, tableCollection};
        return as<IRCollectionTaggedUnionType>(builder.emitIntrinsicInst(
            nullptr,
            kIROp_CollectionTaggedUnionType,
            elements.getCount(),
            elements.getBuffer()));
    }

    IRUnboundedCollection* makeUnbounded()
    {
        IRBuilder builder(module);
        return as<IRUnboundedCollection>(
            builder.emitIntrinsicInst(nullptr, kIROp_UnboundedCollection, 0, nullptr));
    }

    IRTypeFlowData* none() { return nullptr; }

    IRInst* findWitnessVal(IRWitnessTable* witnessTable, IRInst* requirementKey)
    {
        // A witness table is basically just a container
        // for key-value pairs, and so the best we can
        // do for now is a naive linear search.
        //
        for (auto entry : witnessTable->getEntries())
        {
            if (requirementKey == entry->getRequirementKey())
            {
                return entry->getSatisfyingVal();
            }
        }

        if (witnessTable->getConformanceType()->getOp() == kIROp_VoidType)
        {
            IRBuilder builder(module);
            return builder.getVoidValue();
        }

        return nullptr;
    }

    IRInst* maybeSpecializeWitnessLookup(IRLookupWitnessMethod* lookupInst)
    {
        // Note: While we currently have named the instruction
        // `lookup_witness_method`, the `method` part is a misnomer
        // and the same instruction can look up *any* interface
        // requirement based on the witness table that provides
        // a conformance, and the "key" that indicates the interface
        // requirement.

        // We can only specialize in the case where the lookup
        // is being done on a concrete witness table, and not
        // the result of a `specialize` instruction or other
        // operation that will yield such a table.
        //
        auto witnessTable = as<IRWitnessTable>(lookupInst->getWitnessTable());
        if (!witnessTable)
        {
            if (auto collection = as<IRTableCollection>(lookupInst->getWitnessTable()))
            {
                auto requirementKey = lookupInst->getRequirementKey();

                HashSet<IRInst*> satisfyingValSet;
                bool skipSpecialization = false;
                forEachInCollection(
                    collection,
                    [&](IRInst* instElement)
                    {
                        if (auto table = as<IRWitnessTable>(instElement))
                        {
                            if (auto satisfyingVal = findWitnessVal(table, requirementKey))
                            {
                                satisfyingValSet.add(satisfyingVal);
                                return;
                            }
                        }

                        // If we reach here, we didn't find a satisfying value.
                        skipSpecialization = true;
                    });

                if (!skipSpecialization)
                {
                    CollectionBuilder cBuilder(lookupInst->getModule());
                    auto newCollection = cBuilder.makeSet(satisfyingValSet);
                    return newCollection;
                }
                else
                    return lookupInst;
            }
            else
            {
                return lookupInst;
            }
        }

        // Because we have a concrete witness table, we can
        // use it to look up the IR value that satisfies
        // the given interface requirement.
        //
        auto requirementKey = lookupInst->getRequirementKey();
        auto satisfyingVal = findWitnessVal(witnessTable, requirementKey);

        // We expect to always find a satisfying value, but
        // we will go ahead and code defensively so that
        // we leave "correct" but unspecialized code if
        // we cannot find a concrete value to use.
        //
        if (!satisfyingVal)
            return lookupInst;
    }

    IRInst* maybeSpecializeInst(IRInst* inst)
    {
        // Route all translation insts through the translation context
        if (as<IRTranslateBase>(inst) || as<IRTranslatedTypeBase>(inst))
            return translationContext.maybeTranslateInst(inst);

        switch (inst->getOp())
        {
        // case kIROp_Specialize:
        //     return specializeGeneric(as<IRSpecialize>(inst));
        case kIROp_LookupWitnessMethod:
            return maybeSpecializeWitnessLookup(as<IRLookupWitnessMethod>(inst));
            // TODO: Handle other cases here.
        case kIROp_ApplyForBwdFuncType:
        case kIROp_ForwardDiffFuncType:
        case kIROp_FuncResultType:
        case kIROp_BwdCallableFuncType:
        case kIROp_BackwardDiffFuncType:
            return translationContext.maybeTranslateInst(inst);
        default:
            return inst;
        }
        return inst;
    }

    IRInst* getResolvedInst(IRInst* inst)
    {
        if (!inst)
            return nullptr;

        if (isGlobalInst(inst))
            return getResolvedGlobalInst(inst);

        return inst;
    }

    // Evaluate any translations and specializations.
    Dictionary<IRInst*, IRInst*> resolvedInstMap;
    IRInst* getResolvedGlobalInst(IRInst* inst)
    {
        if (!inst)
            return nullptr;

        SLANG_ASSERT(isGlobalInst(inst));

        switch (inst->getOp())
        {
        case kIROp_InterfaceType:
        case kIROp_Func:
        case kIROp_Generic:
        case kIROp_StructType:
        case kIROp_WitnessTable:
            return inst;
        }

        if (as<IRCollectionBase>(inst))
            return inst;

        if (auto found = resolvedInstMap.tryGetValue(inst))
            return *found;

        // Translate all operands first.
        List<IRInst*> newOperands;
        bool changed = false;
        for (auto ii = 0; ii < inst->getOperandCount(); ii++)
        {
            auto operand = inst->getOperand(ii);
            auto resolvedOperand = getResolvedGlobalInst(operand);
            if (resolvedOperand != operand)
                changed = true;

            newOperands.add(resolvedOperand);
        }

        auto newDataType =
            (inst->getDataType()) ? getResolvedGlobalInst(inst->getDataType()) : nullptr;
        if (newDataType != inst->getDataType())
            changed = true;

        IRInst* newInst;
        if (changed)
        {
            IRBuilder builder(inst->getModule());
            newInst = builder.emitIntrinsicInst(
                (IRType*)newDataType,
                inst->getOp(),
                (UInt)newOperands.getCount(),
                newOperands.getBuffer());
        }
        else
        {
            newInst = inst;
        }

        IRInst* processedInst = newInst;
        do
        {
            newInst = processedInst;
            processedInst = maybeSpecializeInst(newInst);
        } while (processedInst != newInst);
        resolvedInstMap[inst] = processedInst;
        return processedInst;
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

        if (as<IRCollectionBase>(inst))
        {
            // If the instruction is itself a collection, return it directly
            return inst;
        }

        if (!inst->getParent())
            return none();

        // If this is a global instruction (parent is module), return concrete info
        if (as<IRModuleInst>(inst->getParent()))
        {
            auto resolvedInst = inst;
            if (as<IRType>(resolvedInst) || as<IRWitnessTable>(resolvedInst) ||
                as<IRFunc>(resolvedInst) || as<IRGeneric>(resolvedInst))
            {
                // We won't directly handle interface types, but rather treat objects of
                // interface type as objects that can be specialized with collections.
                //
                if (as<IRInterfaceType>(resolvedInst))
                    return none();

                if (as<IRGeneric>(resolvedInst) &&
                    as<IRInterfaceType>(getGenericReturnVal(resolvedInst)))
                    return none();

                // TODO: We really should return something like Singleton(collectionInst) here
                // instead of directly returning the collection.
                //
                return cBuilder.makeSingletonSet(resolvedInst);
            }
            else if (as<IRVoidLit>(inst))
            {
                // TODO: Should this be.. InstanceOf(TypeCollection(VoidType))?
                return none();
            }
            else if (cBuilder.getCollectionTypeForInst(inst) != kIROp_Invalid)
            {
                // If the inst fits into one of the collection types, then it should be a
                // singleton set of itself.
                //
                return cBuilder.makeSingletonSet(inst);
            }
            else
            {
                // Out of options..
                return none();
            }
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
    //
    // Use this when you want to propagate new information to an existing instruction.
    // This will union the new info with existing info and add users to work queue if changed
    //
    void updateInfo(
        IRInst* context,
        IRInst* inst,
        IRInst* newInfo,
        bool takeUnion,
        WorkQueue& workQueue)
    {
        auto existingInfo = tryGetInfo(context, inst);
        auto unionedInfo = (takeUnion) ? unionPropagationInfo(existingInfo, newInfo) : newInfo;

        // Only proceed if info actually changed
        if (areInfosEqual(existingInfo, unionedInfo))
            return;

        // Update the propagation map
        propagationMap[Element(context, inst)] = unionedInfo;

        // Add all users to appropriate work items
        addUsersToWorkQueue(context, inst, unionedInfo, workQueue);
    }

    bool isFuncParam(IRParam* param)
    {
        auto paramBlock = as<IRBlock>(param->getParent());
        auto paramFunc = as<IRFunc>(paramBlock->getParent());
        return (paramFunc && paramFunc->getFirstBlock() == paramBlock);
    }

    void addContextUsersToWorkQueue(IRInst* context, WorkQueue& workQueue)
    {
        if (this->funcCallSites.containsKey(context))
            for (auto callSite : this->funcCallSites[context])
            {
                workQueue.enqueue(WorkItem(
                    InterproceduralEdge::Direction::Return,
                    callSite.context,
                    as<IRCall>(callSite.inst),
                    context));
            }
    }

    // Helper to add users of an instruction to the work queue based on how they use it
    // This handles intra-procedural edges, inter-procedural edges, and return value propagation
    void addUsersToWorkQueue(IRInst* context, IRInst* inst, IRInst* info, WorkQueue& workQueue)
    {
        if (auto param = as<IRParam>(inst))
            if (isFuncParam(param))
                addContextUsersToWorkQueue(context, workQueue);

        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();

            if (!isFuncOrGenericChild(user))
                continue;

            workQueue.enqueue(WorkItem(context, user));

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
                        workQueue.enqueue(WorkItem(context, succIter.getEdge()));
                    }
                }
            }

            // If user is a return instruction, handle function return propagation
            if (as<IRReturn>(user))
                updateFuncReturnInfo(context, info, workQueue);

            // If the user is a top-level inout/out parameter, we need to handle it
            // like we would a func-return.
            //
            if (auto param = as<IRParam>(user))
                if (isFuncParam(param))
                    addContextUsersToWorkQueue(context, workQueue);
        }
    }

    // Helper method to update function return info and propagate to call sites
    void updateFuncReturnInfo(IRInst* callable, IRInst* returnInfo, WorkQueue& workQueue)
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
                    workQueue.enqueue(WorkItem(
                        InterproceduralEdge::Direction::Return,
                        callSite.context,
                        as<IRCall>(callSite.inst),
                        callable));
                }
            }
        }
    }

    void processBlock(IRInst* context, IRBlock* block, WorkQueue& workQueue)
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

    bool isEntryFunc(IRFunc* func)
    {
        return func->findDecoration<IREntryPointDecoration>() ||
               func->findDecoration<IRKeepAliveDecoration>();
    }

    void performInformationPropagation()
    {
        // Global worklist for interprocedural analysis
        WorkQueue workQueue;

        // Add all global functions to worklist
        for (auto inst : module->getGlobalInsts())
            if (auto func = as<IRFunc>(inst))
                if (isEntryFunc(func))
                    discoverContext(func, workQueue);

        // Process until fixed point
        while (workQueue.hasItems())
        {
            auto item = workQueue.dequeue();

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

    IRInst* upcastCollection(IRInst* context, IRInst* arg, IRType* destInfo)
    {
        auto argInfo = arg->getDataType();
        if (!argInfo || !destInfo)
            return arg;

        if (as<IRCollectionTaggedUnionType>(argInfo) && as<IRCollectionTaggedUnionType>(destInfo))
        {
            // Handle upcasting between collection tagged unions
            auto argTUType = as<IRCollectionTaggedUnionType>(argInfo);
            auto destTUType = as<IRCollectionTaggedUnionType>(destInfo);

            if (getCollectionCount(argTUType) != getCollectionCount(destTUType))
            {
                // Technically, IRCollectionTaggedUnionType is not a TupleType,
                // but in practice it works the same way so we'll re-use Slang's
                // tuple accessors & constructors
                //
                IRBuilder builder(arg->getModule());
                setInsertAfterOrdinaryInst(&builder, arg);
                auto argTableTag = builder.emitGetTupleElement(
                    (IRType*)makeTagType(argTUType->getTableCollection()),
                    arg,
                    0);
                auto reinterpretedTag = upcastCollection(
                    context,
                    argTableTag,
                    (IRType*)makeTagType(destTUType->getTableCollection()));

                auto argVal =
                    builder.emitGetTupleElement((IRType*)argTUType->getTypeCollection(), arg, 1);
                auto reinterpretedVal =
                    upcastCollection(context, argVal, (IRType*)destTUType->getTypeCollection());
                return builder.emitMakeTuple(
                    (IRType*)destTUType,
                    {reinterpretedTag, reinterpretedVal});
            }
        }
        else if (as<IRTupleType>(argInfo) && as<IRTupleType>(destInfo))
        {
            auto argTupleType = as<IRTupleType>(argInfo);
            auto destTupleType = as<IRTupleType>(destInfo);

            List<IRInst*> upcastedElements;
            bool hasUpcastedElements = false;

            IRBuilder builder(module);
            setInsertAfterOrdinaryInst(&builder, arg);

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
                setInsertAfterOrdinaryInst(&builder, arg);
                return builder
                    .emitIntrinsicInst((IRType*)destInfo, kIROp_GetTagForSuperCollection, 1, &arg);
            }
        }
        else if (as<IRCollectionBase>(argInfo) && as<IRCollectionBase>(destInfo))
        {
            if (getCollectionCount(as<IRCollectionBase>(argInfo)) !=
                getCollectionCount(as<IRCollectionBase>(destInfo)))
            {
                // If the sets of witness tables are not equal, reinterpret to the parameter
                // type
                IRBuilder builder(module);
                setInsertAfterOrdinaryInst(&builder, arg);
                return builder.emitReinterpret((IRType*)destInfo, arg);
            }
        }
        else if (!as<IRCollectionBase>(argInfo) && as<IRCollectionBase>(destInfo))
        {
            IRBuilder builder(module);
            setInsertAfterOrdinaryInst(&builder, arg);
            return builder.emitPackAnyValue((IRType*)destInfo, arg);
        }

        return arg; // Can use as-is.
    }

    struct TranslationBaseInfo
    {
        List<IRInst*> operands;

        enum Flavor
        {
            Unknown,
            Simple,    // Direct reference to func
            Specialize // Indirect reference that requires specialization.
        } flavor = Flavor::Unknown;

        List<IRInst*> specializeOperands;
    };

    bool addOperand(TranslationBaseInfo& baseInfo, IRInst* operand)
    {
        if (operand)
        {
            if (auto funcInst = as<IRFunc>(operand))
            {
                baseInfo.operands.add(funcInst);
                if (baseInfo.operands.getCount() == 1)
                {
                    baseInfo.flavor = TranslationBaseInfo::Flavor::Simple;
                }
                else if (baseInfo.flavor != TranslationBaseInfo::Flavor::Simple)
                    return false; // Can't unify with existing operand.

                return true;
            }
            else if (auto specializeInst = as<IRSpecialize>(operand))
            {
                // If the base is a specialization, we need to collect the operands.
                IRGeneric* genericInst = cast<IRGeneric>(specializeInst->getBase());
                baseInfo.operands.add(genericInst);

                if (baseInfo.operands.getCount() == 1)
                {
                    for (UInt i = 0; i < specializeInst->getArgCount(); i++)
                        baseInfo.specializeOperands.add(specializeInst->getArg(i));
                    baseInfo.flavor = TranslationBaseInfo::Flavor::Specialize;
                }
                else if (baseInfo.flavor != TranslationBaseInfo::Flavor::Specialize)
                    return false; // Can't unify with existing operand.
                else
                {
                    // check that the operands match
                    if (baseInfo.specializeOperands.getCount() != specializeInst->getArgCount())
                        return false; // Can't unify with existing operands.
                    for (UInt i = 0; i < baseInfo.specializeOperands.getCount(); i++)
                    {
                        if (baseInfo.specializeOperands[i] != specializeInst->getArg(i))
                            return false; // Can't unify with existing operands.
                    }
                }

                return true;
            }
            else
            {
                return false;
            }
        }
    }

    IRInst* materialize(IRBuilder* builder, TranslationBaseInfo& info, IRInst* resultInst)
    {
        if (info.flavor == TranslationBaseInfo::Flavor::Simple)
        {
            // Simple case, just return the function.
            return resultInst;
        }
        else if (info.flavor == TranslationBaseInfo::Flavor::Specialize)
        {
            auto outerGeneric = resultInst;
            SLANG_ASSERT(as<IRGeneric>(resultInst));
            auto innerVal = getGenericReturnVal(resultInst);
            if (as<IRFunc>(innerVal) || innerVal->getDataType()->getOp() == kIROp_FuncType)
            {
                return specializeGeneric(
                    as<IRSpecialize>(builder->emitSpecializeInst(
                        (IRType*)specializeGeneric(
                            as<IRSpecialize>(builder->emitSpecializeInst(
                                builder->getTypeKind(),
                                resultInst->getDataType(),
                                info.specializeOperands))),
                        resultInst,
                        info.specializeOperands)));
            }
            else if (as<IRStructType>(innerVal))
            {
                return (IRType*)specializeGeneric(
                    as<IRSpecialize>(builder->emitSpecializeInst(
                        builder->getTypeKind(),
                        resultInst,
                        info.specializeOperands)));
            }
            else
            {
                SLANG_UNEXPECTED("Unexpected result inst type for specialization.");
            }
        }
        else
        {
            SLANG_UNEXPECTED("Unknown translation request flavor.");
        }
    }

    TranslationBaseInfo getTranslationBaseInfo(IRInst* inst)
    {
        TranslationBaseInfo baseInfo;
        for (UIndex ii = 0; ii < inst->getOperandCount(); ii++)
        {
            bool valid = true;
            valid &= addOperand(baseInfo, inst->getOperand(ii));

            if (!valid)
                return TranslationBaseInfo();
        }

        return baseInfo;
    }

    IRInst* getSubstitutedFuncType(IRInst* context, IRFuncType* funcType)
    {
        auto substituteSets = [&](IRInst* type) -> IRInst*
        {
            if (auto info = tryGetInfo(context, type))
            {
                if (auto infoCollectionTag = as<IRCollectionTagType>(info))
                {
                    if (infoCollectionTag->isSingleton())
                        return infoCollectionTag->getCollection()->getElement(0);
                    else
                        return infoCollectionTag->getCollection();
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

        return builder.getFuncType(
            newParamTypes.getCount(),
            newParamTypes.getBuffer(),
            (IRType*)substituteSets(funcType->getResultType()));
    }

    IRInst* analyzeTranslation(IRInst* context, IRInst* inst)
    {
        List<IRInst*> operandInfos;
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            // Translation insts should _never_ operate on collections.
            // We should only encounter collections of translations.
            //
            if (auto info = tryGetInfo(context, inst->getOperand(i)))
            {
                SLANG_ASSERT(as<IRCollectionBase>(info) || as<IRCollectionTagType>(info));
                SLANG_ASSERT(info->getOperandCount() == 1);
                operandInfos.add(info->getOperand(0));
            }
            else
            {
                return none();
            }
        }

        IRBuilder builder(module);
        builder.setInsertBefore(inst);

        auto substFuncType =
            (IRType*)getSubstitutedFuncType(context, cast<IRFuncType>(inst->getDataType()));

        if (isGlobalInst(substFuncType))
        {
            auto translationInst = builder.emitIntrinsicInst(
                substFuncType,
                inst->getOp(),
                operandInfos.getCount(),
                operandInfos.getBuffer());
            return cBuilder.makeSingletonSet(translationInst);
        }
        else
        {
            return none();
        }
    }

    void processInstForPropagation(IRInst* context, IRInst* inst, WorkQueue& workQueue)
    {
        IRInst* info;

        if (as<IRTranslateBase>(inst) || as<IRTranslatedTypeBase>(inst))
        {
            info = analyzeTranslation(context, inst);
        }

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
        case kIROp_AttributedType:
            info = analyzeAttributedType(context, as<IRAttributedType>(inst), workQueue);
            break;
        case kIROp_Specialize:
            info = analyzeSpecialize(context, as<IRSpecialize>(inst), workQueue);
            break;
        case kIROp_Load:
        case kIROp_RWStructuredBufferLoad:
        case kIROp_StructuredBufferLoad:
            info = analyzeLoad(context, inst);
            break;
        case kIROp_MakeStruct:
            info = analyzeMakeStruct(context, as<IRMakeStruct>(inst), workQueue);
            break;
        case kIROp_Store:
            info = analyzeStore(context, as<IRStore>(inst), workQueue);
            break;
        case kIROp_GetElementPtr:
            info = analyzeGetElementPtr(context, as<IRGetElementPtr>(inst));
            break;
        case kIROp_FieldAddress:
            info = analyzeFieldAddress(context, as<IRFieldAddress>(inst));
            break;
        case kIROp_FieldExtract:
            info = analyzeFieldExtract(context, as<IRFieldExtract>(inst));
            break;
        default:
            info = analyzeDefault(context, inst);
            break;
        }

        // TODO: Remove this workaround.. there are a few insts
        // where we shouldn't
        bool takeUnion = !as<IRSpecialize>(inst);
        updateInfo(context, inst, info, takeUnion, workQueue);
    }

    IRInst* analyzeCreateExistentialObject(IRInst* context, IRCreateExistentialObject* inst)
    {
        SLANG_UNUSED(context);
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
                    as<IRTableCollection>(
                        cBuilder.createCollection(kIROp_TableCollection, tables)));
            else
                return none();
        }

        return none();
    }

    IRInst* analyzeMakeExistential(IRInst* context, IRMakeExistential* inst)
    {
        auto witnessTable = inst->getWitnessTable();

        // If we're building an existential for a COM pointer,
        // we won't try to lower that.
        //
        if (isComInterfaceType(inst->getDataType()))
            return makeUnbounded();

        // Get the witness table info
        auto witnessTableInfo = tryGetInfo(context, witnessTable);

        if (!witnessTableInfo)
            return none();

        if (as<IRUnboundedCollection>(witnessTableInfo))
            return makeUnbounded();

        if (auto collectionTag = as<IRCollectionTagType>(witnessTableInfo))
            return makeExistential(cast<IRTableCollection>(collectionTag->getCollection()));

        // Assume we hit the concrete/unresolved-concrete case.

        auto resolvedTable = resolve(witnessTable);
        if (resolvedTable != witnessTable)
            witnessTable->replaceUsesWith(resolvedTable);

        if (as<IRWitnessTable>(resolvedTable))
        {
            return makeExistential(as<IRTableCollection>(cBuilder.makeSingletonSet(resolvedTable)));
        }


        SLANG_UNEXPECTED("Unexpected witness table info type in analyzeMakeExistential");
    }

    IRInst* analyzeMakeStruct(IRInst* context, IRMakeStruct* makeStruct, WorkQueue& workQueue)
    {
        // We'll process this in the same way as a field-address, but for
        // all fields of the struct.
        //
        auto structType = as<IRStructType>(makeStruct->getDataType());
        if (!structType)
            return none();

        UIndex operandIndex = 0;
        for (auto field : structType->getFields())
        {
            auto operand = makeStruct->getOperand(operandIndex);
            if (auto operandInfo = tryGetInfo(context, operand))
            {
                IRInst* existingInfo = nullptr;
                this->fieldInfo.tryGetValue(field, existingInfo);
                auto newInfo = unionPropagationInfo(existingInfo, operandInfo);
                if (newInfo && !areInfosEqual(existingInfo, newInfo))
                {
                    // Update the field info map
                    this->fieldInfo[field] = newInfo;

                    if (this->fieldUseSites.containsKey(field))
                        for (auto useSite : this->fieldUseSites[field])
                            workQueue.enqueue(WorkItem(useSite.context, useSite.inst));
                }
            }

            operandIndex++;
        }

        return none(); // the make struct itself doesn't have any info.
    }

    bool isResourcePointer(IRInst* inst)
    {
        if (isPointerToResourceType(inst->getDataType()) ||
            inst->getOp() == kIROp_RWStructuredBufferGetElementPtr)
            return true;

        if (as<IRGlobalParam>(inst))
            return true;

        if (auto ptr = as<IRGetElementPtr>(inst))
            return isResourcePointer(ptr->getBase());

        if (auto fieldAddress = as<IRFieldAddress>(inst))
            return isResourcePointer(fieldAddress->getBase());

        return false;
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
                                    cBuilder.createCollection(kIROp_TableCollection, tables)));
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
                            as<IRTableCollection>(
                                cBuilder.createCollection(kIROp_TableCollection, tables)));
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

    IRInst* analyzeStore(IRInst* context, IRStore* storeInst, WorkQueue& workQueue)
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

    IRInst* analyzeFieldAddress(IRInst* context, IRFieldAddress* fieldAddress)
    {
        // The base info should be in Ptr<T> form, so we just need to return Ptr<T> as the
        // result.
        //
        IRBuilder builder(module);
        builder.setInsertAfter(fieldAddress);
        auto basePtr = fieldAddress->getBase();

        if (auto basePtrType = as<IRPtrTypeBase>(basePtr->getDataType()))
        {
            if (auto structType = as<IRStructType>(basePtrType->getValueType()))
            {
                auto structField =
                    findStructField(structType, as<IRStructKey>(fieldAddress->getField()));

                // Register this as a user of the field so updates will invoke this function
                // again.
                this->fieldUseSites.addIfNotExists(structField, HashSet<Element>());
                this->fieldUseSites[structField].add(Element(context, fieldAddress));

                if (this->fieldInfo.containsKey(structField))
                {
                    return builder.getPtrTypeWithAddressSpace(
                        (IRType*)this->fieldInfo[structField],
                        as<IRPtrTypeBase>(fieldAddress->getDataType()));
                }
            }
        }
        return none();
    }

    IRInst* analyzeFieldExtract(IRInst* context, IRFieldExtract* fieldExtract)
    {
        IRBuilder builder(module);

        if (auto structType = as<IRStructType>(fieldExtract->getBase()->getDataType()))
        {
            auto structField =
                findStructField(structType, as<IRStructKey>(fieldExtract->getField()));

            // Register this as a user of the field so updates will invoke this function again.
            this->fieldUseSites.addIfNotExists(structField, HashSet<Element>());
            this->fieldUseSites[structField].add(Element(context, fieldExtract));

            if (this->fieldInfo.containsKey(structField))
            {
                return this->fieldInfo[structField];
            }
        }
        return none();
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
            IRBuilder builder(module);
            HashSet<IRInst*> results;
            forEachInCollection(
                cast<IRTableCollection>(tagType->getCollection()),
                [&](IRInst* table)
                {
                    if (as<IRSpecialize>(table))
                    {
                        auto specialize = cast<IRSpecialize>(table);
                        auto generic = cast<IRGeneric>(specialize->getBase());
                        auto genericReturnVal = findGenericReturnVal(generic);

                        // Resolve table..
                        // Note that we're modifying the generic & then specializing it.
                        // rather than specializing the result, since it allows us to reuse
                        // the generic.
                        //
                        switch (genericReturnVal->getOp())
                        {
                        case kIROp_SynthesizedBackwardDerivativeWitnessTable:
                        case kIROp_SynthesizedForwardDerivativeWitnessTable:
                        case kIROp_SynthesizedBackwardDerivativeWitnessTableFromLegacyBwdDiffFunc:
                            {
                                auto translatedInst =
                                    translationContext.maybeTranslateInst(genericReturnVal);
                                genericReturnVal->replaceUsesWith(translatedInst);
                                genericReturnVal = translatedInst;
                                break;
                            }
                        default:
                            break;
                        }

                        table = specializeGeneric(specialize);
                    }

                    auto resolvedTable = resolve(table);
                    auto result = findWitnessTableEntry(cast<IRWitnessTable>(resolvedTable), key);
                    result->setFullType((IRType*)resolve(result->getFullType()));
                    results.add(result);
                });
            return makeTagType(cBuilder.makeSet(results));
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
            return makeTagType(taggedUnion->getTableCollection());

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
            return makeTagType(taggedUnion->getTypeCollection());

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
            return taggedUnion->getTypeCollection();

        return none();
    }

    IRInst* analyzeSpecialize(IRInst* context, IRSpecialize* inst, WorkQueue& workQueue)
    {
        // return _analyzeInvoke(context, inst, workQueue);

        auto operand = inst->getBase();
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
            // If any of the specialization arguments need a tag (or the generic itself is a
            // tag), we need the result to also be wrapped in a tag type.
            bool needsTag = false;

            List<IRInst*> specializationArgs;
            for (UInt i = 0; i < inst->getArgCount(); ++i)
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
                    if (argCollectionTag->isSingleton())
                        specializationArgs.add(argCollectionTag->getCollection()->getElement(0));
                    else
                    {
                        needsTag = true;
                        specializationArgs.add(argCollectionTag->getCollection());
                    }
                }
                else
                {
                    SLANG_UNEXPECTED("Unhandled PropagationJudgment in analyzeSpecialize");
                }
            }

            IRType* typeOfSpecialization = nullptr;
            if (isGlobalInst(inst->getDataType()))
                typeOfSpecialization = inst->getDataType();
            else if (auto funcType = as<IRFuncType>(inst->getDataType()))
            {
                typeOfSpecialization = (IRType*)getSubstitutedFuncType(context, funcType);
            }
            else if (auto typeInfo = tryGetInfo(context, inst->getDataType()))
            {
                // There's one other case we'd like to handle, where the func-type itself is a
                // dynamic IRSpecialize. In this situation, we'd want to use the type inst's
                // info to find the collection-based specialization and create a func-type from
                // it.
                //
                if (auto tag = as<IRCollectionTagType>(typeInfo))
                {
                    SLANG_ASSERT(tag->isSingleton());
                    auto specializeInst = cast<IRSpecialize>(tag->getCollection()->getElement(0));
                    auto specializedFuncType = cast<IRFuncType>(specializeGeneric(specializeInst));
                    typeOfSpecialization = specializedFuncType;
                }
                else if (auto collection = as<IRCollectionBase>(typeInfo))
                {
                    SLANG_ASSERT(collection->isSingleton());
                    auto specializeInst = cast<IRSpecialize>(collection->getElement(0));
                    auto specializedFuncType = cast<IRFuncType>(specializeGeneric(specializeInst));
                    typeOfSpecialization = specializedFuncType;
                }
                else
                {
                    return none();
                }
            }
            else if (as<IRWitnessTableType>(inst->getDataType()))
            {
                // Keep going.. we'll avoid trying to create
                // proper witness table types, since the point of this pass
                // is to eliminate them anyway.
                // We'll intentionally make an invalid dummy type, so that
                // if we _dont_ eliminate this later, it will fail validation/codegen
                //
                IRBuilder builder(module);
                typeOfSpecialization = builder.getWitnessTableType(nullptr);
            }
            else
            {
                // We don't have a type we can work with just yet.
                return none(); // No info for the type
            }

            if (!isGlobalInst(typeOfSpecialization))
            {
                // Our func-type operand is not yet been lifted.
                // For now, we can't say anything.
                //
                return none();
            }
            else
            {
                typeOfSpecialization = (IRType*)getResolvedGlobalInst(typeOfSpecialization);
            }

            IRCollectionBase* collection = nullptr;
            if (auto _collection = as<IRCollectionBase>(operandInfo))
            {
                collection = _collection;
            }
            else if (auto collectionTagType = as<IRCollectionTagType>(operandInfo))
            {
                needsTag = true;
                collection = collectionTagType->getCollection();
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
                return makeTagType(cBuilder.makeSet(specializedSet));
            else
                return cBuilder.makeSet(specializedSet);
        }

        SLANG_UNEXPECTED("Unhandled PropagationJudgment in analyzeExtractExistentialWitnessTable");
    }

    void discoverContext(IRInst* context, WorkQueue& workQueue)
    {
        if (this->availableContexts.add(context))
        {
            // Newly discovered context. Initialize it.
            switch (context->getOp())
            {
            case kIROp_Func:
                {
                    IRFunc* func = cast<IRFunc>(context);

                    // Initialize the first block parameters
                    initializeFirstBlockParameters(context, func);

                    // Add all blocks to the work queue
                    for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
                        workQueue.enqueue(WorkItem(context, block));
                    break;
                }
            case kIROp_Specialize:
                {
                    auto specialize = cast<IRSpecialize>(context);
                    auto generic = cast<IRGeneric>(specialize->getBase());
                    auto genericReturnVal = findGenericReturnVal(generic);

                    if (as<IRTranslateBase>(genericReturnVal) ||
                        as<IRTranslatedTypeBase>(genericReturnVal))
                    {
                        switch (genericReturnVal->getOp())
                        {
                        // Instructions that need function bodys to be analyzed.
                        // In this case, we'll translate the generic and then
                        // specialize it with the same arguments.
                        //
                        // This is primarily to avoid excessive invocations of auto-diff since
                        // specialization parameters can be frequently updated during
                        // the type-flow analysis, and trying to stamp out and differentiate
                        // a specialized version for transient cases can be wasteful.
                        //
                        case kIROp_ForwardDifferentiate:
                        case kIROp_TrivialForwardDifferentiate:
                        case kIROp_BackwardDifferentiatePrimal:
                        case kIROp_BackwardDifferentiatePropagate:
                        case kIROp_BackwardContextGetPrimalVal:
                        case kIROp_BackwardDiffIntermediateContextType:
                        case kIROp_TrivialBackwardDifferentiatePrimal:
                        case kIROp_TrivialBackwardDifferentiatePropagate:
                        case kIROp_TrivialBackwardContextGetPrimalVal:
                        case kIROp_TrivialBackwardDiffIntermediateContextType:
                            {
                                auto baseInfo = getTranslationBaseInfo(genericReturnVal);
                                IRBuilder builder(module);
                                auto genericTranslationInst = builder.emitIntrinsicInst(
                                    nullptr,
                                    genericReturnVal->getOp(),
                                    baseInfo.operands.getCount(),
                                    baseInfo.operands.getBuffer());
                                auto resolvedInst = resolve(genericTranslationInst);
                                auto materializedVal =
                                    materialize(&builder, baseInfo, resolvedInst);

                                if (resolvedInst != genericTranslationInst)
                                    genericReturnVal->replaceUsesWith(materializedVal);

                                genericReturnVal = materializedVal;
                            }
                            break;

                        // Other instructions are wrappers, and only need a function reference..
                        // we can just directly translate them in this case.
                        //
                        case kIROp_LegacyBackwardDifferentiate:
                        case kIROp_FunctionCopy:
                            {
                                auto translatedInst =
                                    translationContext.maybeTranslateInst(genericReturnVal);

                                if (as<IRSpecialize>(translatedInst))
                                {
                                    // If we ended up translating to a specialize,
                                    // we need to materialize it now.
                                    translatedInst =
                                        specializeGeneric(as<IRSpecialize>(translatedInst));
                                }

                                if (translatedInst != genericReturnVal)
                                    genericReturnVal->replaceUsesWith(translatedInst);

                                genericReturnVal = translatedInst;
                            }
                            break;
                        case kIROp_BackwardPrimalFromLegacyBwdDiffFunc:
                        case kIROp_BackwardPropagateFromLegacyBwdDiffFunc:
                        case kIROp_BackwardContextGetValFromLegacyBwdDiffFunc:
                        case kIROp_BackwardContextFromLegacyBwdDiffFunc:
                            {
                                auto translatedInst =
                                    translationContext.maybeTranslateInst(genericReturnVal);

                                if (as<IRSpecialize>(translatedInst))
                                {
                                    // If we ended up translating to a specialize,
                                    // we need to materialize it now.
                                    translatedInst =
                                        specializeGeneric(as<IRSpecialize>(translatedInst));
                                }

                                if (translatedInst != genericReturnVal)
                                    genericReturnVal->replaceUsesWith(translatedInst);

                                genericReturnVal = translatedInst;
                            }
                            break;
                        default:
                            SLANG_UNEXPECTED("Unsupported translation in dynamic generic");
                            break;
                        }
                    }

                    auto func = cast<IRFunc>(genericReturnVal);

                    // Transfer information from specialization arguments to the params in the
                    // first generic block.
                    //
                    IRParam* param = generic->getFirstBlock()->getFirstParam();
                    for (UInt index = 0; index < specialize->getArgCount() && param;
                         ++index, param = param->getNextParam())
                    {
                        // Map the specialization argument to the corresponding parameter
                        auto arg = specialize->getArg(index);
                        if (as<IRIntLit>(arg))
                            continue;

                        if (auto collection = as<IRCollectionBase>(arg))
                        {
                            updateInfo(context, param, makeTagType(collection), true, workQueue);
                        }
                        else if (as<IRType>(arg) || as<IRWitnessTable>(arg))
                        {
                            updateInfo(
                                context,
                                param,
                                makeTagType(cBuilder.makeSingletonSet(arg)),
                                true,
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
                        workQueue.enqueue(WorkItem(context, block));

                    for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
                        workQueue.enqueue(WorkItem(context, block));
                }
            }
        }
    }

    IRInst* _analyzeInvoke(IRInst* context, IRInst* inst, WorkQueue& workQueue)
    {
        SLANG_ASSERT(as<IRSpecialize>(inst) || as<IRCall>(inst));

        auto callee = resolve(inst->getOperand(0));
        auto calleeInfo = tryGetInfo(context, callee);

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

            // If we're looking at an untranslated callee (i.e. autodiff),
            // now is the time to perform the translation and materialize
            // the function so we can analyze the body.
            //
            // Note that doing this only when we need to create an inter-procedural
            // edge makes sure that translations only happen when necessary.
            //
            auto resolvedCallee = getResolvedGlobalInst(callee);
            if (resolvedCallee != callee)
                callee->replaceUsesWith(resolvedCallee);

            discoverContext(resolvedCallee, workQueue);

            this->funcCallSites.addIfNotExists(resolvedCallee, HashSet<Element>());
            if (this->funcCallSites[resolvedCallee].add(Element(context, inst)))
            {
                // If this is a new call site, add a propagation task to the queue (in case
                // there's already information about this function)
                workQueue.enqueue(WorkItem(
                    InterproceduralEdge::Direction::Return,
                    context,
                    inst,
                    resolvedCallee));
            }
            workQueue.enqueue(
                WorkItem(InterproceduralEdge::Direction::Invoke, context, inst, resolvedCallee));
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

    IRInst* analyzeCall(IRInst* context, IRCall* inst, WorkQueue& workQueue)
    {
        return _analyzeInvoke(context, inst, workQueue);
    }

    IRInst* analyzeAttributedType(IRInst* context, IRAttributedType* inst, WorkQueue& workQueue)
    {
        auto base = inst->getBaseType();
        if (auto baseInfo = tryGetInfo(context, base))
        {
            // Wrap all elements into the attributed type.
            IRBuilder builder(module);
            HashSet<IRInst*> attributedSet;
            List<IRAttr*> attributes;
            for (UIndex ii = 1; ii < inst->getOperandCount(); ++ii)
                attributes.add(cast<IRAttr>(inst->getOperand(ii)));

            if (auto tag = as<IRCollectionTagType>(baseInfo))
            {
                forEachInCollection(
                    tag,
                    [&](IRInst* element)
                    {
                        attributedSet.add(builder.getAttributedType((IRType*)element, attributes));
                    });
                return makeTagType(cBuilder.makeSet(attributedSet));
            }
            else if (auto collection = as<IRCollectionBase>(baseInfo))
            {
                forEachInCollection(
                    collection,
                    [&](IRInst* element)
                    {
                        attributedSet.add(builder.getAttributedType((IRType*)element, attributes));
                    });
                return cBuilder.makeSet(attributedSet);
            }
            else
            {
                SLANG_UNEXPECTED("Unexpected PropagationJudgment in analyzeAttributedType");
            }
        }

        return none();
    }

    void maybeUpdatePtr(IRInst* context, IRInst* inst, IRInst* info, WorkQueue& workQueue)
    {
        if (auto getElementPtr = as<IRGetElementPtr>(inst))
        {
            if (auto thisPtrInfo = as<IRPtrTypeBase>(info))
            {
                auto thisValueType = thisPtrInfo->getValueType();

                IRInst* baseValueType =
                    as<IRPtrTypeBase>(getElementPtr->getBase()->getDataType())->getValueType();
                SLANG_ASSERT(as<IRArrayType>(baseValueType));

                // Propagate 'this' information to the base by wrapping it as a pointer to
                // array.
                IRBuilder builder(module);
                auto baseInfo = builder.getPtrTypeWithAddressSpace(
                    builder.getArrayType(
                        (IRType*)thisValueType,
                        as<IRArrayType>(baseValueType)->getElementCount()),
                    as<IRPtrTypeBase>(getElementPtr->getBase()->getDataType()));
                maybeUpdatePtr(context, getElementPtr->getBase(), baseInfo, workQueue);
            }
        }
        else if (auto fieldAddress = as<IRFieldAddress>(inst))
        {
            // If this is a field address, update the fieldInfos map.
            if (as<IRPtrTypeBase>(info))
            {
                IRBuilder builder(module);
                auto baseStructPtrType = as<IRPtrTypeBase>(fieldAddress->getBase()->getDataType());
                auto baseStructType = as<IRStructType>(baseStructPtrType->getValueType());
                if (!baseStructType)
                    return; // Do nothing..

                if (auto fieldKey = as<IRStructKey>(fieldAddress->getField()))
                {
                    IRStructField* foundField = findStructField(baseStructType, fieldKey);
                    IRInst* existingInfo = nullptr;
                    this->fieldInfo.tryGetValue(foundField, existingInfo);

                    if (existingInfo)
                        existingInfo = builder.getPtrTypeWithAddressSpace(
                            (IRType*)existingInfo,
                            as<IRPtrTypeBase>(fieldAddress->getDataType()));

                    if (auto newInfo = unionPropagationInfo(info, existingInfo))
                    {
                        if (newInfo != existingInfo)
                        {
                            auto newInfoValType = cast<IRPtrTypeBase>(newInfo)->getValueType();

                            // Update the field info map
                            this->fieldInfo[foundField] = newInfoValType;

                            // Add a work item to update the field extract
                            if (this->fieldUseSites.containsKey(foundField))
                                for (auto useSite : this->fieldUseSites[foundField])
                                    workQueue.enqueue(WorkItem(useSite.context, useSite.inst));
                        }
                    }
                }
            }
        }
        else if (auto var = as<IRVar>(inst))
        {
            // If we hit a local var, we'll update it's info.
            updateInfo(context, var, info, true, workQueue);
        }
        else if (auto param = as<IRParam>(inst))
        {
            // We'll also update function parameters,
            // but first change the info from PtrTypeBase<T>
            // to the specific pointer type for the parameter.
            //
            IRBuilder builder(param->getModule());
            auto newInfo = builder.getPtrTypeWithAddressSpace(
                (IRType*)as<IRPtrTypeBase>(info)->getValueType(),
                as<IRPtrTypeBase>(param->getDataType()));
            updateInfo(context, param, newInfo, true, workQueue);
        }
        else
        {
            // If we hit something unsupported, assume no information.
            return;
        }
    }

    void propagateWithinFuncEdge(IRInst* context, IREdge edge, WorkQueue& workQueue)
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
        UInt paramIndex = 0;
        for (auto param : successorBlock->getParams())
        {
            if (paramIndex < unconditionalBranch->getArgCount())
            {
                auto arg = unconditionalBranch->getArg(paramIndex);
                if (auto argInfo = tryGetInfo(context, arg))
                {
                    // Use centralized update method
                    updateInfo(context, param, argInfo, true, workQueue);
                }
            }
            paramIndex++;
        }
    }

    bool isGlobalInst(IRInst* inst) { return inst->getParent()->getOp() == kIROp_ModuleInst; }

    bool isIntrinsic(IRInst* inst)
    {
        auto func = as<IRFunc>(inst);
        if (auto specialize = as<IRSpecialize>(inst))
        {
            auto generic = specialize->getBase();
            func = as<IRFunc>(getGenericReturnVal(generic));
        }

        if (!func)
            return false;

        if (func->getFirstBlock() == nullptr)
            return true;

        return false;
    }

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

        for (auto param : func->getParams())
        {
            if (auto newInfo = tryGetInfo(context, param))
                if (getLoweredType(newInfo) != nullptr) // Check that info isn't unbounded
                {
                    effectiveTypes.add((IRType*)newInfo);
                    continue;
                }

            // Fallback.. no new info, just use the param type.
            effectiveTypes.add(param->getDataType());
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

    List<ParameterDirectionInfo> getParamDirections(IRInst* context)
    {
        List<ParameterDirectionInfo> directions;
        if (as<IRFunc>(context))
        {
            for (auto param : as<IRFunc>(context)->getParams())
            {
                const auto [direction, type] = splitDirectionAndType(param->getDataType());
                directions.add(direction);
            }
        }
        else if (auto specialize = as<IRSpecialize>(context))
        {
            auto generic = specialize->getBase();
            auto innerFunc = getGenericReturnVal(generic);
            for (auto param : as<IRFunc>(innerFunc)->getParams())
            {
                const auto [direction, type] = splitDirectionAndType(param->getDataType());
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

    void propagateInterproceduralEdge(InterproceduralEdge edge, WorkQueue& workQueue)
    {
        // Handle interprocedural edge
        auto invokeInst = edge.callInst;
        auto targetCallee = edge.targetContext;

        switch (edge.direction)
        {
        case InterproceduralEdge::Direction::Invoke:
            {
                // Propagate argument info from call site to function parameters
                IRBlock* firstBlock = nullptr;

                if (as<IRFunc>(targetCallee))
                    firstBlock = targetCallee->getFirstBlock();
                else if (auto specInst = as<IRSpecialize>(targetCallee))
                    firstBlock = getGenericReturnVal(specInst->getBase())->getFirstBlock();

                if (!firstBlock)
                    return;

                UInt argIndex = 1; // Skip callee (operand 0)
                for (auto param : firstBlock->getParams())
                {
                    if (argIndex < invokeInst->getOperandCount())
                    {
                        auto arg = invokeInst->getOperand(argIndex);
                        const auto [paramDirection, paramType] =
                            splitDirectionAndType(param->getDataType());

                        // Only update if
                        // 1. The paramType is a global inst and an interface type
                        // 2. The paramType is a local inst.
                        // all other cases, continue.
                        if (isGlobalInst(paramType) && !as<IRInterfaceType>(paramType))
                        {
                            argIndex++;
                            continue;
                        }

                        IRInst* argInfo = tryGetInfo(edge.callerContext, arg);

                        switch (paramDirection.kind)
                        {
                        case ParameterDirectionInfo::Kind::Out:
                        case ParameterDirectionInfo::Kind::InOut:
                        case ParameterDirectionInfo::Kind::ConstRef:
                            {
                                IRBuilder builder(module);
                                if (!argInfo)
                                {
                                    if (isGlobalInst(arg->getDataType()) &&
                                        !as<IRInterfaceType>(
                                            as<IRPtrTypeBase>(arg->getDataType())->getValueType()))
                                        argInfo = arg->getDataType();
                                }

                                if (!argInfo)
                                    break;

                                auto newInfo = fromDirectionAndType(
                                    &builder,
                                    paramDirection,
                                    as<IRPtrTypeBase>(argInfo)->getValueType());
                                updateInfo(edge.targetContext, param, newInfo, true, workQueue);
                                break;
                            }
                        case ParameterDirectionInfo::Kind::In:
                            {
                                // Use centralized update method
                                if (!argInfo)
                                {
                                    if (isGlobalInst(arg->getDataType()) &&
                                        !as<IRInterfaceType>(arg->getDataType()))
                                        argInfo = arg->getDataType();
                                }
                                updateInfo(edge.targetContext, param, argInfo, true, workQueue);
                                break;
                            }
                        default:
                            SLANG_UNEXPECTED(
                                "Unhandled parameter direction in interprocedural edge");
                        }
                    }
                    argIndex++;
                }
                break;
            }
        case InterproceduralEdge::Direction::Return:
            {
                // Propagate return value info from function to call site
                auto returnInfo = funcReturnInfo.tryGetValue(targetCallee);
                if (returnInfo)
                {
                    // Use centralized update method
                    updateInfo(edge.callerContext, invokeInst, *returnInfo, true, workQueue);
                }

                if (auto callInst = as<IRCall>(invokeInst))
                {

                    // Also update infos of any out parameters
                    auto paramInfos = getParamInfos(edge.targetContext);
                    auto paramDirections = getParamDirections(edge.targetContext);
                    UIndex argIndex = 0;
                    for (auto paramInfo : paramInfos)
                    {
                        if (paramInfo)
                        {
                            if (paramDirections[argIndex].kind ==
                                    ParameterDirectionInfo::Kind::Out ||
                                paramDirections[argIndex].kind ==
                                    ParameterDirectionInfo::Kind::InOut)
                            {
                                auto arg = callInst->getArg(argIndex);
                                auto argPtrType = as<IRPtrTypeBase>(arg->getDataType());

                                IRBuilder builder(module);
                                updateInfo(
                                    edge.callerContext,
                                    arg,
                                    builder.getPtrTypeWithAddressSpace(
                                        (IRType*)as<IRPtrTypeBase>(paramInfo)->getValueType(),
                                        argPtrType),
                                    true,
                                    workQueue);
                            }
                        }
                        argIndex++;
                    }
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

        return as<T>(cBuilder.createCollection(
            collection1->getOp(),
            allValues)); // Create a new collection with the union of values
    }

    IRInst* unionPropagationInfo(IRInst* info1, IRInst* info2)
    {
        if (!info1)
            return info2;
        if (!info2)
            return info1;

        if (info1 == info2)
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
        SLANG_UNUSED(context);
        // Check if this is a global concrete type, witness table, or function.
        // If so, it's a concrete element. We'll create a singleton set for it.
        if (isGlobalInst(inst) &&
            (!as<IRInterfaceType>(inst) &&
             (as<IRType>(inst) || as<IRWitnessTable>(inst) || as<IRFunc>(inst))))
            return cBuilder.makeSingletonSet(inst);

        auto instType = inst->getDataType();
        if (isGlobalInst(inst))
        {
            if (as<IRType>(instType) && !(as<IRInterfaceType>(instType)))
                return none(); // We'll avoid storing propagation info for concrete insts. (can
                               // just use the inst directly)

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

    bool specializeInstsInBlock(IRInst* context, IRBlock* block)
    {
        List<IRInst*> instsToLower;
        bool hasChanges = false;
        for (auto inst : block->getChildren())
            instsToLower.add(inst);

        for (auto inst : instsToLower)
        {
            hasChanges |= specializeInst(context, inst);
        }

        return hasChanges;
    }

    bool specializeStructType(IRStructType* structType)
    {
        bool hasChanges = false;
        for (auto field : structType->getFields())
        {
            IRInst* info = nullptr;
            this->fieldInfo.tryGetValue(field, info);
            if (!info)
                continue;

            auto specializedFieldType = getLoweredType(info);
            if (specializedFieldType != field->getFieldType())
            {
                hasChanges = true;
                field->setFieldType(specializedFieldType);
            }
        }

        /*
        for (auto field : structType->getFields())
        {
            if (auto resolvedInst = getResolvedInst(field->getFieldType()))
            {
                if (resolvedInst != field->getFieldType())
                {
                    hasChanges = true;
                    field->setFieldType((IRType*)resolvedInst);
                }
            }
        }
        */

        return hasChanges;
    }

    bool specializeFunc(IRFunc* func)
    {
        // Don't make any changes to non-global or intrinsic functions
        if (!isGlobalInst(func) || isIntrinsic(func))
            return false;

        bool hasChanges = false;
        for (auto block : func->getBlocks())
            hasChanges |= specializeInstsInBlock(func, block);

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
                    if (auto specializedType = getLoweredType(getFuncReturnInfo(func)))
                    {
                        auto newReturnVal =
                            upcastCollection(func, returnInst->getVal(), specializedType);
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

    bool resolveTypesInFunc(IRFunc* func, HashSet<IRType*>& aggTypesToResolve)
    {
        bool hasChanges = false;
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                auto newType = resolve(inst->getDataType());
                if (newType != inst->getDataType())
                {
                    hasChanges = true;
                    inst->setFullType((IRType*)newType);
                }

                // If a value of aggregate type appears in the function,
                // we'll need to resolve its fields so that we can
                // actually code-gen the struct.
                //
                if (as<IRStructType>(newType))
                    aggTypesToResolve.add((IRType*)newType);
            }
        }
        return hasChanges;
    }

    bool resolveAggTypeComponents(IRType* type)
    {
        switch (type->getOp())
        {
        case kIROp_StructType:
            {
                bool hasChanges = false;
                auto structType = cast<IRStructType>(type);
                for (auto field : structType->getFields())
                {
                    auto newFieldType = resolve(field->getFieldType());
                    if (newFieldType != field->getFieldType())
                    {
                        hasChanges = true;
                        field->setFieldType((IRType*)newFieldType);
                    }

                    if (as<IRStructType>(newFieldType))
                        resolveAggTypeComponents((IRType*)newFieldType);
                }
                return hasChanges;
            }
            break;
        default:
            return false;
        }
    }

    bool performDynamicInstLowering()
    {
        List<IRFunc*> funcsToProcess;
        List<IRStructType*> structsToProcess;

        for (auto func : this->availableContexts)
            if (auto funcInst = as<IRFunc>(func))
                funcsToProcess.add(funcInst);

        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto structType = as<IRStructType>(globalInst))
                structsToProcess.add(structType);
        }

        bool hasChanges = false;

        // Lower struct types first so that data access can be
        // marshalled properly during func specializeing.
        //
        for (auto structType : structsToProcess)
            hasChanges |= specializeStructType(structType);

        for (auto func : funcsToProcess)
            hasChanges |= specializeFunc(func);

        HashSet<IRType*> aggTypesToResolve;
        for (auto func : funcsToProcess)
            hasChanges |= resolveTypesInFunc(func, aggTypesToResolve);

        for (auto s : aggTypesToResolve)
            hasChanges |= resolveAggTypeComponents(s);

        return hasChanges;
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
            if (auto specializedValueType = getLoweredType(ptrType->getValueType()))
            {
                return builder.getPtrTypeWithAddressSpace((IRType*)specializedValueType, ptrType);
            }
            else
                return nullptr;
        }

        if (auto arrayType = as<IRArrayType>(info))
        {
            IRBuilder builder(module);
            if (auto specializedElementType = getLoweredType(arrayType->getElementType()))
            {
                return builder.getArrayType(
                    (IRType*)specializedElementType,
                    arrayType->getElementCount());
            }
            else
                return nullptr;
        }

        if (auto taggedUnion = as<IRCollectionTaggedUnionType>(info))
        {
            // If this is a tagged union, we need to create a tuple type
            // return getTypeForExistential(taggedUnion);
            return (IRType*)taggedUnion;
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
            // Don't specialize these collections.. they should be used through
            // tag types, or be processed out during specializeing.
            //
            return nullptr;
        }

        return (IRType*)info;
        // SLANG_UNEXPECTED("Unhandled IRTypeFlowData type in getLoweredType");
    }

    bool replaceType(IRInst* context, IRInst* inst)
    {
        if (as<IRModuleInst>(inst->getParent()))
        {
            if (as<IRType>(inst) || as<IRWitnessTable>(inst) || as<IRFunc>(inst) ||
                as<IRGeneric>(inst))
            {
                // Don't replace global concrete vals.
                return false;
            }
        }

        if (auto info = tryGetInfo(context, inst))
        {
            if (auto specializedType = getLoweredType(info))
            {
                if (specializedType == inst->getDataType())
                    return false; // No change
                inst->setFullType(specializedType);
                return true;
            }
        }
        return false;
    }

    bool specializeInst(IRInst* context, IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_LookupWitnessMethod:
            return specializeLookupWitnessMethod(context, as<IRLookupWitnessMethod>(inst));
        case kIROp_ExtractExistentialWitnessTable:
            return specializeExtractExistentialWitnessTable(
                context,
                as<IRExtractExistentialWitnessTable>(inst));
        case kIROp_ExtractExistentialType:
            return specializeExtractExistentialType(context, as<IRExtractExistentialType>(inst));
        case kIROp_ExtractExistentialValue:
            return specializeExtractExistentialValue(context, as<IRExtractExistentialValue>(inst));
        case kIROp_Call:
            return specializeCall(context, as<IRCall>(inst));
        case kIROp_MakeExistential:
            return specializeMakeExistential(context, as<IRMakeExistential>(inst));
        case kIROp_MakeStruct:
            return specializeMakeStruct(context, as<IRMakeStruct>(inst));
        case kIROp_CreateExistentialObject:
            return specializeCreateExistentialObject(context, as<IRCreateExistentialObject>(inst));
        case kIROp_RWStructuredBufferLoad:
        case kIROp_StructuredBufferLoad:
            return specializeStructuredBufferLoad(context, inst);
        case kIROp_Specialize:
            return specializeSpecialize(context, as<IRSpecialize>(inst));
        case kIROp_GetValueFromBoundInterface:
            return specializeGetValueFromBoundInterface(
                context,
                as<IRGetValueFromBoundInterface>(inst));
        case kIROp_Load:
            return specializeLoad(context, inst);
        case kIROp_Store:
            return specializeStore(context, as<IRStore>(inst));
        case kIROp_GetSequentialID:
            return specializeGetSequentialID(context, as<IRGetSequentialID>(inst));
        case kIROp_IsType:
            return specializeIsType(context, as<IRIsType>(inst));
        default:
            {
                // Default case: replace inst type with specialized type (if available)
                if (tryGetInfo(context, inst))
                    return replaceType(context, inst);
                /*else if (auto resolvedType = getResolvedInst(inst->getDataType()))
                {
                    if (resolvedType == inst->getDataType())
                        return false;

                    // If the type has changed due to other specializations, update it.
                    inst->setFullType((IRType*)resolvedType);
                    return true;
                }*/

                return false;
            }
        }
    }

    bool specializeLookupWitnessMethod(IRInst* context, IRLookupWitnessMethod* inst)
    {
        // Handle trivial case.
        if (auto witnessTable = as<IRWitnessTable>(inst->getWitnessTable()))
        {
            inst->replaceUsesWith(findWitnessTableEntry(witnessTable, inst->getRequirementKey()));
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

        if (auto typeCollection = as<IRTypeCollection>(collectionTagType->getCollection()))
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

    bool specializeExtractExistentialWitnessTable(
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
            // Replace with GetElement(specializedInst, 0) -> uint
            auto operand = inst->getOperand(0);
            auto element = builder.emitGetTupleElement((IRType*)collectionTagType, operand, 0);
            inst->replaceUsesWith(element);
            inst->removeAndDeallocate();
            return true;
        }
    }

    bool specializeExtractExistentialValue(IRInst* context, IRExtractExistentialValue* inst)
    {
        SLANG_UNUSED(context);

        auto existential = inst->getOperand(0);
        auto existentialInfo = existential->getDataType();
        if (as<IRCollectionTaggedUnionType>(existentialInfo))
        {
            auto valType = existentialInfo->getOperand(0);
            IRBuilder builder(inst);
            builder.setInsertAfter(inst);

            auto val = builder.emitGetTupleElement((IRType*)valType, existential, 1);
            inst->replaceUsesWith(val);
            inst->removeAndDeallocate();
            return true;
        }

        return false;
    }

    bool specializeExtractExistentialType(IRInst* context, IRExtractExistentialType* inst)
    {
        auto info = tryGetInfo(context, inst);
        auto collectionTagType = as<IRCollectionTagType>(info);
        if (!collectionTagType)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        if (collectionTagType->isSingleton())
        {
            // Found a single possible type. Simple replacement.
            auto singletonValue = collectionTagType->getCollection()->getElement(0);
            inst->replaceUsesWith(singletonValue);
            inst->removeAndDeallocate();
            return true;
        }

        // Replace the instruction with the collection type.
        inst->replaceUsesWith(collectionTagType->getCollection());
        inst->removeAndDeallocate();
        return true;
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
            auto newCollection = cBuilder.createCollection(collection->getOp(), collectionElements);
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
        else if (
            as<IRCollectionTaggedUnionType>(currentType) &&
            as<IRCollectionTaggedUnionType>(newType))
        {
            // Merge the elements of both tagged unions into a new tuple type
            return (IRType*)makeExistential((as<IRTableCollection>(updateType(
                (IRType*)as<IRCollectionTaggedUnionType>(currentType)->getTableCollection(),
                (IRType*)as<IRCollectionTaggedUnionType>(newType)->getTableCollection()))));
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
            auto newCollection =
                cBuilder.createCollection(kIROp_TypeCollection, collectionElements);
            return (IRType*)newCollection;
        }
    }

    IRFuncType* getEffectiveFuncType(IRInst* callee)
    {
        IRBuilder builder(module);

        List<IRType*> paramTypes;
        IRType* resultType = nullptr;

        auto updateParamType = [&](Index index, IRType* paramType) -> IRType*
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
                auto [currentDirection, currentType] = splitDirectionAndType(paramTypes[index]);
                auto [newDirection, newType] = splitDirectionAndType(paramType);
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

            for (Index i = 0; i < paramEffectiveTypes.getCount(); i++)
                updateParamType(i, getLoweredType(paramEffectiveTypes[i]));

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
        // Add in extra parameter types for a call to a non-concrete callee.
        //

        List<IRType*> extraParamTypes;
        // If the callee is a collection, then we need a tag as input.
        if (auto funcCollection = as<IRFuncCollection>(callee))
        {
            // If this is a non-trivial collection, we need to add a tag type for the collection
            // as the first parameter.
            if (getCollectionCount(funcCollection) > 1)
                extraParamTypes.add((IRType*)makeTagType(funcCollection));
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
            return context; // Not specialized yet.

        if (this->loweredContexts.containsKey(context))
            return this->loweredContexts[context];
        else
            this->contextsToLower.add(context);

        return context;
    }

    List<IRInst*> getArgsForDynamicSpecialization(IRSpecialize* specializedCallee)
    {
        List<IRInst*> callArgs;
        for (UInt ii = 0; ii < specializedCallee->getArgCount(); ii++)
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

    bool specializeCallToDynamicGeneric(IRInst* context, IRCall* inst)
    {
        auto specializedCallee = as<IRSpecialize>(inst->getCallee());
        auto calleeInfo = tryGetInfo(context, specializedCallee);
        auto calleeCollection = as<IRCollectionBase>(calleeInfo);
        if (!calleeCollection || getCollectionCount(calleeCollection) != 1)
            return false;

        auto targetContext = getCollectionElement(calleeCollection, 0);

        List<IRInst*> callArgs;
        for (UInt ii = 0; ii < specializedCallee->getArgCount(); ii++)
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
                    // For now, we will not specialize this case.
                    SLANG_UNEXPECTED("Unhandled type-flow-collection in dynamic generic call");
                }
            }
        }

        for (UInt ii = 0; ii < inst->getArgCount(); ii++)
            callArgs.add(inst->getArg(ii));

        IRBuilder builder(inst->getModule());
        builder.setInsertBefore(inst);
        auto newCallInst = builder.emitCallInst(
            as<IRFuncType>(targetContext->getDataType())->getResultType(),
            getCalleeForContext(targetContext),
            callArgs);
        inst->replaceUsesWith(newCallInst);
        inst->removeAndDeallocate();
        return true;
    }

    void maybeSpecializeCalleeType(IRInst* callee)
    {
        if (auto specializeInst = as<IRSpecialize>(callee->getDataType()))
        {
            if (isGlobalInst(specializeInst))
                callee->setFullType((IRType*)specializeGeneric(specializeInst));
        }
    }

    bool specializeCall(IRInst* context, IRCall* inst)
    {
        auto callee = resolve(inst->getCallee());
        IRInst* calleeTagInst = nullptr;

        // This is a bit of a workaround for specialized callee's
        // whose function types haven't been specialized yet (can
        // occur for concrete IRSpecialize insts that are created
        // during the specializeing process).
        //
        // maybeSpecializeCalleeType(callee);

        // If we're calling using a tag, place a call to the collection,
        // with the tag as the first argument. So the callee is
        // the collection itself.
        //
        if (auto collectionTag = as<IRCollectionTagType>(callee->getDataType()))
        {
            if (!collectionTag->isSingleton())
                calleeTagInst = callee; // Only keep the tag if there are multiple elements.
            callee = collectionTag->getCollection();
        }

        // If by this point, we haven't resolved our callee into a global inst (
        // either a collection or a single function), then we can't specialize it (likely
        // unbounded)
        //
        if (!isGlobalInst(callee) || isIntrinsic(callee))
            return false;

        // One other case to avoid is if the function is a global LookupWitnessMethod
        // which can be created when optional witnesses are specialized.
        //
        // if (as<IRLookupWitnessMethod>(callee))
        //    return false;

        auto expectedFuncType = getEffectiveFuncType(callee);

        List<IRInst*> newArgs;
        IRInst* newCallee = nullptr;

        // Determine a new callee.
        auto calleeCollection = as<IRFuncCollection>(callee);
        if (!calleeCollection)
            newCallee = callee; // Not a collection, no need to specialize
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
        UCount extraArgCount = newArgs.getCount();
        for (UInt i = 0; i < inst->getArgCount(); i++)
        {
            auto arg = inst->getArg(i);
            const auto [paramDirection, paramType] =
                splitDirectionAndType(expectedFuncType->getParamType(i + extraArgCount));

            switch (paramDirection.kind)
            {
            case ParameterDirectionInfo::Kind::In:
                newArgs.add(upcastCollection(context, arg, paramType));
                break;
            case ParameterDirectionInfo::Kind::Out:
            case ParameterDirectionInfo::Kind::InOut:
            case ParameterDirectionInfo::Kind::ConstRef:
            case ParameterDirectionInfo::Kind::Ref:
                {
                    newArgs.add(arg);
                    break;
                }
            default:
                SLANG_UNEXPECTED("Unhandled parameter direction in specializeCall");
            }
        }

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        bool changed = false;
        if (((UInt)newArgs.getCount()) != inst->getArgCount())
            changed = true;
        else
        {
            for (Index i = 0; i < newArgs.getCount(); i++)
            {
                if (newArgs[i] != inst->getArg((UInt)i))
                {
                    changed = true;
                    break;
                }
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

    bool specializeMakeStruct(IRInst* context, IRMakeStruct* inst)
    {
        auto structType = as<IRStructType>(inst->getDataType());
        if (!structType)
            return false;

        // Reinterpret any of the arguments as necessary.
        bool changed = false;
        UIndex operandIndex = 0;
        for (auto field : structType->getFields())
        {
            auto arg = inst->getOperand(operandIndex);
            auto newArg = upcastCollection(context, arg, field->getFieldType());

            if (arg != newArg)
            {
                changed = true;
                inst->setOperand(operandIndex, newArg);
            }

            operandIndex++;
        }

        return changed;
    }

    bool specializeMakeExistential(IRInst* context, IRMakeExistential* inst)
    {
        auto info = tryGetInfo(context, inst);
        auto taggedUnion = as<IRCollectionTaggedUnionType>(info);
        if (!taggedUnion)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Collect types from the witness tables to determine the any-value type
        auto tableCollection = taggedUnion->getTableCollection();
        auto typeCollection = taggedUnion->getTypeCollection();

        IRInst* witnessTableID = nullptr;
        if (auto witnessTable = as<IRWitnessTable>(inst->getWitnessTable()))
        {
            auto singletonTagType = makeTagType(cBuilder.makeSingletonSet(witnessTable));
            auto zeroValueOfTagType = builder.getIntValue((IRType*)singletonTagType, 0);
            witnessTableID = builder.emitIntrinsicInst(
                (IRType*)makeTagType(tableCollection),
                kIROp_GetTagForSuperCollection,
                1,
                &zeroValueOfTagType);
        }
        else if (as<IRCollectionTagType>(inst->getWitnessTable()->getDataType()))
        {
            // Dynamic. Use the witness table inst as a tag
            witnessTableID = inst->getWitnessTable();
        }

        // Create the appropriate any-value type
        auto collectionType = typeCollection->isSingleton() ? (IRType*)typeCollection->getElement(0)
                                                            : (IRType*)typeCollection;

        // Pack the value
        auto packedValue = as<IRTypeCollection>(collectionType)
                               ? builder.emitPackAnyValue(collectionType, inst->getWrappedValue())
                               : inst->getWrappedValue();

        auto taggedUnionType = getLoweredType(taggedUnion);

        // Create tuple (table_unique_id, PackAnyValue(val))
        IRInst* tupleArgs[] = {witnessTableID, packedValue};
        auto tuple = builder.emitMakeTuple(taggedUnionType, 2, tupleArgs);

        inst->replaceUsesWith(tuple);
        inst->removeAndDeallocate();
        return true;
    }

    bool specializeCreateExistentialObject(IRInst* context, IRCreateExistentialObject* inst)
    {
        auto info = tryGetInfo(context, inst);
        auto taggedUnion = as<IRCollectionTaggedUnionType>(info);
        if (!taggedUnion)
            return false;

        auto taggedUnionType = as<IRCollectionTaggedUnionType>(getLoweredType(taggedUnion));

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        List<IRInst*> args;
        args.add(inst->getDataType());
        args.add(inst->getTypeID());
        auto translatedTag = builder.emitIntrinsicInst(
            (IRType*)makeTagType(taggedUnionType->getTableCollection()),
            kIROp_GetTagFromSequentialID,
            args.getCount(),
            args.getBuffer());

        IRInst* packedValue = nullptr;
        auto collection = taggedUnionType->getTypeCollection();
        if (!collection->isSingleton())
        {
            packedValue = builder.emitPackAnyValue((IRType*)collection, inst->getValue());
        }
        else
        {
            packedValue = builder.emitReinterpret(
                (IRType*)taggedUnionType->getTypeCollection(),
                inst->getValue());
        }

        auto newInst = builder.emitMakeTuple(
            (IRType*)taggedUnionType,
            List<IRInst*>({translatedTag, packedValue}));

        inst->replaceUsesWith(newInst);
        inst->removeAndDeallocate();
        return true;
    }

    bool specializeStructuredBufferLoad(IRInst* context, IRInst* inst)
    {
        auto valInfo = tryGetInfo(context, inst);

        if (!valInfo)
            return false;

        auto bufferType = (IRType*)inst->getOperand(0)->getDataType();
        auto bufferBaseType = (IRType*)bufferType->getOperand(0);

        auto specializedValType = (IRType*)getLoweredType(valInfo);
        if (bufferBaseType != specializedValType)
        {
            if (as<IRInterfaceType>(bufferBaseType) && !isComInterfaceType(bufferBaseType) &&
                !isBuiltin(bufferBaseType))
            {
                // If we're dealing with a loading a known tagged union value from
                // an interface-typed pointer, we'll cast the pointer itself and
                // defer the specializeing of the load until later.
                //
                // This avoids having to change the source pointer type
                // and confusing any future runs of the type flow
                // analysis pass.
                //
                IRBuilder builder(inst);
                builder.setInsertAfter(inst);
                auto bufferHandle = inst->getOperand(0);
                auto newHandle = builder.emitIntrinsicInst(
                    builder.getPtrType(specializedValType),
                    kIROp_CastInterfaceToTaggedUnionPtr,
                    1,
                    &bufferHandle);
                List<IRInst*> newLoadOperands = {newHandle, inst->getOperand(1)};
                auto newLoad = builder.emitIntrinsicInst(
                    specializedValType,
                    inst->getOp(),
                    newLoadOperands.getCount(),
                    newLoadOperands.getBuffer());

                inst->replaceUsesWith(newLoad);
                inst->removeAndDeallocate();

                return true;
            }
        }
        else if (inst->getDataType() != bufferBaseType)
        {
            // If the data type is not the same, we need to update it.
            inst->setFullType((IRType*)getLoweredType(valInfo));
            return true;
        }

        return false;
    }

    bool specializeSpecialize(IRInst* context, IRSpecialize* inst)
    {
        bool isDynamic = false;

        // TODO: Would checking this inst's info be enough instead?
        // This seems long-winded.
        if (auto concreteGeneric = as<IRGeneric>(inst->getBase()))
            isDynamic = as<IRFunc>(getGenericReturnVal(concreteGeneric)) ||
                        as<IRWitnessTable>(getGenericReturnVal(concreteGeneric));
        else if (auto tagType = as<IRCollectionTagType>(inst->getBase()->getDataType()))
        {
            auto firstConcreteGeneric = as<IRGeneric>(tagType->getCollection()->getElement(0));
            isDynamic = as<IRFunc>(getGenericReturnVal(firstConcreteGeneric)) ||
                        as<IRWitnessTable>(getGenericReturnVal(firstConcreteGeneric));
        }

        // If the result needs to be dynamic, we'll emit a tag inst.
        //
        // TODO: What if we have multiple elements with dynamic generics?
        //
        if (isDynamic)
        {
            if (auto info = tryGetInfo(context, inst))
            {
                // If our inst represents a collection directly (no run-time info),
                // there's nothing to do except replace the type (if necessary)
                //
                if (as<IRCollectionBase>(info))
                    return replaceType(context, inst);

                auto specializedCollectionTag = as<IRCollectionTagType>(info);

                // If the inst represents a singleton collection, there's nothing
                // to do except replace the type (if necessary)
                //
                if (getCollectionCount(specializedCollectionTag) <= 1)
                    return replaceType(context, inst);

                List<IRInst*> mappingOperands;

                // Add the base tag as the first operand. The mapping operands follow
                mappingOperands.add(inst->getBase());

                forEachInCollection(
                    specializedCollectionTag,
                    [&](IRInst* element)
                    {
                        // Emit the GetTagForSpecializedCollection for each element.
                        auto specInst = cast<IRSpecialize>(element);
                        auto baseGeneric = cast<IRGeneric>(specInst->getBase());

                        mappingOperands.add(baseGeneric);
                        mappingOperands.add(specInst);
                    });

                IRBuilder builder(inst);
                setInsertBeforeOrdinaryInst(&builder, inst);
                auto newInst = builder.emitIntrinsicInst(
                    (IRType*)info,
                    kIROp_GetTagForSpecializedCollection,
                    mappingOperands.getCount(),
                    mappingOperands.getBuffer());

                inst->replaceUsesWith(newInst);
                inst->removeAndDeallocate();
                return true;
            }
            else
                return false;
        }

        // For all other specializations, we'll 'drop' the dynamic tag information.
        bool changed = false;
        List<IRInst*> args;
        for (UIndex i = 0; i < inst->getArgCount(); i++)
        {
            auto arg = inst->getArg(i);
            auto argDataType = arg->getDataType();
            if (auto collectionTagType = as<IRCollectionTagType>(argDataType))
            {
                // If this is a tag type, replace with collection.
                changed = true;
                args.add(collectionTagType->getCollection());
            }
            else
            {
                args.add(arg);
            }
        }

        IRBuilder builder(inst);
        IRType* typeForSpecialization = builder.getTypeKind();

        if (changed)
        {
            auto newInst = builder.emitSpecializeInst(
                typeForSpecialization,
                inst->getBase(),
                args.getCount(),
                args.getBuffer());

            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();
            return true;
        }

        return false;
    }


    bool specializeGetValueFromBoundInterface(IRInst* context, IRGetValueFromBoundInterface* inst)
    {
        SLANG_UNUSED(context);
        auto destType = inst->getDataType();
        auto operandInfo = inst->getOperand(0)->getDataType();
        if (as<IRCollectionTaggedUnionType>(operandInfo))
        {
            IRBuilder builder(inst);
            setInsertAfterOrdinaryInst(&builder, inst);
            auto newInst = builder.emitGetTupleElement((IRType*)destType, inst->getOperand(0), 1);
            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();
            return true;
        }
        return false;
    }

    bool specializeLoad(IRInst* context, IRInst* inst)
    {
        auto valInfo = tryGetInfo(context, inst);

        if (!valInfo)
            return false;

        auto loadPtr = as<IRLoad>(inst)->getPtr();
        auto loadPtrType = as<IRPtrTypeBase>(loadPtr->getDataType());
        auto ptrValType = loadPtrType->getValueType();

        IRType* specializedType = (IRType*)getLoweredType(valInfo);
        if (ptrValType != specializedType)
        {
            SLANG_ASSERT(!as<IRParam>(inst));

            if (as<IRInterfaceType>(ptrValType) && !isComInterfaceType(ptrValType) &&
                !isBuiltin(ptrValType))
            {
                // If we're dealing with a loading a known tagged union value from
                // an interface-typed pointer, we'll cast the pointer itself and
                // defer the specializeing of the load until later.
                //
                // This avoids having to change the source pointer type
                // and confusing any future runs of the type flow
                // analysis pass.
                //
                IRBuilder builder(inst);
                builder.setInsertAfter(inst);
                auto newLoadPtr = builder.emitIntrinsicInst(
                    builder.getPtrTypeWithAddressSpace(specializedType, loadPtrType),
                    kIROp_CastInterfaceToTaggedUnionPtr,
                    1,
                    &loadPtr);
                auto newLoad = builder.emitLoad(specializedType, newLoadPtr);

                inst->replaceUsesWith(newLoad);
                inst->removeAndDeallocate();

                return true;
            }
        }
        else if (inst->getDataType() != ptrValType)
        {
            inst->setFullType((IRType*)getLoweredType(valInfo));
            return true;
        }

        return false;
    }

    bool handleDefaultStore(IRInst* context, IRStore* inst)
    {
        SLANG_UNUSED(context);
        SLANG_ASSERT(inst->getVal()->getOp() == kIROp_DefaultConstruct);
        auto ptr = inst->getPtr();
        auto destInfo = as<IRPtrTypeBase>(ptr->getDataType())->getValueType();
        auto valInfo = inst->getVal()->getDataType();

        // "Legalize" the store type.
        if (destInfo != valInfo)
        {
            inst->getVal()->setFullType(destInfo);
            return true;
        }
        else
            return false;
    }

    bool specializeStore(IRInst* context, IRStore* inst)
    {
        auto ptr = inst->getPtr();
        auto ptrInfo = as<IRPtrTypeBase>(ptr->getDataType())->getValueType();

        // Special case for default initialization:
        //
        // Raw default initialization has been almost entirely
        // removed from Slang, but the auto-diff process can sometimes
        // produce a store of default-constructed value.
        //
        if (as<IRDefaultConstruct>(inst->getVal()))
            return handleDefaultStore(context, inst);

        auto specializedVal = upcastCollection(context, inst->getVal(), ptrInfo);

        if (specializedVal != inst->getVal())
        {
            // If the value was changed, we need to update the store instruction.
            IRBuilder builder(inst);
            builder.replaceOperand(inst->getValUse(), specializedVal);
            return true;
        }

        return false;
    }

    bool specializeGetSequentialID(IRInst* context, IRGetSequentialID* inst)
    {
        SLANG_UNUSED(context);
        auto arg = inst->getOperand(0);
        if (auto tagType = as<IRCollectionTagType>(arg->getDataType()))
        {
            IRBuilder builder(inst);
            setInsertAfterOrdinaryInst(&builder, inst);
            auto firstElement = tagType->getCollection()->getElement(0);
            auto interfaceType =
                as<IRInterfaceType>(as<IRWitnessTable>(firstElement)->getConformanceType());
            List<IRInst*> args = {interfaceType, arg};
            auto newInst = builder.emitIntrinsicInst(
                (IRType*)builder.getUIntType(),
                kIROp_GetSequentialIDFromTag,
                args.getCount(),
                args.getBuffer());

            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();
            return true;
        }

        return false;
    }

    bool specializeIsType(IRInst* context, IRIsType* inst)
    {
        SLANG_UNUSED(context);
        auto witnessTableArg = inst->getValueWitness();
        if (auto tagType = as<IRCollectionTagType>(witnessTableArg->getDataType()))
        {
            IRBuilder builder(inst);
            setInsertAfterOrdinaryInst(&builder, inst);
            auto firstElement = tagType->getCollection()->getElement(0);
            auto interfaceType =
                as<IRInterfaceType>(as<IRWitnessTable>(firstElement)->getConformanceType());

            // TODO: This is a rather suboptimal implementation that involves using
            // global sequential IDs even though we could do it via local IDs.
            //

            List<IRInst*> args = {interfaceType, witnessTableArg};
            auto valueSeqID = builder.emitIntrinsicInst(
                (IRType*)builder.getUIntType(),
                kIROp_GetSequentialIDFromTag,
                args.getCount(),
                args.getBuffer());

            auto targetSeqID = builder.emitGetSequentialIDInst(inst->getTargetWitness());
            auto eqlInst = builder.emitEql(valueSeqID, targetSeqID);

            inst->replaceUsesWith(eqlInst);
            inst->removeAndDeallocate();
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

    bool processModule()
    {
        bool hasChanges = false;

        // Phase 1: Information Propagation
        performInformationPropagation();

        // Phase 2: Dynamic Instruction Lowering
        hasChanges |= performDynamicInstLowering();

        return hasChanges;
    }

    TypeFlowSpecializationContext(IRModule* module, DiagnosticSink* sink)
        : module(module), sink(sink), cBuilder(module), translationContext(module, sink)
    {
    }


    // Basic context
    IRModule* module;
    DiagnosticSink* sink;

    // Mapping from instruction to propagation information
    Dictionary<Element, IRInst*> propagationMap;

    // Mapping from function to return value propagation information
    Dictionary<IRInst*, IRInst*> funcReturnInfo;

    // Mapping from struct fields to propagation information
    Dictionary<IRStructField*, IRInst*> fieldInfo;

    // Mapping from functions to call-sites.
    Dictionary<IRInst*, HashSet<Element>> funcCallSites;

    // Mapping from fields to use-sites.
    Dictionary<IRStructField*, HashSet<Element>> fieldUseSites;

    // Mapping from specialized instruction to their any-value types
    Dictionary<IRInst*, IRInst*> loweredInstToAnyValueType;

    // Set of open contexts
    HashSet<IRInst*> availableContexts;

    // Contexts requiring lowering
    HashSet<IRInst*> contextsToLower;

    // Lowered contexts.
    Dictionary<IRInst*, IRInst*> loweredContexts;

    // Context for building collection insts
    CollectionBuilder cBuilder;

    // Translation context for resolving global on-demand
    // translation insts.
    //
    TranslationContext translationContext;
};

// Main entry point
bool specializeDynamicInsts(IRModule* module, DiagnosticSink* sink)
{
    TypeFlowSpecializationContext context(module, sink);
    return context.processModule();
}

} // namespace Slang
