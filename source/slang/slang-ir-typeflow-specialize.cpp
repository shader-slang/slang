#include "slang-ir-typeflow-specialize.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-clone.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize.h"
#include "slang-ir-typeflow-collection.h"
#include "slang-ir-util.h"
#include "slang-ir-witness-table-wrapper.h"
#include "slang-ir.h"


namespace Slang
{

// Basic unit for which we keep track of propagation information.
//
// This unit has two components: an 'inst' and a 'context' under which we
// are recording propagation info.
//
// The 'inst' must be inside a block (with either generic or func parent), since
// we assume everything in the global scope is concrete.
//
// The 'context' can be one of two cases:
// 1. an IRFunc ONLY if it is not generic (func is in the global scope). 'inst' must
//    be inside the func.
// 2. an IRSpecialize(generic, ...). 'inst' must be inside the generic. the
//    specialization args must all be global values (either concrete types/values, or collections).
//
// All other possibilites for 'context' are illegal.
// `InstWithContext::validateInstWithContext` enforces these rules.
//
// For an inst inside a generic, it is possible to have different propagation information
// depending on the specialization args, which is why we use the pair to keep track of the context.
//
struct InstWithContext
{
    IRInst* context;
    IRInst* inst;

    InstWithContext()
        : context(nullptr), inst(nullptr)
    {
    }

    InstWithContext(IRInst* context, IRInst* inst)
        : context(context), inst(inst)
    {
        validateInstWithContext();
    }

    void validateInstWithContext() const
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
                SLANG_UNEXPECTED("Invalid context for InstWithContext");
            }
        }
    }

    // If a context is not specified, we assume it is not in a generic, and
    // simply use the parent func.
    //
    InstWithContext(IRInst* inst)
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

    bool operator==(const InstWithContext& other) const
    {
        return context == other.context && inst == other.inst;
    }

    HashCode64 getHashCode() const { return combineHash(HashCode(context), HashCode(inst)); }
};

// Test if inst represents a pointer to a global resource.
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

bool isNoneCallee(IRInst* callee)
{
    if (auto lookupWitness = as<IRLookupWitnessMethod>(callee))
    {
        if (auto table = as<IRWitnessTable>(lookupWitness->getWitnessTable()))
        {
            return table->getConcreteType()->getOp() == kIROp_VoidType;
        }
    }

    return false;
}

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
    IRInst* targetContext; // The function/specialized-generic being called or returned from

    InterproceduralEdge() = default;
    InterproceduralEdge(Direction dir, IRInst* callerContext, IRCall* call, IRInst* func)
        : direction(dir), callerContext(callerContext), callInst(call), targetContext(func)
    {
    }
};

// Representation of a work item used to register work for the main propagation queue.
// When the propagation information for a particular inst is modified non-trivially, new
// 'WorkItem' objects are added to the queue to further propagate the changes.
//
// The "Type" captures the granularity & type of the propagation work, while the union
// holds on to any auxiliary information.
//
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
        InstWithContext(context, inst).validateInstWithContext();
    }

    WorkItem(IRInst* context, IRBlock* block)
        : type(Type::Block), block(block), context(context)
    {
        SLANG_ASSERT(context != nullptr && block != nullptr);
        // Validate that the context is appropriate for the block
        InstWithContext(context, block->getFirstChild()).validateInstWithContext();
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

// Returns true if the two propagation infos are equal.
bool areInfosEqual(IRInst* a, IRInst* b)
{
    // Since all inst opcodes that are used to represent propagation information
    // are hoistable and automatically de-duplicated by the Slang IR infrastructure,
    // we can simply test pointer equality
    //
    return a == b;
}

// Helper data-structure for efficient enqueueing/dequeueing of work items.
//
// Our 'List' data-structure are currently designed to be efficient at operating as a stack
// but have poor performance for queue-like operations, so this rolls two stacks into a queue
// structure.
//
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

// Tests whether a generic can be fully specialized, or if it requires a dynamic information.
//
// This test is primarily used to determine if additional parameters are requried to place a call to
// this callee.
//
bool isSetSpecializedGeneric(IRInst* callee)
{
    // If the callee is a specialization, and at least one of its arguments
    // is a collection, then it needs dynamic-dispatch logic to be generated.
    //
    if (auto specialize = as<IRSpecialize>(callee))
    {
        for (UInt i = 0; i < specialize->getArgCount(); i++)
        {
            // Only functions need set-aware specialization.
            auto generic = specialize->getBase();
            if (getGenericReturnVal(generic)->getOp() != kIROp_Func)
                return false;

            auto arg = specialize->getArg(i);
            if (as<IRSetBase>(arg))
                return true; // Found a set argument
        }
        return false; // No set arguments found
    }

    return false;
}

//
// Helper struct to represent a parameter's direction and type component.
// This is used by the type flow system to figure out which direction to propagate
// information for each parameter.
//
struct ParameterDirectionInfo
{
    enum Kind
    {
        In,
        BorrowIn,
        Out,
        BorrowInOut,
        Ref
    } kind;

    // For Ref and BorrowInOut
    AddressSpace addressSpace;

    ParameterDirectionInfo(Kind kind, AddressSpace addressSpace = (AddressSpace)0)
        : kind(kind), addressSpace(addressSpace)
    {
    }

    ParameterDirectionInfo()
        : kind(Kind::In), addressSpace((AddressSpace)0)
    {
    }

    bool operator==(const ParameterDirectionInfo& other) const
    {
        return kind == other.kind && addressSpace == other.addressSpace;
    }
};

// Split parameter type into a direction and a type
std::tuple<ParameterDirectionInfo, IRType*> splitParameterDirectionAndType(IRType* paramType)
{
    if (as<IROutParamType>(paramType))
        return {
            ParameterDirectionInfo(ParameterDirectionInfo::Kind::Out),
            as<IROutParamType>(paramType)->getValueType()};
    else if (as<IRBorrowInOutParamType>(paramType))
        return {
            ParameterDirectionInfo(ParameterDirectionInfo::Kind::BorrowInOut),
            as<IRBorrowInOutParamType>(paramType)->getValueType()};
    else if (as<IRRefParamType>(paramType))
        return {
            ParameterDirectionInfo(
                ParameterDirectionInfo::Kind::Ref,
                as<IRRefParamType>(paramType)->getAddressSpace()),
            as<IRRefParamType>(paramType)->getValueType()};
    else if (as<IRBorrowInParamType>(paramType))
        return {
            ParameterDirectionInfo(
                ParameterDirectionInfo::Kind::BorrowIn,
                as<IRBorrowInParamType>(paramType)->getAddressSpace()),
            as<IRBorrowInParamType>(paramType)->getValueType()};
    else
        return {ParameterDirectionInfo(ParameterDirectionInfo::Kind::In), paramType};
}

// Join parameter direction and a type back into a parameter type
IRType* fromDirectionAndType(IRBuilder* builder, ParameterDirectionInfo info, IRType* type)
{
    switch (info.kind)
    {
    case ParameterDirectionInfo::Kind::In:
        return type;
    case ParameterDirectionInfo::Kind::Out:
        return builder->getOutParamType(type);
    case ParameterDirectionInfo::Kind::BorrowInOut:
        return builder->getBorrowInOutParamType(type);
    case ParameterDirectionInfo::Kind::BorrowIn:
        return builder->getBorrowInParamType(type, info.addressSpace);
    case ParameterDirectionInfo::Kind::Ref:
        return builder->getRefParamType(type, info.addressSpace);
    default:
        SLANG_UNEXPECTED("Unhandled parameter info in fromDirectionAndType");
    }
}

// Helper to check if an IRParam is a function parameter (vs. a phi param or generic param)
bool isFuncParam(IRParam* param)
{
    auto paramBlock = as<IRBlock>(param->getParent());
    auto paramFunc = as<IRFunc>(paramBlock->getParent());
    return (paramFunc && paramFunc->getFirstBlock() == paramBlock);
}

// Helper to test if an inst is in the global scope.
bool isGlobalInst(IRInst* inst)
{
    return inst->getParent()->getOp() == kIROp_ModuleInst;
}

// Helper to test if a function or generic contains a body (i.e. is intrinsic/external)
// For the purposes of type-flow, if a function body is not available, we can't analyze it.
//
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

// Parent context for the full type-flow pass.
struct TypeFlowSpecializationContext
{
    // Create a tagged-union-type out of a given collection of tables.
    //
    // This type can be used for insts that are semantically a tuple of a tag (to select a table)
    // and a payload to contain the existential value.
    //
    IRTaggedUnionType* makeTaggedUnionType(IRWitnessTableSet* tableSet)
    {
        IRBuilder builder(module);
        HashSet<IRInst*> typeSet;

        // Create a type collection out of the base types from each table.
        forEachInSet(
            tableSet,
            [&](IRInst* witnessTable)
            {
                if (auto table = as<IRWitnessTable>(witnessTable))
                    typeSet.add(table->getConcreteType());
            });

        // Create the tagged union type out of the type and table collection.
        return builder.getTaggedUnionType(
            tableSet,
            cast<IRTypeSet>(builder.getSet(kIROp_TypeSet, typeSet)));
    }

    // Create an unbounded collection.
    //
    // This collection is a catch-all for
    // all cases where we can't enumerate the possibilites. We use this as
    // a sentinel value to figure out when NOT to specialize a given inst.
    //
    // Most commonly occurs with COM interface types.
    //
    IRUnboundedSet* makeUnbounded()
    {
        IRBuilder builder(module);
        return as<IRUnboundedSet>(
            builder.emitIntrinsicInst(nullptr, kIROp_UnboundedSet, 0, nullptr));
    }

    // Creates an 'empty' inst (denoted by nullptr), that
    // can be used to denote one of two things:
    //
    // 1. This inst does not have any dynamic components to specialize.
    //      e.g. an inst with a concrete int-type.
    // 2. No possibilties have been propagated for this inst yet. This is the
    //    default starting state of all insts.
    //
    // From an order-theoretic perspective, 'none' is the bottom of the lattice.
    //
    IRInst* none() { return nullptr; }

    IRUntaggedUnionType* makeUntaggedUnionType(IRTypeSet* typeSet)
    {
        IRBuilder builder(module);
        return builder.getUntaggedUnionType(typeSet);
    }

    IRElementOfSetType* makeElementOfSetType(IRSetBase* collection)
    {
        IRBuilder builder(module);
        return builder.getElementOfSetType(collection);
    }

    IRSetTagType* makeTagType(IRSetBase* collection)
    {
        IRBuilder builder(module);
        return builder.getSetTagType(collection);
    }

    IRInst* _tryGetInfo(InstWithContext element)
    {
        auto found = propagationMap.tryGetValue(element);
        if (found)
            return *found;
        return none(); // Default info for any inst that we haven't registered.
    }

    bool isConcreteType(IRInst* inst)
    {
        if (!isGlobalInst(inst) || as<IRInterfaceType>(inst) ||
            as<IRWitnessTableType>(inst) && as<IRFuncType>(inst))
            return false;

        if (as<IRPtrTypeBase>(inst))
        {
            auto ptrType = as<IRPtrTypeBase>(inst);
            return isConcreteType(ptrType->getValueType());
        }

        if (as<IRArrayType>(inst))
        {
            auto arrayType = as<IRArrayType>(inst);
            return isConcreteType(arrayType->getElementType()) &&
                   isGlobalInst(arrayType->getElementCount());
        }

        if (as<IROptionalType>(inst))
        {
            auto optionalType = as<IROptionalType>(inst);
            return isConcreteType(optionalType->getValueType());
        }

        return true;
    }

    IRInst* tryGetArgInfo(IRInst* context, IRInst* inst)
    {
        if (auto info = tryGetInfo(context, inst))
            return info;

        IRBuilder builder(module);
        if (auto ptrType = as<IRPtrTypeBase>(inst->getDataType()))
        {
            if (isConcreteType(ptrType->getValueType()))
                return builder.getPtrTypeWithAddressSpace(
                    builder.getUntaggedUnionType(
                        cast<IRTypeSet>(builder.getSingletonSet(ptrType->getValueType()))),
                    ptrType);
            else
                return none();
        }

        if (auto arrayType = as<IRArrayType>(inst->getDataType()))
        {
            if (isConcreteType(arrayType))
            {
                return builder.getArrayType(
                    builder.getUntaggedUnionType(
                        cast<IRTypeSet>(builder.getSingletonSet(arrayType->getElementType()))),
                    arrayType->getElementCount());
            }
            else
                return none();
        }

        if (isConcreteType(inst->getDataType()))
            return builder.getUntaggedUnionType(
                cast<IRTypeSet>(builder.getSingletonSet(inst->getDataType())));
        else
            return none();
    }

    //
    // Bottleneck method to fetch the current propagation info
    // for a given instruction under context.
    //
    IRInst* tryGetInfo(IRInst* context, IRInst* inst)
    {
        if (inst->getDataType())
        {
            switch (inst->getDataType()->getOp())
            {
            case kIROp_TaggedUnionType:
            case kIROp_UntaggedUnionType:
            case kIROp_ElementOfSetType:
                // These insts directly represent type-flow information,
                // so we return them directly.
                return inst->getDataType();
            }
        }

        // A small check for de-allocated insts.
        if (!inst->getParent())
            return none();

        // Global insts always have no info.
        if (as<IRModuleInst>(inst->getParent()))
            return none();

        return _tryGetInfo(InstWithContext(context, inst));
    }

    // Performs set-union over the two collections, and returns a new
    // inst to represent the collection.
    //
    template<typename T>
    T* unionSet(T* collection1, T* collection2)
    {
        // It may be possible to accelerate this further, but we usually
        // don't have to deal with overly large sets (usually 3-20 elements)
        //

        SLANG_ASSERT(as<IRSetBase>(collection1) && as<IRSetBase>(collection2));
        SLANG_ASSERT(collection1->getOp() == collection2->getOp());

        if (!collection1)
            return collection2;
        if (!collection2)
            return collection1;
        if (collection1 == collection2)
            return collection1;

        HashSet<IRInst*> allValues;
        // Collect all values from both collections
        forEachInSet(collection1, [&](IRInst* value) { allValues.add(value); });
        forEachInSet(collection2, [&](IRInst* value) { allValues.add(value); });

        IRBuilder builder(module);
        return as<T>(builder.getSet(
            collection1->getOp(),
            allValues)); // Create a new collection with the union of values
    }

    // Find the union of two propagation info insts, and return and
    // inst representing the result.
    //
    IRInst* unionPropagationInfo(IRInst* info1, IRInst* info2)
    {
        // This is similar to unionSet, but must consider structures that
        // can be built out of collections.
        //
        // We allow some level of nesting of collections into other type instructions,
        // to let us propagate information elegantly for pointers, parameters, arrays
        // and existential tuples.
        //
        // A few interesting cases are missing, but could be added in easily in the future:
        //    - TupleType (will allow us to propagate information for each tuple element)
        //    - OptionalType
        //

        // Basic cases: if either is null, it is considered "empty"
        // if they're equal, union must be the same inst.

        if (!info1)
            return info2;

        if (!info2)
            return info1;

        if (areInfosEqual(info1, info2))
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

        if (as<IRUnboundedSet>(info1) && as<IRUnboundedSet>(info2))
        {
            // If either info is unbounded, the union is unbounded
            return makeUnbounded();
        }

        // For all other cases which are structured composites of collections,
        // we simply take the collection union for all the collection operands.
        //

        if (as<IRTaggedUnionType>(info1) && as<IRTaggedUnionType>(info2))
        {
            return makeTaggedUnionType(unionSet<IRWitnessTableSet>(
                as<IRTaggedUnionType>(info1)->getWitnessTableSet(),
                as<IRTaggedUnionType>(info2)->getWitnessTableSet()));
        }

        if (as<IRSetTagType>(info1) && as<IRSetTagType>(info2))
        {
            return makeTagType(unionSet<IRSetBase>(
                cast<IRSetBase>(info1->getOperand(0)),
                cast<IRSetBase>(info2->getOperand(0))));
        }

        if (as<IRElementOfSetType>(info1) && as<IRElementOfSetType>(info2))
        {
            return makeElementOfSetType(unionSet<IRSetBase>(
                cast<IRSetBase>(info1->getOperand(0)),
                cast<IRSetBase>(info2->getOperand(0))));
        }

        if (as<IRUntaggedUnionType>(info1) && as<IRUntaggedUnionType>(info2))
        {
            return makeUntaggedUnionType(unionSet<IRTypeSet>(
                cast<IRTypeSet>(info1->getOperand(0)),
                cast<IRTypeSet>(info2->getOperand(0))));
        }

        SLANG_UNEXPECTED("Unhandled propagation info types in unionPropagationInfo");
    }

    // Centralized method to update propagation info and add
    // relevant work items to the work queue if the info changed.
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
        propagationMap[InstWithContext(context, inst)] = unionedInfo;

        // Add all users to appropriate work items
        addUsersToWorkQueue(context, inst, unionedInfo, workQueue);
    }

    void addContextUsersToWorkQueue(IRInst* context, WorkQueue& workQueue)
    {
        if (this->funcCallSites.containsKey(context))
            for (auto callSite : this->funcCallSites[context])
            {
                workQueue.enqueue(WorkItem(
                    InterproceduralEdge::Direction::FuncToCall,
                    callSite.context,
                    as<IRCall>(callSite.inst),
                    context));
            }
    }

    // Helper method to add new work items to the queue based on the
    // flavor of the instruction whose info has been updated.
    //
    void addUsersToWorkQueue(IRInst* context, IRInst* inst, IRInst* info, WorkQueue& workQueue)
    {
        // This method is responsible for ensuring the following property:
        //
        // If inst's information has changed, then all insts that may potentially depend
        // (directly) on it should be added to the work queue.
        //
        // This includes a few cases:
        //
        // 1. In the default case, we simply add all users of the insts.
        //
        // 2. For insts that are used as phi-arguments, we add an intra-procedural
        //    edge to the target block.
        //
        // 3. For insts that are used as return values, we add an inter-procedural edge
        //    to all call sites. We do this by calling `updateFuncReturnInfo`, which takes
        //    care of modifying the workQueue.
        //

        if (auto param = as<IRParam>(inst))
            if (isFuncParam(param))
                addContextUsersToWorkQueue(context, workQueue);

        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();

            // If user is in a different block (or the inst is a param), add that block to work
            // queue.
            //
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

            // TODO: Stopgap workaround.
            // Add an analyzeFuncType for this..
            if (auto funcType = as<IRFuncType>(user))
                if (as<IRBlock>(funcType->getParent()))
                    addUsersToWorkQueue(context, funcType, none(), workQueue);
        }
    }

    // Helper method to update function's return info and propagate back to call sites
    void updateFuncReturnInfo(IRInst* callable, IRInst* returnInfo, WorkQueue& workQueue)
    {
        // Don't update info if the callee has a concrete return type.
        auto callableFuncType = cast<IRFuncType>(callable->getDataType());
        if (isConcreteType(callableFuncType->getResultType()))
            return;

        auto existingReturnInfo = getFuncReturnInfo(callable);
        auto newReturnInfo = unionPropagationInfo(existingReturnInfo, returnInfo);

        if (!areInfosEqual(existingReturnInfo, newReturnInfo))
        {
            funcReturnInfo[callable] = newReturnInfo;

            // Add interprocedural edges from the function back to all callsites.
            if (funcCallSites.containsKey(callable))
            {
                for (auto callSite : funcCallSites[callable])
                {
                    workQueue.enqueue(WorkItem(
                        InterproceduralEdge::Direction::FuncToCall,
                        callSite.context,
                        as<IRCall>(callSite.inst),
                        callable));
                }
            }
        }
    }

    void performInformationPropagation()
    {
        // This method contains the main loop responsible for propagating information across all
        // relevant functions, generics in the call graph.
        //
        // The mechanism is similar to data-flow analysis:
        // 1. We start by initializing the propagation info for all instructions in functions
        //    that may be externally called.
        //
        // 2. For each instruction that received a non-trivial update, we add their users to the
        // queue
        //    for further propagation.
        //
        // 3. Continue (2) until no more information has changed.
        //
        // This process is guaranteed to terminate because our propagation 'states' (i.e.
        // collection insts and their wrapped versions) form a lattice. This is an order-theoretic
        // structure that implies that
        //      (i) each update moves us strictly 'upward', and
        //      (ii) that there are a finite number of possible states.
        //

        // Global worklist for interprocedural analysis.
        WorkQueue workQueue;

        // Add all global functions to worklist.
        //
        // This could potentially be narrowed down to just entry points, but for
        // now we are being conservative. Missing a potential entry point is worse
        // than analyzing something that isn't used.
        //
        for (auto inst : module->getGlobalInsts())
            if (auto func = as<IRFunc>(inst))
                discoverContext(func, workQueue);

        // Process until fixed point.
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


    void processInstForPropagation(IRInst* context, IRInst* inst, WorkQueue& workQueue)
    {
        IRInst* info = nullptr;

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
        case kIROp_MakeOptionalNone:
            info = analyzeMakeOptionalNone(context, as<IRMakeOptionalNone>(inst));
            break;
        case kIROp_MakeOptionalValue:
            info = analyzeMakeOptionalValue(context, as<IRMakeOptionalValue>(inst));
            break;
        }

        if (!info && inst->getDataType())
        {
            if (auto dataTypeInfo = tryGetInfo(context, inst->getDataType()))
            {
                if (auto elementOfSetType = as<IRElementOfSetType>(dataTypeInfo))
                {
                    info = makeUntaggedUnionType(cast<IRTypeSet>(elementOfSetType->getSet()));
                }
            }
        }

        if (info)
            updateInfo(context, inst, info, false, workQueue);
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
            auto val = returnInfo->getVal();
            updateFuncReturnInfo(context, tryGetArgInfo(context, val), workQueue);
        }
    };

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
                if (auto argInfo = tryGetArgInfo(context, arg))
                {
                    // Use centralized update method
                    updateInfo(context, param, argInfo, true, workQueue);
                }
            }
            paramIndex++;
        }
    }

    void propagateInterproceduralEdge(InterproceduralEdge edge, WorkQueue& workQueue)
    {
        // Handle interprocedural edge
        auto callInst = edge.callInst;
        auto targetCallee = edge.targetContext;

        switch (edge.direction)
        {
        case InterproceduralEdge::Direction::CallToFunc:
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
                    if (argIndex < callInst->getOperandCount())
                    {
                        auto arg = callInst->getOperand(argIndex);
                        const auto [paramDirection, paramType] =
                            splitParameterDirectionAndType(param->getDataType());

                        // Only update if the parameter is not a concrete type.
                        //
                        // This is primarily just an optimization.
                        // Without this, we'd be storing 'singleton' sets for parameters with
                        // regular concrete types (i.e. 99% of cases), which can clog up
                        // the propagation dictionary when analyzing large modules.
                        // This optimization ignores them and re-derives the info
                        // from the data-type.
                        //
                        if (isConcreteType(paramType))
                        {
                            argIndex++;
                            continue;
                        }

                        IRInst* argInfo = tryGetArgInfo(edge.callerContext, arg);

                        switch (paramDirection.kind)
                        {
                        case ParameterDirectionInfo::Kind::Out:
                        case ParameterDirectionInfo::Kind::BorrowInOut:
                        case ParameterDirectionInfo::Kind::BorrowIn:
                            {
                                IRBuilder builder(module);
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
        case InterproceduralEdge::Direction::FuncToCall:
            {
                // If the call inst cannot accept anything dynamic, then
                // no need to propagate anything to the result of the call inst.
                //
                // We'll still need to consider out parameters separately.
                //
                if (!isConcreteType(callInst->getDataType()))
                {
                    auto returnInfoPtr = funcReturnInfo.tryGetValue(targetCallee);
                    auto returnInfo = (returnInfoPtr) ? *returnInfoPtr : nullptr;
                    if (!returnInfo)
                    {
                        // If the targetCallee's return type is concrete, but the
                        // callInst's return type is not, we should still propagate the
                        // known concrete type.
                        //
                        auto concreteReturnType =
                            cast<IRFuncType>(targetCallee->getDataType())->getResultType();
                        if (isConcreteType(concreteReturnType))
                        {
                            IRBuilder builder(module);
                            returnInfo = builder.getUntaggedUnionType(
                                cast<IRTypeSet>(builder.getSingletonSet(concreteReturnType)));
                        }
                    }

                    if (returnInfo)
                    {
                        // Use centralized update method
                        updateInfo(edge.callerContext, callInst, returnInfo, true, workQueue);
                    }
                }

                // Also update infos of any out parameters
                auto paramInfos = getParamInfos(edge.targetContext);
                auto paramDirections = getParamDirections(edge.targetContext);
                UIndex argIndex = 0;
                for (auto paramInfo : paramInfos)
                {
                    if (paramInfo)
                    {
                        if (paramDirections[argIndex].kind == ParameterDirectionInfo::Kind::Out ||
                            paramDirections[argIndex].kind ==
                                ParameterDirectionInfo::Kind::BorrowInOut)
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

                break;
            }
        default:
            SLANG_UNEXPECTED("Unhandled interprocedural edge direction");
            return;
        }
    }

    IRInst* analyzeCreateExistentialObject(IRInst* context, IRCreateExistentialObject* inst)
    {
        SLANG_UNUSED(context);

        IRBuilder builder(module);
        if (auto interfaceType = as<IRInterfaceType>(inst->getDataType()))
        {
            if (isComInterfaceType(interfaceType) || isBuiltin(interfaceType))
            {
                // If this is a COM interface, we treat it as unbounded
                return makeUnbounded();
            }

            auto tables = collectExistentialTables(interfaceType);
            if (tables.getCount() > 0)
                return makeTaggedUnionType(
                    as<IRWitnessTableSet>(builder.getSet(kIROp_WitnessTableSet, tables)));
            else
            {
                sink->diagnose(
                    inst,
                    Diagnostics::noTypeConformancesFoundForInterface,
                    interfaceType);
                return none();
            }
        }

        return none();
    }

    IRInst* analyzeMakeExistential(IRInst* context, IRMakeExistential* inst)
    {
        IRBuilder builder(module);
        auto witnessTable = inst->getWitnessTable();

        // If we're building an existential for a COM interface,
        // we always assume it is unbounded, since we can receive
        // types that we know nothing about in the current linkage.
        //
        if (isComInterfaceType(inst->getDataType()))
            return makeUnbounded();

        // Concrete case.
        if (as<IRWitnessTable>(witnessTable))
            return makeTaggedUnionType(
                as<IRWitnessTableSet>(builder.getSingletonSet(witnessTable)));

        // Get the witness table info
        auto witnessTableInfo = tryGetInfo(context, witnessTable);

        if (!witnessTableInfo)
            return none();

        if (as<IRUnboundedSet>(witnessTableInfo))
            return makeUnbounded();

        if (auto elementOfSetType = as<IRElementOfSetType>(witnessTableInfo))
            return makeTaggedUnionType(cast<IRWitnessTableSet>(elementOfSetType->getSet()));

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

    IRInst* analyzeLoad(IRInst* context, IRInst* inst)
    {
        IRBuilder builder(module);
        if (auto loadInst = as<IRLoad>(inst))
        {
            // If we have a simple load, theres one of two cases:
            //
            // 1. If we're loading from a resource pointer, we need to treat it
            //    as unspecializable. If it's a COM interface, we consider it truly
            //    unbounded. Otherwise, we can simply enumerate all tables for the interface
            //    type.
            //
            // 2. In the default case, we can look up the registered information
            //    for the pointer, which should be of the form PtrTypeBase(valueInfo), and
            //    use the valueInfo
            //

            if (isResourcePointer(loadInst->getPtr()))
            {
                if (auto interfaceType = as<IRInterfaceType>(loadInst->getDataType()))
                {
                    if (!isComInterfaceType(interfaceType) && !isBuiltin(interfaceType))
                    {
                        auto tables = collectExistentialTables(interfaceType);
                        if (tables.getCount() > 0)
                            return makeTaggedUnionType(as<IRWitnessTableSet>(
                                builder.getSet(kIROp_WitnessTableSet, tables)));
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
            // In case of a buffer load, we know we're dealing with a location that cannot
            // be specialized, so the logic is similar to case (1) from above.
            //
            if (auto interfaceType = as<IRInterfaceType>(inst->getDataType()))
            {
                if (!isComInterfaceType(interfaceType) && !isBuiltin(interfaceType))
                {
                    auto tables = collectExistentialTables(interfaceType);
                    if (tables.getCount() > 0)
                        return makeTaggedUnionType(
                            as<IRWitnessTableSet>(builder.getSet(kIROp_WitnessTableSet, tables)));
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
        // For a simple store, we will attempt to update the location with
        // the information from the stored value.
        //
        // Since the pointer can be an access chain, we have to recursively transfer
        // the information down to the base. This logic is handled by `maybeUpdateInfoForAddress`
        //
        // If the value has "info", we construct an appropriate PtrType(info) and
        // update the ptr with it.
        //

        auto address = storeInst->getPtr();
        if (auto valInfo = tryGetInfo(context, storeInst->getVal()))
        {
            IRBuilder builder(module);
            auto ptrInfo = builder.getPtrTypeWithAddressSpace(
                (IRType*)valInfo,
                as<IRPtrTypeBase>(address->getDataType()));

            // Propagate the information up the access chain to the base location.
            maybeUpdateInfoForAddress(context, address, ptrInfo, workQueue);
        }

        // The store inst itself doesn't produce anything, so it has no info
        return none();
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

        return none(); // No info for the base pointer => no info for the result.
    }

    IRInst* analyzeFieldAddress(IRInst* context, IRFieldAddress* fieldAddress)
    {
        // In this case, we don't look up the base's info, but rather, we find
        // the IRStructField being accessed, and look up the info in the fieldInfos
        // map.
        //
        // This info will be the in the value form, so we need to wrap it in a
        // pointer since the result is an address.
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

                // Register this as a user of the field so updates will invoke this function again.
                this->fieldUseSites.addIfNotExists(structField, HashSet<InstWithContext>());
                this->fieldUseSites[structField].add(InstWithContext(context, fieldAddress));

                if (this->fieldInfo.containsKey(structField))
                {
                    return builder.getPtrTypeWithAddressSpace(
                        (IRType*)this->fieldInfo[structField],
                        as<IRPtrTypeBase>(fieldAddress->getDataType()));
                }
            }
        }

        return none(); // No info for the field => no info for the result.
    }

    IRInst* analyzeFieldExtract(IRInst* context, IRFieldExtract* fieldExtract)
    {
        // Very similar logic to `analyzeFieldAddress`, but without having to
        // wrap the result in a pointer.
        //

        IRBuilder builder(module);

        if (auto structType = as<IRStructType>(fieldExtract->getBase()->getDataType()))
        {
            auto structField =
                findStructField(structType, as<IRStructKey>(fieldExtract->getField()));

            // Register this as a user of the field so updates will invoke this function again.
            this->fieldUseSites.addIfNotExists(structField, HashSet<InstWithContext>());
            this->fieldUseSites[structField].add(InstWithContext(context, fieldExtract));

            if (this->fieldInfo.containsKey(structField))
            {
                return this->fieldInfo[structField];
            }
        }
        return none();
    }

    // Locate the 'none' witness table in the global scope
    // of the module in context. This will be the table
    // that conforms to 'nullptr' and has 'void' as the concrete type
    //
    IRWitnessTable* findNoneWitness()
    {
        IRBuilder builder(module);
        auto voidType = builder.getVoidType();
        for (auto inst : module->getGlobalInsts())
        {
            if (auto witnessTable = as<IRWitnessTable>(inst))
            {
                if (witnessTable->getConcreteType() == voidType &&
                    witnessTable->getConformanceType() == nullptr)
                    return witnessTable;
            }
        }

        return nullptr;
    }

    // Get the witness table inst to be used for the 'none' case of
    // an optional witness table.
    //
    IRWitnessTable* getNoneWitness()
    {
        if (auto table = findNoneWitness())
            return table;

        IRBuilder builder(module);
        auto voidType = builder.getVoidType();

        return builder.createWitnessTable(nullptr, voidType);
    }

    // Returns true if the inst is of the form OptionalType<InterfaceType>
    bool isOptionalExistentialType(IRInst* inst)
    {
        if (auto optionalType = as<IROptionalType>(inst))
            if (auto interfaceType = as<IRInterfaceType>(optionalType->getValueType()))
                return !isComInterfaceType(interfaceType) && !isBuiltin(interfaceType);
        return false;
    }

    IRInst* analyzeMakeOptionalNone(IRInst* context, IRMakeOptionalNone* inst)
    {
        // If the optional type we're dealing with is an optional concrete type, we won't
        // touch this case, since there's nothing dynamic to specialize.
        //
        // If the type inside the optional is an interface type, then we will treat it slightly
        // differently by including 'none' as one of the possible candidates of the existential
        // value.
        //
        // The `MakeOptionalNone` case represents the creating of an existential out of the
        // 'none' witness table and a void value, so we'll represent that using the tagged union
        // type.
        //
        SLANG_UNUSED(context);
        IRBuilder builder(module);
        if (isOptionalExistentialType(inst->getDataType()))
        {
            auto noneTableSet =
                cast<IRWitnessTableSet>(builder.getSet(kIROp_WitnessTableSet, getNoneWitness()));
            return makeTaggedUnionType(noneTableSet);
        }

        return none();
    }

    IRInst* analyzeMakeOptionalValue(IRInst* context, IRMakeOptionalValue* inst)
    {
        // If the optional type we're dealing with is an optional concrete type, we won't
        // touch this case, since there's nothing dynamic to specialize.
        //
        // If the type inside the optional is an interface type, then we will treat it slightly
        // differently, by conceptually treating it as an interface type that has all the possible
        // elements of the interface type plus an additional 'none' element.
        //
        // The `MakeOptionalValue` case is then very similar to the `MakeExistential` case, only we
        // already have an existential as input.
        //
        // Thus, we simply pass the input existential info as-is.
        //
        // Note: we don't actually have to add a new 'none' table to the collection, since that will
        // automatically occur if this value ever merges with a value created using
        // `MakeOptionalNone`
        //
        if (isOptionalExistentialType(inst->getDataType()))
        {
            if (auto info = tryGetInfo(context, inst->getValue()))
            {
                SLANG_ASSERT(as<IRTaggedUnionType>(info));
                return info;
            }
        }

        return none();
    }

    IRInst* analyzeGetOptionalValue(IRInst* context, IRGetOptionalValue* inst)
    {
        if (isOptionalExistentialType(inst->getDataType()))
        {
            // This is an interesting case.. technically, at this point we could go
            // from a larger collection to a smaller one (without the none-type).
            //
            // However, for simplicitly reasons, we currently only allow up-casting,
            // so for now we'll just passthrough all types (so the result will
            // assume that 'none-type' is a possiblity even though we statically know
            // that it isn't).
            //
            if (auto info = tryGetInfo(context, inst->getOperand(0)))
            {
                SLANG_ASSERT(as<IRTaggedUnionType>(info));
                return info;
            }
        }
    }

    IRInst* analyzeLookupWitnessMethod(IRInst* context, IRLookupWitnessMethod* inst)
    {
        // A LookupWitnessMethod is assumed to by dynamic, so we
        // (i) construct a collection of the results by looking up the given
        //     key in each of the input witness tables
        // (ii) wrap the result in a tag type, since the lookup inst is logically holding
        //     on to run-time information about which element of the collection is active.
        //
        // Note that the input must be a set of concrete witness tables (or none/unbounded).
        // If this is not the case and we see anything abstract, then something has gone
        // wrong somewhere when analyzing a previous instruction.
        //

        auto key = inst->getRequirementKey();

        auto witnessTable = inst->getWitnessTable();
        auto witnessTableInfo = tryGetInfo(context, witnessTable);

        if (auto elementOfSetType = as<IRElementOfSetType>(witnessTableInfo))
        {
            IRBuilder builder(module);
            HashSet<IRInst*> results;
            forEachInSet(
                cast<IRWitnessTableSet>(elementOfSetType->getSet()),
                [&](IRInst* table)
                { results.add(findWitnessTableEntry(cast<IRWitnessTable>(table), key)); });
            return makeElementOfSetType(builder.getSet(results));
        }

        if (!witnessTableInfo)
            return none();

        if (as<IRUnboundedSet>(witnessTableInfo))
            return makeUnbounded();

        SLANG_UNEXPECTED("Unexpected witness table info type in analyzeLookupWitnessMethod");
    }

    IRInst* analyzeExtractExistentialWitnessTable(
        IRInst* context,
        IRExtractExistentialWitnessTable* inst)
    {
        // An ExtractExistentialWitnessTable inst is assumed to by dynamic, so we
        // extract the set of witness tables from the input existential and
        // state that the info of the result is a tag-type of that collection.
        //
        // Note that since ExtractExistentialWitnessTable can only be used on
        // an existential, the input info must be a TaggedUnionType of
        // concrete table and type collections (or none/unbounded)
        //

        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(context, operand);

        if (!operandInfo)
            return none();

        if (as<IRUnboundedSet>(operandInfo))
            return makeUnbounded();

        if (auto taggedUnion = as<IRTaggedUnionType>(operandInfo))
            return makeElementOfSetType(taggedUnion->getWitnessTableSet());

        SLANG_UNEXPECTED("Unhandled info type in analyzeExtractExistentialWitnessTable");
    }

    IRInst* analyzeExtractExistentialType(IRInst* context, IRExtractExistentialType* inst)
    {
        // An ExtractExistentialType inst is assumed to be dynamic, so we
        // extract the set of witness tables from the input existential and
        // state that the info of the result is a tag-type of that collection.
        //
        // Note: Since ExtractExistentialType can only be used on
        // an existential, the input info must be a TaggedUnionType of
        // concrete table and type collections (or none/unbounded)
        //

        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(context, operand);

        if (!operandInfo)
            return none();

        if (as<IRUnboundedSet>(operandInfo))
            return makeUnbounded();

        if (auto taggedUnion = as<IRTaggedUnionType>(operandInfo))
            return makeElementOfSetType(taggedUnion->getTypeSet());

        SLANG_UNEXPECTED("Unhandled info type in analyzeExtractExistentialType");
    }

    IRInst* analyzeExtractExistentialValue(IRInst* context, IRExtractExistentialValue* inst)
    {
        // Logically, an ExtractExistentialValue inst is carrying a payload
        // of a union type.
        //
        // We represent this by setting its info to be equal to the type-collection,
        // which will later lower into an any-value-type.
        //
        // Note that there is no 'tag' here since ExtractExistentialValue is not representing
        // tag information about which type in the collection is active, but is representing
        // a value of the collection's union type.
        //

        auto operand = inst->getOperand(0);
        auto operandInfo = tryGetInfo(context, operand);

        if (!operandInfo)
            return none();

        if (as<IRUnboundedSet>(operandInfo))
            return makeUnbounded();

        if (auto taggedUnion = as<IRTaggedUnionType>(operandInfo))
            return makeUntaggedUnionType(taggedUnion->getTypeSet());

        return none();
    }

    IRInst* analyzeSpecialize(IRInst* context, IRSpecialize* inst)
    {
        // Analyzing an IRSpecialize inst is an interesting case.
        //
        // If we hit this case, it means we are encountering this instruction
        // inside a block, so the arguments to the specialization likely have some
        // dynamic types or witness tables.
        //
        // We'll first look at the specialization base, which may be a single generic
        // or a collection of generics.
        //
        // Then, for each generic, we'll create a specialized version by using the
        // collection info for each argument in place of the argument.
        //     e.g. Specialize(G, A0, A1) becomes Specialize(G, info(A1).collection,
        //     info(A2).collection)
        //         (i.e. if the args are tag-types, we only use the collection part)
        //
        // This transformation is important to lift the 'dynamic' specialize instruction into a
        // global specialize instruction while still retaining the information about what types and
        // tables the resulting generic should support.
        //
        // Finally, we put all the specialized vesions back into a collection and return that info.
        //

        auto operand = inst->getBase();
        auto operandInfo = tryGetInfo(context, operand);

        if (as<IRUnboundedSet>(operandInfo))
            return makeUnbounded();

        if (as<IRTaggedUnionType>(operandInfo))
        {
            SLANG_UNEXPECTED(
                "Unexpected ExtractExistentialWitnessTable on Set (should be Existential)");
        }

        // Handle the 'many' or 'one' cases.
        if (as<IRElementOfSetType>(operandInfo) || isGlobalInst(operand))
        {
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

                // If any of the args are 'empty' sets, we can't generate a specialization just yet.
                if (!argInfo)
                    return none();

                if (as<IRUnboundedSet>(argInfo) || as<IRTaggedUnionType>(argInfo))
                {
                    SLANG_UNEXPECTED("Unexpected Existential operand in specialization argument.");
                }

                if (auto elementOfSetType = as<IRElementOfSetType>(argInfo))
                {
                    if (elementOfSetType->getSet()->isSingleton())
                        specializationArgs.add(elementOfSetType->getSet()->getElement(0));
                    else
                    {
                        if (auto typeSet = as<IRTypeSet>(elementOfSetType->getSet()))
                        {
                            specializationArgs.add(makeUntaggedUnionType(typeSet));
                        }
                        else if (as<IRWitnessTableSet>(elementOfSetType->getSet()))
                        {
                            specializationArgs.add(elementOfSetType->getSet());
                        }
                        else
                        {
                            SLANG_UNEXPECTED(
                                "Unexpected collection type in specialization argument.");
                        }
                    }
                }
                else
                {
                    SLANG_UNEXPECTED("Unhandled PropagationJudgment in analyzeSpecialize");
                }
            }

            // This part creates a correct type for the specialization, by following the same
            // process: replace all operands in the composite type with their propagated collection.
            //

            IRType* typeOfSpecialization = nullptr;
            if (inst->getDataType()->getParent()->getOp() == kIROp_ModuleInst)
                typeOfSpecialization = inst->getDataType();
            else if (auto funcType = as<IRFuncType>(inst->getDataType()))
            {
                auto substituteSets = [&](IRInst* type) -> IRInst*
                {
                    if (auto info = tryGetInfo(context, type))
                    {
                        if (auto elementOfSetType = as<IRElementOfSetType>(info))
                        {
                            if (elementOfSetType->getSet()->isSingleton())
                                return elementOfSetType->getSet()->getElement(0);
                            else
                                return makeUntaggedUnionType(
                                    cast<IRTypeSet>(elementOfSetType->getSet()));
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
            else if (auto typeInfo = tryGetInfo(context, inst->getDataType()))
            {
                // There's one other case we'd like to handle, where the func-type itself is a
                // dynamic IRSpecialize. In this situation, we'd want to use the type inst's info to
                // find the collection-based specialization and create a func-type from it.
                //
                if (auto elementOfSetType = as<IRElementOfSetType>(typeInfo))
                {
                    SLANG_ASSERT(elementOfSetType->getSet()->isSingleton());
                    auto specializeInst =
                        cast<IRSpecialize>(elementOfSetType->getSet()->getElement(0));
                    auto specializedFuncType = cast<IRFuncType>(specializeGeneric(specializeInst));
                    typeOfSpecialization = specializedFuncType;
                }
                else
                {
                    return none();
                }
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

            // Specialize each element in the set
            HashSet<IRInst*> specializedSet;

            IRSetBase* collection = nullptr;
            if (auto elementOfSetType = as<IRElementOfSetType>(operandInfo))
            {
                collection = elementOfSetType->getSet();

                forEachInSet(
                    collection,
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
            }
            else
            {
                // Concrete case..
                IRBuilder builder(module);
                builder.setInsertInto(module);
                specializedSet.add(
                    builder.emitSpecializeInst(typeOfSpecialization, operand, specializationArgs));
            }

            IRBuilder builder(module);
            return makeElementOfSetType(builder.getSet(specializedSet));
        }

        if (!operandInfo)
            return none();

        SLANG_UNEXPECTED("Unhandled PropagationJudgment in analyzeExtractExistentialWitnessTable");
    }

    void discoverContext(IRInst* context, WorkQueue& workQueue)
    {
        // "Discovering" a context, essentially means we check if this is the first
        // time we're trying to propagate information into this context. A context
        // is a global-scope IRFunc or IRSpecialize.
        //
        // If it is the first, we enqueue some work to perform  initialization of all
        // the insts in the body of the func.
        //
        // Since discover context is only called 'on-demand' as the type-flow propagation
        // happens, we avoid having to deal with functions/generics that are never used,
        // and minimize the amount of work being performed.
        //

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
                        workQueue.enqueue(WorkItem(context, block));
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
                    for (UInt index = 0; index < specialize->getArgCount() && param;
                         ++index, param = param->getNextParam())
                    {
                        // Map the specialization argument to the corresponding parameter
                        auto arg = specialize->getArg(index);
                        if (as<IRIntLit>(arg))
                            continue;

                        if (auto collection = as<IRSetBase>(arg))
                        {
                            updateInfo(
                                context,
                                param,
                                makeElementOfSetType(collection),
                                true,
                                workQueue);
                        }
                        else if (as<IRType>(arg) || as<IRWitnessTable>(arg))
                        {
                            IRBuilder builder(module);
                            updateInfo(
                                context,
                                param,
                                makeElementOfSetType(builder.getSingletonSet(arg)),
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

    IRInst* analyzeCall(IRInst* context, IRCall* inst, WorkQueue& workQueue)
    {
        // We don't perform the propagation here, but instead we add inter-procedural
        // edges to the work queue.
        // The propagation logic is handled in `propagateInterproceduralEdge()`
        //

        auto callee = inst->getCallee();
        auto calleeInfo = tryGetInfo(context, callee);

        if (isNoneCallee(callee))
            return none();

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

            this->funcCallSites.addIfNotExists(callee, HashSet<InstWithContext>());
            if (this->funcCallSites[callee].add(InstWithContext(context, inst)))
            {
                // If this is a new call site, add a propagation task to the queue (in case there's
                // already information about this function)
                workQueue.enqueue(
                    WorkItem(InterproceduralEdge::Direction::FuncToCall, context, inst, callee));
            }
            workQueue.enqueue(
                WorkItem(InterproceduralEdge::Direction::CallToFunc, context, inst, callee));
        };

        // If we have a collection of functions (with or without a dynamic tag), register
        // each one.
        //
        if (auto elementOfSetType = as<IRElementOfSetType>(calleeInfo))
        {
            forEachInSet(
                elementOfSetType->getSet(),
                [&](IRInst* func) { propagateToCallSite(func); });
        }
        else if (isGlobalInst(callee))
        {
            propagateToCallSite(callee);
        }

        if (auto callInfo = tryGetInfo(context, inst))
            return callInfo;
        else
            return none();
    }

    // Updates the information for an address.
    void maybeUpdateInfoForAddress(
        IRInst* context,
        IRInst* inst,
        IRInst* info,
        WorkQueue& workQueue)
    {
        // This method recursively walks up the access chain until it hits a location.
        //
        // Pointers don't have any unique information attached to them because two pointers to the
        // same location (directly or indirectly) must mirror the same propagation info.
        //
        // Thus, our approach will be to update the info for the base value (usually an IRVar or
        // IRParam), and then register users for updates to propagate the updated information
        // to any access chain instructions.
        //

        if (auto getElementPtr = as<IRGetElementPtr>(inst))
        {
            if (auto thisPtrInfo = as<IRPtrTypeBase>(info))
            {
                // For get-element-ptr, we propagate information by
                // wrapping the result's info into the array type.
                //
                // a : PtrType(ArrayType(T, count)) = ...
                // b : PtrType(T) = GetElementPtr(a)
                //
                // info(a) = ArrayType(info(b), count)
                //

                auto thisValueInfo = thisPtrInfo->getValueType();

                IRInst* baseValueType =
                    as<IRPtrTypeBase>(getElementPtr->getBase()->getDataType())->getValueType();
                SLANG_ASSERT(as<IRArrayType>(baseValueType));

                // Propagate 'this' information to the base by wrapping it as a pointer to array.
                IRBuilder builder(module);
                auto baseInfo = builder.getPtrTypeWithAddressSpace(
                    builder.getArrayType(
                        (IRType*)thisValueInfo,
                        as<IRArrayType>(baseValueType)->getElementCount()),
                    as<IRPtrTypeBase>(getElementPtr->getBase()->getDataType()));

                // Recursively try to update the base pointer.
                maybeUpdateInfoForAddress(context, getElementPtr->getBase(), baseInfo, workQueue);
            }
        }
        else if (auto fieldAddress = as<IRFieldAddress>(inst))
        {
            if (as<IRPtrTypeBase>(info))
            {
                // Field address is also treated as a base case for the recursion.
                //
                // For field-address, we record the information against the field itself
                // by using the fieldInfos map (after unwrapping the pointer)
                //
                // a : PtrType(S) = ...
                // b : PtrType(T) = GetFieldAddress(a, fieldKey)
                //
                // infos[findField(S, fieldKey)] = info(T)
                //

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

                    // Manually update the prop info add work items for all users of this field.
                    //
                    // This case is not handled by updateInfo(), though in the future
                    // it makes sense to include this as a case in updateInfo()
                    //
                    if (auto newInfo = unionPropagationInfo(info, existingInfo))
                    {
                        if (newInfo != existingInfo)
                        {
                            auto newInfoValType = cast<IRPtrTypeBase>(newInfo)->getValueType();

                            // Update the field info map
                            this->fieldInfo[foundField] = newInfoValType;

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
            // If we hit a local var, we'll update its info.
            //
            // This is one of the base cases for the recursion.
            //
            updateInfo(context, var, info, true, workQueue);
        }
        else if (auto param = as<IRParam>(inst))
        {
            // We'll also update function parameters,
            // but first change the info from PtrTypeBase<T>
            // to the specific pointer type for the parameter.
            //
            // (e.g. parameter may use a BorrowInOutType, but the info
            // may be some other PtrType)
            //
            // This is one of the base cases for the recursion.
            //
            IRBuilder builder(param->getModule());
            auto newInfo = builder.getPtrTypeWithAddressSpace(
                (IRType*)as<IRPtrTypeBase>(info)->getValueType(),
                as<IRPtrTypeBase>(param->getDataType()));
            updateInfo(context, param, newInfo, true, workQueue);
        }
        else
        {
            // If we hit something unsupported, assume there's nothing to update.
            return;
        }
    }

    // Returns the effective parameter types for a given calling context, after
    // the type-flow propagation is complete.
    //
    List<IRType*> getEffectiveParamTypes(IRInst* context)
    {
        // This proceeds by looking at the propagation info for each parameter,
        // then returning the info if one exists.
        //
        // If one does not exist, it means the parameter has a concrete type
        // (not dynamic or generic), and we can just use that for the parameter.
        //

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

    // Helper to get any recorded propagation info for each parameter of a calling context.
    List<IRInst*> getParamInfos(IRInst* context)
    {
        List<IRInst*> infos;
        if (as<IRFunc>(context))
        {
            for (auto param : as<IRFunc>(context)->getParams())
                infos.add(tryGetArgInfo(context, param));
        }
        else if (auto specialize = as<IRSpecialize>(context))
        {
            auto generic = specialize->getBase();
            auto innerFunc = getGenericReturnVal(generic);
            for (auto param : as<IRFunc>(innerFunc)->getParams())
                infos.add(tryGetArgInfo(context, param));
        }
        else
        {
            // If it's not a function or a specialization, we can't get parameter info
            SLANG_UNEXPECTED("Unexpected context type for parameter info retrieval");
        }

        return infos;
    }

    // Helper to extract the directions of each parameter for a calling context.
    List<ParameterDirectionInfo> getParamDirections(IRInst* context)
    {
        // Note that this method does not actually have to retreive any propagation info,
        // since the directions/address-spaces of parameters are always concrete.
        //

        List<ParameterDirectionInfo> directions;
        if (as<IRFunc>(context))
        {
            for (auto param : as<IRFunc>(context)->getParams())
            {
                const auto [direction, type] = splitParameterDirectionAndType(param->getDataType());
                directions.add(direction);
            }
        }
        else if (auto specialize = as<IRSpecialize>(context))
        {
            auto generic = specialize->getBase();
            auto innerFunc = getGenericReturnVal(generic);
            for (auto param : as<IRFunc>(innerFunc)->getParams())
            {
                const auto [direction, type] = splitParameterDirectionAndType(param->getDataType());
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

    // Extract the return value information for a given calling context
    IRInst* getFuncReturnInfo(IRInst* context)
    {
        // We record the information in a separate map, rather than using
        // a specific inst.
        //
        // This is because we need the union of the infos of all Return instructions
        // in the function, but there's no physical instruction that represents this.
        // (unlike the block control flow case, where phi params exist)
        //
        // Effectively, this is a 'virtual' inst that represents the union of all
        // the return values.
        //
        funcReturnInfo.addIfNotExists(context, none());
        return funcReturnInfo[context];
    }

    // Set up initial information for parameters based on their types.
    void initializeFirstBlockParameters(IRInst* context, IRFunc* func)
    {
        // This method primarily just initializes known COM & Builtin interface
        // types to 'unbounded', to avoid specializing any instructions derived from
        // these parameters.
        //

        auto firstBlock = func->getFirstBlock();
        if (!firstBlock)
            return;

        // Initialize parameters with COM/Builtin interface types to 'unbounded' and
        // everything else to none.
        //
        for (auto param : firstBlock->getParams())
        {
            auto paramType = param->getDataType();
            auto paramInfo = tryGetInfo(context, param);
            if (paramInfo)
                continue; // Already has some information

            if (auto interfaceType = as<IRInterfaceType>(paramType))
            {
                if (isComInterfaceType(interfaceType) || isBuiltin(interfaceType))
                    propagationMap[InstWithContext(context, param)] = makeUnbounded();
                else
                    propagationMap[InstWithContext(context, param)] = none();
            }
            else
            {
                propagationMap[InstWithContext(context, param)] = none();
            }
        }
    }

    // Specialize the fields of a struct type based on the recorded field info (if we
    // have a non-trivial specilialization)
    //
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

        return hasChanges;
    }

    bool specializeInstsInBlock(IRInst* context, IRBlock* block)
    {
        List<IRInst*> instsToLower;
        bool hasChanges = false;
        for (auto inst : block->getChildren())
            instsToLower.add(inst);

        for (auto inst : instsToLower)
            hasChanges |= specializeInst(context, inst);

        return hasChanges;
    }

    bool specializeFunc(IRFunc* func)
    {
        // When specializing a func, we
        // (i) rewrite the types and insts by calling `specializeInstsInBlock` and
        // (ii) handle 'merge' points where the collections need to be upcasted.
        //
        // The merge points are places where a specialized inst might be passed as
        // argument to a parameter that has a 'wider' type.
        //
        // This frequently occurs with phi parameters.
        //
        // For example:
        //   A   B
        //    \ /
        //     C
        //
        // After specialization, A could pass a value of type TagType(WitnessTableSet{T1,
        // T2}) while B passes a value of type TagType(WitnessTableSet{T2, T3}), while the
        // phi parameter's type in C has the union type `TagType(WitnessTableSet{T1, T2,
        // T3})`
        //
        // In this case, we use `upcastSet` to insert a cast from
        // TagType(WitnessTableSet{T1, T2}) -> TagType(WitnessTableSet{T1, T2, T3})
        // before passing the result as a phi argument.
        //
        // The same logic applies for the return values. The function's caller expects a union type
        // of all possible return statements, so we cast each return inst if there is a mismatch.
        //

        // Don't make any changes to non-global or intrinsic functions.
        //
        // If a function is inside a generic, we wait until the main specialization pass
        // turns it into a regular func and the typeflow pass is re-run again.
        // This approach is much simpler that trying to incorporate generic parameters into the
        // typeflow specialization logic.
        //
        if (!isGlobalInst(func) || isIntrinsic(func))
            return false;

        bool hasChanges = false;
        for (auto block : func->getBlocks())
            hasChanges |= specializeInstsInBlock(func, block);

        for (auto block : func->getBlocks())
        {
            UIndex paramIndex = 0;
            for (auto param : block->getParams())
            {
                auto paramInfo = _tryGetInfo(InstWithContext(func, param));
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
                        IRBuilder builder(module);
                        builder.setInsertBefore(unconditionalBranch);
                        auto newArg = upcastSet(&builder, arg, param->getDataType());

                        if (newArg != arg)
                        {
                            hasChanges = true;

                            // Replace the argument in the branch instruction with the
                            // properly casted argument.
                            //
                            if (auto loop = as<IRLoop>(unconditionalBranch))
                                loop->setOperand(3 + paramIndex, newArg);
                            else
                                unconditionalBranch->setOperand(1 + paramIndex, newArg);
                        }
                    }
                }

                paramIndex++;
            }

            // If the terminator is a return instruction, perform the same upcasting to
            // match the registered return value type for this function.
            //
            if (auto returnInst = as<IRReturn>(block->getTerminator()))
            {
                if (!as<IRVoidType>(returnInst->getVal()->getDataType()))
                {
                    if (auto specializedType = getLoweredType(getFuncReturnInfo(func)))
                    {
                        IRBuilder builder(module);
                        builder.setInsertBefore(returnInst);
                        auto newReturnVal =
                            upcastSet(&builder, returnInst->getVal(), specializedType);
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

        // Update the func type for this func accordingly.
        auto effectiveFuncType = getEffectiveFuncType(func);
        if (effectiveFuncType != func->getFullType())
        {
            hasChanges = true;
            func->setFullType(effectiveFuncType);
        }

        return hasChanges;
    }

    // Implements phase 2 of the type-flow specialization pass.
    //
    // This method is called after information propagation is complete and
    // stabilized, and it replaces dynamic insts and types with specialized versions
    // based on the collected information.
    //
    // After this pass is run, there should be no dynamic insts or types remaining,
    // _except_ for those that are considered unbounded.
    //
    // i.e. `ExtractExistentialType`, `ExtractExistentialWitnessTable`, `ExtractExistentialValue`,
    //      `MakeExistential`, `LookupWitness` (and more) are rewritten to concrete tag translation
    //      insts (e.g. `GetTagForMappedSet`, `GetTagForSpecializedSet`, etc.)
    //
    bool performDynamicInstLowering()
    {
        List<IRFunc*> funcsToProcess;
        List<IRStructType*> structsToProcess;

        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(globalInst))
                funcsToProcess.add(func);
            else if (auto structType = as<IRStructType>(globalInst))
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

        return hasChanges;
    }

    // Returns an effective type to use for an inst, given its
    // info.
    //
    // This basically recursively walks the info and applies the array/ptr-type
    // wrappers, while replacing unbounded collection with a nullptr.
    //
    // If the result of this is null, then the inst should keep its current type.
    //
    IRType* getLoweredType(IRInst* info)
    {
        if (!info)
            return nullptr;

        if (as<IRUnboundedSet>(info))
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

        if (auto taggedUnion = as<IRTaggedUnionType>(info))
        {
            return (IRType*)taggedUnion;
        }

        if (auto elementOfSetType = as<IRElementOfSetType>(info))
        {
            // Replace element-of-collection types with tag types.
            return makeTagType(elementOfSetType->getSet());
        }

        if (auto valOfSetType = as<IRUntaggedUnionType>(info))
        {
            if (valOfSetType->getSet()->isSingleton())
            {
                // If there's only one type in the collection, return it directly
                return (IRType*)valOfSetType->getSet()->getElement(0);
            }

            return valOfSetType;
        }

        if (as<IRFuncSet>(info) || as<IRWitnessTableSet>(info))
        {
            // Don't specialize these collections.. they should be used through
            // tag types, or be processed out during specializeing.
            //
            return nullptr;
        }

        return (IRType*)info;
    }

    // Replace an insts type with its effective type as determined by the analysis.
    bool replaceType(IRInst* context, IRInst* inst)
    {
        // If the inst is a global val, we won't modify it.
        if (as<IRModuleInst>(inst->getParent()))
        {
            if (as<IRType>(inst) || as<IRWitnessTable>(inst) || as<IRFunc>(inst) ||
                as<IRGeneric>(inst))
            {
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
        case kIROp_GetElementFromTag:
            return specializeGetElementFromTag(context, as<IRGetElementFromTag>(inst));
        case kIROp_Load:
            return specializeLoad(context, inst);
        case kIROp_Store:
            return specializeStore(context, as<IRStore>(inst));
        case kIROp_GetSequentialID:
            return specializeGetSequentialID(context, as<IRGetSequentialID>(inst));
        case kIROp_IsType:
            return specializeIsType(context, as<IRIsType>(inst));
        case kIROp_MakeOptionalNone:
            return specializeMakeOptionalNone(context, as<IRMakeOptionalNone>(inst));
        case kIROp_MakeOptionalValue:
            return specializeMakeOptionalValue(context, as<IRMakeOptionalValue>(inst));
        case kIROp_OptionalHasValue:
            return specializeOptionalHasValue(context, as<IROptionalHasValue>(inst));
        case kIROp_GetOptionalValue:
            return specializeGetOptionalValue(context, as<IRGetOptionalValue>(inst));
        default:
            {
                // Default case: replace inst type with specialized type (if available)
                if (tryGetInfo(context, inst))
                    return replaceType(context, inst);
                return false;
            }
        }
    }

    bool specializeLookupWitnessMethod(IRInst* context, IRLookupWitnessMethod* inst)
    {
        // Handle trivial case where inst's operand is a concrete table.
        if (auto witnessTable = as<IRWitnessTable>(inst->getWitnessTable()))
        {
            inst->replaceUsesWith(findWitnessTableEntry(witnessTable, inst->getRequirementKey()));
            inst->removeAndDeallocate();
            return true;
        }

        // Otherwise, we go off the info for the inst.
        auto info = tryGetInfo(context, inst);
        if (!info)
            return false;

        // If we didn't resolve anything for this inst, don't modify it.
        auto elementOfSetType = as<IRElementOfSetType>(info);
        if (!elementOfSetType)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // If there's a single element, we can do a simple replacement.
        if (elementOfSetType->getSet()->getCount() == 1)
        {
            auto element = elementOfSetType->getSet()->getElement(0);
            inst->replaceUsesWith(element);
            inst->removeAndDeallocate();
            return true;
        }

        // If we reach here, we have a truly dynamic case. Multiple elements.
        // We need to emit a run-time inst to keep track of the tag.
        //
        // We use the GetTagForMappedSet inst to do this, and set its data type to
        // the appropriate tag-type.
        //

        auto witnessTableInst = inst->getWitnessTable();
        auto witnessTableInfo = witnessTableInst->getDataType();

        if (as<IRSetTagType>(witnessTableInfo))
        {
            auto thisInstInfo = cast<IRElementOfSetType>(tryGetInfo(context, inst));
            if (thisInstInfo->getSet() != nullptr)
            {
                List<IRInst*> operands = {witnessTableInst, inst->getRequirementKey()};

                auto newInst = builder.emitIntrinsicInst(
                    (IRType*)makeTagType(thisInstInfo->getSet()),
                    kIROp_GetTagForMappedSet,
                    operands.getCount(),
                    operands.getBuffer());

                inst->replaceUsesWith(newInst);
                inst->removeAndDeallocate();
                return true;
            }
        }

        return false;
    }

    bool specializeExtractExistentialWitnessTable(
        IRInst* context,
        IRExtractExistentialWitnessTable* inst)
    {
        // If we have a non-trivial info registered, it must of
        // SetTagType(WitnessTableSet(...))
        //
        // Further, the operand must be an existential (TaggedUnionType), which is
        // conceptually a pair of TagType(tableSet) and a
        // UntaggedUnionType(typeSet)
        //
        // We will simply extract the first element of this tuple.
        //

        auto info = tryGetInfo(context, inst);
        if (!info)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        auto elementOfSetType = as<IRElementOfSetType>(info);
        if (!elementOfSetType)
            return false;

        if (elementOfSetType->getSet()->getCount() == 1)
        {
            // Found a single possible type. Simple replacement.
            inst->replaceUsesWith(elementOfSetType->getSet()->getElement(0));
            inst->removeAndDeallocate();
            return true;
        }
        else
        {
            // Replace with GetElement(specializedInst, 0) -> TagType(tableSet)
            // which retreives a 'tag' (i.e. a run-time identifier for one of the elements
            // of the collection)
            //
            auto operand = inst->getOperand(0);
            auto element = builder.emitGetTagFromTaggedUnion(operand);
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
        if (as<IRTaggedUnionType>(existentialInfo))
        {
            IRBuilder builder(inst);
            builder.setInsertAfter(inst);

            auto val = builder.emitGetValueFromTaggedUnion(existential);
            inst->replaceUsesWith(val);
            inst->removeAndDeallocate();
            return true;
        }

        return false;
    }

    bool specializeExtractExistentialType(IRInst* context, IRExtractExistentialType* inst)
    {
        auto info = tryGetInfo(context, inst);
        if (auto elementOfSetType = as<IRElementOfSetType>(info))
        {
            if (elementOfSetType->getSet()->isSingleton())
            {
                // Found a single possible type. Statically known concrete type.
                auto singletonValue = elementOfSetType->getSet()->getElement(0);
                inst->replaceUsesWith(singletonValue);
                inst->removeAndDeallocate();
                return true;
            }
            else
            {
                // Multiple elements, emit a tag inst.
                IRBuilder builder(inst);
                builder.setInsertBefore(inst);
                auto newInst = builder.emitGetTypeTagFromTaggedUnion(inst->getOperand(0));
                inst->replaceUsesWith(newInst);
                inst->removeAndDeallocate();
                return true;
            }
        }

        return false;
    }

    bool isTaggedUnionType(IRInst* type) { return as<IRTaggedUnionType>(type) != nullptr; }

    IRType* updateType(IRType* currentType, IRType* newType)
    {
        if (auto valOfSetType = as<IRUntaggedUnionType>(currentType))
        {
            HashSet<IRInst*> collectionElements;
            forEachInSet(
                valOfSetType->getSet(),
                [&](IRInst* element) { collectionElements.add(element); });

            if (auto newValOfSetType = as<IRUntaggedUnionType>(newType))
            {
                // If the new type is also a collection, merge the two collections
                forEachInSet(
                    newValOfSetType->getSet(),
                    [&](IRInst* element) { collectionElements.add(element); });
            }
            else
            {
                // Otherwise, just add the new type to the collection
                collectionElements.add(newType);
            }

            // If this is a collection, we need to create a new collection with the new type
            IRBuilder builder(module);
            auto newSet = builder.getSet(kIROp_TypeSet, collectionElements);
            return makeUntaggedUnionType(cast<IRTypeSet>(newSet));
        }
        else if (currentType == newType)
        {
            return currentType;
        }
        else if (currentType == nullptr)
        {
            return newType;
        }
        else if (as<IRTaggedUnionType>(currentType) && as<IRTaggedUnionType>(newType))
        {
            // Merge the elements of both tagged unions into a new tuple type
            return (IRType*)makeTaggedUnionType((unionSet<IRWitnessTableSet>(
                as<IRTaggedUnionType>(currentType)->getWitnessTableSet(),
                as<IRTaggedUnionType>(newType)->getWitnessTableSet())));
        }
        else // Need to create a new collection.
        {
            HashSet<IRInst*> collectionElements;

            SLANG_ASSERT(!as<IRSetBase>(currentType) && !as<IRSetBase>(newType));

            collectionElements.add(currentType);
            collectionElements.add(newType);

            // If this is a collection, we need to create a new collection with the new type
            IRBuilder builder(module);
            auto newSet = builder.getSet(kIROp_TypeSet, collectionElements);
            return makeUntaggedUnionType(cast<IRTypeSet>(newSet));
        }
    }

    IRFuncType* getEffectiveFuncTypeForDispatcher(
        IRWitnessTableSet* tableSet,
        IRStructKey* key,
        IRFuncSet* resultFuncSet)
    {
        SLANG_UNUSED(key);

        List<IRType*> extraParamTypes;
        extraParamTypes.add((IRType*)makeTagType(tableSet));

        auto innerFuncType = getEffectiveFuncTypeForSet(resultFuncSet);
        List<IRType*> allParamTypes;
        allParamTypes.addRange(extraParamTypes);
        for (auto paramType : innerFuncType->getParamTypes())
            allParamTypes.add(paramType);

        IRBuilder builder(module);
        return builder.getFuncType(allParamTypes, innerFuncType->getResultType());
    }

    // Get an effective func type to use for the callee.
    // The callee may be a collection, in which case, this returns a union-ed functype.
    //
    IRFuncType* getEffectiveFuncTypeForSet(IRFuncSet* calleeSet)
    {
        // The effective func type for a callee is calculated as follows:
        //
        // (i) we build up the effective parameter types for the callee
        //     by taking the union of each parameter type
        //     for each callee in the collection.
        //
        // (ii) build up the effective result type in a similar manner.
        //
        // (iii) add extra tag parameters as necessary:
        //
        //      - if we have multiple callees, then a parameter of TagType(callee) is appended
        //        to the beginning to select the callee.
        //
        //      - if our callee is Specialize inst with collection args, then for each
        //        table-collection argument, a tag is required as input.
        //

        IRBuilder builder(module);

        List<IRType*> paramTypes;
        IRType* resultType = nullptr;

        auto updateParamType = [&](Index index, IRType* paramType) -> IRType*
        {
            if (paramTypes.getCount() <= index)
            {
                // If this index hasn't been seen yet, expand the buffer and initialize
                // the type.
                //
                paramTypes.growToCount(index + 1);
                paramTypes[index] = paramType;
                return paramType;
            }
            else
            {
                // Otherwise, update the existing type
                auto [currentDirection, currentType] =
                    splitParameterDirectionAndType(paramTypes[index]);
                auto [newDirection, newType] = splitParameterDirectionAndType(paramType);
                auto updatedType = updateType(currentType, newType);
                SLANG_ASSERT(currentDirection == newDirection);
                paramTypes[index] = fromDirectionAndType(&builder, currentDirection, updatedType);
                return updatedType;
            }
        };

        List<IRInst*> calleesToProcess;
        forEachInSet(calleeSet, [&](IRInst* func) { calleesToProcess.add(func); });

        for (auto context : calleesToProcess)
        {
            auto paramEffectiveTypes = getEffectiveParamTypes(context);
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
        // Add in extra parameter types for a call to a dynamic generic callee
        //

        List<IRType*> extraParamTypes;

        // If the any of the elements in the callee (or the callee itself in case
        // of a singleton) is a dynamic specialization, each non-singleton WitnessTableSet,
        // requries a corresponding tag input.
        //
        if (calleeSet->isSingleton() && isSetSpecializedGeneric(calleeSet->getElement(0)))
        {
            auto specializeInst = as<IRSpecialize>(calleeSet->getElement(0));

            // If this is a dynamic generic, we need to add a tag type for each
            // WitnessTableSet in the callee.
            //
            for (UIndex i = 0; i < specializeInst->getArgCount(); i++)
                if (auto tableSet = as<IRWitnessTableSet>(specializeInst->getArg(i)))
                    extraParamTypes.add((IRType*)makeTagType(tableSet));
        }

        List<IRType*> allParamTypes;
        allParamTypes.addRange(extraParamTypes);
        allParamTypes.addRange(paramTypes);

        return builder.getFuncType(allParamTypes, resultType);
    }

    IRFuncType* getEffectiveFuncType(IRInst* callee)
    {
        IRBuilder builder(module);
        return getEffectiveFuncTypeForSet(cast<IRFuncSet>(builder.getSingletonSet(callee)));
    }

    // Helper function for specializing calls.
    //
    // For a `Specialize` instruction that has dynamic tag arguments,
    // extract all the tags and return them as a list.
    //
    List<IRInst*> getArgsForSetSpecializedGeneric(IRSpecialize* specializedCallee)
    {
        List<IRInst*> callArgs;
        for (UInt ii = 0; ii < specializedCallee->getArgCount(); ii++)
        {
            auto specArg = specializedCallee->getArg(ii);
            auto argInfo = specArg->getDataType();

            // Pull all tag-type arguments from the specialization arguments
            // and add them to the call arguments.
            //
            if (auto tagType = as<IRSetTagType>(argInfo))
                if (as<IRWitnessTableSet>(tagType->getSet()))
                    callArgs.add(specArg);
        }

        return callArgs;
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
        // The overall goal is to remove any dynamic-ness in the call inst
        // (i.e. the callee as well as types of arguments should be global
        // insts)
        //
        // There are a few cases we need to handle when specializing a call
        // inst.
        //
        // First, we handle the callee:
        //
        // - If the callee is already a concrete function, there's nothing to do
        //
        // - If the callee is a dynamic inst of tag type, we replace
        //   the callee with the collection itself, and pass the tag inst as
        //   the first operand. Effectively, we are placing a call to a set of functions
        //   and using the tag to specify which function to call.
        //
        //     e.g.
        //        let tag : TagType(funcSet) = /* ... */;
        //        let val = Call(tag, arg1, arg2, ...);
        //     becomes
        //        let tag : TagType(funcSet) = /* ... */;
        //        let val = Call(funcSet, tag, arg1, arg2, ...);
        //
        // - If any the callee is a dynamic specialization of a generic, we need to add any dynamic
        // witness
        //   table insts as arguments to the call.
        //
        //      e.g.:
        //          Call(
        //              Specialize(g, specArgs...), callArgs...);
        //          where atleast one of specialization args is a dynamic tag inst.
        //
        //      Our convention for dynamic generics is that the dynamic witness table
        //      operands are added to the front of the regular call arguments.
        //
        //      So, we'll turn this into:
        //          Call(
        //              Specialize(g, staticFormOfSpecArgs...), dynamicSpecArgs..., callArgs...);
        //          where the new callee is a specialization where any dynamic insts are
        //          replaced with their static collections.
        //
        //      // --- before specialization ---
        //      let s1 : TagType(WitnessTableSet(tA, tB, tC)) = /* ... */;
        //      let s2 : TagType(TypeSet(A, B, C)) = /* ... */;
        //      let specCallee = Specialize(generic, s1, s2);
        //      let val = Call(specCallee, /* call args */);
        //
        //      // --- after specialization ---
        //      let s1 : TagType(WitnessTableSet(tA, tB, tC)) = /* ... */;
        //      let s2 : TagType(TypeSet(A, B, C)) = /* ... */;
        //      let newSpecCallee = Specialize(generic,
        //          WitnessTableSet(tA, tB, tC), TypeSet(A, B, C));
        //      let newVal = Call(newSpecCallee, s1, /* call args */);
        //
        //
        //  - In case the callee is a collection of dynamically specialized
        //    generics, _both_ of the above transformations are applied, with
        //    the callee's tag going first, followed by any witness table tags
        //    and finally the regular call arguments.
        //    However, this case is NOT currently well supported because the func-collection
        //    tag does not encode the additional tags that need to be passed, so this
        //    is likely to fail currently.
        //    This is a rare scenario that only occurs on trying to specialize an existential
        //    method with existential arguments, which we don't officially support.
        //
        // Secondly, we handle the argument types:
        //    It is possible that
        //    the parameters in the callee have been specialized to accept
        //    a wider collection compared to the arguments from this call site
        //
        //    In this case, we just upcast them using `upcastSet` before
        //    creating a new call inst
        //

        auto callee = inst->getCallee();

        if (isNoneCallee(callee))
            return false;

        // IRInst* calleeTagInst = nullptr;
        List<IRInst*> callArgs;

        // This is a bit of a workaround for specialized callee's
        // whose function types haven't been specialized yet (can
        // occur for concrete IRSpecialize insts that are created
        // during the specializeing process).
        //
        maybeSpecializeCalleeType(callee);

        // If we're calling using a tag, place a call to the collection,
        // with the tag as the first argument. So the callee is
        // the collection itself.
        //
        if (auto collectionTag = as<IRSetTagType>(callee->getDataType()))
        {
            if (!collectionTag->isSingleton())
            {
                // Multiple callees case:
                //
                // If we need to use a tag, we'll do a bit of an optimization here..
                //
                // Instead of building a dispatcher on then func-collection, we'll
                // build it on the table collection that it is looked up from. This
                // avoids the extra map.
                //
                // This works primarily because this is the only way to call a dynamic
                // function. If we ever have the ability to pass functions around more
                // flexibly, then this should just become a specific case.

                if (auto tagMapOperand = as<IRGetTagForMappedSet>(callee))
                {
                    auto tableTag = tagMapOperand->getOperand(0);
                    auto lookupKey = cast<IRStructKey>(tagMapOperand->getOperand(1));

                    auto tableSet = cast<IRWitnessTableSet>(
                        cast<IRSetTagType>(tableTag->getDataType())->getSet());
                    IRBuilder builder(module);

                    callee = builder.emitGetDispatcher(
                        getEffectiveFuncTypeForDispatcher(
                            tableSet,
                            lookupKey,
                            cast<IRFuncSet>(collectionTag->getSet())),
                        tableSet,
                        lookupKey);

                    callArgs.add(tableTag);
                }
                else if (auto specializedTagMapOperand = as<IRGetTagForSpecializedSet>(callee))
                {
                    auto innerTagMapOperand =
                        cast<IRGetTagForMappedSet>(specializedTagMapOperand->getOperand(0));
                    auto tableTag = innerTagMapOperand->getOperand(0);
                    auto tableSet = cast<IRWitnessTableSet>(
                        cast<IRSetTagType>(tableTag->getDataType())->getSet());
                    auto lookupKey = cast<IRStructKey>(innerTagMapOperand->getOperand(1));

                    List<IRInst*> specArgs;
                    for (UInt argIdx = 1; argIdx < specializedTagMapOperand->getOperandCount();
                         ++argIdx)
                    {
                        auto arg = specializedTagMapOperand->getOperand(argIdx);
                        if (auto tagType = as<IRSetTagType>(arg->getDataType()))
                        {
                            SLANG_ASSERT(!tagType->getSet()->isSingleton());
                            if (as<IRWitnessTableSet>(tagType->getSet()))
                            {
                                callArgs.add(arg);
                                specArgs.add(tagType->getSet());
                            }
                            else
                            {
                                specArgs.add(tagType->getSet());
                            }
                        }
                        else
                        {
                            SLANG_ASSERT(isGlobalInst(arg));
                            specArgs.add(arg);
                        }
                    }

                    IRBuilder builder(module);
                    builder.setInsertBefore(callee);
                    callee = builder.emitGetSpecializedDispatcher(
                        getEffectiveFuncTypeForDispatcher(
                            tableSet,
                            lookupKey,
                            cast<IRFuncSet>(collectionTag->getSet())),
                        tableSet,
                        lookupKey,
                        specArgs);

                    callArgs.add(tableTag);
                }
                else
                {
                    SLANG_UNEXPECTED(
                        "Cannot specialize call with non-singleton collection tag callee");
                }
            }
            else if (isSetSpecializedGeneric(collectionTag->getSet()->getElement(0)))
            {
                // Single element which is a set specialized generic.
                callArgs.addRange(getArgsForSetSpecializedGeneric(cast<IRSpecialize>(callee)));
                callee = collectionTag->getSet()->getElement(0);

                auto funcType = getEffectiveFuncType(callee);
                callee->setFullType(funcType);
            }
            else
            {
                // If we reach here, then something is wrong. If our callee is an inst of tag-type,
                // we expect it to either be a `GetTagForMappedSet`, `Specialize` or
                // `GetTagForSpecializedSet`.
                // Any other case should never occur (in the current design of the compiler)
                //
                SLANG_UNEXPECTED(
                    "Unexpected operand type for type-flow specialization of Call inst");
            }
        }
        else if (isGlobalInst(callee) && !isIntrinsic(callee))
        {
            // If our callee is not a tag-type, then it is necessarily a simple concrete function.
            // We will fix-up the function type so that it has the effective types as determined
            // by the analysis.
            //
            auto funcType = getEffectiveFuncType(callee);
            callee->setFullType(funcType);
        }

        // If by this point, we haven't resolved our callee into a global inst (
        // either a collection or a single function), then we can't specialize it (likely unbounded)
        //
        if (!isGlobalInst(callee) || isIntrinsic(callee))
            return false;

        // One other case to avoid is if the function is a global LookupWitnessMethod
        // which can be created when optional witnesses are specialized.
        //
        if (as<IRLookupWitnessMethod>(callee))
            return false;

        // First, we'll legalize all operands by upcasting if necessary.
        // This needs to be done even if the callee is not a collection.
        //
        UCount extraArgCount = callArgs.getCount();
        for (UInt i = 0; i < inst->getArgCount(); i++)
        {
            auto arg = inst->getArg(i);
            const auto [paramDirection, paramType] = splitParameterDirectionAndType(
                cast<IRFuncType>(callee->getFullType())->getParamType(i + extraArgCount));

            switch (paramDirection.kind)
            {
            // We'll upcast any in-parameters.
            case ParameterDirectionInfo::Kind::In:
                {
                    IRBuilder builder(context);
                    builder.setInsertBefore(inst);
                    callArgs.add(upcastSet(&builder, arg, paramType));
                    break;
                }

            // Out parameters are handled at the callee's end
            case ParameterDirectionInfo::Kind::Out:

            // For all other modes, collections must match ('subtyping' is not allowed)
            case ParameterDirectionInfo::Kind::BorrowInOut:
            case ParameterDirectionInfo::Kind::BorrowIn:
            case ParameterDirectionInfo::Kind::Ref:
                {
                    callArgs.add(arg);
                    break;
                }
            default:
                SLANG_UNEXPECTED("Unhandled parameter direction in specializeCall");
            }
        }

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        bool changed = false;
        if (((UInt)callArgs.getCount()) != inst->getArgCount())
            changed = true;
        else
        {
            for (Index i = 0; i < callArgs.getCount(); i++)
            {
                if (callArgs[i] != inst->getArg((UInt)i))
                {
                    changed = true;
                    break;
                }
            }
        }

        if (callee != inst->getCallee())
        {
            changed = true;
        }

        auto calleeFuncType = cast<IRFuncType>(callee->getFullType());

        if (changed)
        {
            auto newCall = builder.emitCallInst(calleeFuncType->getResultType(), callee, callArgs);
            inst->replaceUsesWith(newCall);
            inst->removeAndDeallocate();
            return true;
        }
        else if (calleeFuncType->getResultType() != inst->getFullType())
        {
            // If we didn't change the callee or the arguments, we still might
            // need to update the result type.
            //
            inst->setFullType(calleeFuncType->getResultType());
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
        // The main thing to handle here is that we might have specialized
        // the fields of the struct, so we need to upcast the arguments
        // if necessary.
        //

        auto structType = as<IRStructType>(inst->getDataType());
        if (!structType)
            return false;

        // Reinterpret any of the arguments as necessary.
        bool changed = false;
        UIndex operandIndex = 0;
        for (auto field : structType->getFields())
        {
            auto arg = inst->getOperand(operandIndex);
            IRBuilder builder(context);
            builder.setInsertBefore(inst);
            auto newArg = upcastSet(&builder, arg, field->getFieldType());

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
        // After specialization, existentials (that are not unbounded) are treated as tuples
        // of a WitnessTableSet tag and a value of type TypeSet.
        //
        // A MakeExistential is just converted into a MakeTaggedUnion, with any necessary
        // upcasts.
        //

        auto info = tryGetInfo(context, inst);
        auto taggedUnion = as<IRTaggedUnionType>(info);
        if (!taggedUnion)
            return false;

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Collect types from the witness tables to determine the any-value type
        auto tableSet = taggedUnion->getWitnessTableSet();
        auto typeSet = taggedUnion->getTypeSet();

        IRInst* witnessTableTag = nullptr;
        if (auto witnessTable = as<IRWitnessTable>(inst->getWitnessTable()))
        {
            auto singletonTagType = makeTagType(builder.getSingletonSet(witnessTable));
            IRInst* tagValue =
                builder.emitGetTagOfElementInSet((IRType*)singletonTagType, witnessTable, tableSet);
            witnessTableTag = builder.emitIntrinsicInst(
                (IRType*)makeTagType(tableSet),
                kIROp_GetTagForSuperSet,
                1,
                &tagValue);
        }
        else if (as<IRSetTagType>(inst->getWitnessTable()->getDataType()))
        {
            // Dynamic. Use the witness table inst as a tag
            witnessTableTag = inst->getWitnessTable();
        }

        // Create the appropriate any-value type
        auto collectionType = typeSet->isSingleton()
                                  ? (IRType*)typeSet->getElement(0)
                                  : builder.getUntaggedUnionType((IRType*)typeSet);

        // Pack the value
        auto packedValue = as<IRTypeSet>(collectionType)
                               ? builder.emitPackAnyValue(collectionType, inst->getWrappedValue())
                               : inst->getWrappedValue();

        auto taggedUnionType = getLoweredType(taggedUnion);

        auto tuple = builder.emitMakeTaggedUnion(taggedUnionType, witnessTableTag, packedValue);

        inst->replaceUsesWith(tuple);
        inst->removeAndDeallocate();
        return true;
    }

    bool specializeCreateExistentialObject(IRInst* context, IRCreateExistentialObject* inst)
    {
        // A CreateExistentialObject uses an user-provided ID to create an object.
        // Note that this ID is not the same as the tags we use. The user-provided ID must be
        // compared against the SequentialID, which is a globally consistent & public ID present
        // on the witness tables.
        //
        // The tags are a locally consistent ID whose semantics are only meaningful within the
        // function. We use a special op `GetTagFromSequentialID` to convert from the user-provided
        // global ID to a local tag ID.
        //

        auto info = tryGetInfo(context, inst);
        auto taggedUnion = as<IRTaggedUnionType>(info);
        if (!taggedUnion)
            return false;

        auto taggedUnionType = as<IRTaggedUnionType>(getLoweredType(taggedUnion));

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        IRInst* args[] = {inst->getDataType(), inst->getTypeID()};
        auto translatedTag = builder.emitIntrinsicInst(
            (IRType*)makeTagType(taggedUnionType->getWitnessTableSet()),
            kIROp_GetTagFromSequentialID,
            2,
            args);

        IRInst* packedValue = nullptr;
        auto collection = taggedUnionType->getTypeSet();
        if (!collection->isSingleton())
        {
            packedValue = builder.emitPackAnyValue(
                (IRType*)builder.getUntaggedUnionType(collection),
                inst->getValue());
        }
        else
        {
            packedValue = builder.emitReinterpret(
                (IRType*)builder.getUntaggedUnionType(collection),
                inst->getValue());
        }

        auto newInst =
            builder.emitMakeTaggedUnion((IRType*)taggedUnionType, translatedTag, packedValue);

        inst->replaceUsesWith(newInst);
        inst->removeAndDeallocate();
        return true;
    }

    bool specializeStructuredBufferLoad(IRInst* context, IRInst* inst)
    {
        // The key thing to take care of here is a load from an
        // interface-typed pointer.
        //
        // Our type-flow analysis will convert the
        // result into a collection of all available implementations of this
        // interface, so we need to cast the result.
        //

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
                // defer the specializing of the load until a later lowering
                // pass.
                //
                // This avoids having to change the source pointer type
                // and confusing any future runs of the type flow
                // analysis pass.
                //
                // This is slightly different from how a local 'load' is handled,
                // because we don't want to modify the pointer (and consequently the global
                // buffer) type, since it is a publicly visible type that is laid out
                // in a certain way.
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
        // When specializing a `Specialize` instruction, we have a few nuances.
        //
        // If we're dealing with specializing a type, witness table, or any other
        // generic, we simply drop all dynamic tag information, and replace all
        // operands with their collection variants.
        //
        // If we're dealing with a function, there are two cases:
        // - A single function when dynamic specialization arguments.
        //      Removing the dynamic tag information will result in the eventual 'call'
        //      inst not have access to these insts.
        //
        //      Instead, we'll just replace the type, and retain the `Specialize` inst with
        //      the dynamic args. It will be specialized out in `specializeCall` instead.
        //
        // - A collection of functions with concrete specialization arguments.
        //     In this case, we will emit an instruction to map from the input generic collection
        //     to the output specialized collection via `GetTagForSpecializedSet`.
        //     This inst encodes the key-value mapping in its operands:
        //          e.g.(input_tag, key0, value0, key1, value1, ...)
        //
        // - The case where there is a collection of functions with dynamic specialization arguments
        //   is not currently properly handled. This case should not arise naturally since we
        //   don't advertise support for it.
        //

        bool isFuncReturn = false;

        if (auto concreteGeneric = as<IRGeneric>(inst->getBase()))
            isFuncReturn = as<IRFunc>(getGenericReturnVal(concreteGeneric)) != nullptr;
        else if (auto tagType = as<IRSetTagType>(inst->getBase()->getDataType()))
        {
            auto firstConcreteGeneric = as<IRGeneric>(tagType->getSet()->getElement(0));
            isFuncReturn = as<IRFunc>(getGenericReturnVal(firstConcreteGeneric)) != nullptr;
        }

        // We'll emit a dynamic tag inst if the result is a func collection with multiple elements
        if (isFuncReturn)
        {
            if (auto info = tryGetInfo(context, inst))
            {
                if (auto elementOfSetType = as<IRElementOfSetType>(info))
                {
                    // Note for future reworks:
                    // Should we make it such that the `GetTagForSpecializedSet`
                    // is emitted in the single func case too?
                    //
                    // Basically, as long as any of the specialization operands are dynamic,
                    // we should probably emit a tag.
                    //
                    // Currently, if the func is a singleton, we leave it as a Specialize inst
                    // with dynamic args to be handled in specializeCall.
                    //

                    if (elementOfSetType->getSet()->isSingleton())
                    {
                        // If the result is a singleton collection, we can just
                        // replace the type (if necessary) and be done with it.
                        return replaceType(context, inst);
                    }
                    else
                    {
                        // Otherwise, we'll emit a tag mapping instruction.
                        IRBuilder builder(inst);
                        setInsertBeforeOrdinaryInst(&builder, inst);

                        List<IRInst*> specOperands;
                        specOperands.add(inst->getBase());

                        for (UInt ii = 0; ii < inst->getArgCount(); ii++)
                            specOperands.add(inst->getArg(ii));

                        auto newInst = builder.emitIntrinsicInst(
                            (IRType*)makeTagType(elementOfSetType->getSet()),
                            kIROp_GetTagForSpecializedSet,
                            specOperands.getCount(),
                            specOperands.getBuffer());

                        inst->replaceUsesWith(newInst);
                        inst->removeAndDeallocate();
                        return true;
                    }
                }
                else
                {
                    SLANG_UNEXPECTED(
                        "Expected element-of-collection type for function specialization");
                }
            }
        }

        // For all other specializations, we'll 'drop' the dynamic tag information.
        bool changed = false;
        List<IRInst*> args;
        for (UIndex i = 0; i < inst->getArgCount(); i++)
        {
            auto arg = inst->getArg(i);
            auto argDataType = arg->getDataType();
            if (auto collectionTagType = as<IRSetTagType>(argDataType))
            {
                // If this is a tag type, replace with collection.
                changed = true;
                if (as<IRWitnessTableSet>(collectionTagType->getSet()))
                {
                    args.add(collectionTagType->getSet());
                }
                else if (auto typeSet = as<IRTypeSet>(collectionTagType->getSet()))
                {
                    IRBuilder builder(inst);
                    args.add(builder.getUntaggedUnionType(typeSet));
                }
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
        // `GetValueFromBoundInterface` is essentially accessing the value component of
        // an existential. If the operand has been specialized into a tagged-union, then we can
        //  turn it into a `GetValueFromTaggedUnion`.
        //

        SLANG_UNUSED(context);

        auto operandInfo = inst->getOperand(0)->getDataType();
        if (as<IRTaggedUnionType>(operandInfo))
        {
            IRBuilder builder(inst);
            setInsertAfterOrdinaryInst(&builder, inst);
            auto newInst = builder.emitGetValueFromTaggedUnion(inst->getOperand(0));
            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();
            return true;
        }
        return false;
    }

    bool specializeGetElementFromTag(IRInst* context, IRGetElementFromTag* inst)
    {
        SLANG_UNUSED(context);
        inst->replaceUsesWith(inst->getOperand(0));
        inst->removeAndDeallocate();
        return true;
    }

    bool specializeLoad(IRInst* context, IRInst* inst)
    {
        // There's two cases to handle..
        //
        // (i) For a simple load, the pointer itself is already specialized.
        // so we just need to replace the type of the load with specialized type.
        //
        // (ii) if there is a mismatch between the two types, the most likely
        // case is that we're trying to load from an interface typed location
        // (whose type we cannot modify), and cast it into a tagged union tuple.
        //
        // This case is similar to `specializeStructuredBufferLoad`, where we
        // cast the _pointer_ to convert its type, and defer the legalization of
        // the load to a later lowering pass.
        //

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
        // This handles a rare case in the compiler, where we
        // try to use default-construct to initialize a field.
        //
        // This is not technically supported, but it can occur
        // during some corner cases during higher-order auto-diff.
        //
        // In case we've specialized the field, we just need to
        // modify the default-construct operand's type to
        // match the field.
        //
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
        // Similar to `specializeLoad`, we handle cases where
        // the pointer has been specialized, so that we upcast
        // our value to match the type before writing to the location.
        //

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

        IRBuilder builder(context);
        builder.setInsertBefore(inst);
        auto specializedVal = upcastSet(&builder, inst->getVal(), ptrInfo);

        if (specializedVal != inst->getVal())
        {
            // If the value was changed, we need to update the store instruction.
            builder.replaceOperand(inst->getValUse(), specializedVal);
            return true;
        }

        return false;
    }

    bool specializeGetSequentialID(IRInst* context, IRGetSequentialID* inst)
    {
        // A sequential ID is a globally unique ID for a witness table, while the
        // the tags we use in the specialization are only locally consistent.
        //
        // To extract the global ID, we'll use a separate op code `GetSequentialIDFromTag`
        // for now and lower it later once all the global sequential IDs have been assigned.
        //
        SLANG_UNUSED(context);
        auto arg = inst->getOperand(0);
        if (auto tagType = as<IRSetTagType>(arg->getDataType()))
        {
            IRBuilder builder(inst);
            setInsertAfterOrdinaryInst(&builder, inst);
            auto firstElement = tagType->getSet()->getElement(0);
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
        // The is-type checks equality between two witness tables
        // via their sequential IDs.
        //
        // If the dynamic part has been specialized into a tag, we emit
        // a `GetSequentialIDFromTag` inst to extract the ID and emit
        // an equality test.
        //
        SLANG_UNUSED(context);
        auto witnessTableArg = inst->getValueWitness();
        if (auto tagType = as<IRSetTagType>(witnessTableArg->getDataType()))
        {
            IRBuilder builder(inst);
            setInsertAfterOrdinaryInst(&builder, inst);

            auto targetTag = builder.emitGetTagOfElementInSet(
                (IRType*)tagType,
                inst->getTargetWitness(),
                tagType->getSet());
            auto eqlInst = builder.emitEql(targetTag, witnessTableArg);

            inst->replaceUsesWith(eqlInst);
            inst->removeAndDeallocate();
            return true;
        }

        return false;
    }

    bool specializeMakeOptionalNone(IRInst* context, IRMakeOptionalNone* inst)
    {
        if (auto taggedUnionType = as<IRTaggedUnionType>(tryGetInfo(context, inst)))
        {
            // If we're dealing with a `MakeOptionalNone` for an existential type, then
            // this just becomes a tagged union tuple where the set of tables is {none}
            // (i.e. singleton set of none witness)
            //

            IRBuilder builder(module);
            builder.setInsertBefore(inst);

            // Create a tuple for the empty type..
            SLANG_ASSERT(taggedUnionType->getWitnessTableSet()->isSingleton());
            auto noneWitnessTable = taggedUnionType->getWitnessTableSet()->getElement(0);

            auto singletonTagType = makeTagType(builder.getSingletonSet(noneWitnessTable));
            IRInst* zeroValueOfTagType = builder.emitGetTagOfElementInSet(
                (IRType*)singletonTagType,
                noneWitnessTable,
                taggedUnionType->getWitnessTableSet());

            auto newTuple = builder.emitMakeTaggedUnion(
                (IRType*)taggedUnionType,
                zeroValueOfTagType,
                builder.emitDefaultConstruct(makeUntaggedUnionType(taggedUnionType->getTypeSet())));

            inst->replaceUsesWith(newTuple);
            propagationMap[InstWithContext(context, newTuple)] = taggedUnionType;
            inst->removeAndDeallocate();

            return true;
        }

        return false;
    }

    bool specializeMakeOptionalValue(IRInst* context, IRMakeOptionalValue* inst)
    {
        SLANG_UNUSED(context);
        if (as<IRTaggedUnionType>(inst->getValue()->getDataType()))
        {
            // If we're dealing with a `MakeOptionalValue` for an existential type,
            // we don't actually have to change anything, since logically, the input and output
            // represent the same set of types and tables.
            //
            // We'll do a simple replace.
            //

            auto newInst = inst->getValue();
            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();

            return true;
        }

        return false;
    }

    bool specializeGetOptionalValue(IRInst* context, IRGetOptionalValue* inst)
    {
        SLANG_UNUSED(context);
        if (as<IRTaggedUnionType>(inst->getOptionalOperand()->getDataType()))
        {
            // Since `GetOptionalValue` is the reverse of `MakeOptionalValue`, and we treat
            // the latter as a no-op, then `GetOptionalValue` is also a no-op (we simply pass
            // the inner existential value as-is)
            //

            auto newInst = inst->getOptionalOperand();
            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();
            return true;
        }
        return false;
    }

    bool specializeOptionalHasValue(IRInst* context, IROptionalHasValue* inst)
    {
        SLANG_UNUSED(context);
        if (auto taggedUnionType = as<IRTaggedUnionType>(inst->getOptionalOperand()->getDataType()))
        {
            // The logic here is similar to specializing IsType, but we'll directly compare
            // tags instead of trying to use sequential ID.
            //
            // There's two cases to handle here:
            // 1. We statically know that it cannot be a 'none' because the
            //    input's collection type doesn't have a 'none'. In this case
            //    we just return a true.
            //
            // 2. 'none' is a possibility. In this case, we create a 0 value of
            //    type TagType(WitnessTableSet(NoneWitness)) and then upcast it
            //    to TagType(inputWitnessTableSet). This will convert the value
            //    to the corresponding value of 'none' in the input's table collection
            //    allowing us to directly compare it against the tag part of the
            //    input tagged union.
            //

            IRBuilder builder(inst);

            bool containsNone = false;
            forEachInSet(
                taggedUnionType->getWitnessTableSet(),
                [&](IRInst* wt)
                {
                    if (wt == getNoneWitness())
                        containsNone = true;
                });

            if (!containsNone)
            {
                // If 'none' isn't a part of the collection, statically set
                // to true.
                //

                auto trueVal = builder.getBoolValue(true);
                inst->replaceUsesWith(trueVal);
                inst->removeAndDeallocate();
                return true;
            }
            else
            {
                // Otherwise, we'll extract the tag and compare against
                // the value for 'none' (in the context of the tag's collection)
                //
                builder.setInsertBefore(inst);

                auto dynTag = builder.emitGetTagFromTaggedUnion(inst->getOptionalOperand());

                // Cast the singleton tag to the target collection tag (will convert the
                // value to the corresponding value for the larger set)
                //
                auto noneWitnessTag = builder.emitGetTagOfElementInSet(
                    (IRType*)makeTagType(taggedUnionType->getWitnessTableSet()),
                    getNoneWitness(),
                    taggedUnionType->getWitnessTableSet());

                auto newInst = builder.emitNeq(dynTag, noneWitnessTag);
                inst->replaceUsesWith(newInst);
                inst->removeAndDeallocate();
                return true;
            }
        }
        return false;
    }

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
        //    This phase propagates type information through the module
        //    and records them into different maps in the current context.
        //
        performInformationPropagation();

        if (sink->getErrorCount() > 0)
        {
            // If there were errors during propagation, we bail out early.
            return false;
        }

        // Phase 2: Dynamic Instruction Specialization
        //    Re-write dynamic instructions into specialized versions based on the
        //    type information in the previous phase.
        //
        hasChanges |= performDynamicInstLowering();

        return hasChanges;
    }

    TypeFlowSpecializationContext(IRModule* module, DiagnosticSink* sink)
        : module(module), sink(sink)
    {
    }


    // Basic context
    IRModule* module;
    DiagnosticSink* sink;

    // Mapping from (context, inst) --> propagated info
    Dictionary<InstWithContext, IRInst*> propagationMap;

    // Mapping from context --> return value info
    Dictionary<IRInst*, IRInst*> funcReturnInfo;

    // Mapping from (struct field) --> propagated info
    Dictionary<IRStructField*, IRInst*> fieldInfo;

    // Mapping from context --> Set<(context, inst)>
    //
    // Maintains a mapping from a callable context to all call-sites
    // (and caller contexts)
    //
    Dictionary<IRInst*, HashSet<InstWithContext>> funcCallSites;

    // Mapping from (struct-field) --> Set<(context, inst)>
    //
    // Maintains a mapping from a struct field to all accesses of that
    // field
    //
    Dictionary<IRStructField*, HashSet<InstWithContext>> fieldUseSites;

    // Set of already discovered contexts.
    HashSet<IRInst*> availableContexts;
};

// Main entry point
bool specializeDynamicInsts(IRModule* module, DiagnosticSink* sink)
{
    TypeFlowSpecializationContext context(module, sink);
    return context.processModule();
}

} // namespace Slang
