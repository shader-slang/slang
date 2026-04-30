#include "slang-ir-typeflow-specialize.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-clone.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-lower-dynamic-dispatch-insts.h"
#include "slang-ir-specialize.h"
#include "slang-ir-translate.h"
#include "slang-ir-typeflow-set.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"


namespace Slang
{


// Helper to extract the underlying IRFunc from any context type
// (IRFunc, IRSpecialize, or IRSpecializeExistentialsInFunc).
IRFunc* getFuncDefinitionForContext(IRInst* context)
{
    if (auto func = as<IRFunc>(context))
        return func;
    if (auto specialize = as<IRSpecialize>(context))
    {
        auto generic = cast<IRGeneric>(specialize->getBase());
        return cast<IRFunc>(findGenericReturnVal(generic));
    }
    if (auto existentialSpecializedFunc = as<IRSpecializeExistentialsInFunc>(context))
    {
        return getFuncDefinitionForContext(existentialSpecializedFunc->getFunc());
    }
    return nullptr;
}

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
                if (as<IRDecoration>(inst))
                    SLANG_ASSERT(inst->getParent()->getParent()->getParent() == context);
                else
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
        case kIROp_SpecializeExistentialsInFunc:
            {
                // SpecializeExistentialsInFunc wraps an IRFunc. The inst must be inside
                // the underlying function.
                auto baseFunc = getFuncDefinitionForContext(context);
                SLANG_ASSERT(baseFunc);

                bool foundParent = false;
                for (auto parent = inst->getParent(); parent; parent = parent->getParent())
                {
                    if (parent == baseFunc)
                    {
                        foundParent = true;
                        break;
                    }
                }
                SLANG_ASSERT(foundParent);

                break;
            }
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

// Test if the callee represents an invalid call. This can arise from looking something up
// on a 'none' witness (in the presence of optional witness tables)
//
bool isNoneCallee(IRInst* callee)
{
    if (auto lookupWitness = as<IRLookupWitnessMethod>(callee))
    {
        if (auto table = as<IRWitnessTable>(lookupWitness->getWitnessTable()))
        {
            return table->getConcreteType()->getOp() == kIROp_VoidType;
        }
    }

    if (as<IRVoidLit>(callee))
        return true;

    return false;
}

IRInst* getInvalidExistentialSpecializationTarget(IRInst* specializedValue)
{
    IRInst* specializationBase = specializedValue;
    if (auto specialize = as<IRSpecialize>(specializationBase))
        specializationBase = specialize->getBase();

    if (auto generic = as<IRGeneric>(specializationBase))
        if (auto retVal = findInnerMostGenericReturnVal(generic))
            specializationBase = retVal;

    if (auto lookupWitness = as<IRLookupWitnessMethod>(specializationBase))
        specializationBase = lookupWitness->getRequirementKey();

    return specializationBase;
}

// Returns true if a specialization argument transitively derives from an existential.
// Traces through LookupWitnessMethod chains to find existential roots,
// handling nested associated types (e.g. outer.Inner.Value) at any depth.
static bool isSpecArgExistentialDerived(IRInst* inst, int depth = 0)
{
    if (depth > 16)
        return false;
    switch (inst->getOp())
    {
    case kIROp_InterfaceType:
    case kIROp_ExtractExistentialType:
    case kIROp_ExtractExistentialWitnessTable:
    case kIROp_MakeExistential:
        return true;
    case kIROp_TypeEqualityWitness:
        return as<IRInterfaceType>(inst->getOperand(0)) != nullptr;
    case kIROp_LookupWitnessMethod:
        return isSpecArgExistentialDerived(
            as<IRLookupWitnessMethod>(inst)->getWitnessTable(),
            depth + 1);
    case kIROp_BuiltinCast:
        return isSpecArgExistentialDerived(inst->getOperand(0), depth + 1);
    default:
        return false;
    }
}

bool isInvalidExistentialSpecialization(IRInst* specializedValue)
{
    if (specializedValue->findDecoration<IRDisallowSpecializationWithExistentialsDecoration>())
        return true;

    // The decoration pass marks IRSpecialize instructions, but specializeModule() may
    // resolve the specialization before the typeflow pass runs, consuming the decorated
    // IRSpecialize and leaving behind a concrete function that contains the
    // TypeEqualityWitness + LookupWitnessMethod pattern in its body. This body scan
    // catches those post-resolution cases.
    if (auto func = as<IRFunc>(specializedValue))
    {
        for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                // Detect ByteAddressBufferLoad/Store specialized with an interface type.
                // When byteAddressBufferLoad<IFoo> is specialized, the result type of
                // the load instruction (or an operand type of the store) will be an
                // interface type, which is not supported.
                if (inst->getOp() == kIROp_ByteAddressBufferLoad)
                {
                    if (as<IRInterfaceType>(inst->getDataType()))
                        return true;
                }
                if (inst->getOp() == kIROp_ByteAddressBufferStore)
                {
                    // Operands: (buffer, offset, alignment, value).
                    // The stored value is at index 3.
                    if (inst->getOperandCount() > 3 &&
                        as<IRInterfaceType>(inst->getOperand(3)->getDataType()))
                        return true;
                }

                auto call = as<IRCall>(inst);
                if (!call)
                    continue;

                auto lookupWitness = as<IRLookupWitnessMethod>(call->getCallee());
                if (!lookupWitness)
                    continue;

                auto typeEqualityWitness = as<IRInst>(lookupWitness->getWitnessTable());
                if (typeEqualityWitness &&
                    typeEqualityWitness->getOp() == kIROp_TypeEqualityWitness &&
                    as<IRInterfaceType>(typeEqualityWitness->getOperand(0)))
                {
                    return true;
                }
            }
        }
    }

    auto specialize = as<IRSpecialize>(specializedValue);
    if (!specialize)
        return false;

    for (UInt i = 0; i < specialize->getArgCount(); ++i)
    {
        auto arg = specialize->getArg(i);
        if (isSpecArgExistentialDerived(arg))
            return true;
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
template<typename T>
struct WorkQueue
{
    List<T> enqueueList;
    List<T> dequeueList;
    Index dequeueIndex = 0;

    void enqueue(const T& item) { enqueueList.add(item); }

    T dequeue()
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

IRInst* getArrayStride(IRArrayType* arrayType)
{
    if (arrayType->getOperandCount() >= 3)
        return arrayType->getStride();
    return nullptr;
}

// Helper to test if an inst is in the global scope.
bool isGlobalInst(IRInst* inst)
{
    return inst->getParent()->getOp() == kIROp_ModuleInst;
}

// This is fairly fundamental check:
// This method checks whether a inst's type cannot accept any further refinement.
//
// Examples:
// 1. an inst of `UInt` type cannot be further refined (under the current scope
// of the type-flow pass), since it has a concrete type, and we cannot replace it
// with a "narrower" type.
//
// 2. an inst of `InterfaceType` represents a tagged union of any type that implements
// the interface, so it can be further refined by determining a smaller set of
// possibilities (i.e. via `TaggedUnionType(tableSet, typeSet)`).
//
// 3. Similarly, an inst of `WitnessTableType` represents any witness table,
// so it can accept a further refinement into `ElementOfSetType(tableSet)`.
//
// In the future, we may want to extend this check to something more nuanced,
// which takes in both the inst and the refined type to determine if we want to
// accept the refinement (this is useful in cases like `UnsizedArrayType`, where
// we only want to refine it if we can determine a concrete size).
//
static bool isConcreteTypeImpl(IRInst* inst, HashSet<IRInst*>* visiting)
{
    if (!inst)
        return false;

    bool isInstGlobal = isGlobalInst(inst);
    if (!isInstGlobal)
        return false;

    switch (inst->getOp())
    {
    case kIROp_InterfaceType: // Can be refined to tagged unions
        return isComInterfaceType(cast<IRInterfaceType>(inst));
    case kIROp_WitnessTableType: // Can be refined into set of concrete tables
        return false;
    case kIROp_FuncType: // Can be refined into set of concrete functions
        return false;
    case kIROp_GenericKind: // Can be refined into set of concrete generics
        return false;
    case kIROp_TypeKind: // Can be refined into set of concrete types
        return false;
    case kIROp_TypeType: // Can be refined into set of concrete types
        return false;
    case kIROp_ThisType:
    case kIROp_AssociatedType: // Shouldn't really appear in general, but we have some synthesized
                               // methods that do use this
        return false;
    case kIROp_ArrayType:
        return isConcreteTypeImpl(cast<IRArrayType>(inst)->getElementType(), visiting) &&
               isGlobalInst(cast<IRArrayType>(inst)->getElementCount());
    case kIROp_OptionalType:
        return isConcreteTypeImpl(cast<IROptionalType>(inst)->getValueType(), visiting);
    case kIROp_ConditionalType:
        {
            auto conditionalType = cast<IRConditionalType>(inst);
            auto hasValueInst = conditionalType->getHasValue();
            if (auto boolLit = as<IRBoolLit>(hasValueInst))
            {
                if (!boolLit->getValue())
                    return true;
                return isConcreteTypeImpl(conditionalType->getValueType(), visiting);
            }
            else if (auto intLit = as<IRIntLit>(hasValueInst))
            {
                if (getIntVal(intLit) == 0)
                    return true;
                return isConcreteTypeImpl(conditionalType->getValueType(), visiting);
            }
            return false;
        }
    case kIROp_DifferentialPairType:
        return isConcreteTypeImpl(cast<IRDifferentialPairTypeBase>(inst)->getValueType(), visiting);
    case kIROp_AttributedType:
        return isConcreteTypeImpl(cast<IRAttributedType>(inst)->getBaseType(), visiting);
    case kIROp_TupleType:
        {
            // Tuple is concrete if all element types are concrete
            for (UInt i = 0; i < inst->getOperandCount(); i++)
            {
                if (!isConcreteTypeImpl(inst->getOperand(i), visiting))
                    return false;
            }
            return true;
        }
    case kIROp_StructType:
        {
            // Struct is concrete only if all field types are concrete.
            // A struct containing an interface-typed field is non-concrete
            // and needs type-flow specialization.
            // Use a visited set to guard against cyclic types (e.g. pack-branch
            // types that resolve back to the same struct).
            HashSet<IRInst*> localVisiting;
            if (!visiting)
                visiting = &localVisiting;
            if (visiting->contains(inst))
                return true; // Cycle detected: conservatively treat as concrete
            visiting->add(inst);
            auto structType = cast<IRStructType>(inst);
            for (auto field : structType->getFields())
            {
                if (!isConcreteTypeImpl(field->getFieldType(), visiting))
                    return false;
            }
            return true;
        }
    default:
        break;
    }

    if (auto ptrType = as<IRPtrTypeBase>(inst))
    {
        if (ptrType->getAddressSpace() == AddressSpace::UserPointer)
            return true; // Don't refine user pointers (for now)
        return isConcreteTypeImpl(ptrType->getValueType(), visiting);
    }

    if (auto generic = as<IRGeneric>(inst))
    {
        if (as<IRFuncType>(getGenericReturnVal(generic)))
            return false; // Can be refined into set of concrete generics.
    }

    return true;
}

bool isConcreteType(IRInst* inst)
{
    return isConcreteTypeImpl(inst, nullptr);
}

// Create info for a concrete type, using `paramType` as a union mask to determine
// how much structural decomposition to perform.
//
// - If `paramType` is concrete, return the bare type (no wrapping needed).
// - If `paramType` is structural and `type` matches the same structural form,
//   recurse into sub-components using `paramType`'s sub-types as sub-masks.
// - Otherwise (non-concrete, non-matching paramType), produce a flat UntaggedUnion.
//
IRInst* makeInfoForConcreteType(IRModule* module, IRInst* type, IRInst* paramType)
{
    SLANG_ASSERT(isConcreteType(type));
    SLANG_ASSERT(paramType);
    IRBuilder builder(module);

    // If paramType is concrete, return the bare type directly.
    // (No wrapping needed since concrete positions can't be further refined.)
    //
    if (isConcreteType(paramType))
        return type;

    // Structural cases - recurse only when both type and paramType match the same form.
    // If they don't match, fall through to the flat union at the bottom.
    //
    if (auto ptrType = as<IRPtrTypeBase>(type))
    {
        if (auto paramPtrType = as<IRPtrTypeBase>(paramType))
        {
            return builder.getPtrTypeWithAddressSpace(
                (IRType*)makeInfoForConcreteType(
                    module,
                    ptrType->getValueType(),
                    paramPtrType->getValueType()),
                ptrType);
        }
    }

    if (auto arrayType = as<IRArrayType>(type))
    {
        if (auto paramArrayType = as<IRArrayType>(paramType))
        {
            return builder.getArrayType(
                (IRType*)makeInfoForConcreteType(
                    module,
                    arrayType->getElementType(),
                    paramArrayType->getElementType()),
                arrayType->getElementCount(),
                getArrayStride(arrayType));
        }
    }

    if (auto attributedType = as<IRAttributedType>(type))
    {
        if (auto paramAttributedType = as<IRAttributedType>(paramType))
        {
            List<IRAttr*>& attrs = *module->getContainerPool().getList<IRAttr>();
            for (auto attr : attributedType->getAllAttrs())
                attrs.add(attr);

            auto newAttributedType = builder.getAttributedType(
                (IRType*)makeInfoForConcreteType(
                    module,
                    attributedType->getBaseType(),
                    paramAttributedType->getBaseType()),
                attrs);
            module->getContainerPool().free(&attrs);
            return newAttributedType;
        }
    }

    if (auto tupleType = as<IRTupleType>(type))
    {
        if (auto paramTupleType = as<IRTupleType>(paramType))
        {
            if (tupleType->getOperandCount() == paramTupleType->getOperandCount())
            {
                List<IRType*> elementInfos;
                for (UInt i = 0; i < tupleType->getOperandCount(); i++)
                {
                    elementInfos.add((IRType*)makeInfoForConcreteType(
                        module,
                        tupleType->getOperand(i),
                        paramTupleType->getOperand(i)));
                }
                return builder.getTupleType(elementInfos);
            }
        }
    }

    if (auto optionalType = as<IROptionalType>(type))
    {
        if (auto paramOptionalType = as<IROptionalType>(paramType))
        {
            return builder.getOptionalType((IRType*)makeInfoForConcreteType(
                module,
                optionalType->getValueType(),
                paramOptionalType->getValueType()));
        }
    }

    // Non-structural or mismatched structural paramType: produce a flat UntaggedUnion.
    return builder.getUntaggedUnionType(
        cast<IRTypeSet>(builder.getSingletonSet(kIROp_TypeSet, type)));
}

// Determines a suitable set opcode to use to represent a set of elements of the given type.
//
// e.g. an inst of WitnessTableType can be be refined using a WitnessTableSet (but it doesn't make
// sense to use a TypeSet or FuncSet).
//
// This is primarily used for `LookupWitnessMethod`, where the resulting element could be
// a generic, witness table, type or function, depending on the type of the lookup inst.
//
IROp getSetOpFromType(IRType* type)
{
    switch (type->getOp())
    {
    case kIROp_WitnessTableType: // Can be refined into set of concrete tables
        return kIROp_WitnessTableSet;
    case kIROp_FuncType: // Can be refined into set of concrete functions
        return kIROp_FuncSet;
    case kIROp_GenericKind: // Can be refined into set of concrete generics
        return kIROp_GenericSet;
    case kIROp_TypeKind: // Can be refined into set of concrete types
        return kIROp_TypeSet;
    case kIROp_TypeType: // Can be refined into set of concrete types
        return kIROp_TypeSet;

    // Translatable function types (all equivalent to FuncType for our purposes)
    case kIROp_FuncTypeOf:
    case kIROp_ForwardDiffFuncType:
    case kIROp_ApplyForBwdFuncType:
    case kIROp_BwdCallableFuncType:
    case kIROp_BackwardDiffFuncType:
    case kIROp_RematFuncType:
        return kIROp_FuncSet;
    default:
        break;
    }

    if (auto generic = as<IRGeneric>(type))
    {
        auto innerValType = getGenericReturnVal(generic);
        if (as<IRFuncType>(innerValType) || as<IRWitnessTableType>(innerValType))
            return kIROp_GenericSet; // Can be refined into set of concrete generics.
    }

    // Slight workaround for the fact that we can have cases where the type has not been specialized
    // yet (particularly from auto-diff)
    //
    if (auto specialize = as<IRSpecialize>(type))
    {
        auto innerValType = getGenericReturnVal(specialize->getBase());
        if (as<IRFuncType>(innerValType))
            return kIROp_FuncSet;
        if (as<IRWitnessTableType>(innerValType))
            return kIROp_WitnessTableSet;
    }

    return kIROp_Invalid;
}

// Helper to check if an IRParam is a function parameter (vs. a phi param or generic param)
bool isFuncParam(IRParam* param)
{
    auto paramBlock = as<IRBlock>(param->getParent());
    auto paramFunc = as<IRFunc>(paramBlock->getParent());
    return (paramFunc && paramFunc->getFirstBlock() == paramBlock);
}

bool isPublicFunc(IRFunc* func)
{
    if (func->findDecoration<IRDllExportDecoration>() ||
        func->findDecoration<IRExternCDecoration>() ||
        func->findDecoration<IRExternCppDecoration>())
    {
        return true;
    }

    return false;
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

    // At the moment, we should never see a bound version of
    // an intrinsic function, so this automatically implies
    // that this is not an intrinsic.
    //
    if (auto existentialSpecializedFunc = as<IRSpecializeExistentialsInFunc>(inst);
        existentialSpecializedFunc)
        return false;

    if (!func)
        return false;

    if (func->getFirstBlock() == nullptr)
        return true;

    return false;
}


// Returns true if the inst is of the form OptionalType<InterfaceType>
bool isOptionalExistentialType(IRInst* inst)
{
    if (auto optionalType = as<IROptionalType>(inst))
        if (auto interfaceType = as<IRInterfaceType>(optionalType->getValueType()))
            return !isComInterfaceType(interfaceType) && !isBuiltin(interfaceType);
    return false;
}

bool isExistentialDifferentialPairType(IRInst* inst)
{
    if (auto diffPairType = as<IRDifferentialPairTypeBase>(inst))
        if (auto interfaceType = as<IRInterfaceType>(diffPairType->getValueType()))
            return !isComInterfaceType(interfaceType) && !isBuiltin(interfaceType);
    return false;
}

// Returns true if the type is or transitively contains an interface type
// that would require dynamic dispatch (i.e., not a COM interface and not builtin).
// Note: does not currently unwrap IRSpecialize or IRPtrTypeBase; these are not
// expected in practice for GPU shader parameters.
static bool typeIncludesDynamicDispatch(IRType* type)
{
    if (auto interfaceType = as<IRInterfaceType>(type))
        return !isComInterfaceType(interfaceType) && !isBuiltin(interfaceType);

    if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            if (typeIncludesDynamicDispatch((IRType*)field->getFieldType()))
                return true;
        }
    }

    if (auto arrayType = as<IRArrayType>(type))
        return typeIncludesDynamicDispatch((IRType*)arrayType->getElementType());

    if (auto optionalType = as<IROptionalType>(type))
        return typeIncludesDynamicDispatch((IRType*)optionalType->getValueType());

    if (auto tupleType = as<IRTupleType>(type))
    {
        for (UInt i = 0; i < tupleType->getOperandCount(); i++)
        {
            if (typeIncludesDynamicDispatch((IRType*)tupleType->getOperand(i)))
                return true;
        }
    }

    return false;
}

// Parent context for the full type-flow pass.
struct TypeFlowSpecializationContext
{
    // Make a tagged-union-type out of a given set of tables.
    //
    // This type can be used for insts that are semantically a tuple of a tag (to select a table)
    // and a payload to contain the existential value.
    //
    IRTaggedUnionType* makeTaggedUnionType(IRWitnessTableSet* tableSet)
    {
        IRBuilder builder(module);
        HashSet<IRInst*> typeSet;

        // Create a type set out of the base types from each table.
        forEachInSet(
            module,
            tableSet,
            [&](IRInst* witnessTable)
            {
                switch (witnessTable->getOp())
                {
                case kIROp_UnboundedWitnessTableElement:
                    typeSet.add(builder.getUnboundedTypeElement(
                        as<IRUnboundedWitnessTableElement>(witnessTable)->getBaseInterfaceType()));
                    break;
                case kIROp_WitnessTable:
                    typeSet.add(as<IRWitnessTable>(witnessTable)->getConcreteType());
                    break;
                case kIROp_UninitializedWitnessTableElement:
                    typeSet.add(builder.getUninitializedTypeElement(
                        as<IRUninitializedWitnessTableElement>(witnessTable)
                            ->getBaseInterfaceType()));
                    break;
                case kIROp_NoneWitnessTableElement:
                    typeSet.add(builder.getNoneTypeElement());
                    break;
                }
            });

        // Create the tagged union type out of the type and table collection.
        return builder.getTaggedUnionType(
            tableSet,
            cast<IRTypeSet>(builder.getSet(kIROp_TypeSet, typeSet)));
    }

    // Check if a witness table set references a [Specialize]-only interface.
    // If so, emit error 52008 — the compiler has determined that dynamic dispatch
    // is needed, but the interface was explicitly marked for specialization only.
    //
    // Returns SLANG_FAIL if the interface is specialize-only (caller should bail out).
    //
    SlangResult rejectSpecializeOnlyInterface(IRWitnessTableSet* tableSet, SourceLoc callSiteLoc)
    {
        IRInst* conformanceType = nullptr;
        forEachInSet(
            module,
            tableSet,
            [&](IRInst* table)
            {
                if (conformanceType)
                    return;
                IRInst* ifaceType = nullptr;
                if (auto witnessTable = as<IRWitnessTable>(table))
                {
                    auto witnessTableType = as<IRWitnessTableType>(witnessTable->getDataType());
                    if (witnessTableType)
                        ifaceType = witnessTableType->getConformanceType();
                }
                else if (auto unbounded = as<IRUnboundedWitnessTableElement>(table))
                {
                    ifaceType = unbounded->getBaseInterfaceType();
                }
                else if (auto uninit = as<IRUninitializedWitnessTableElement>(table))
                {
                    ifaceType = uninit->getBaseInterfaceType();
                }
                if (ifaceType && ifaceType->findDecoration<IRSpecializeDecoration>())
                    conformanceType = ifaceType;
            });

        if (conformanceType)
        {
            sink->diagnose(Diagnostics::DynamicDispatchOnSpecializeOnlyInterface{
                .conformanceType = conformanceType,
                .location = callSiteLoc});
            return SLANG_FAIL;
        }
        return SLANG_OK;
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

    // Make an untagged-union type out of a given set of types.
    //
    // This is used to denote insts whose value can be of multiple possible types,
    // Note that unlike tagged-unions, untagged-unions do not have any information
    // on which type is currently held.
    //
    // Typically used as the type of the value part of existential objects.
    //
    IRUntaggedUnionType* makeUntaggedUnionType(IRTypeSet* typeSet)
    {
        IRBuilder builder(module);
        return builder.getUntaggedUnionType(typeSet);
    }

    // Make an element-of-set type out of a given set.
    //
    // ElementOfSetType can be used as the type of an inst whose
    // _value_ is known to be one of the elements of the set.
    //
    // e.g.
    //   if we have IR of the form:
    //   %1 : ElementOfSetType<Set<Int, Float>> = ExtractExistentialType(%existentialObj)
    //
    //   then %1 is Int or Float.
    //   (Note that this is different from saying %1's value is an int or a float)
    //
    IRElementOfSetType* makeElementOfSetType(IRSetBase* set)
    {
        IRBuilder builder(module);
        return builder.getElementOfSetType(set);
    }

    // Make a tag-type for a given set.
    //
    // `TagType(set)` is used to denote insts that are carrying an identifier for one of the
    // elements of the set.
    // These insts cannot be used directly as one of the values (must be used with `GetDispatcher`
    // or `GetElementFromTag`) before they can be used as values.
    //
    IRSetTagType* makeTagType(IRSetBase* set)
    {
        IRBuilder builder(module);
        return builder.getSetTagType(set);
    }

    IRInst* _tryGetInfo(InstWithContext element)
    {
        auto found = propagationMap.tryGetValue(element);
        if (found)
            return (*found)->getOperand(0);
        return none(); // Default info for any inst that we haven't registered.
    }

    // Find information for an inst that is being used as an argument
    // under a given context.
    //
    // This is a bit different from `tryGetInfo`, in that we want to
    // obtain an info that can be passed to a parameter (which can have multiple
    // possibilities stemming from different argument types)
    //
    // This key difference is that even if the argument has no propagated info because it
    // is not a dynamic or abstract type, it's possible that the parameter's type is
    // abstract.
    // Thus, we will construct an `UntaggedUnionType` of the concrete type so that it
    // can be union-ed with other argument infos.
    //
    // Such 'argument' cases arise for phi-args and function args.
    //
    // `paramType` provides the declared type of the merge-point (parameter, return type, etc.)
    // so that `makeInfoForConcreteType` can produce info in the correct structural shape.
    //
    IRInst* tryGetArgInfo(IRInst* context, IRInst* inst, IRInst* paramType)
    {
        if (auto info = tryGetInfo(context, inst))
            return info;

        if (isConcreteType(inst->getDataType()))
        {
            // If the inst has a concrete type, we can make a default info for it,
            // considering it as a singleton set. Note that this needs to be
            // nested into any relevant type structure that we want to propagate through
            // `makeInfoForConcreteType` handles this logic.
            //
            // When paramType is provided, the info is shaped to match the merge-point's
            // structural expectations.
            //
            return makeInfoForConcreteType(module, inst->getDataType(), paramType);
        }

        return none();
    }

    // Bottleneck method to fetch the current propagation info
    // for a given instruction under context.
    //
    IRInst* tryGetInfo(IRInst* context, IRInst* inst)
    {
        if (inst->getDataType())
        {
            // If the data-type is already a tagged union or untagged union or
            // element-of-set type, then the refinement occured during a previous phase.
            //
            // For now, we simply re-use that info directly.
            //
            // In the future, it makes sense to treat it as non-concrete and use
            // them as an upper-bound for further refinement.
            //
            switch (inst->getDataType()->getOp())
            {
            case kIROp_TaggedUnionType:
            case kIROp_UntaggedUnionType:
            case kIROp_ElementOfSetType:
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

    // Performs set-union over the two sets, and returns a new
    // inst to represent the set.
    //
    template<typename T>
    T* unionSet(T* set1, T* set2)
    {
        // It's possible to accelerate this further, but we usually
        // don't have to deal with overly large sets (usually 3-20 elements)
        //

        SLANG_ASSERT(as<IRSetBase>(set1) && as<IRSetBase>(set2));
        SLANG_ASSERT(set1->getOp() == set2->getOp());

        if (!set1)
            return set2;
        if (!set2)
            return set1;
        if (set1 == set2)
            return set1;

        HashSet<IRInst*> allValues;
        // Collect all values from both sets
        forEachInSet(module, set1, [&](IRInst* value) { allValues.add(value); });
        forEachInSet(module, set2, [&](IRInst* value) { allValues.add(value); });

        IRBuilder builder(module);
        return as<T>(builder.getSet(
            set1->getOp(),
            allValues)); // Create a new set with the union of values
    }

    // Performs a flat (non-structural) union of two propagation infos that are
    // both composites of sets (TaggedUnion, UntaggedUnion, ElementOfSet, SetTag).
    //
    // This is the leaf-level union used when the union mask is non-structural
    // and non-concrete (e.g., InterfaceType, WitnessTableType, FuncType).
    //
    IRInst* flatUnionPropagationInfo(IRInst* info1, IRInst* info2)
    {
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

        if (as<IROptionalType>(info1) && as<IROptionalNoneType>(info2))
            return info1;

        if (as<IROptionalNoneType>(info1) && (as<IROptionalType>(info2)))
            return info2;

        SLANG_UNEXPECTED("Incompatible propagation infos during union");
    }

    // Union-mask-aware union of two propagation infos using a three-input structural approach.
    //
    // `typeUnionMask` is the declared type of the merge point (e.g., parameter type, field type,
    // return type). It determines how much structural decomposition to perform during union:
    //
    // - If typeUnionMask is structural (Tuple, Array, Ptr, Optional, Attributed), and both infos
    //   match that structure, recurse element-wise using typeUnionMask's sub-types as sub-masks.
    //
    // - If typeUnionMask is concrete, both infos should represent the same thing; return info1.
    //
    // - If typeUnionMask is non-concrete and non-structural (e.g., InterfaceType, WitnessTableType,
    //   FuncType, LookupWitnessMethod result), perform a flat union without structural
    //   decomposition.
    //
    IRInst* unionPropagationInfo(IRInst* typeUnionMask, IRInst* info1, IRInst* info2)
    {
        // Basic cases: if either info is null, it is considered "empty";
        // if they're equal, union must be the same inst.
        if (!info1)
            return info2;
        if (!info2)
            return info1;
        if (areInfosEqual(info1, info2))
            return info1;

        // If union mask is concrete, infos should be equivalent representations
        // of the same concrete type. Return the existing info.
        if (isConcreteType(typeUnionMask))
            return info1;

        // --- Structural union masks ---
        // If typeUnionMask is structural and both infos match that structure,
        // recurse element-wise using typeUnionMask's sub-types as sub-masks.

        // Tuple
        if (auto tupleUnionMask = as<IRTupleType>(typeUnionMask))
        {
            auto tuple1 = as<IRTupleType>(info1);
            auto tuple2 = as<IRTupleType>(info2);

            IRBuilder builder(module);
            List<IRType*> elementInfos;
            for (UInt i = 0; i < tupleUnionMask->getOperandCount(); i++)
            {
                elementInfos.add((IRType*)unionPropagationInfo(
                    tupleUnionMask->getOperand(i),
                    tuple1->getOperand(i),
                    tuple2->getOperand(i)));
            }
            return builder.getTupleType(elementInfos);
        }

        // Array
        if (auto arrayUnionMask = as<IRArrayType>(typeUnionMask))
        {
            auto arr1 = as<IRArrayType>(info1);
            auto arr2 = as<IRArrayType>(info2);
            if (arr1 && arr2)
            {
                IRBuilder builder(module);
                return builder.getArrayType(
                    (IRType*)unionPropagationInfo(
                        arrayUnionMask->getElementType(),
                        arr1->getOperand(0),
                        arr2->getOperand(0)),
                    arr1->getElementCount(),
                    getArrayStride(arr1));
            }
        }

        // Ptr
        if (auto ptrUnionMask = as<IRPtrTypeBase>(typeUnionMask))
        {
            auto ptr1 = as<IRPtrTypeBase>(info1);
            auto ptr2 = as<IRPtrTypeBase>(info2);
            if (ptr1 && ptr2)
            {
                IRBuilder builder(module);
                return builder.getPtrTypeWithAddressSpace(
                    (IRType*)unionPropagationInfo(
                        ptrUnionMask->getValueType(),
                        ptr1->getOperand(0),
                        ptr2->getOperand(0)),
                    ptr1);
            }
        }

        // Optional (non-existential)
        if (as<IROptionalType>(typeUnionMask) && !isOptionalExistentialType(typeUnionMask))
        {
            auto optUnionMask = as<IROptionalType>(typeUnionMask);

            // Handle OptionalNone + Optional cases
            if (as<IROptionalType>(info1) && as<IROptionalNoneType>(info2))
                return info1;
            if (as<IROptionalNoneType>(info1) && as<IROptionalType>(info2))
                return info2;

            auto opt1 = as<IROptionalType>(info1);
            auto opt2 = as<IROptionalType>(info2);
            if (opt1 && opt2)
            {
                IRBuilder builder(module);
                return builder.getOptionalType((IRType*)unionPropagationInfo(
                    optUnionMask->getValueType(),
                    opt1->getOperand(0),
                    opt2->getOperand(0)));
            }
        }

        // AttributedType
        if (auto attrUnionMask = as<IRAttributedType>(typeUnionMask))
        {
            auto attr1 = as<IRAttributedType>(info1);
            auto attr2 = as<IRAttributedType>(info2);
            if (attr1 && attr2)
            {
                IRBuilder builder(module);
                List<IRAttr*>& attrs = *module->getContainerPool().getList<IRAttr>();
                for (auto attr : attr1->getAllAttrs())
                    attrs.add(attr);
                auto result = builder.getAttributedType(
                    (IRType*)unionPropagationInfo(
                        attrUnionMask->getBaseType(),
                        attr1->getOperand(0),
                        attr2->getOperand(0)),
                    attrs);
                module->getContainerPool().free(&attrs);
                return result;
            }
        }

        if (auto diffPairUnionMask = as<IRDifferentialPairType>(typeUnionMask))
        {
            auto attr1 = as<IRTupleType>(info1);
            auto attr2 = as<IRTupleType>(info2);
            if (attr1 && attr2)
            {
                IRBuilder builder(module);
                List<IRType*> elementInfos;

                elementInfos.add((IRType*)unionPropagationInfo(
                    diffPairUnionMask->getValueType(),
                    attr1->getOperand(0),
                    attr2->getOperand(0)));

                elementInfos.add((IRType*)unionPropagationInfo(
                    diffPairUnionMask->getValueType(),
                    attr1->getOperand(1),
                    attr2->getOperand(1)));

                return builder.getTupleType(elementInfos);
            }

            auto diffPair1 = as<IRDifferentialPairType>(info1);
            auto diffPair2 = as<IRDifferentialPairType>(info2);
            if (diffPair1 && diffPair2)
            {
                // Expect either a type or an ElementOfSetType.
                // and either a  getWitness()
            }
        }

        // --- Non-structural, non-concrete union masks ---
        // For any remaining case (InterfaceType, WitnessTableType, FuncType,
        // LookupWitnessMethod result, etc.), or when the structural match failed,
        // perform a flat union over the set-based composites.
        //
        return flatUnionPropagationInfo(info1, info2);
    }

    // Replace the propagation info for an instruction (no union with existing info).
    // Used at single-definition sites where there is exactly one incoming edge.
    //
    void updateInfo(IRInst* context, IRInst* inst, IRInst* newInfo, WorkQueue<WorkItem>& workQueue)
    {
        if (isConcreteType(inst->getDataType()))
            return;

        auto existingInfo = tryGetInfo(context, inst);
        if (areInfosEqual(existingInfo, newInfo))
            return;

        IRBuilder builder(module);
        propagationMap[InstWithContext(context, inst)] = builder.getWeakUse(newInfo);
        addUsersToWorkQueue(context, inst, newInfo, workQueue);
    }

    // Union new propagation info with existing info at a merge point, then update.
    // Used at sites with multiple incoming edges (phi params, func params, call returns,
    // struct fields, vars with multiple stores).
    //
    // `mergePointType` is the declared type of the merge point and controls
    // how much structural decomposition is performed during the union.
    //
    void updateInfoForMerge(
        IRInst* context,
        IRInst* inst,
        IRInst* newInfo,
        IRInst* mergePointType,
        WorkQueue<WorkItem>& workQueue)
    {
        if (isConcreteType(inst->getDataType()))
            return;

        auto existingInfo = tryGetInfo(context, inst);
        auto unionedInfo = unionPropagationInfo(mergePointType, existingInfo, newInfo);

        if (areInfosEqual(existingInfo, unionedInfo))
            return;

        IRBuilder builder(module);
        propagationMap[InstWithContext(context, inst)] = builder.getWeakUse(unionedInfo);
        addUsersToWorkQueue(context, inst, unionedInfo, workQueue);
    }

    // Helper method to add work items for all call sites of a function/generic.
    void addContextUsersToWorkQueue(IRInst* context, WorkQueue<WorkItem>& workQueue)
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
    void addUsersToWorkQueue(
        IRInst* context,
        IRInst* inst,
        IRInst* info,
        WorkQueue<WorkItem>& workQueue)
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

            // TODO: This is a tiny bit of a hack.. but we don't currently need to
            // analyze FuncType insts, but we do want to make sure that any changes
            // are propagated through to their users.
            //
            if (auto funcType = as<IRFuncType>(user))
                if (as<IRBlock>(funcType->getParent()))
                    addUsersToWorkQueue(context, funcType, none(), workQueue);
        }
    }

    // Helper method to update function's return info and propagate back to call sites
    void updateFuncReturnInfo(IRInst* callable, IRInst* returnInfo, WorkQueue<WorkItem>& workQueue)
    {
        // Don't update info if the callee has a concrete return type.
        IRInst* callableForType = callable;
        if (auto fwb = as<IRSpecializeExistentialsInFunc>(callable))
            callableForType = getFuncDefinitionForContext(fwb);
        auto callableFuncType = cast<IRFuncType>(callableForType->getDataType());
        if (isConcreteType(callableFuncType->getResultType()))
            return;

        auto existingReturnInfo = getFuncReturnInfo(callable);
        // Use the function's declared return type as union mask for structural union.
        auto newReturnInfo =
            unionPropagationInfo(callableFuncType->getResultType(), existingReturnInfo, returnInfo);

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

    bool isEntryPoint(IRFunc* func)
    {
        for (auto decoration : func->getDecorations())
        {
            switch (decoration->getOp())
            {
            case kIROp_PyExportDecoration:
            case kIROp_EntryPointDecoration:
            case kIROp_DllExportDecoration:
            case kIROp_HLSLExportDecoration:
            case kIROp_CudaDeviceExportDecoration:
            case kIROp_CudaKernelDecoration:
            case kIROp_ExternCDecoration:
            case kIROp_ExternCppDecoration:
                return true;
            default:
                break;
            }
        }
        return false;
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
        // sets and their composites) form a lattice. This is an order-theoretic
        // structure that implies that
        //      (i) each update moves us strictly 'upward', and
        //      (ii) that there are a finite number of possible states.
        //

        // Global worklist for interprocedural analysis.
        WorkQueue<WorkItem> workQueue;

        // Add all global functions to worklist. Functions that are invalid existential
        // specializations are skipped here; the diagnostic for them is emitted when
        // discoverContext encounters the corresponding IRSpecialize context or when
        // propagateToCallSite/specializeCall resolves the callee.
        //
        for (auto inst : module->getGlobalInsts())
            if (auto func = as<IRFunc>(inst))
                if (isEntryPoint(func) && !isInvalidExistentialSpecialization(func))
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

    IRInst* analyzeByType(IRInst* context, IRInst* inst)
    {
        if (!inst->getDataType())
            return none();

        if (auto dataTypeInfo = tryGetInfo(context, inst->getDataType()))
        {
            if (auto elementOfSetType =
                    as<IRElementOfSetType, IRDynamicCastBehavior::NoUnwrap>(dataTypeInfo))
            {
                return makeUntaggedUnionType(cast<IRTypeSet>(elementOfSetType->getSet()));
            }
        }

        return none();
    }

    void diagnoseUnsupportedBitCast(IRInst* inst)
    {
        auto resultType = inst->getDataType();
        if (!resultType || isConcreteType(resultType))
            return;

        if (diagnosedBitCasts.contains(inst))
            return;

        diagnosedBitCasts.add(inst);
        sink->diagnose(Diagnostics::BitCastToNonConcreteType{.location = inst->sourceLoc});
    }

    void resolveAndReplaceIfGlobal(IRInst* context, IRInst* inst)
    {
        SLANG_UNUSED(context);
        if (isGlobalInst(inst))
        {
            translationContext.resolveInst(inst);
        }
    }

    void processInstForPropagation(IRInst* context, IRInst* inst, WorkQueue<WorkItem>& workQueue)
    {
        IRInst* info = nullptr;

        if (inst->getDataType())
            resolveAndReplaceIfGlobal(context, inst->getDataType());

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
        case kIROp_BitCast:
            diagnoseUnsupportedBitCast(inst);
            return;
        case kIROp_Load:
        case kIROp_RWStructuredBufferLoad:
        case kIROp_StructuredBufferLoad:
            info = analyzeLoad(context, inst);
            break;
        case kIROp_LoadFromUninitializedMemory:
            info = analyzeLoadFromUninitializedMemory(context, inst);
            break;
        case kIROp_MakeStruct:
            info = analyzeMakeStruct(context, as<IRMakeStruct>(inst), workQueue);
            break;
        case kIROp_MakeArray:
            info = analyzeMakeArray(context, as<IRMakeArray>(inst));
            break;
        case kIROp_MakeArrayFromElement:
            info = analyzeMakeArrayFromElement(context, as<IRMakeArrayFromElement>(inst));
            break;
        case kIROp_MakeTuple:
            info = analyzeMakeTuple(context, inst);
            break;
        case kIROp_Store:
            info = analyzeStore(context, as<IRStore>(inst), workQueue);
            break;
        case kIROp_SwizzledStore:
            info = analyzeSwizzledStore(context, as<IRSwizzledStore>(inst), workQueue);
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
        case kIROp_GetElement:
            info = analyzeGetElement(context, as<IRGetElement>(inst));
            break;
        case kIROp_GetTupleElement:
            info = analyzeGetTupleElement(context, as<IRGetTupleElement>(inst));
            break;
        case kIROp_Swizzle:
            info = analyzeSwizzle(context, as<IRSwizzle>(inst));
            break;
        case kIROp_SwizzleSet:
            info = analyzeSwizzleSet(context, as<IRSwizzleSet>(inst));
            break;
        case kIROp_UpdateElement:
            info = analyzeUpdateElement(context, as<IRUpdateElement>(inst));
            break;
        case kIROp_MakeOptionalNone:
            info = analyzeMakeOptionalNone(context, as<IRMakeOptionalNone>(inst));
            break;
        case kIROp_MakeOptionalValue:
            info = analyzeMakeOptionalValue(context, as<IRMakeOptionalValue>(inst));
            break;
        case kIROp_GetOptionalValue:
            info = analyzeGetOptionalValue(context, as<IRGetOptionalValue>(inst));
            break;
        case kIROp_MakeDifferentialPair:
            info = analyzeMakeDifferentialPair(context, as<IRMakeDifferentialPair>(inst));
            break;
        case kIROp_DifferentialPairGetDifferential:
            info = analyzeDifferentialPairGetDifferential(
                context,
                as<IRDifferentialPairGetDifferential>(inst));
            break;
        case kIROp_DifferentialPairGetPrimal:
            info = analyzeDifferentialPairGetPrimal(context, as<IRDifferentialPairGetPrimal>(inst));
            break;
        case kIROp_AttributedType:
            info = analyzeAttributedType(context, as<IRAttributedType>(inst));
            break;
            // case kIROp_PtrType:
            //     info = analyzePtrType(context, as<IRPtrType>(inst));
            //     break;
        }

        // If we didn't get any info from inst-specific analysis, we'll try to get
        // info from the data-type's info.
        //
        if (!info)
            info = analyzeByType(context, inst);

        if (info)
            updateInfo(context, inst, info, workQueue);
    }

    void processBlock(IRInst* context, IRBlock* block, WorkQueue<WorkItem>& workQueue)
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
            if (!as<IRVoidType>(val->getDataType()))
            {
                // Get the function's declared return type as union mask for the arg info shape.
                auto func = getFuncDefinitionForContext(context);
                SLANG_ASSERT(func);
                IRType* returnType = cast<IRFuncType>(func->getDataType())->getResultType();
                updateFuncReturnInfo(context, tryGetArgInfo(context, val, returnType), workQueue);
            }
        }
    };

    void propagateWithinFuncEdge(IRInst* context, IREdge edge, WorkQueue<WorkItem>& workQueue)
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
                if (auto argInfo = tryGetArgInfo(context, arg, param->getDataType()))
                {
                    // Use merge update with phi param's type as union mask.
                    updateInfoForMerge(context, param, argInfo, param->getDataType(), workQueue);
                }
            }
            paramIndex++;
        }
    }

    // Match the AttributedType wrapping of `info` to match `targetType`.
    // If targetType has AttributedType wrapping but info doesn't, wrap info with the same
    // attributes. If info has AttributedType wrapping but targetType doesn't, strip it.
    //
    IRInst* matchAttributes(IRInst* info, IRType* targetType)
    {
        if (!info)
            return info;

        auto targetAttributed = as<IRAttributedType>(targetType);
        auto infoAttributed = as<IRAttributedType>(info);

        if (targetAttributed && !infoAttributed)
        {
            // Target has attributes but info doesn't - wrap info with the target's attributes.
            IRBuilder builder(module);
            List<IRAttr*>& attrs = *module->getContainerPool().getList<IRAttr>();
            for (auto attr : targetAttributed->getAllAttrs())
                attrs.add(attr);
            auto result = builder.getAttributedType((IRType*)info, attrs);
            module->getContainerPool().free(&attrs);
            return result;
        }

        if (!targetAttributed && infoAttributed)
        {
            // Info has attributes but target doesn't - strip them.
            return infoAttributed->getBaseType();
        }

        // Both have attributes or neither does - return as-is.
        return info;
    }

    void propagateInterproceduralEdge(InterproceduralEdge edge, WorkQueue<WorkItem>& workQueue)
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
                else if (
                    auto existentialSpecializedFunc =
                        as<IRSpecializeExistentialsInFunc>(targetCallee))
                {
                    auto baseFunc = getFuncDefinitionForContext(existentialSpecializedFunc);
                    if (baseFunc)
                        firstBlock = baseFunc->getFirstBlock();
                }

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

                        // Reject __ref and __constref parameters that involve
                        // interface types in dynamic dispatch contexts.
                        // These are incompatible with the tagged union representation.
                        //
                        if ((paramDirection.kind == ParameterDirectionInfo::Kind::Ref ||
                             paramDirection.kind == ParameterDirectionInfo::Kind::BorrowIn) &&
                            typeIncludesDynamicDispatch(paramType))
                        {
                            if (!diagnosedRefParams.contains(param))
                            {
                                diagnosedRefParams.add(param);
                                sink->diagnose(
                                    Diagnostics::RefParamWithInterfaceTypeInDynamicDispatch{
                                        .paramKind =
                                            paramDirection.kind == ParameterDirectionInfo::Kind::Ref
                                                ? String("__ref")
                                                : String("__constref"),
                                        .paramType = paramType,
                                        .location = param->sourceLoc});
                            }
                            argIndex++;
                            continue;
                        }

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

                        IRInst* argInfo =
                            tryGetArgInfo(edge.callerContext, arg, param->getDataType());

                        switch (paramDirection.kind)
                        {
                        case ParameterDirectionInfo::Kind::Out:
                        case ParameterDirectionInfo::Kind::BorrowInOut:
                        case ParameterDirectionInfo::Kind::BorrowIn:
                        case ParameterDirectionInfo::Kind::Ref:
                            {
                                IRBuilder builder(module);
                                if (!argInfo)
                                    break;

                                auto valueInfo = matchAttributes(
                                    as<IRPtrTypeBase>(argInfo)->getValueType(),
                                    paramType);
                                auto newInfo = fromDirectionAndType(
                                    &builder,
                                    paramDirection,
                                    (IRType*)valueInfo);
                                updateInfoForMerge(
                                    edge.targetContext,
                                    param,
                                    newInfo,
                                    param->getDataType(),
                                    workQueue);
                                break;
                            }
                        case ParameterDirectionInfo::Kind::In:
                            {
                                argInfo = matchAttributes(argInfo, paramType);
                                updateInfoForMerge(
                                    edge.targetContext,
                                    param,
                                    argInfo,
                                    param->getDataType(),
                                    workQueue);
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
                        IRInst* calleeForType = targetCallee;
                        if (auto fwb = as<IRSpecializeExistentialsInFunc>(targetCallee))
                            calleeForType = getFuncDefinitionForContext(fwb);
                        auto concreteReturnType =
                            cast<IRFuncType>(calleeForType->getDataType())->getResultType();
                        if (isConcreteType(concreteReturnType))
                        {
                            IRBuilder builder(module);
                            returnInfo = makeInfoForConcreteType(
                                module,
                                concreteReturnType,
                                callInst->getDataType());
                        }
                    }

                    if (returnInfo)
                    {
                        // Match attribute wrapping to the call site's return type.
                        returnInfo = matchAttributes(returnInfo, callInst->getDataType());
                        // Use merge update with call inst's type as context.
                        updateInfoForMerge(
                            edge.callerContext,
                            callInst,
                            returnInfo,
                            callInst->getDataType(),
                            workQueue);
                    }
                }

                auto callSiteFuncTypeCtx =
                    this->callSiteFuncType[InstWithContext(edge.callerContext, callInst)];

                // Also update infos of any out parameters
                auto paramInfos =
                    getParamInfos(edge.targetContext, maybeExpandFuncType(callSiteFuncTypeCtx));
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
                            auto valueInfo = matchAttributes(
                                as<IRPtrTypeBase>(paramInfo)->getValueType(),
                                argPtrType ? argPtrType->getValueType() : nullptr);
                            updateInfoForMerge(
                                edge.callerContext,
                                arg,
                                builder.getPtrTypeWithAddressSpace((IRType*)valueInfo, argPtrType),
                                arg->getDataType(),
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
            if (isComInterfaceType(interfaceType))
            {
                // If this is a COM interface, we ignore it.
                return none();
            }

            HashSet<IRInst*>& tables = *module->getContainerPool().getHashSet<IRInst>();
            collectExistentialTables(interfaceType, tables);
            if (tables.getCount() > 0)
            {
                auto resultTaggedUnionType = makeTaggedUnionType(
                    as<IRWitnessTableSet>(builder.getSet(kIROp_WitnessTableSet, tables)));
                module->getContainerPool().free(&tables);
                return resultTaggedUnionType;
            }
            else
            {
                StringBuilder typeStr;
                printDiagnosticArg(typeStr, interfaceType);
                sink->diagnose(Diagnostics::NoTypeConformancesFoundForInterface{
                    .interfaceType = typeStr.produceString(),
                    .location = inst->sourceLoc});
                module->getContainerPool().free(&tables);
                return none();
            }
        }

        return none();
    }

    IRInst* analyzeMakeExistential(IRInst* context, IRMakeExistential* inst)
    {
        IRBuilder builder(module);

        // If we're building an existential for a COM interface,
        // we always assume it is unbounded, since we can receive
        // types that we know nothing about in the current linkage.
        //
        if (isComInterfaceType(inst->getDataType()))
        {
            return none();
        }

        resolveAndReplaceIfGlobal(context, inst->getWitnessTable());
        auto witnessTable = inst->getWitnessTable();

        // Concrete case.
        if (as<IRWitnessTable>(witnessTable))
            return makeTaggedUnionType(as<IRWitnessTableSet>(
                builder.getSingletonSet(kIROp_WitnessTableSet, witnessTable)));

        // Get the witness table info
        auto witnessTableInfo = tryGetInfo(context, witnessTable);

        if (!witnessTableInfo)
            return none();

        if (auto elementOfSetType = as<IRElementOfSetType>(witnessTableInfo))
            return makeTaggedUnionType(cast<IRWitnessTableSet>(elementOfSetType->getSet()));

        SLANG_UNEXPECTED("Unexpected witness table info type in analyzeMakeExistential");
    }

    IRInst* analyzeMakeStruct(
        IRInst* context,
        IRMakeStruct* makeStruct,
        WorkQueue<WorkItem>& workQueue)
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
                // Use the field's declared type as context for structural union.
                auto newInfo =
                    unionPropagationInfo(field->getFieldType(), existingInfo, operandInfo);
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

    IRInst* analyzeMakeArray(IRInst* context, IRMakeArray* makeArray)
    {
        auto arrayType = as<IRArrayType>(makeArray->getDataType());

        // If array is concrete, no need to proceed.
        if (isConcreteType(arrayType))
            return none();

        //
        // OpMakeArray's effective type is the union of all element types used
        // to construct it.
        //

        IRInst* unionInfo = none();
        for (UInt i = 0; i < makeArray->getOperandCount(); i++)
        {
            auto element = makeArray->getOperand(i);
            if (auto elementInfo = tryGetInfo(context, element))
            {
                // Use the array's declared element type as context for structural union.
                unionInfo =
                    unionPropagationInfo(arrayType->getElementType(), unionInfo, elementInfo);
            }
            else
            {
                // If any element has no info, we can't proceed.
                return none();
            }
        }

        IRBuilder builder(module);
        return builder.getArrayType(
            (IRType*)unionInfo,
            arrayType->getElementCount(),
            getArrayStride(arrayType));
    }

    IRInst* analyzeMakeArrayFromElement(IRInst* context, IRMakeArrayFromElement* makeArray)
    {
        // MakeArrayFromElement creates an array where all elements have the same value.
        // We propagate the element's info to the array type.
        //
        auto arrayType = as<IRArrayType>(makeArray->getDataType());
        if (isConcreteType(arrayType))
            return none();

        auto element = makeArray->getOperand(0);
        auto elementInfo = tryGetInfo(context, element);
        if (!elementInfo)
            return none();

        IRBuilder builder(module);
        return builder.getArrayType(
            (IRType*)elementInfo,
            arrayType->getElementCount(),
            getArrayStride(arrayType));
    }

    IRInst* analyzeMakeTuple(IRInst* context, IRInst* makeTuple)
    {
        auto tupleType = as<IRTupleType>(makeTuple->getDataType());

        // If tuple is concrete, no need to proceed.
        if (isConcreteType(tupleType))
            return none();

        //
        // MakeTuple's effective type is a tuple of the element infos.
        // Each element can have different type info.
        //

        List<IRType*> elementInfos;
        for (UInt i = 0; i < makeTuple->getOperandCount(); i++)
        {
            auto element = makeTuple->getOperand(i);
            auto elementType = (IRType*)tupleType->getOperand(i);
            if (auto elementInfo = tryGetInfo(context, element))
            {
                elementInfos.add((IRType*)elementInfo);
            }
            else if (isConcreteType(elementType))
            {
                // For concrete element types, just use the type itself.
                elementInfos.add(elementType);
            }
            else
            {
                // If any non-concrete element has no info, we can't proceed.
                return none();
            }
        }

        IRBuilder builder(module);
        return builder.getTupleType(elementInfos);
    }

    IRInst* analyzeGetTupleElement(IRInst* context, IRGetTupleElement* getTupleElement)
    {
        // If the base info is a tuple type, we can return the element info at the specified index.
        //

        auto baseInfo = tryGetInfo(context, getTupleElement->getTuple());
        if (!baseInfo)
            return none();

        if (auto tupleInfo = as<IRTupleType>(baseInfo))
        {
            auto elementIndex = getTupleElement->getElementIndex();
            if (auto intLit = as<IRIntLit>(elementIndex))
            {
                auto index = intLit->getValue();
                if (index >= 0 && (UInt)index < tupleInfo->getOperandCount())
                {
                    return tupleInfo->getOperand((UInt)index);
                }
            }
        }

        return none();
    }

    IRInst* analyzeLoadFromUninitializedMemory(IRInst* context, IRInst* inst)
    {
        SLANG_UNUSED(context);
        IRBuilder builder(module);
        if (as<IRInterfaceType>(inst->getDataType()) && !isConcreteType(inst->getDataType()))
        {
            auto uninitializedSet = builder.getSingletonSet(
                kIROp_WitnessTableSet,
                builder.getUninitializedWitnessTableElement(inst->getDataType()));
            return makeTaggedUnionType(as<IRWitnessTableSet>(uninitializedSet));
        }

        return none();
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
            auto findGlobalTables = [&](IRInterfaceType* interfaceType) -> IRInst*
            {
                HashSet<IRInst*>& tables = *module->getContainerPool().getHashSet<IRInst>();
                collectExistentialTables(interfaceType, tables);
                if (tables.getCount() > 0)
                {
                    auto resultTaggedUnionType = makeTaggedUnionType(
                        as<IRWitnessTableSet>(builder.getSet(kIROp_WitnessTableSet, tables)));
                    module->getContainerPool().free(&tables);
                    return resultTaggedUnionType;
                }
                else
                {
                    StringBuilder typeStr;
                    printDiagnosticArg(typeStr, interfaceType);
                    sink->diagnose(Diagnostics::NoTypeConformancesFoundForInterface{
                        .interfaceType = typeStr.produceString(),
                        .location = loadInst->sourceLoc});
                    module->getContainerPool().free(&tables);
                    return none();
                }
            };

            if (isResourcePointer(loadInst->getPtr()))
            {
                if (auto interfaceType = as<IRInterfaceType>(loadInst->getDataType()))
                {
                    if (!isComInterfaceType(interfaceType))
                    {
                        return findGlobalTables(interfaceType);
                    }
                    else
                    {
                        return none();
                    }
                }
                else if (
                    auto boundInterfaceType = as<IRBoundInterfaceType>(loadInst->getDataType()))
                {
                    return makeTaggedUnionType(cast<IRWitnessTableSet>(builder.getSingletonSet(
                        kIROp_WitnessTableSet,
                        boundInterfaceType->getWitnessTable())));
                }
                else
                {
                    // Loading from a resource pointer that isn't a direct interface?s
                    // Just return no info.
                    return none();
                }
            }

            // If the load is from a pointer, we can transfer the info directly
            auto address = as<IRLoad>(loadInst)->getPtr();
            if (auto addrInfo = tryGetInfo(context, address))
            {
                auto valueInfo = as<IRPtrTypeBase>(addrInfo)->getValueType();
                return valueInfo;
            }

            // If there is no type flow info for the address but the loaded type
            // is an interface, enumerate all conforming types globally.
            // This handles groupshared global variables (and arrays thereof)
            // with interface types, where the address traces back to a module-scope
            // global variable that type flow analysis cannot track.
            if (auto interfaceType = as<IRInterfaceType>(loadInst->getDataType()))
            {
                if (!isComInterfaceType(interfaceType))
                {
                    return findGlobalTables(interfaceType);
                }
            }

            return none();
        }
        else if (as<IRRWStructuredBufferLoad>(inst) || as<IRStructuredBufferLoad>(inst))
        {
            // In case of a buffer load, we know we're dealing with a location that cannot
            // be specialized, so the logic is similar to case (1) from above.
            //
            if (auto interfaceType = as<IRInterfaceType>(inst->getDataType()))
            {
                if (!isComInterfaceType(interfaceType))
                {
                    HashSet<IRInst*>& tables = *module->getContainerPool().getHashSet<IRInst>();
                    collectExistentialTables(interfaceType, tables);
                    if (tables.getCount() > 0)
                    {
                        auto resultTaggedUnionType = makeTaggedUnionType(
                            as<IRWitnessTableSet>(builder.getSet(kIROp_WitnessTableSet, tables)));
                        module->getContainerPool().free(&tables);
                        return resultTaggedUnionType;
                    }
                    else
                    {
                        StringBuilder typeStr;
                        printDiagnosticArg(typeStr, interfaceType);
                        sink->diagnose(Diagnostics::NoTypeConformancesFoundForInterface{
                            .interfaceType = typeStr.produceString(),
                            .location = inst->sourceLoc});
                        module->getContainerPool().free(&tables);
                        return none();
                    }
                }
                else
                {
                    return none();
                }
            }
            else if (auto boundInterfaceType = as<IRBoundInterfaceType>(inst->getDataType()))
            {
                return makeTaggedUnionType(cast<IRWitnessTableSet>(builder.getSingletonSet(
                    kIROp_WitnessTableSet,
                    boundInterfaceType->getWitnessTable())));
            }
        }

        return none(); // No info for other load types
    }

    IRInst* analyzeStore(IRInst* context, IRStore* storeInst, WorkQueue<WorkItem>& workQueue)
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

    IRInst* analyzeSwizzledStore(
        IRInst* context,
        IRSwizzledStore* swizzledStore,
        WorkQueue<WorkItem>& workQueue)
    {
        // SwizzledStore stores elements from source into specific indices of the dest pointer.
        // Similar to Store, we need to propagate information to the destination.
        //
        auto dest = swizzledStore->getDest();
        auto source = swizzledStore->getSource();
        auto destPtrType = as<IRPtrTypeBase>(dest->getDataType());
        if (!destPtrType)
            return none();

        auto destValueType = as<IRTupleType>(destPtrType->getValueType());
        if (!destValueType)
            return none();

        auto elementCount = (Index)destValueType->getOperandCount();

        // Get current info for the destination (if any)
        auto destInfo = tryGetInfo(context, dest);
        auto destPtrInfo = as<IRPtrTypeBase>(destInfo);
        auto destTupleInfo = destPtrInfo ? as<IRTupleType>(destPtrInfo->getValueType()) : nullptr;

        // Build new tuple info starting from existing dest info or the type
        List<IRType*> elementInfos;
        for (Index i = 0; i < elementCount; i++)
        {
            auto elemType = destValueType->getOperand(i);
            if (destTupleInfo)
            {
                elementInfos.add((IRType*)destTupleInfo->getOperand(i));
            }
            else if (isConcreteType(elemType))
            {
                elementInfos.add((IRType*)elemType);
            }
            else
            {
                return none();
            }
        }

        // Update elements at the swizzle indices with infos from the source
        auto sourceInfo = tryGetInfo(context, source);
        auto sourceTupleInfo = as<IRTupleType>(sourceInfo);
        auto sourceType = as<IRTupleType>(source->getDataType());

        for (UInt i = 0; i < swizzledStore->getElementCount(); i++)
        {
            auto elementIndex = swizzledStore->getElementIndex(i);
            if (auto intLit = as<IRIntLit>(elementIndex))
            {
                auto destIndex = (Index)intLit->getValue();
                if (destIndex < 0 || destIndex >= elementCount)
                    return none();

                // Get the source element's info
                IRInst* sourceElemInfo = nullptr;
                if (sourceTupleInfo)
                {
                    sourceElemInfo = sourceTupleInfo->getOperand(i);
                }
                else if (sourceInfo)
                {
                    // Source is a single value (not a tuple)
                    sourceElemInfo = sourceInfo;
                }
                else if (sourceType)
                {
                    auto sourceElemType = sourceType->getOperand(i);
                    if (isConcreteType(sourceElemType))
                        sourceElemInfo = sourceElemType;
                    else
                        return none();
                }
                else
                {
                    // Single value source with concrete type
                    if (isConcreteType(source->getDataType()))
                        sourceElemInfo = source->getDataType();
                    else
                        return none();
                }

                elementInfos[destIndex] = (IRType*)sourceElemInfo;
            }
            else
            {
                return none();
            }
        }

        // Construct the new pointer info and propagate to the destination
        IRBuilder builder(module);
        auto newTupleInfo = builder.getTupleType(elementInfos);
        auto ptrInfo = builder.getPtrTypeWithAddressSpace((IRType*)newTupleInfo, destPtrType);

        // Propagate the information up the access chain to the base location.
        maybeUpdateInfoForAddress(context, dest, ptrInfo, workQueue);

        // The store inst itself doesn't produce anything, so it has no info
        return none();
    }

    IRInst* analyzeGetElementPtr(IRInst* context, IRGetElementPtr* getElementPtr)
    {
        // The base info should be in Ptr<Array<T>> or Ptr<Tuple<...>> form,
        // so we just need to unpack and return Ptr<ElementType> as the result.
        //
        IRBuilder builder(module);
        builder.setInsertAfter(getElementPtr);
        auto basePtr = getElementPtr->getBase();
        if (auto ptrType = as<IRPtrTypeBase>(tryGetInfo(context, basePtr)))
        {
            if (auto arrayType = as<IRArrayType>(ptrType->getValueType()))
            {
                return builder.getPtrTypeWithAddressSpace(arrayType->getElementType(), ptrType);
            }

            if (auto tupleType = as<IRTupleType>(ptrType->getValueType()))
            {
                auto elementIndex = getElementPtr->getIndex();
                if (auto intLit = as<IRIntLit>(elementIndex))
                {
                    auto index = intLit->getValue();
                    if (index >= 0 && (UInt)index < tupleType->getOperandCount())
                    {
                        return builder.getPtrTypeWithAddressSpace(
                            (IRType*)tupleType->getOperand((UInt)index),
                            ptrType);
                    }
                }
            }
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

                // When accessing an interface-typed field and no fieldInfo has been
                // propagated, fall back to enumerating global witness tables.
                // See the analogous logic in analyzeLoad for direct interface
                // loads from resource pointers.
                if (auto interfaceType = as<IRInterfaceType>(structField->getFieldType()))
                {
                    if (!isComInterfaceType(interfaceType))
                    {
                        HashSet<IRInst*>& tables = *module->getContainerPool().getHashSet<IRInst>();
                        collectExistentialTables(interfaceType, tables);
                        if (tables.getCount() > 0)
                        {
                            auto valueInfo = makeTaggedUnionType(as<IRWitnessTableSet>(
                                builder.getSet(kIROp_WitnessTableSet, tables)));
                            module->getContainerPool().free(&tables);
                            return builder.getPtrTypeWithAddressSpace(
                                (IRType*)valueInfo,
                                as<IRPtrTypeBase>(fieldAddress->getDataType()));
                        }
                        module->getContainerPool().free(&tables);
                    }
                }
                else if (
                    auto boundInterfaceType = as<IRBoundInterfaceType>(structField->getFieldType()))
                {
                    auto valueInfo =
                        makeTaggedUnionType(cast<IRWitnessTableSet>(builder.getSingletonSet(
                            kIROp_WitnessTableSet,
                            boundInterfaceType->getWitnessTable())));
                    return builder.getPtrTypeWithAddressSpace(
                        (IRType*)valueInfo,
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

            // When extracting an interface-typed field from a struct and no fieldInfo
            // has been propagated (e.g. the struct was loaded from a constant buffer
            // rather than constructed via MakeStruct), fall back to enumerating all
            // globally available witness tables for the interface. This mirrors the
            // logic in analyzeLoad for direct interface loads from resource pointers.
            if (auto interfaceType = as<IRInterfaceType>(structField->getFieldType()))
            {
                if (!isComInterfaceType(interfaceType))
                {
                    HashSet<IRInst*>& tables = *module->getContainerPool().getHashSet<IRInst>();
                    collectExistentialTables(interfaceType, tables);
                    if (tables.getCount() > 0)
                    {
                        auto result = makeTaggedUnionType(
                            as<IRWitnessTableSet>(builder.getSet(kIROp_WitnessTableSet, tables)));
                        module->getContainerPool().free(&tables);
                        return result;
                    }
                    module->getContainerPool().free(&tables);
                }
            }
            else if (
                auto boundInterfaceType = as<IRBoundInterfaceType>(structField->getFieldType()))
            {
                return makeTaggedUnionType(cast<IRWitnessTableSet>(builder.getSingletonSet(
                    kIROp_WitnessTableSet,
                    boundInterfaceType->getWitnessTable())));
            }
        }
        return none();
    }

    IRInst* analyzeAttributedType(IRInst* context, IRAttributedType* inst)
    {
        // An AttributedType wraps a base type with attributes (e.g., unorm, snorm).
        // Since analysis is only used on instructions in blocks, we're seeing
        //
        // this attributed type in a block, and it is by default specialized.
        //
        // We need to propagate the info from the base type
        // wrapped in an attributed type.
        //
        auto baseType = inst->getBaseType();
        if (auto baseInfo = tryGetInfo(context, baseType))
        {
            IRBuilder builder(module);
            List<IRAttr*>& attrs = *module->getContainerPool().getList<IRAttr>();
            for (auto attr : inst->getAllAttrs())
                attrs.add(attr);
            auto result = builder.getAttributedType((IRType*)baseInfo, attrs);
            module->getContainerPool().free(&attrs);
            return result;
        }
        return none();
    }

    IRInst* analyzeTupleType(IRInst* context, IRTupleType* inst)
    {
        // A TupleType in a block context may have non-concrete element types.
        // We need to propagate the info from each element type,
        // constructing a new tuple type with the element infos.
        //
        bool anyInfo = false;
        List<IRType*> elementInfos;
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            auto elementType = inst->getOperand(i);
            if (auto elemInfo = tryGetInfo(context, elementType))
            {
                elementInfos.add((IRType*)elemInfo);
                anyInfo = true;
            }
            else
            {
                elementInfos.add((IRType*)elementType);
            }
        }

        if (!anyInfo)
            return none();

        IRBuilder builder(module);
        return builder.getTupleType(elementInfos);
    }

    IRInst* analyzePtrType(IRInst* context, IRPtrType* inst)
    {
        // A PtrType wraps a value type. If the value type is non-concrete,
        // we need to propagate the info from the value type wrapped in a pointer type.
        //
        auto valueType = inst->getValueType();
        if (auto valueInfo = tryGetInfo(context, valueType))
        {
            IRBuilder builder(module);
            return makeElementOfSetType(builder.getSingletonSet(
                kIROp_TypeSet,
                builder.getPtrTypeWithAddressSpace((IRType*)valueInfo, inst)));
        }
        return none();
    }

    IRInst* analyzeMakeDifferentialPair(IRInst* context, IRMakeDifferentialPair* inst)
    {
        SLANG_UNUSED(context);

        if (isExistentialDifferentialPairType(inst->getDataType()))
        {
            // DifferentialPair<IFoo>
            IRBuilder builder(module);

            auto primalInfo = as<IRTaggedUnionType>(tryGetInfo(context, inst->getPrimal()));
            if (!primalInfo)
                return none();

            auto diffInfo = as<IRTaggedUnionType>(tryGetInfo(context, inst->getDifferential()));
            if (!diffInfo)
                return none();

            return builder.getTupleType(List<IRType*>({primalInfo, diffInfo}));
        }
        else if (!isGlobalInst(inst->getDataType()))
        {
            // DifferentialPair<T> where T is generic/abstract
            IRBuilder builder(module);
            auto primalInfo = as<IRUntaggedUnionType>(tryGetInfo(context, inst->getPrimal()));
            if (!primalInfo)
                return none();
            auto diffInfo = as<IRUntaggedUnionType>(tryGetInfo(context, inst->getDifferential()));
            if (!diffInfo)
                return none();

            if (primalInfo->getSet()->isSingleton() && diffInfo->getSet()->isSingleton())
                return none();

            return builder.getTupleType(List<IRType*>({primalInfo, diffInfo}));
        }
        else
        {
            // DifferentialPair<ConcreteType> - concrete type.
            return none();
        }
    }

    IRInst* analyzeDifferentialPairGetPrimal(IRInst* context, IRDifferentialPairGetPrimal* inst)
    {
        SLANG_UNUSED(context);
        IRBuilder builder(module);
        if (auto pairInfo = tryGetInfo(context, inst->getOperand(0)))
        {
            if (auto pairType = as<IRTupleType>(pairInfo))
            {
                return pairType->getOperand(0);
            }
        }

        return none();
    }

    IRInst* analyzeDifferentialPairGetDifferential(
        IRInst* context,
        IRDifferentialPairGetDifferential* inst)
    {
        SLANG_UNUSED(context);
        IRBuilder builder(module);
        if (auto pairInfo = tryGetInfo(context, inst->getOperand(0)))
        {
            if (auto pairType = as<IRTupleType>(pairInfo))
            {
                return pairType->getOperand(1);
            }
        }

        return none();
    }
    IRInst* analyzeGetElement(IRInst* context, IRGetElement* getElement)
    {
        // If the base info is an array of some type, we can return that element type.
        // as the info for the get-element inst.
        //

        IRBuilder builder(module);

        auto baseInfo = tryGetInfo(context, getElement->getBase());
        if (!baseInfo)
            return none();

        if (auto arrayInfo = as<IRArrayType>(baseInfo))
        {
            return arrayInfo->getElementType();
        }

        return none();
    }

    IRInst* analyzeSwizzle(IRInst* context, IRSwizzle* swizzle)
    {
        // For swizzle on tuples, we extract the element infos at the specified indices
        // and create a new tuple type with those infos.
        //
        auto baseInfo = tryGetInfo(context, swizzle->getBase());
        if (!baseInfo)
            return none();

        auto tupleInfo = as<IRTupleType>(baseInfo);
        if (!tupleInfo)
            return none();

        // If only one element, return just that element's info
        if (swizzle->getElementCount() == 1)
        {
            auto elementIndex = swizzle->getElementIndex(0);
            if (auto intLit = as<IRIntLit>(elementIndex))
            {
                auto index = intLit->getValue();
                if (index >= 0 && (UInt)index < tupleInfo->getOperandCount())
                {
                    return tupleInfo->getOperand((UInt)index);
                }
            }
            return none();
        }

        // Multiple elements - build a tuple of the element infos
        List<IRType*> elementInfos;
        for (UInt i = 0; i < swizzle->getElementCount(); i++)
        {
            auto elementIndex = swizzle->getElementIndex(i);
            if (auto intLit = as<IRIntLit>(elementIndex))
            {
                auto index = intLit->getValue();
                if (index >= 0 && (UInt)index < tupleInfo->getOperandCount())
                {
                    elementInfos.add((IRType*)tupleInfo->getOperand((UInt)index));
                }
                else
                {
                    return none();
                }
            }
            else
            {
                return none();
            }
        }

        IRBuilder builder(module);
        return builder.getTupleType(elementInfos);
    }

    IRInst* analyzeSwizzleSet(IRInst* context, IRSwizzleSet* swizzleSet)
    {
        // SwizzleSet takes a base tuple and a source, and returns a new tuple with
        // some elements replaced at specified indices.
        //
        auto base = swizzleSet->getBase();
        auto source = swizzleSet->getSource();
        auto baseType = as<IRTupleType>(base->getDataType());
        if (!baseType)
            return none();

        auto elementCount = (Index)baseType->getOperandCount();

        // Start with the base tuple's element infos
        List<IRType*> elementInfos;
        auto baseInfo = tryGetInfo(context, base);
        auto baseTupleInfo = as<IRTupleType>(baseInfo);

        for (Index i = 0; i < elementCount; i++)
        {
            auto elemType = baseType->getOperand(i);
            if (baseTupleInfo)
            {
                elementInfos.add((IRType*)baseTupleInfo->getOperand(i));
            }
            else if (isConcreteType(elemType))
            {
                elementInfos.add((IRType*)elemType);
            }
            else
            {
                return none();
            }
        }

        // Now replace elements at the swizzle indices with infos from the source
        auto sourceInfo = tryGetInfo(context, source);
        auto sourceTupleInfo = as<IRTupleType>(sourceInfo);
        auto sourceTupleType = as<IRTupleType>(source->getDataType());

        for (UInt i = 0; i < swizzleSet->getElementCount(); i++)
        {
            auto elementIndex = swizzleSet->getElementIndex(i);
            if (auto intLit = as<IRIntLit>(elementIndex))
            {
                auto baseIndex = (Index)intLit->getValue();
                if (baseIndex < 0 || baseIndex >= elementCount)
                    return none();

                // Get the source element's info
                IRInst* sourceElemInfo = nullptr;
                if (sourceTupleInfo)
                {
                    sourceElemInfo = sourceTupleInfo->getOperand(i);
                }
                else if (sourceInfo)
                {
                    // Source is a single value (not a tuple)
                    sourceElemInfo = sourceInfo;
                }
                else if (sourceTupleType)
                {
                    auto sourceElemType = sourceTupleType->getOperand(i);
                    if (isConcreteType(sourceElemType))
                        sourceElemInfo = sourceElemType;
                    else
                        return none(); // If a non-concrete element type has no info, we can't
                                       // proceed
                }
                else
                {
                    // Single value source with concrete type
                    if (isConcreteType(source->getDataType()))
                        sourceElemInfo = source->getDataType();
                    else
                        return none(); // If a non-concrete element type has no info, we can't
                                       // proceed
                }

                elementInfos[baseIndex] = (IRType*)sourceElemInfo;
            }
            else
            {
                return none();
            }
        }

        IRBuilder builder(module);
        return builder.getTupleType(elementInfos);
    }

    IRInst* analyzeUpdateElement(IRInst* context, IRUpdateElement* updateElement)
    {
        // UpdateElement replaces an element in a composite type (tuple or array) at a given index.
        // We need to construct a new info where the element at the specified index has its info
        // replaced with the new value's info.
        //
        auto oldValue = updateElement->getOldValue();
        auto elementValue = updateElement->getElementValue();
        auto oldValueType = oldValue->getDataType();

        // Get the index - UpdateElement can have multiple access keys for nested access,
        // but we only handle the single-index case for now.
        if (updateElement->getAccessKeyCount() != 1)
            return none();

        auto accessKey = updateElement->getAccessKey(0);
        auto intLit = as<IRIntLit>(accessKey);
        if (!intLit)
            return none();

        auto index = (Index)intLit->getValue();

        if (auto tupleType = as<IRTupleType>(oldValueType))
        {
            // For tuples, construct a new tuple info with the element at index replaced.
            auto elementCount = (Index)tupleType->getOperandCount();
            if (index < 0 || index >= elementCount)
                return none();

            List<IRType*> elementInfos;
            for (Index i = 0; i < elementCount; i++)
            {
                if (i == index)
                {
                    // Use the new element's info
                    auto newElemInfo = tryGetInfo(context, elementValue);
                    auto elemType = tupleType->getOperand(i);
                    if (!newElemInfo)
                    {
                        if (isConcreteType(elemType))
                            newElemInfo = elemType;
                        else
                            return none();
                    }
                    elementInfos.add((IRType*)newElemInfo);
                }
                else
                {
                    // Use the old value's element info
                    auto oldValueInfo = tryGetInfo(context, oldValue);
                    auto elemType = tupleType->getOperand(i);
                    if (auto oldTupleInfo = as<IRTupleType>(oldValueInfo))
                    {
                        elementInfos.add((IRType*)oldTupleInfo->getOperand(i));
                    }
                    else if (isConcreteType(elemType))
                    {
                        elementInfos.add((IRType*)elemType);
                    }
                    else
                    {
                        return none();
                    }
                }
            }

            IRBuilder builder(module);
            return builder.getTupleType(elementInfos);
        }
        else if (as<IRArrayType>(oldValueType))
        {
            // For arrays, we can't track per-element info, so just return the old value's info.
            auto oldValueInfo = tryGetInfo(context, oldValue);
            if (oldValueInfo)
                return oldValueInfo;
        }

        return none();
    }

    // Get the witness table inst to be used for the 'none' case of
    // an optional witness table.
    //
    IRInst* getNoneWitness()
    {
        IRBuilder builder(module);
        return builder.getNoneWitnessTableElement();
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

        // For non-existential optional types with non-concrete value types,
        // we propagate the optional wrapper with an appropriate value info.
        auto optionalType = as<IROptionalType>(inst->getDataType());
        if (optionalType && !isConcreteType(optionalType->getValueType()))
        {
            // Return an optional info wrapping the value type's info.
            // Since this is 'none', we use the concrete value type info.
            return builder.getOptionalNoneType();
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
        // Note: we don't actually have to add a new 'none' table to the set, since that will
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

            return none();
        }

        // For non-existential optional types with non-concrete value types,
        // we wrap the value's info in an optional info.
        auto optionalType = as<IROptionalType>(inst->getDataType());
        if (optionalType && !isConcreteType(optionalType->getValueType()))
        {
            IRBuilder builder(module);
            if (auto valueInfo = tryGetInfo(context, inst->getValue()))
            {
                return builder.getOptionalType((IRType*)valueInfo);
            }
            else
            {
                return none();
            }
        }

        return none();
    }

    IRInst* analyzeGetOptionalValue(IRInst* context, IRGetOptionalValue* inst)
    {
        if (isOptionalExistentialType(inst->getOperand(0)->getDataType()))
        {
            // TODO: Document.
            if (auto info = tryGetInfo(context, inst->getOperand(0)))
            {
                SLANG_ASSERT(as<IRTaggedUnionType>(info));

                IRBuilder builder(module);
                auto taggedUnion = as<IRTaggedUnionType>(info);
                return builder.getTaggedUnionType(
                    cast<IRWitnessTableSet>(filterNoneElements(taggedUnion->getWitnessTableSet())),
                    cast<IRTypeSet>(filterNoneElements(taggedUnion->getTypeSet())));
            }

            return none();
        }

        // For non-existential optional types with non-concrete value types,
        // we extract the value's info from the optional info.
        auto optionalType = as<IROptionalType>(inst->getOperand(0)->getDataType());
        if (optionalType && !isConcreteType(optionalType->getValueType()))
        {
            if (auto info = tryGetInfo(context, inst->getOperand(0)))
            {
                if (auto optionalInfo = as<IROptionalType>(info))
                {
                    return optionalInfo->getValueType();
                }
            }
        }

        return none();
    }

    IRInst* analyzeLookupWitnessMethod(IRInst* context, IRLookupWitnessMethod* inst)
    {
        // A LookupWitnessMethod is assumed to by dynamic, so we
        // (i) construct a set of the results by looking up the given
        //     key in each of the input witness tables
        // (ii) wrap the result in a tag type, since the lookup inst is logically holding
        //     on to run-time information about which element of the set is active.
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
            HashSet<IRInst*>& results = *module->getContainerPool().getHashSet<IRInst>();
            forEachInSet(
                module,
                cast<IRWitnessTableSet>(elementOfSetType->getSet()),
                [&](IRInst* table)
                {
                    if (as<IRUnboundedWitnessTableElement>(table))
                    {
                        if (inst->getDataType()->getOp() == kIROp_FuncType)
                            results.add(builder.getUnboundedFuncElement());
                        else if (inst->getDataType()->getOp() == kIROp_WitnessTableType)
                            results.add(builder.getUnboundedWitnessTableElement(
                                as<IRWitnessTableType>(inst->getDataType())->getConformanceType()));
                        else if (inst->getDataType()->getOp() == kIROp_TypeKind)
                        {
                            SLANG_UNEXPECTED(
                                "TypeKind result from LookupWitnessMethod not supported");
                        }
                        return;
                    }

                    auto resolvedTable = translationContext.resolveInst(table);

                    // TODO: Make 'none witness' values a proper thing instead of three different
                    // possibilities..
                    //
                    if (as<IRNoneWitnessTableElement>(table) || as<IRVoidLit>(table) ||
                        (as<IRWitnessTable>(resolvedTable)->getConformanceType()->getOp() ==
                         kIROp_VoidType))
                    {
                        results.add(builder.getVoidValue());
                        return;
                    }

                    results.add(findWitnessTableEntry(cast<IRWitnessTable>(resolvedTable), key));
                });

            auto setOp = getSetOpFromType(inst->getDataType());
            auto resultSetType = makeElementOfSetType(builder.getSet(setOp, results));
            module->getContainerPool().free(&results);

            return resultSetType;
        }

        if (!witnessTableInfo)
            return none();

        SLANG_UNEXPECTED("Unexpected witness table info type in analyzeLookupWitnessMethod");
    }

    // After specialization has lowered every dispatch site it could, walk any
    // remaining `lookupWitnessMethod` insts whose witness-table operand is not
    // a concrete `IRWitnessTable` and whose interface has no registered
    // conformances.  Those insts will ICE in codegen ("Unhandled local inst
    // lookupWitness") with no indication that the real problem is missing
    // conformance registration.  Emit E50100 instead.  Catches transitively-
    // nested cases (e.g. interface field inside a struct loaded from a
    // `StructuredBuffer`) that the load-site check in `analyzeLoad` misses —
    // see #9445.
    //
    // Skips lookups that have no uses: those are dead code that the
    // downstream DCE pass will eliminate before codegen, so they cannot
    // ICE. Imported library modules (e.g. slangpy's `sgl/device/print`)
    // routinely produce such dead lookups inside generic / variadic helper
    // functions whose `IPrintable arg` overload is never reached from the
    // entry point that imports them. Diagnosing those was a regression
    // against pre-PR behaviour where DCE simply removed them.
    void diagnoseUnresolvedLookupWitnesses()
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            auto func = as<IRFunc>(globalInst);
            if (!func)
                continue;
            // Only diagnose lookups inside top-level functions
            // that codegen actually emits as standalone callable
            // bodies — shader entry points plus the various
            // export decorations (CUDA kernels, DLL exports,
            // extern-C, etc.). Those are the bodies that survive
            // into codegen unchanged, so any unresolved lookup
            // there will reach the unhandled-inst ICE the walker
            // exists to prevent.
            //
            // Use the same `isEntryPoint(func)` predicate that
            // `performDynamicInstLowering` uses to seed its work-
            // list, so the walker's diagnostic coverage matches
            // the lowering pass's coverage exactly.
            //
            // For non-entry-point helper functions, the typeflow
            // pass cannot tell whether the helper is reachable
            // (callers may exist but be themselves dead) or whether
            // the helper is going to be inlined / DCE'd before
            // codegen. Diagnosing those is a false positive — the
            // canonical example is imported-library helpers like
            // slangpy's `sgl/device/print.slang::write_arg(IPrintable)`
            // whose `IPrintable arg` parameter is type-erased only
            // inside the helper. The actual call sites supply
            // concrete types via generic-pack expansion, but the
            // pre-inlined helper body still contains an unresolved
            // lookup at the moment the walker runs. If a real
            // unresolved lookup escapes into codegen via a non-
            // entry-point helper, the underlying ICE still fires
            // and points at the same source location.
            if (!isEntryPoint(func))
                continue;
            for (auto block : func->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    auto lookup = as<IRLookupWitnessMethod>(inst);
                    if (!lookup)
                        continue;
                    if (!lookup->firstUse)
                        continue;
                    auto witnessTable = lookup->getWitnessTable();
                    if (as<IRWitnessTable>(witnessTable))
                        continue;
                    auto witnessTableType = as<IRWitnessTableTypeBase>(witnessTable->getDataType());
                    if (!witnessTableType)
                        continue;
                    auto interfaceType =
                        as<IRInterfaceType>(witnessTableType->getConformanceType());
                    if (!interfaceType || isComInterfaceType(interfaceType))
                        continue;
                    if (!diagnosedNoTypeConformancesInterfaces.add(interfaceType))
                        continue;
                    HashSet<IRInst*>& tables = *module->getContainerPool().getHashSet<IRInst>();
                    collectExistentialTables(interfaceType, tables);
                    if (tables.getCount() == 0)
                    {
                        StringBuilder typeStr;
                        printDiagnosticArg(typeStr, interfaceType);
                        sink->diagnose(Diagnostics::NoTypeConformancesFoundForInterface{
                            .interfaceType = typeStr.produceString(),
                            .location = lookup->sourceLoc});
                    }
                    module->getContainerPool().free(&tables);
                }
            }
        }
    }

    // Check if an existential operand is an entry-point parameter of interface type
    // that lacks typeflow info. This catches targets (like CUDA compute) that skip
    // collectEntryPointUniformParams and moveEntryPointUniformParamsToGlobalScope,
    // leaving interface-typed params as plain IRParam without typeflow info.
    // Emits E50100 when no conformances are registered, or E50104 when conformances
    // exist but the target cannot handle interface-typed entry-point params.
    void diagnoseEntryPointInterfaceParamIfNeeded(IRParam* param, IRInst* inst)
    {
        auto interfaceType = as<IRInterfaceType>(param->getDataType());
        if (!interfaceType || isComInterfaceType(interfaceType) || !isFuncParam(param))
            return;
        if (diagnosedEntryPointInterfaceParams.contains(param))
            return;
        auto paramFunc = as<IRFunc>(as<IRBlock>(param->getParent())->getParent());
        if (!paramFunc || !paramFunc->findDecoration<IREntryPointDecoration>())
            return;

        diagnosedEntryPointInterfaceParams.add(param);
        StringBuilder typeStr;
        printDiagnosticArg(typeStr, interfaceType);

        HashSet<IRInst*>& tables = *module->getContainerPool().getHashSet<IRInst>();
        collectExistentialTables(interfaceType, tables);
        if (tables.getCount() == 0)
        {
            sink->diagnose(Diagnostics::NoTypeConformancesFoundForInterface{
                .interfaceType = typeStr.produceString(),
                .location = inst->sourceLoc});
        }
        else
        {
            sink->diagnose(Diagnostics::InterfaceTypedEntryPointParamNotSupported{
                .interfaceType = typeStr.produceString(),
                .location = inst->sourceLoc});
        }
        module->getContainerPool().free(&tables);
    }

    IRInst* analyzeExtractExistentialWitnessTable(
        IRInst* context,
        IRExtractExistentialWitnessTable* inst)
    {
        // An ExtractExistentialWitnessTable inst is assumed to by dynamic, so we
        // extract the set of witness tables from the input existential and
        // state that the info of the result is a tag-type of that set.
        //
        // Note that since ExtractExistentialWitnessTable can only be used on
        // an existential, the input info must be a TaggedUnionType of
        // concrete table and type sets (or none/unbounded)
        //

        auto operand = inst->getOperand(0);

        if (isComInterfaceType(inst->getOperand(0)->getDataType()))
        {
            IRBuilder builder(module);
            return makeElementOfSetType(builder.getSingletonSet(
                kIROp_WitnessTableSet,
                builder.getUnboundedWitnessTableElement(inst->getOperand(0)->getDataType())));
        }

        auto operandInfo = tryGetInfo(context, operand);
        if (!operandInfo)
        {
            if (auto param = as<IRParam>(operand))
                diagnoseEntryPointInterfaceParamIfNeeded(param, inst);
            return none();
        }

        if (auto taggedUnion = as<IRTaggedUnionType>(operandInfo))
        {
            auto tableSet = taggedUnion->getWitnessTableSet();
            if (auto uninitElement = tableSet->tryGetUninitializedElement())
            {
                StringBuilder sb;
                printDiagnosticArg(sb, uninitElement->getOperand(0));
                sink->diagnose(Diagnostics::DynamicDispatchOnPotentiallyUninitializedExistential{
                    .object = sb.produceString(),
                    .location = inst->sourceLoc});

                return none(); // We'll return none so that the analysis doesn't
                               // crash early, before we can detect the error count
                               // and exit gracefully.
            }

            return makeElementOfSetType(tableSet);
        }

        SLANG_UNEXPECTED("Unhandled info type in analyzeExtractExistentialWitnessTable");
    }

    IRInst* analyzeExtractExistentialType(IRInst* context, IRExtractExistentialType* inst)
    {
        // An ExtractExistentialType inst is assumed to be dynamic, so we
        // extract the set of witness tables from the input existential and
        // state that the info of the result is a tag-type of that set.
        //
        // Note: Since ExtractExistentialType can only be used on
        // an existential, the input info must be a TaggedUnionType of
        // concrete table and type sets (or none/unbounded)
        //

        auto operand = inst->getOperand(0);

        if (isComInterfaceType(inst->getOperand(0)->getDataType()))
        {
            IRBuilder builder(module);
            return makeElementOfSetType(builder.getSingletonSet(
                kIROp_TypeSet,
                builder.getUnboundedTypeElement(inst->getOperand(0)->getDataType())));
        }

        auto operandInfo = tryGetInfo(context, operand);
        if (!operandInfo)
        {
            if (auto param = as<IRParam>(operand))
                diagnoseEntryPointInterfaceParamIfNeeded(param, inst);
            return none();
        }

        if (auto taggedUnion = as<IRTaggedUnionType>(operandInfo))
            return makeElementOfSetType(taggedUnion->getTypeSet());

        SLANG_UNEXPECTED("Unhandled info type in analyzeExtractExistentialType");
    }

    IRInst* analyzeExtractExistentialValue(IRInst* context, IRExtractExistentialValue* inst)
    {
        // Logically, an ExtractExistentialValue inst is carrying a payload
        // of a union type.
        //
        // We represent this by setting its info to be equal to the type-set,
        // which will later lower into an any-value-type.
        //
        // Note that there is no 'tag' here since ExtractExistentialValue is not representing
        // tag information about which type in the set is active, but is representing
        // a value of the set's union type.
        //

        auto operand = inst->getOperand(0);
        if (isComInterfaceType(inst->getOperand(0)->getDataType()))
        {
            IRBuilder builder(module);
            return makeUntaggedUnionType(cast<IRTypeSet>(builder.getSingletonSet(
                kIROp_TypeSet,
                builder.getUnboundedTypeElement(inst->getOperand(0)->getDataType()))));
        }

        auto operandInfo = tryGetInfo(context, operand);
        if (!operandInfo)
        {
            if (auto param = as<IRParam>(operand))
                diagnoseEntryPointInterfaceParamIfNeeded(param, inst);
            return none();
        }

        if (auto taggedUnion = as<IRTaggedUnionType>(operandInfo))
            return makeUntaggedUnionType(taggedUnion->getTypeSet());

        return none();
    }

    // TODO: These substituteXYZ methods should be unified with
    // union construction. Currently there's way too much duplication.

    IRInst* substituteLeafSet(IRInst* info)
    {
        auto elementOfSetType = as<IRElementOfSetType>(info);
        if (!elementOfSetType)
            return none();

        if (elementOfSetType->getSet()->isSingleton())
            return elementOfSetType->getSet()->getElement(0);

        if (auto unboundedElement = elementOfSetType->getSet()->tryGetUnboundedElement())
        {
            IRBuilder builder(module);
            return makeUntaggedUnionType(
                cast<IRTypeSet>(builder.getSingletonSet(kIROp_TypeSet, unboundedElement)));
        }

        return makeUntaggedUnionType(cast<IRTypeSet>(elementOfSetType->getSet()));
    }

    IRInst* substituteSetsInTypeLike(IRInst* typeLike)
    {
        if (auto substitutedLeaf = substituteLeafSet(typeLike))
            return substitutedLeaf;

        if (auto ptrType = as<IRPtrTypeBase>(typeLike))
        {
            IRBuilder builder(module);
            return builder.getPtrTypeWithAddressSpace(
                (IRType*)substituteSetsInTypeLike(ptrType->getValueType()),
                ptrType);
        }

        if (auto arrayType = as<IRArrayType>(typeLike))
        {
            IRBuilder builder(module);
            return builder.getArrayType(
                (IRType*)substituteSetsInTypeLike(arrayType->getElementType()),
                arrayType->getElementCount(),
                getArrayStride(arrayType));
        }

        if (auto attributedType = as<IRAttributedType>(typeLike))
        {
            IRBuilder builder(module);
            List<IRAttr*>& attrs = *module->getContainerPool().getList<IRAttr>();
            for (auto attr : attributedType->getAllAttrs())
                attrs.add(attr);

            auto result = builder.getAttributedType(
                (IRType*)substituteSetsInTypeLike(attributedType->getBaseType()),
                attrs);
            module->getContainerPool().free(&attrs);
            return result;
        }

        if (auto tupleType = as<IRTupleType>(typeLike))
        {
            List<IRType*> newElementTypes;
            for (UInt i = 0; i < tupleType->getOperandCount(); i++)
                newElementTypes.add((IRType*)substituteSetsInTypeLike(tupleType->getOperand(i)));
            IRBuilder builder(module);
            return builder.getTupleType(newElementTypes);
        }

        if (auto optionalType = as<IROptionalType>(typeLike);
            optionalType && !isOptionalExistentialType(typeLike))
        {
            IRBuilder builder(module);
            return builder.getOptionalType(
                (IRType*)substituteSetsInTypeLike(optionalType->getValueType()));
        }

        if (auto diffPairType = as<IRDifferentialPairType>(typeLike))
        {
            IRBuilder builder(module);
            auto substitutedValueType =
                (IRType*)substituteSetsInTypeLike(diffPairType->getValueType());
            return builder.getTupleType(substitutedValueType, substitutedValueType);
        }

        return typeLike;
    }

    IRInst* substituteSets(IRInst* context, IRInst* type)
    {
        if (auto info = tryGetInfo(context, type))
            return substituteSetsInTypeLike(info);

        if (auto ptrType = as<IRPtrTypeBase>(type))
        {
            IRBuilder builder(module);
            return builder.getPtrTypeWithAddressSpace(
                (IRType*)substituteSets(context, ptrType->getValueType()),
                ptrType);
        }

        if (auto arrayType = as<IRArrayType>(type))
        {
            IRBuilder builder(module);
            return builder.getArrayType(
                (IRType*)substituteSets(context, arrayType->getElementType()),
                arrayType->getElementCount(),
                getArrayStride(arrayType));
        }

        if (auto attributedType = as<IRAttributedType>(type))
        {
            IRBuilder builder(module);
            List<IRAttr*>& attrs = *module->getContainerPool().getList<IRAttr>();
            for (auto attr : attributedType->getAllAttrs())
                attrs.add(attr);

            auto result = builder.getAttributedType(
                (IRType*)substituteSets(context, attributedType->getBaseType()),
                attrs);
            module->getContainerPool().free(&attrs);
            return result;
        }

        if (auto tupleType = as<IRTupleType>(type))
        {
            List<IRType*> newElementTypes;
            for (UInt i = 0; i < tupleType->getOperandCount(); i++)
                newElementTypes.add((IRType*)substituteSets(context, tupleType->getOperand(i)));
            IRBuilder builder(module);
            return builder.getTupleType(newElementTypes);
        }

        if (auto optionalType = as<IROptionalType>(type);
            optionalType && !isOptionalExistentialType(type))
        {
            IRBuilder builder(module);
            return builder.getOptionalType(
                (IRType*)substituteSets(context, optionalType->getValueType()));
        }

        if (auto diffPairType = as<IRDifferentialPairType>(type))
        {
            IRBuilder builder(module);
            auto substitutedValueType =
                (IRType*)substituteSets(context, diffPairType->getValueType());
            return builder.getTupleType(substitutedValueType, substitutedValueType);
        }

        return type;
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
        // or a set of generics.
        //
        // Then, for each generic, we'll create a specialized version by using the
        // set info for each argument in place of the argument.
        //     e.g. Specialize(G, A0, A1) becomes Specialize(G, info(A1).set,
        //     info(A2).set)
        //         (i.e. if the args are tag-types, we only use the set part)
        //
        // This transformation is important to lift the 'dynamic' specialize instruction into a
        // global specialize instruction while still retaining the information about what types and
        // tables the resulting generic should support.
        //
        // Finally, we put all the specialized vesions back into a set and return that info.
        //

        auto operand = inst->getBase();
        auto operandInfo = tryGetInfo(context, operand);

        if (as<IRTaggedUnionType>(operandInfo))
        {
            SLANG_UNEXPECTED("Unexpected operand for IRSpecialize");
        }

        // Handle the 'many' or 'one' cases.
        if (as<IRElementOfSetType>(operandInfo) || isGlobalInst(operand))
        {
            List<IRInst*>& specializationArgs = *module->getContainerPool().getList<IRInst>();
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
                {
                    module->getContainerPool().free(&specializationArgs);
                    return none();
                }

                if (as<IRTaggedUnionType>(argInfo))
                {
                    SLANG_UNEXPECTED("Unexpected Existential operand in specialization argument.");
                }

                if (auto elementOfSetType = as<IRElementOfSetType>(argInfo))
                {
                    if (elementOfSetType->getSet()->isSingleton())
                        specializationArgs.add(elementOfSetType->getSet()->getElement(0));
                    else if (
                        auto unboundedElement =
                            elementOfSetType->getSet()->tryGetUnboundedElement())
                    {
                        // Infinite set.
                        //
                        // While our sets allow for encoding an unbounded case along with known
                        // cases, when it comes to specializing a function or placing a call to a
                        // function, we will default to the single unbounded element case.
                        //
                        IRBuilder builder(module);
                        SLANG_ASSERT(unboundedElement);
                        auto setOp = getSetOpFromType(inst->getArg(i)->getDataType());
                        auto pureUnboundedSet = builder.getSingletonSet(setOp, unboundedElement);
                        if (auto typeSet = as<IRTypeSet>(pureUnboundedSet))
                            specializationArgs.add(makeUntaggedUnionType(typeSet));
                        else
                            specializationArgs.add(pureUnboundedSet);
                    }
                    else
                    {
                        // Dealing with a non-singleton, but finite set.
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
                            module->getContainerPool().free(&specializationArgs);
                            return none();
                            // SLANG_UNEXPECTED("Unexpected set type in specialization argument.");
                        }
                    }
                }
                else
                {
                    SLANG_UNEXPECTED("Unhandled PropagationJudgment in analyzeSpecialize");
                }
            }

            // This part creates a correct type for the specialization, by following the same
            // process: replace all operands in the composite type with their propagated set.
            //

            IRType* typeOfSpecialization = nullptr;
            if (inst->getDataType()->getParent()->getOp() == kIROp_ModuleInst)
                typeOfSpecialization = inst->getDataType();
            else if (auto funcType = as<IRFuncType>(inst->getDataType()))
            {
                List<IRType*>& newParamTypes = *module->getContainerPool().getList<IRType>();
                for (auto paramType : funcType->getParamTypes())
                    newParamTypes.add((IRType*)substituteSets(context, paramType));
                IRBuilder builder(module);
                builder.setInsertInto(module);
                typeOfSpecialization = builder.getFuncType(
                    newParamTypes.getCount(),
                    newParamTypes.getBuffer(),
                    (IRType*)substituteSets(context, funcType->getResultType()));
                module->getContainerPool().free(&newParamTypes);
            }
            else if (auto witnessTableType = as<IRWitnessTableType>(inst->getDataType()))
            {
                IRBuilder builder(module);
                builder.setInsertInto(module);

                if (!isGlobalInst(witnessTableType))
                {
                    // Just use a dummy sentinel type.. we don't actually care about the
                    // structure of the witness table type, since everything is
                    // inferred from data-flow.
                    //
                    typeOfSpecialization = builder.getWitnessTableType(builder.getVoidType());
                }
                else
                    typeOfSpecialization = inst->getDataType();
            }
            else if (auto typeInfo = tryGetInfo(context, inst->getDataType()))
            {
                // There's one other case we'd like to handle, where the func-type itself is a
                // dynamic IRSpecialize. In this situation, we'd want to use the type inst's info to
                // find the set-based specialization and create a func-type from it.
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
                    module->getContainerPool().free(&specializationArgs);
                    return none();
                }
            }
            else
            {
                // We don't have a type we can work with just yet.
                module->getContainerPool().free(&specializationArgs);
                return none(); // No info for the type
            }

            if (!isGlobalInst(typeOfSpecialization))
            {
                // Our func-type operand is not yet been lifted.
                // For now, we can't say anything.
                //
                module->getContainerPool().free(&specializationArgs);
                return none();
            }

            // Specialize each element in the set
            HashSet<IRInst*>& specializedSet = *module->getContainerPool().getHashSet<IRInst>();

            IRSetBase* set = nullptr;
            if (auto elementOfSetType = as<IRElementOfSetType>(operandInfo))
            {
                set = elementOfSetType->getSet();

                forEachInSet(
                    module,
                    set,
                    [&](IRInst* arg)
                    {
                        // Create a new specialized instruction for each argument
                        IRBuilder builder(module);
                        builder.setInsertInto(module);

                        if (as<IRUnboundedGenericElement>(arg))
                        {
                            // Infinite set.
                            //
                            // We currently only support specializing generic functions
                            // in this way, so we'll assume its an unbounded-func-element
                            //
                            specializedSet.add(builder.getUnboundedFuncElement());
                            return;
                        }

                        auto newSpec = builder.emitSpecializeInst(
                            typeOfSpecialization,
                            arg,
                            specializationArgs);
                        specializedSet.add(newSpec);
                    });
            }
            else
            {
                // Concrete case..
                IRBuilder builder(module);
                builder.setInsertInto(module);
                auto newSpec =
                    builder.emitSpecializeInst(typeOfSpecialization, operand, specializationArgs);
                specializedSet.add(newSpec);
            }

            IRBuilder builder(module);
            auto setOp = getSetOpFromType(inst->getDataType());

            // There are a few types of specializations (particularly with generics that return
            // values), that we don't handle in the type-flow pass. We'll just avoid specializing
            // these.
            //
            if (setOp == kIROp_Invalid)
            {
                module->getContainerPool().free(&specializedSet);
                module->getContainerPool().free(&specializationArgs);
                return none();
            }

            auto resultSetType = makeElementOfSetType(builder.getSet(setOp, specializedSet));
            module->getContainerPool().free(&specializedSet);
            module->getContainerPool().free(&specializationArgs);
            return resultSetType;
        }

        if (!operandInfo)
            return none();

        SLANG_UNEXPECTED("Unhandled PropagationJudgment in analyzeExtractExistentialWitnessTable");
    }

    // Helper to check if propagation info represents a singleton (a single concrete type).
    bool isSingletonInfo(IRInst* info)
    {
        if (auto taggedUnion = as<IRTaggedUnionType>(info))
            return taggedUnion->isSingleton();
        if (auto untaggedUnion = as<IRUntaggedUnionType>(info))
            return untaggedUnion->getSet()->isSingleton();
        if (auto elementOfSet = as<IRElementOfSetType>(info))
            return elementOfSet->getSet()->isSingleton();
        return false;
    }

    // Given a callee and information about the call arguments, determine if we need
    // a SpecializeExistentialsInFunc context instead of the bare callee.
    //
    // This is the core mechanism for specializing existential calls:
    // when a function has interface-typed parameters and we know the concrete type
    // at the call site, we create a SpecializeExistentialsInFunc to represent that
    // specialized version.
    //
    // Returns nullptr if the callee has non-concrete parameters but we don't have
    // info for them yet (not ready to propagate). The caller should skip adding
    // the propagation edge in this case.
    //
    IRInst* maybeGetBoundFunc(IRInst* callee, List<IRInst*>& paramInfos, WorkQueue<WorkItem>&)
    {
        // Only handle direct (non-generic, non-intrinsic) functions.
        IRFunc* func = as<IRFunc>(callee);
        if (!func)
            return callee;
        if (isIntrinsic(func))
            return callee;
        if (!func->getFirstBlock())
            return callee;

        // If our callee is a specialize, we won't create separate bound functions yet,
        // since it's currently tricky to lower a SpecializeExistentialsInFunc(Specialize(...)).
        //
        // Most of the complexity is that that we use `specializeGeneric` which loses cloning
        // information needed to transfer our propagation analysis to the newly created function.
        //
        if (as<IRSpecialize>(callee))
            return callee;

        // Build per-parameter bindings from the provided arg infos.
        // VoidLit is used for concrete parameters (no binding needed).
        bool hasAnyBinding = false;
        List<IRInst*> bindings;
        // List<IRType*> paramTypes;
        UInt argIndex = 0;
        for (auto param : func->getFirstBlock()->getParams())
        {
            const auto [paramDirection, paramType] =
                splitParameterDirectionAndType(param->getDataType());

            // There are three cases where we don't need to create a binding for a parameter:
            // (i) Concrete parameter: if the parameter is already concrete, we don't need to bind
            //     it.
            // (ii) Out parameter: we currently only support specialization based on input
            //     parameters, so
            // (iii) FuncType parameter: we currently don't handle
            //     de-functionalization here, and do so in a
            //     later pass. If we eventually merged these two passes, we could remove
            //     this exception and handle FuncType parameters with bindings as well.
            //
            if (isConcreteType(paramType) || paramDirection == ParameterDirectionInfo::Kind::Out ||
                paramType->getOp() == kIROp_FuncType)
            {
                IRBuilder builder(module);
                bindings.add(builder.getVoidValue());
                // paramTypes.add(paramType);
                argIndex++;
                continue;
            }

            // Non-concrete parameter: check if we have param binding info.
            IRInst* paramInfo =
                (argIndex < (UInt)paramInfos.getCount()) ? paramInfos[argIndex] : nullptr;

            if (!paramInfo)
            {
                // Non-concrete param with no info yet - use VoidLit so the
                // interprocedural edge is still registered. This allows
                // propagation to proceed (e.g. diagnostics for __ref params
                // with dynamic dispatch types) even before full info arrives.
                IRBuilder builder(module);
                bindings.add(builder.getVoidValue());
                argIndex++;
                continue;
            }

            hasAnyBinding = true;
            bindings.add(paramInfo);
            // paramTypes.add((IRType*)paramInfo);
            argIndex++;
        }

        if (!hasAnyBinding)
            return callee; // No bindings to specialize on

        IRBuilder builder(module);
        List<IRInst*> operands;
        operands.add(callee);

        for (auto& binding : bindings)
            operands.add(binding);

        auto existentialSpecializedFunc =
            cast<IRSpecializeExistentialsInFunc>(builder.emitIntrinsicInst(
                nullptr, // Copy over the original func type, we'll replace the func-type
                         // later once we have all the info.
                kIROp_SpecializeExistentialsInFunc,
                (UInt)operands.getCount(),
                operands.getBuffer()));

        existentialSpecializedFuncCache.addIfNotExists(
            callee,
            List<IRSpecializeExistentialsInFunc*>());
        existentialSpecializedFuncCache[callee].add(existentialSpecializedFunc);

        return existentialSpecializedFunc;
    }

    // Initialize parameter info from a SpecializeExistentialsInFunc's binding operands.
    void initializeBindingsForSpecializeExistentials(
        IRSpecializeExistentialsInFunc* existentialSpecializedFunc,
        IRFunc* func,
        WorkQueue<WorkItem>& workQueue)
    {
        auto firstBlock = func->getFirstBlock();
        if (!firstBlock)
            return;

        UInt paramIndex = 0;
        for (auto param : firstBlock->getParams())
        {
            if (paramIndex + 1 < existentialSpecializedFunc->getOperandCount())
            {
                auto binding = existentialSpecializedFunc->getOperand(paramIndex + 1);
                if (binding && !as<IRVoidLit>(binding))
                {
                    updateInfoForMerge(
                        existentialSpecializedFunc,
                        param,
                        binding,
                        param->getDataType(),
                        workQueue);
                }
            }
            paramIndex++;
        }
    }


    // If any arguments in a call is a value pack, we will expand them into the argument list,
    // so that the call has no arguments of type `IRTypePack`.
    // For example, we will turn `f(MakeValuePack(a, b))` into `f(a, b)`.
    //
    IRCall* tryExpandArgPack(IRCall* call)
    {
        bool anyArgPack = false;
        for (UInt i = 0; i < call->getArgCount(); i++)
        {
            auto arg = call->getArg(i);
            if (as<IRTypePack>(arg->getDataType()))
            {
                anyArgPack = true;
                break;
            }
        }
        if (!anyArgPack)
            return call;
        IRBuilder builder(call);
        builder.setInsertBefore(call);
        List<IRInst*> newArgs;
        for (UInt i = 0; i < call->getArgCount(); i++)
        {
            auto arg = call->getArg(i);
            if (auto typePack = as<IRTypePack>(arg->getDataType()))
            {
                for (UInt elementIndex = 0; elementIndex < typePack->getOperandCount();
                     elementIndex++)
                {
                    auto newArg = builder.emitGetTupleElement(
                        (IRType*)typePack->getOperand(elementIndex),
                        arg,
                        elementIndex);
                    newArgs.add(newArg);
                }
            }
            else
            {
                newArgs.add(arg);
            }
        }
        auto newCall =
            builder.emitCallInst(call->getFullType(), call->getCallee(), newArgs.getArrayView());
        call->replaceUsesWith(newCall);
        call->transferDecorationsTo(newCall);
        call->removeAndDeallocate();

        return newCall;
    }

    IRFuncType* maybeExpandFuncType(IRFuncType* funcType)
    {
        List<IRType*> newArgTypes;

        for (auto paramType : funcType->getParamTypes())
        {
            if (auto typePack = as<IRTypePack>(paramType))
            {
                for (UInt elementIndex = 0; elementIndex < typePack->getOperandCount();
                     elementIndex++)
                {
                    newArgTypes.add((IRType*)typePack->getOperand(elementIndex));
                }
            }
            else
            {
                newArgTypes.add(paramType);
            }
        }

        IRBuilder builder(module);
        return as<IRFuncType>(
            builder.getFuncType(newArgTypes.getArrayView(), funcType->getResultType()));
    }

    void expandPacksInFunc(IRFunc* func)
    {
        tryExpandParameterPack(func);

        // Go through any IRCall insts in func and expand any argument packs.
        List<IRCall*> callsToConsider;
        for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
        {
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                if (auto call = as<IRCall>(inst))
                {
                    callsToConsider.add(call);
                }
            }
        }

        for (auto call : callsToConsider)
        {
            tryExpandArgPack(call);
        }
    }

    bool tryExpandParameterPack(IRFunc* func)
    {
        if (!func)
            return false;

        ShortList<IRInst*> params;
        for (auto param : func->getParams())
        {
            if (as<IRTypePack>(param->getDataType()))
                params.add(param);
            if (as<IRExpand>(param->getDataType()))
            {
                return false;
            }
        }
        if (params.getCount() == 0)
            return false;

        IRBuilder builder(func);
        for (auto param : params)
        {
            builder.setInsertBefore(param);
            auto typePack = as<IRTypePack>(param->getDataType());
            ShortList<IRInst*> newParams;
            for (UInt i = 0; i < typePack->getOperandCount(); i++)
            {
                auto newParam = builder.createParam((IRType*)typePack->getOperand(i));
                newParam->insertBefore(param);
                newParams.add(newParam);
            }
            setInsertBeforeOrdinaryInst(&builder, param);
            auto val = builder.emitMakeValuePack(
                typePack,
                (UInt)newParams.getCount(),
                newParams.getArrayView().getBuffer());
            param->replaceUsesWith(val);
            param->removeAndDeallocate();
        }

        fixUpFuncType(func);
        return true;
    }

    void discoverContext(IRInst* context, WorkQueue<WorkItem>& workQueue)
    {
        // "Discovering" a context, essentially means we check if this is the first
        // time we're trying to propagate information into this context. A context
        // is a global-scope IRFunc or IRSpecialize.
        //
        // If it is the first, we enqueue some work to perform initialization of all
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

                    // Add all blocks to the work queue
                    for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
                        workQueue.enqueue(WorkItem(context, block));

                    if (this->uniqueDefs.add(func))
                        expandPacksInFunc(func);

                    break;
                }
            case kIROp_Specialize:
                {
                    auto specialize = cast<IRSpecialize>(context);
                    if (isInvalidExistentialSpecialization(specialize))
                    {
                        emitExistentialSpecializationDiagnostic(
                            specialize,
                            specialize->sourceLoc,
                            specialize);
                        return;
                    }

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

                        if (auto set = as<IRSetBase>(arg))
                        {
                            updateInfoForMerge(
                                context,
                                param,
                                makeElementOfSetType(set),
                                param->getDataType(),
                                workQueue);
                        }
                        else if (auto untaggedUnion = as<IRUntaggedUnionType>(arg))
                        {
                            IRBuilder builder(module);
                            updateInfoForMerge(
                                context,
                                param,
                                makeElementOfSetType(untaggedUnion->getSet()),
                                param->getDataType(),
                                workQueue);
                        }
                        else if (as<IRType>(arg))
                        {
                            IRBuilder builder(module);
                            updateInfoForMerge(
                                context,
                                param,
                                makeElementOfSetType(builder.getSingletonSet(kIROp_TypeSet, arg)),
                                param->getDataType(),
                                workQueue);
                        }
                        else if (as<IRWitnessTable>(arg))
                        {
                            IRBuilder builder(module);
                            updateInfoForMerge(
                                context,
                                param,
                                makeElementOfSetType(
                                    builder.getSingletonSet(kIROp_WitnessTableSet, arg)),
                                param->getDataType(),
                                workQueue);
                        }
                        else
                        {
                            SLANG_UNEXPECTED("Unexpected argument type in specialization");
                        }
                    }

                    // Add all blocks to the work queue for an initial sweep
                    for (auto block = generic->getFirstBlock(); block;
                         block = block->getNextBlock())
                        workQueue.enqueue(WorkItem(context, block));

                    for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
                        workQueue.enqueue(WorkItem(context, block));

                    break;
                }
            case kIROp_SpecializeExistentialsInFunc:
                {
                    auto existentialSpecializedFunc = cast<IRSpecializeExistentialsInFunc>(context);
                    func = getFuncDefinitionForContext(existentialSpecializedFunc);

                    if (this->uniqueDefs.add(func))
                        expandPacksInFunc(func);

                    // Initialize parameter infos from the bindings
                    initializeBindingsForSpecializeExistentials(
                        existentialSpecializedFunc,
                        func,
                        workQueue);

                    // Add all blocks to the work queue
                    for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
                        workQueue.enqueue(WorkItem(context, block));

                    break;
                }
            }
        }
    }

    IRInst* analyzeCall(IRInst* context, IRCall* inst, WorkQueue<WorkItem>& workQueue)
    {
        // We don't perform the propagation here, but instead we add inter-procedural
        // edges to the work queue.
        // The propagation logic is handled in `propagateInterproceduralEdge()`
        //

        auto callee = inst->getCallee();
        auto calleeInfo = tryGetInfo(context, callee);

        if (isNoneCallee(callee))
            return none();

        HashSet<IRInst*> calleeSet;
        auto propagateToCallSite = [&](IRInst* callee)
        {
            if (isInvalidExistentialSpecialization(callee))
            {
                emitExistentialSpecializationDiagnostic(callee, inst->sourceLoc, inst);
                return;
            }

            if (as<IRUnboundedFuncElement>(callee))
            {
                // An unbounded element represents an unknown function,
                // so we can't propagate anything in this case.
                //
                return;
            }

            // Register the call site in the map to allow for the
            // return-edge to be created.
            //
            // We use an explicit map instead of walking the uses of the
            // func, since we might have functions that are called indirectly
            // through lookups.
            //
            discoverContext(callee, workQueue);

            calleeSet.add(callee);

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

        auto convertArgInfosToParamInfos = [&](IRFuncType* calleeFuncType) -> List<IRInst*>
        {
            List<IRInst*> paramInfos;

            UInt argIdx = 1; // Skip callee (operand 0)
            for (auto paramType : calleeFuncType->getParamTypes())
            {
                IRInst* paramInfo = nullptr;
                if (argIdx < inst->getOperandCount())
                {
                    auto arg = inst->getOperand(argIdx);
                    auto argInfo = tryGetArgInfo(context, arg, paramType);

                    // For pointer-wrapped params (out/inout/borrow), unwrap to get value info
                    if (argInfo)
                    {
                        const auto [paramDirection, _] = splitParameterDirectionAndType(paramType);
                        switch (paramDirection.kind)
                        {
                        case ParameterDirectionInfo::Kind::Out:
                        case ParameterDirectionInfo::Kind::BorrowInOut:
                        case ParameterDirectionInfo::Kind::BorrowIn:
                            {
                                IRBuilder builder(module);
                                paramInfo = builder.getPtrTypeWithAddressSpace(
                                    as<IRPtrTypeBase>(argInfo)->getValueType(),
                                    as<IRPtrTypeBase>(paramType));
                                break;
                            }

                        default:
                            {
                                paramInfo = argInfo;
                                break;
                            }
                        }
                    }
                }

                paramInfos.add(paramInfo);
                argIdx++;
            }

            return paramInfos;
        };

        // If we have a set of functions (with or without a dynamic tag), register
        // each one.
        //
        if (auto elementOfSetType = as<IRElementOfSetType>(calleeInfo))
        {
            if (elementOfSetType->getSet()->isUnbounded())
            {
                // An unbounded set represents potentially infinite callees, so we won't propagate
                // anything.
                //
                // Note: Currently, an unbounded set can only have an unbounded element (and nothing
                // else), but in the future, we can allow for a mix of unbounded and known elements.
                //
                return none();
            }

            List<IRInst*>& funcs = *module->getContainerPool().getList<IRInst>();

            forEachInSet(
                module,
                elementOfSetType->getSet(),
                [&](IRInst* func) { funcs.add(func); });

            for (auto func : funcs)
            {
                auto resolvedFunc = translationContext.resolveInst(func);
                if (!resolvedFunc)
                {
                    // This usually indicates an error. We'll fail gracefully here.
                    module->getContainerPool().free(&funcs);
                    return none();
                }
                auto paramInfos =
                    convertArgInfosToParamInfos(cast<IRFuncType>(resolvedFunc->getDataType()));
                if (auto boundCallee = maybeGetBoundFunc(resolvedFunc, paramInfos, workQueue))
                    propagateToCallSite(boundCallee);
            }

            module->getContainerPool().free(&funcs);
        }
        else if (isGlobalInst(callee))
        {
            auto resolvedCallee = translationContext.resolveInst(callee);
            if (!resolvedCallee)
                return none();

            auto paramInfos =
                convertArgInfosToParamInfos(cast<IRFuncType>(resolvedCallee->getDataType()));
            if (auto boundCallee = maybeGetBoundFunc(resolvedCallee, paramInfos, workQueue))
                propagateToCallSite(boundCallee);
        }

        if (calleeSet.getCount() != 0)
        {
            if (this->callSiteInfo.containsKey(InstWithContext(context, inst)))
            {
                // Remove ref counts for all callees in the old set, since we're replacing the set
                // with new information.
                //
                forEachInSet(
                    module,
                    cast<IRElementOfSetType>(this->callSiteInfo[InstWithContext(context, inst)])
                        ->getSet(),
                    [&](IRInst* callee)
                    {
                        // Remove call sites that aren't in the new set.
                        // New call sites have already been added in `propagateToCallSite()`.
                        //
                        if (this->funcCallSites.containsKey(callee) && !calleeSet.contains(callee))
                            this->funcCallSites[callee].remove(InstWithContext(context, inst));

                        this->calleeRefCounts[callee]--;
                    });
            }

            IRBuilder builder(module);
            this->callSiteInfo[InstWithContext(context, inst)] =
                makeElementOfSetType(builder.getSet(kIROp_FuncSet, calleeSet));

            // Store the callee's declared func type before specialization replaces it
            // with tag/set types. This is the abstract func type from the interface.
            if (auto calleeFuncType = as<IRFuncType>(inst->getCallee()->getDataType()))
                this->callSiteFuncType[InstWithContext(context, inst)] = calleeFuncType;

            for (auto _callee : calleeSet)
            {
                this->calleeRefCounts.addIfNotExists(_callee, 0);
                this->calleeRefCounts[_callee]++;
            }
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
        WorkQueue<WorkItem>& workQueue)
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
                if (as<IRArrayType>(baseValueType))
                {
                    // Propagate 'this' information to the base by wrapping it as a pointer to
                    // array.
                    IRBuilder builder(module);
                    auto baseInfo = builder.getPtrTypeWithAddressSpace(
                        builder.getArrayType(
                            (IRType*)thisValueInfo,
                            as<IRArrayType>(baseValueType)->getElementCount(),
                            getArrayStride(as<IRArrayType>(baseValueType))),
                        as<IRPtrTypeBase>(getElementPtr->getBase()->getDataType()));

                    // Recursively try to update the base pointer.
                    maybeUpdateInfoForAddress(
                        context,
                        getElementPtr->getBase(),
                        baseInfo,
                        workQueue);
                }
                else if (as<IRTupleType>(baseValueType))
                {
                    // Build effective tuple type by replacing the element type at the given index.
                    IRBuilder builder(module);
                    auto elementIndex = getElementPtr->getIndex();
                    if (auto intLit = as<IRIntLit>(elementIndex))
                    {
                        auto index = (UInt)intLit->getValue();
                        if (index < baseValueType->getOperandCount())
                        {
                            List<IRType*> elementTypes;
                            for (UInt i = 0; i < baseValueType->getOperandCount(); i++)
                            {
                                if (i == index)
                                    elementTypes.add((IRType*)thisValueInfo);
                                else
                                    elementTypes.add(
                                        isConcreteType(baseValueType->getOperand(i))
                                            ? (IRType*)baseValueType->getOperand(i)
                                            : (IRType*)nullptr);
                            }
                            auto baseInfo = builder.getPtrTypeWithAddressSpace(
                                builder.getTupleType(elementTypes),
                                as<IRPtrTypeBase>(getElementPtr->getBase()->getDataType()));

                            // Recursively try to update the base pointer.
                            maybeUpdateInfoForAddress(
                                context,
                                getElementPtr->getBase(),
                                baseInfo,
                                workQueue);
                        }
                    }
                }
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
                    // Use the field address's pointer type as context for structural union.
                    if (auto newInfo =
                            unionPropagationInfo(fieldAddress->getDataType(), info, existingInfo))
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
            updateInfoForMerge(context, var, info, var->getDataType(), workQueue);
        }
        else if (auto param = as<IRParam>(inst))
        {
            if (isFuncParam(param) && isPublicFunc(getParentFunc(param)))
            {
                // Do nothing, since we should not by modifying public function parameters.
                return;
            }
            else
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

                updateInfoForMerge(context, param, newInfo, param->getDataType(), workQueue);
            }
        }
        else
        {
            // If we hit something unsupported, assume there's nothing to update.
            return;
        }
    }

    List<IRInst*> getParamInfos(IRInst* context, IRFuncType* funcTypeUnionMask)
    {
        auto baseFunc = getFuncDefinitionForContext(context);
        List<IRInst*> paramInfos;
        UInt index = 0;
        for (auto param : baseFunc->getParams())
        {
            if (auto paramInfo = tryGetInfo(context, param))
                paramInfos.add(paramInfo);
            else if (isConcreteType(param->getDataType()))
            {
                paramInfos.add(makeInfoForConcreteType(
                    module,
                    param->getDataType(),
                    funcTypeUnionMask->getParamType(index)));
            }
            else
            {
                // Info not ready yet.
                paramInfos.add(nullptr);
            }

            index++;
        }
        return paramInfos;
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
        else if (auto existentialSpecializedFunc = as<IRSpecializeExistentialsInFunc>(context))
        {
            func = getFuncDefinitionForContext(existentialSpecializedFunc);
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
                infos.add(tryGetArgInfo(context, param, param->getDataType()));
        }
        else if (auto specialize = as<IRSpecialize>(context))
        {
            auto generic = specialize->getBase();
            auto innerFunc = getGenericReturnVal(generic);
            for (auto param : as<IRFunc>(innerFunc)->getParams())
                infos.add(tryGetArgInfo(context, param, param->getDataType()));
        }
        else if (auto existentialSpecializedFunc = as<IRSpecializeExistentialsInFunc>(context))
        {
            auto baseFunc = getFuncDefinitionForContext(existentialSpecializedFunc);
            for (auto param : baseFunc->getParams())
                infos.add(tryGetArgInfo(context, param, param->getDataType()));
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
        else if (auto existentialSpecializedFunc = as<IRSpecializeExistentialsInFunc>(context))
        {
            auto baseFunc = getFuncDefinitionForContext(existentialSpecializedFunc);
            for (auto param : baseFunc->getParams())
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
            auto paramInfo = tryGetInfo(context, param);
            if (paramInfo)
                continue; // Already has some information
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

    bool specializeInstsInBlock(
        IRInst* context,
        IRBlock* block,
        WorkQueue<IRInst*>& globalsWorkList)
    {
        List<IRInst*>& instsToLower = *module->getContainerPool().getList<IRInst>();

        bool hasChanges = false;
        for (auto inst : block->getChildren())
            instsToLower.add(inst);

        for (auto inst : instsToLower)
            hasChanges |= specializeInst(context, inst, globalsWorkList);

        module->getContainerPool().free(&instsToLower);
        return hasChanges;
    }

    bool removeAnnotations(IRFunc* func)
    {
        bool hasChanges = false;

        List<IRInst*>& annotationsToRemove = *module->getContainerPool().getList<IRInst>();
        for (auto block : func->getBlocks())
        {
            for (auto child : block->getChildren())
                if (as<IRAnnotation>(child))
                    annotationsToRemove.add(child);
        }

        for (auto annotation : annotationsToRemove)
        {
            hasChanges = true;
            annotation->removeAndDeallocate();
        }

        module->getContainerPool().free(&annotationsToRemove);
        return hasChanges;
    }

    bool specializeFunc(IRFunc* func, WorkQueue<IRInst*>& globalsWorkList)
    {
        // When specializing a func, we
        // (i) rewrite the types and insts by calling `specializeInstsInBlock` and
        // (ii) handle 'merge' points where the sets need to be upcasted.
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
            hasChanges |= specializeInstsInBlock(func, block, globalsWorkList);

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

    bool resolveTypesInFunc(IRFunc* func /*, HashSet<IRType*>& aggTypesToResolve*/)
    {
        bool hasChanges = false;
        for (auto block : func->getBlocks())
        {
            // TODO: This workList is a workaround for the fact that sometimes, we end up with
            // types that are not global (usually because of arithmetic ops in types)
            // We should make sure such ops are also resolved during the type-flow pass, but in the
            // meantime, this allows us to resolve any types that might have been missed.
            //
            List<IRInst*>& workList = *module->getContainerPool().getList<IRInst>();
            for (auto inst : block->getChildren())
                workList.add(inst);

            for (auto inst : workList)
            {
                if (inst->getDataType())
                    translationContext.resolveInst(inst->getDataType());
            }

            module->getContainerPool().free(&workList);
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

    // Lower a SpecializeExistentialsInFunc context by cloning the base function
    // and transferring propagation info to the clone.
    //
    // Returns the cloned function, or nullptr if the base function is not found.
    //
    IRFunc* lowerSpecializeExistentialsInFunc(
        IRSpecializeExistentialsInFunc* existentialSpecializedFunc)
    {
        IRBuilder builder(module);
        auto entry = builder.fetchCompilerDictionaryEntry(
            module->getTranslationDict(),
            existentialSpecializedFunc);

        if (auto existingVal = entry->getValue())
            return as<IRFunc>(existingVal);

        auto baseFunc = existentialSpecializedFunc->getOperand(0);
        SLANG_ASSERT(baseFunc);

        if (as<IRSpecialize>(baseFunc))
        {
            // If the base function is a specialization, we need to make sure it's fully resolved
            // before cloning, since the clone will be used as the new context for looking up
            // propagation info.
            //
            // But then this may change the existentialSpecializedFunc inst itself, and we won't
            // have the right key to find the propagation info..
            //
            // translationContext.resolveInst(baseFunc);
            // For now, assert out.
            SLANG_UNEXPECTED(
                "SpecializeExistentialsInFunc with a specialization as the base function is "
                "not supported yet");
        }

        // Clone the base function
        IRCloneEnv cloneEnv;
        cloneEnv.squashChildrenMapping = true;
        builder.setInsertBefore(baseFunc);
        auto clonedFunc = cast<IRFunc>(cloneInst(&cloneEnv, &builder, baseFunc));

        // Transfer propagation info from the SpecializeExistentialsInFunc context
        // to the cloned function (using the cloned func as the new context).
        //
        for (auto& kv : cloneEnv.mapOldValToNew)
        {
            if (isGlobalInst(kv.first))
                continue;

            if (auto info = _tryGetInfo(InstWithContext(existentialSpecializedFunc, kv.first)))
            {
                IRBuilder infoBuilder(module);
                propagationMap[InstWithContext(clonedFunc, kv.second)] =
                    infoBuilder.getWeakUse(info);
            }

            if (as<IRCall>(kv.first))
            {
                // If the call had recorded call-site info, transfer that as well.
                if (this->callSiteInfo.containsKey(
                        InstWithContext(existentialSpecializedFunc, kv.first)))
                {
                    this->callSiteInfo[InstWithContext(clonedFunc, kv.second)] =
                        this->callSiteInfo[InstWithContext(existentialSpecializedFunc, kv.first)];
                }
                if (this->callSiteFuncType.containsKey(
                        InstWithContext(existentialSpecializedFunc, kv.first)))
                {
                    this->callSiteFuncType[InstWithContext(clonedFunc, kv.second)] =
                        this->callSiteFuncType[InstWithContext(
                            existentialSpecializedFunc,
                            kv.first)];
                }
            }
        }

        // Transfer return info
        auto returnInfoPtr = funcReturnInfo.tryGetValue(existentialSpecializedFunc);
        if (returnInfoPtr)
            funcReturnInfo[clonedFunc] = *returnInfoPtr;

        builder.setCompilerDictionaryEntryValue(entry, clonedFunc);
        return clonedFunc;
    }

    bool performDynamicInstLowering()
    {
        WorkQueue<IRInst*> globalWorkList;
        List<IRStructType*> structsToProcess;

        for (auto inst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
                if (isEntryPoint(func))
                    globalWorkList.enqueue(func);

            if (auto structType = as<IRStructType>(inst))
                structsToProcess.add(structType);
        }

        bool hasChanges = false;

        // Lower struct types first so that data access can be
        // marshalled properly during func specializeing.
        //
        for (auto structType : structsToProcess)
            hasChanges |= specializeStructType(structType);

        HashSet<IRInst*> processedSet;
        while (globalWorkList.hasItems())
        {
            if (sink->getErrorCount() > 0)
                break;

            auto globalInst = globalWorkList.dequeue();

            switch (globalInst->getOp())
            {
            case kIROp_Func:
                {
                    auto func = as<IRFunc>(globalInst);
                    if (processedSet.contains(globalInst))
                        continue;

                    hasChanges |= removeAnnotations(func);
                    hasChanges |= eliminateDeadCode(func);
                    hasChanges |= specializeFunc(func, globalWorkList);

                    if (sink->getErrorCount() > 0)
                        break;

                    hasChanges |= eliminateDeadCode(func);
                    hasChanges |= resolveTypesInFunc(func);

                    processedSet.add(globalInst);
                    break;
                }
            case kIROp_Specialize:
                {
                    // Resolve and replace.
                    translationContext.resolveInst(globalInst);
                    break;
                }
            default:
                SLANG_UNEXPECTED("Unexpected global inst type during specialization");
            }
        }

        return hasChanges;
    }

    // Returns an effective type to use for an inst, given its
    // info.
    //
    // This basically recursively walks the info and applies the array/ptr-type
    // wrappers, while replacing unbounded set with a nullptr.
    //
    // If the result of this is null, then the inst should keep its current type.
    //
    IRType* getLoweredType(IRInst* info)
    {
        if (!info)
            return nullptr;

        if (auto attributedType = as<IRAttributedType>(info))
        {
            if (auto specializedValueType = getLoweredType(attributedType->getBaseType()))
            {
                IRBuilder builder(module);

                List<IRAttr*>& attrs = *module->getContainerPool().getList<IRAttr>();
                for (auto attr : attributedType->getAllAttrs())
                    attrs.add(attr);

                auto newAttributedType =
                    builder.getAttributedType((IRType*)specializedValueType, attrs);
                module->getContainerPool().free(&attrs);
                return newAttributedType;
            }
            else
                return nullptr;
        }

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
                    arrayType->getElementCount(),
                    getArrayStride(arrayType));
            }
            else
                return nullptr;
        }

        if (auto tupleType = as<IRTupleType>(info))
        {
            IRBuilder builder(module);
            List<IRType*> specializedElements;
            bool anySpecialized = false;
            for (UInt i = 0; i < tupleType->getOperandCount(); i++)
            {
                if (auto specializedElementType = getLoweredType((IRType*)tupleType->getOperand(i)))
                {
                    specializedElements.add(specializedElementType);
                    anySpecialized = true;
                }
                else
                {
                    specializedElements.add((IRType*)tupleType->getOperand(i));
                }
            }
            if (anySpecialized || tupleType->getOperandCount() == 0)
                return builder.getTupleType(specializedElements);
            else
                return nullptr;
        }

        if (auto optionalType = as<IROptionalType>(info))
        {
            IRBuilder builder(module);
            if (auto specializedValueType = getLoweredType(optionalType->getValueType()))
            {
                return builder.getOptionalType((IRType*)specializedValueType);
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
            // Replace element-of-set types with tag types.
            return makeTagType(elementOfSetType->getSet());
        }

        if (auto valOfSetType = as<IRUntaggedUnionType>(info))
        {
            if (valOfSetType->getSet()->isSingleton())
            {
                // If there's only one type in the set, return it directly
                return (IRType*)valOfSetType->getSet()->getElement(0);
            }

            return valOfSetType;
        }

        if (as<IRFuncSet>(info) || as<IRWitnessTableSet>(info))
        {
            // Don't specialize these sets.. they should be used through
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

    bool specializeInst(IRInst* context, IRInst* inst, WorkQueue<IRInst*>& globalsWorkList)
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
            return specializeCall(context, as<IRCall>(inst), globalsWorkList);
        case kIROp_MakeExistential:
            return specializeMakeExistential(context, as<IRMakeExistential>(inst));
        case kIROp_WrapExistential:
            return specializeWrapExistential(context, as<IRWrapExistential>(inst));
        case kIROp_MakeStruct:
            return specializeMakeStruct(context, as<IRMakeStruct>(inst));
        case kIROp_MakeArray:
            return specializeMakeArray(context, as<IRMakeArray>(inst));
        case kIROp_MakeArrayFromElement:
            return specializeMakeArrayFromElement(context, as<IRMakeArrayFromElement>(inst));
        case kIROp_MakeTuple:
            return specializeMakeTuple(context, inst);
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
        case kIROp_SwizzledStore:
            return specializeSwizzledStore(context, as<IRSwizzledStore>(inst));
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
        case kIROp_MakeDifferentialPair:
            return specializeMakeDifferentialPair(context, as<IRMakeDifferentialPair>(inst));
        case kIROp_DifferentialPairGetDifferential:
            return specializeDifferentialPairGetDifferential(
                context,
                as<IRDifferentialPairGetDifferential>(inst));
        case kIROp_DifferentialPairGetPrimal:
            return specializeDifferentialPairGetPrimal(
                context,
                as<IRDifferentialPairGetPrimal>(inst));
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
        if (elementOfSetType->getSet()->isSingleton())
        {
            auto element = elementOfSetType->getSet()->getElement(0);
            inst->replaceUsesWith(element);
            inst->removeAndDeallocate();
            return true;
        }
        else if (elementOfSetType->getSet()->isEmpty())
        {
            auto poison = builder.emitPoison(inst->getDataType());
            inst->replaceUsesWith(poison);
            inst->removeAndDeallocate();
            return true;
        }
        else if (elementOfSetType->getSet()->isUnbounded())
        {
            // Leave the lookup as-is for unbounded sets, since this
            // will become an actual dynamic lookup at run-time.
            //
            return false;
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
                IRInst* operands[] = {witnessTableInst, inst->getRequirementKey()};

                auto newInst = builder.emitIntrinsicInst(
                    (IRType*)makeTagType(thisInstInfo->getSet()),
                    kIROp_GetTagForMappedSet,
                    2,
                    operands);

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

        if (auto elementOfSetType = as<IRElementOfSetType>(info))
        {
            if (elementOfSetType->getSet()->isSingleton())
            {
                // Found a single possible type. Simple replacement.
                inst->replaceUsesWith(elementOfSetType->getSet()->getElement(0));
                inst->removeAndDeallocate();
                return true;
            }
            else if (elementOfSetType->getSet()->isUnbounded())
            {
                // We'll leave extracts from unbounded sets alone.
                // This typically implies COM interfaces or other run-time dynamic
                // lookups. These are further handled in lowerComInterfaces()
                //
                return false;
            }
            else if (elementOfSetType->getSet()->isEmpty())
            {
                inst->replaceUsesWith(builder.emitPoison(inst->getDataType()));
                inst->removeAndDeallocate();
                return true;
            }
            else
            {
                // Replace with GetElement(specializedInst, 0) -> TagType(tableSet)
                // which retreives a 'tag' (i.e. a run-time identifier for one of the elements
                // of the set)
                //
                auto operand = inst->getOperand(0);
                auto element = builder.emitGetTagFromTaggedUnion(operand);
                inst->replaceUsesWith(element);
                inst->removeAndDeallocate();
                return true;
            }
        }
        else
        {
            SLANG_UNEXPECTED("Unexpected info type for ExtractExistentialWitnessTable");
        }
    }

    bool specializeExtractExistentialValue(IRInst* context, IRExtractExistentialValue* inst)
    {
        SLANG_UNUSED(context);

        auto existential = inst->getOperand(0);
        auto existentialInfo = existential->getDataType();
        auto thisInfo = tryGetInfo(context, inst);
        if (as<IRTaggedUnionType>(existentialInfo))
        {
            IRBuilder builder(inst);
            builder.setInsertAfter(inst);

            auto val = builder.emitGetValueFromTaggedUnion(existential);
            inst->replaceUsesWith(val);
            inst->removeAndDeallocate();
            return true;
        }
        else if (as<IRPoison>(existential))
        {
            IRBuilder builder(inst);
            builder.setInsertAfter(inst);

            inst->replaceUsesWith(builder.emitPoison(inst->getDataType()));
            inst->removeAndDeallocate();
            return true;
        }
        else if (
            as<IRUntaggedUnionType>(thisInfo) &&
            as<IRUntaggedUnionType>(thisInfo)->getSet()->isUnbounded())
        {
            inst->replaceUsesWith(existential);
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
            else if (elementOfSetType->getSet()->isEmpty())
            {
                IRBuilder builder(inst);
                builder.setInsertBefore(inst);
                inst->replaceUsesWith(builder.emitPoison(inst->getDataType()));
                inst->removeAndDeallocate();
                return true;
            }
            else if (elementOfSetType->getSet()->isUnbounded())
            {
                auto unboundedElement =
                    cast<IRUnboundedTypeElement>(elementOfSetType->getSet()->getElement(0));
                IRBuilder builder(inst);
                inst->replaceUsesWith(unboundedElement->getBaseInterfaceType());
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

    // Union two lowered types using a union mask for structural decomposition.
    //
    // `typeUnionMask` is the declared type of the position being aggregated
    // (e.g., the declared parameter type from the interface's func type).
    // Concrete lowered types are first wrapped via makeInfoForConcreteType so
    // that unionPropagationInfo can decompose them structurally.
    //
    IRType* updateType(IRType* typeUnionMask, IRType* currentType, IRType* newType)
    {
        // If our context doesn't allow any specialization.. then it should serve as
        // the final type.
        //
        if (isConcreteType(typeUnionMask))
            return typeUnionMask;

        return (IRType*)unionPropagationInfo(typeUnionMask, currentType, (IRType*)newType);
    }

    IRFuncType* getEffectiveFuncTypeForDispatcher(
        IRWitnessTableSet* tableSet,
        IRFuncSet* resultFuncSet,
        IRFuncType* funcTypeUnionMask)
    {
        SLANG_ASSERT(funcTypeUnionMask);

        List<IRType*>& extraParamTypes = *module->getContainerPool().getList<IRType>();
        extraParamTypes.add((IRType*)makeTagType(tableSet));

        auto innerFuncType = getEffectiveFuncTypeForSet(resultFuncSet, funcTypeUnionMask);
        List<IRType*>& allParamTypes = *module->getContainerPool().getList<IRType>();
        allParamTypes.addRange(extraParamTypes);
        for (auto paramType : innerFuncType->getParamTypes())
            allParamTypes.add(paramType);

        IRBuilder builder(module);
        auto resultFuncType = builder.getFuncType(allParamTypes, innerFuncType->getResultType());

        module->getContainerPool().free(&extraParamTypes);
        module->getContainerPool().free(&allParamTypes);

        return resultFuncType;
    }

    // Get an effective func type to use for the callee.
    // The callee may be a set, in which case, this returns a union-ed functype.
    //
    IRFuncType* getEffectiveFuncTypeForSet(IRFuncSet* calleeSet, IRFuncType* funcTypeUnionMask)
    {
        // The effective func type for a callee set is calculated as follows:
        //
        // (i) we build up the effective parameter types for the callee
        //     by taking the union of each parameter type
        //     for each callee in the set.
        //
        // (ii) build up the effective result type in a similar manner.
        //
        // (iii) add extra tag parameters as necessary:
        //
        //      - if we have multiple callees, then a parameter of TagType(callee) is appended
        //        to the beginning to select the callee.
        //
        //      - if our callee is Specialize inst with set args, then for each
        //        table-set argument, a tag is required as input.
        //

        SLANG_ASSERT(funcTypeUnionMask);

        if (calleeSet->isUnbounded())
        {
            IRUnboundedFuncElement* unboundedFuncElement =
                cast<IRUnboundedFuncElement>(calleeSet->tryGetUnboundedElement());
            return cast<IRFuncType>(unboundedFuncElement->getOperand(0));
        }

        IRBuilder builder(module);

        List<IRType*>& paramTypes = *module->getContainerPool().getList<IRType>();
        IRType* resultType = nullptr;

        List<IRInst*>& calleesToProcess = *module->getContainerPool().getList<IRInst>();
        forEachInSet(module, calleeSet, [&](IRInst* func) { calleesToProcess.add(func); });

        auto updateParamType = [&](Index index, IRInst* paramInfo) -> IRType*
        {
            auto paramTypeUnionMask = funcTypeUnionMask->getParamType((UInt)index);

            if (paramTypes.getCount() <= index)
            {
                // If this index hasn't been seen yet, expand the buffer and initialize
                // the type.
                //
                paramTypes.growToCount(index + 1);
                paramTypes[index] = nullptr;
            }

            {
                // Then, update the existing type using structural union.
                auto [maskDirection, baseTypeUnionMask] =
                    splitParameterDirectionAndType(paramTypeUnionMask);
                auto [newDirection, newType] = splitParameterDirectionAndType((IRType*)paramInfo);

                IRType* currentBaseType = nullptr;
                if (paramTypes[index] != nullptr)
                {
                    auto [_, _currentBaseType] = splitParameterDirectionAndType(paramTypes[index]);
                    currentBaseType = _currentBaseType;
                }

                auto updatedType = updateType(baseTypeUnionMask, currentBaseType, newType);
                SLANG_ASSERT(maskDirection == newDirection);
                paramTypes[index] = fromDirectionAndType(&builder, maskDirection, updatedType);
                return updatedType;
            }
        };

        IRType* resultTypeUnionMask = funcTypeUnionMask->getResultType();

        for (auto context : calleesToProcess)
        {
            auto paramInfos = getParamInfos(context, maybeExpandFuncType(funcTypeUnionMask));

            for (Index i = 0; i < paramInfos.getCount(); i++)
                updateParamType(i, paramInfos[i]);

            if (auto resultInfo = getFuncReturnInfo(context))
            {
                resultType = updateType(resultTypeUnionMask, resultType, (IRType*)resultInfo);
            }
            else if (auto funcType = as<IRFuncType>(context->getDataType()))
            {
                SLANG_ASSERT(isGlobalInst(funcType->getResultType()));
                resultType = updateType(
                    resultTypeUnionMask,
                    resultType,
                    (IRType*)makeInfoForConcreteType(
                        module,
                        funcType->getResultType(),
                        resultTypeUnionMask));
            }
            else if (auto boundFuncContext = as<IRSpecializeExistentialsInFunc>(context))
            {
                auto baseFuncForType = getFuncDefinitionForContext(boundFuncContext);
                auto funcType2 = cast<IRFuncType>(baseFuncForType->getDataType());
                SLANG_ASSERT(isGlobalInst(funcType2->getResultType()));
                resultType = updateType(
                    resultTypeUnionMask,
                    resultType,
                    (IRType*)makeInfoForConcreteType(
                        module,
                        funcType2->getResultType(),
                        resultTypeUnionMask));
            }
            else
            {
                SLANG_UNEXPECTED("Cannot determine result type for context");
            }
        }

        module->getContainerPool().free(&calleesToProcess);

        // Normalize infos into types.
        for (Index i = 0; i < paramTypes.getCount(); i++)
        {
            if (auto lowered = getLoweredType(paramTypes[i]))
                paramTypes[i] = lowered;
        }

        if (auto lowered = getLoweredType(resultType))
            resultType = lowered;


        //
        // Add in extra parameter types for a call to a dynamic generic callee
        //

        List<IRType*>& extraParamTypes = *module->getContainerPool().getList<IRType>();

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

        List<IRType*>& allParamTypes = *module->getContainerPool().getList<IRType>();
        allParamTypes.addRange(extraParamTypes);
        allParamTypes.addRange(paramTypes);

        auto resultFuncType = builder.getFuncType(allParamTypes, resultType);

        module->getContainerPool().free(&paramTypes);
        module->getContainerPool().free(&extraParamTypes);
        module->getContainerPool().free(&allParamTypes);

        return resultFuncType;
    }

    // Get the effective func type for a single callee (func, specialize, or
    // existentialSpecializedFunc). This directly reads the parameter and return info without any
    // union logic, since there is only one callee.
    //
    IRFuncType* getEffectiveFuncType(IRInst* callee)
    {
        IRBuilder builder(module);

        auto paramEffectiveTypes = getEffectiveParamTypes(callee);

        List<IRType*>& paramTypes = *module->getContainerPool().getList<IRType>();
        for (auto paramType : paramEffectiveTypes)
        {
            if (auto lowered = getLoweredType(paramType))
                paramTypes.add(lowered);
            else
                paramTypes.add(paramType);
        }

        // Determine the result type.
        IRType* resultType = nullptr;
        auto returnInfo = getFuncReturnInfo(callee);
        if (auto loweredResult = getLoweredType(returnInfo))
        {
            resultType = loweredResult;
        }
        else if (auto funcType = as<IRFuncType>(callee->getDataType()))
        {
            resultType = funcType->getResultType();
        }
        else if (auto boundFunc = as<IRSpecializeExistentialsInFunc>(callee))
        {
            auto baseFunc = getFuncDefinitionForContext(boundFunc);
            resultType = cast<IRFuncType>(baseFunc->getDataType())->getResultType();
        }
        else
        {
            SLANG_UNEXPECTED("Cannot determine result type for callee");
        }

        // Handle set-specialized generics: add tag types for dynamic WitnessTableSet args.
        List<IRType*>& extraParamTypes = *module->getContainerPool().getList<IRType>();
        if (isSetSpecializedGeneric(callee))
        {
            auto specializeInst = as<IRSpecialize>(callee);
            for (UIndex i = 0; i < specializeInst->getArgCount(); i++)
                if (auto tableSet = as<IRWitnessTableSet>(specializeInst->getArg(i)))
                    extraParamTypes.add((IRType*)makeTagType(tableSet));
        }

        List<IRType*>& allParamTypes = *module->getContainerPool().getList<IRType>();
        allParamTypes.addRange(extraParamTypes);
        allParamTypes.addRange(paramTypes);

        auto resultFuncType = builder.getFuncType(allParamTypes, resultType);

        module->getContainerPool().free(&paramTypes);
        module->getContainerPool().free(&extraParamTypes);
        module->getContainerPool().free(&allParamTypes);

        return resultFuncType;
    }

    // Helper function for specializing calls.
    //
    // For a `Specialize` instruction that has dynamic tag arguments,
    // extract all the tags and return them as a list.
    //
    void addArgsForSetSpecializedGeneric(
        IRSpecialize* specializedCallee,
        List<IRInst*>& outCallArgs)
    {
        for (UInt ii = 0; ii < specializedCallee->getArgCount(); ii++)
        {
            auto specArg = specializedCallee->getArg(ii);
            auto argInfo = specArg->getDataType();

            // Pull all tag-type arguments from the specialization arguments
            // and add them to the call arguments.
            //
            if (auto tagType = as<IRSetTagType>(argInfo))
                if (as<IRWitnessTableSet>(tagType->getSet()))
                    outCallArgs.add(specArg);
        }
    }

    IRInst* maybeSpecializeCalleeType(IRInst* callee)
    {
        if (auto specializeInst = as<IRSpecialize>(callee->getDataType()))
        {
            if (isGlobalInst(specializeInst))
            {
                auto specialized = specializeGeneric(specializeInst);
                if (!specialized)
                    return callee;
                IRBuilder builder(module);
                return builder.replaceOperand(&callee->typeUse, specialized);
            }
        }

        return callee;
    }

    // Filter out `NoneTypeElement` and `NoneWitnessTableElement` from a set (if any exist)
    // and construct a new set of the same kind.
    //
    IRSetBase* filterNoneElements(IRSetBase* set)
    {
        auto setOp = set->getOp();

        IRBuilder builder(module);
        HashSet<IRInst*>& filteredElements = *module->getContainerPool().getHashSet<IRInst>();
        bool containsNone = false;
        forEachInSet(
            module,
            set,
            [&](IRInst* element)
            {
                switch (element->getOp())
                {
                case kIROp_NoneTypeElement:
                case kIROp_NoneWitnessTableElement:
                    containsNone = true;
                    break;
                default:
                    filteredElements.add(element);
                    break;
                }
            });

        if (!containsNone)
        {
            // Return the same set if there were no None elements
            module->getContainerPool().free(&filteredElements);
            return set;
        }

        // Create a new set without the filtered elements
        auto newFuncSet = cast<IRSetBase>(builder.getSet(setOp, filteredElements));
        module->getContainerPool().free(&filteredElements);
        return newFuncSet;
    }

    // Represents a single action to apply when building a dispatcher.
    // Actions are collected by walking the callee operand chain backward
    // and are applied in order to each witness table entry.
    //
    struct DispatchAction
    {
        enum class Kind
        {
            Lookup,           // From IRGetTagForMappedSet: look up an entry by key
            Specialize,       // From IRGetTagForSpecializedSet: specialize a generic
            BindExistentials, // From calleeSet inspection: bind existential arguments
        };

        Kind kind;
        IRStructKey* lookupKey = nullptr;
        List<IRInst*> specArgs;
        List<IRInst*> bindings;
    };

    // Walk the callee operand chain backward, collecting DispatchActions
    // and returning the base tag operand (whose type is SetTagType(witnessTableSet)).
    //
    // Also inspects the calleeSet for IRSpecializeExistentialsInFunc and appends a
    // BindExistentials action if any are found.
    //
    // Additionally, any spec args that are tags of witness table sets are
    // added to extraCallArgs (they become extra call arguments for dispatch selection).
    //
    IRInst* collectDispatchActions(
        IRInst* callee,
        IRFuncSet* calleeSet,
        IRCall* inst,
        List<DispatchAction>& actions,
        List<IRInst*>& extraCallArgs)
    {
        IRInst* baseOperand = callee;

        while (true)
        {
            if (auto mapped = as<IRGetTagForMappedSet>(baseOperand))
            {
                DispatchAction action;
                action.kind = DispatchAction::Kind::Lookup;
                action.lookupKey = cast<IRStructKey>(mapped->getOperand(1));
                actions.add(action);
                baseOperand = mapped->getOperand(0);
            }
            else if (auto specialized = as<IRGetTagForSpecializedSet>(baseOperand))
            {
                DispatchAction action;
                action.kind = DispatchAction::Kind::Specialize;
                for (UInt argIdx = 1; argIdx < specialized->getOperandCount(); ++argIdx)
                {
                    auto arg = specialized->getOperand(argIdx);
                    if (auto tagType = as<IRSetTagType>(arg->getDataType()))
                    {
                        SLANG_ASSERT(!tagType->getSet()->isSingleton());
                        if (as<IRWitnessTableSet>(tagType->getSet()))
                        {
                            extraCallArgs.add(arg);
                            action.specArgs.add(tagType->getSet());
                        }
                        else
                        {
                            action.specArgs.add(tagType->getSet());
                        }
                    }
                    else
                    {
                        SLANG_ASSERT(isGlobalInst(arg));
                        action.specArgs.add(arg);
                    }
                }
                actions.add(action);
                baseOperand = specialized->getOperand(0);
            }
            else
            {
                break;
            }
        }

        actions.reverse();

        // Check calleeSet for bindings (IRSpecializeExistentialsInFunc).
        IRBuilder builder(module);
        List<IRInst*> bindings;
        for (UInt i = 0; i < inst->getOperandCount() - 1; i++)
            bindings.add(builder.getVoidValue());

        bool hasNonTrivialBindings = false;
        forEachInSet(
            module,
            calleeSet,
            [&](IRInst* element)
            {
                if (auto existentialSpecializedFunc = as<IRSpecializeExistentialsInFunc>(element))
                {
                    for (UInt i = 1; i < existentialSpecializedFunc->getOperandCount(); i++)
                    {
                        if (!as<IRVoidLit>(bindings[i - 1]))
                        {
                            SLANG_ASSERT(
                                bindings[i - 1] == existentialSpecializedFunc->getOperand(i));
                        }
                        else
                        {
                            bindings[i - 1] = existentialSpecializedFunc->getOperand(i);
                        }
                    }
                }
            });

        for (auto binding : bindings)
        {
            if (!as<IRVoidLit>(binding))
            {
                hasNonTrivialBindings = true;
                break;
            }
        }

        if (hasNonTrivialBindings)
        {
            DispatchAction action;
            action.kind = DispatchAction::Kind::BindExistentials;
            action.bindings = bindings;
            actions.add(action);
        }

        return baseOperand;
    }

    // Create a dispatcher function that dispatches to the correct function based on
    // a witness table tag.
    //
    // For each witness table in the set, applies the full list of DispatchActions
    // in order (lookup, specialize, bind) to resolve the concrete function, then
    // builds a dispatch function (switch-case) over all concrete functions.
    //
    // Returns the dispatch function, or nullptr if the dispatcher has no uses.
    //
    IRFunc* getDispatcher(
        IRWitnessTableSet* witnessTableSet,
        IRFuncType* dispatchFuncType,
        List<DispatchAction>& actions,
        WorkQueue<IRInst*>& globalsWorkList)
    {
        IRBuilder builder(module);

        Dictionary<IRInst*, std::pair<IRInst*, IRFuncType*>> elements;
        forEachInSet(
            module,
            witnessTableSet,
            [&](IRInst* table)
            {
                auto tag = builder.emitGetTagOfElementInSet(
                    builder.getSetTagType(witnessTableSet),
                    table,
                    witnessTableSet);

                IRInst* val = table;

                for (auto& action : actions)
                {
                    switch (action.kind)
                    {
                    case DispatchAction::Kind::Lookup:
                        val = findWitnessTableEntry(cast<IRWitnessTable>(val), action.lookupKey);
                        break;

                    case DispatchAction::Kind::Specialize:
                        {
                            auto generic = cast<IRGeneric>(val);
                            auto genericType = cast<IRGeneric>(generic->getDataType());
                            auto innerType = getGenericReturnVal(genericType);

                            if (as<IRFuncType>(innerType))
                            {
                                auto specializedFuncType = (IRType*)specializeGeneric(
                                    cast<IRSpecialize>(builder.emitSpecializeInst(
                                        builder.getTypeKind(),
                                        generic->getDataType(),
                                        action.specArgs.getCount(),
                                        action.specArgs.getBuffer())));

                                val = builder.emitSpecializeInst(
                                    specializedFuncType,
                                    generic,
                                    action.specArgs.getCount(),
                                    action.specArgs.getBuffer());
                            }
                            else if (as<IRWitnessTableType>(innerType))
                            {
                                auto specializedWitnessTableType = (IRType*)specializeGeneric(
                                    cast<IRSpecialize>(builder.emitSpecializeInst(
                                        builder.getTypeKind(),
                                        generic->getDataType(),
                                        action.specArgs.getCount(),
                                        action.specArgs.getBuffer())));

                                val = builder.emitSpecializeInst(
                                    specializedWitnessTableType,
                                    generic,
                                    action.specArgs.getCount(),
                                    action.specArgs.getBuffer());

                                val = translationContext.resolveInst(val);
                            }
                            else
                            {
                                SLANG_UNEXPECTED(
                                    "Unexpected generic return type for specialization");
                            }
                            break;
                        }

                    case DispatchAction::Kind::BindExistentials:
                        {
                            List<IRInst*> boundFuncOperands;
                            boundFuncOperands.add(val);
                            for (auto binding : action.bindings)
                                boundFuncOperands.add(binding);

                            auto existentialSpecializedFunc =
                                cast<IRSpecializeExistentialsInFunc>(builder.emitIntrinsicInst(
                                    nullptr,
                                    kIROp_SpecializeExistentialsInFunc,
                                    (UInt)boundFuncOperands.getCount(),
                                    boundFuncOperands.getBuffer()));

                            auto loweredFunc =
                                lowerSpecializeExistentialsInFunc(existentialSpecializedFunc);
                            if (loweredFunc)
                                val = loweredFunc;
                            break;
                        }
                    }
                }

                globalsWorkList.enqueue(val);

                auto effectiveFuncType = getEffectiveFuncType(val);
                elements.add(tag, {val, effectiveFuncType});
            });

        if (dispatchFuncType == nullptr)
            return nullptr;

        auto dispatchFunc = createDispatchFunc(dispatchFuncType, elements);

        // Add a name hint based on the actions.
        {
            builder.setInsertBefore(dispatchFunc);
            StringBuilder sb;
            sb << "s_dispatch";
            for (auto& action : actions)
            {
                switch (action.kind)
                {
                case DispatchAction::Kind::Lookup:
                    if (auto nameHint = action.lookupKey->findDecoration<IRNameHintDecoration>())
                        sb << "_" << nameHint->getName();
                    break;
                case DispatchAction::Kind::Specialize:
                    for (auto specArg : action.specArgs)
                    {
                        sb << "_";
                        getTypeNameHint(sb, specArg);
                    }
                    break;
                default:
                    break;
                }
            }

            builder.addNameHintDecoration(dispatchFunc, sb.getUnownedSlice());
        }

        return dispatchFunc;
    }

    bool specializeCall(IRInst* context, IRCall* inst, WorkQueue<IRInst*>& globalsWorkList)
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
        // The callee can only be in a few specific patterns:
        //
        // 1. If the callee is already a concrete function, there's nothing to do
        //
        // 2. If the callee is a dynamic inst of tag type with multiple callees,
        //    we walk the callee operand chain backward (through GetTagForMappedSet,
        //    GetTagForSpecializedSet, etc.) collecting a list of DispatchActions,
        //    until we reach a base tag of a witness table set. The dispatcher then
        //    applies these actions to each witness table to resolve the concrete function.
        //
        //    For example, a chain like:
        //          let tableTag : TagType(witnessTableSet) = /* ... */;
        //          let tag : TagType(genericSet) = GetTagForMappedSet(tableTag, key);
        //          let specializedTag :
        //               TagType(funcSet) = GetTagForSpecializedSet(tag, specArgs...);
        //          let val = Call(specializedTag, arg1, arg2, ...);
        //    becomes:
        //          let tableTag : TagType(witnessTableSet) = /* ... */;
        //          let dispatcher : FuncType(...) = /* dispatcher over witnessTableSet */;
        //          let val = Call(dispatcher, tableTag, arg1, arg2, ...);
        //    where the dispatcher absorbs the lookup + specialization into itself.
        //
        // 3. If the callee is a Specialize of a concrete generic with dynamic
        //    specialization arguments (set-specialized generic), then the arguments
        //    are lowered into extra call parameters.
        //
        // After the callee has been selected, we handle the argument types:
        //    It is possible that the parameters in the callee have been specialized
        //    to accept a super-set of types compared to the arguments from this call site
        //
        //    In this case, we just upcast them using `upcastSet` before
        //    creating a new call inst
        //

        auto callee = inst->getCallee();

        IRFuncType* effectiveFuncType = nullptr;

        if (isNoneCallee(callee))
            return false;

        // Check for invalid existential specialization (e.g. specialize with
        // interface type argument) before resolving the callee. These callees
        // may have dangling operands after the specialize pass lowered their
        // witness-lookup bases.
        if (isInvalidExistentialSpecialization(callee))
        {
            emitExistentialSpecializationDiagnostic(callee, inst->sourceLoc, inst);
            return false;
        }

        if (isGlobalInst(callee))
            callee = translationContext.resolveInst(callee);

        List<IRInst*>& callArgs = *module->getContainerPool().getList<IRInst>();

        // This is a bit of a workaround for specialized callee's
        // whose function types haven't been specialized yet (can
        // occur for concrete IRSpecialize insts that are created
        // during the specializeing process).
        //
        callee = maybeSpecializeCalleeType(callee);

        // If we're calling using a tag, place a call to the set,
        // with the tag as the first argument. So the callee is
        // the set itself.
        //
        if (auto setTag = as<IRSetTagType>(callee->getDataType()))
        {
            // Expect a callee-set to be associated with call.
            // The info-propagation phase may not record this call site when a
            // struct with an existential field is passed by value — bail out
            // gracefully instead of crashing on a missing dictionary entry.
            auto callSiteInfoPtr = this->callSiteInfo.tryGetValue(InstWithContext(context, inst));
            if (!callSiteInfoPtr || !*callSiteInfoPtr)
            {
                module->getContainerPool().free(&callArgs);
                return false;
            }
            auto calleeSet = cast<IRElementOfSetType>(*callSiteInfoPtr)->getSet();
            if (!calleeSet->isSingleton() && !calleeSet->isEmpty())
            {
                // Multiple callees case:
                //
                // Walk the callee operand chain backward, collecting dispatch
                // actions (lookups, specializations, bindings) that can be
                // absorbed into the dispatcher. This generalizes the previously
                // separate GetTagForMappedSet and GetTagForSpecializedSet cases.

                List<DispatchAction> actions;
                auto tableTag = collectDispatchActions(
                    callee,
                    cast<IRFuncSet>(calleeSet),
                    inst,
                    actions,
                    callArgs);

                auto tableSet =
                    cast<IRWitnessTableSet>(cast<IRSetTagType>(tableTag->getDataType())->getSet());

                if (SLANG_FAILED(rejectSpecializeOnlyInterface(tableSet, inst->sourceLoc)))
                {
                    module->getContainerPool().free(&callArgs);
                    return false;
                }

                auto contextFuncTypePtr =
                    this->callSiteFuncType.tryGetValue(InstWithContext(context, inst));
                SLANG_ASSERT(contextFuncTypePtr);
                auto funcTypeUnionMask = *contextFuncTypePtr;

                effectiveFuncType = getEffectiveFuncTypeForDispatcher(
                    tableSet,
                    cast<IRFuncSet>(calleeSet),
                    maybeExpandFuncType(funcTypeUnionMask));

                callee = getDispatcher(tableSet, effectiveFuncType, actions, globalsWorkList);

                if (shouldReportDynamicDispatchSites)
                {
                    // Collect specArgs from the actions for reporting.
                    List<IRInst*> allSpecArgs;
                    bool hasSpecArgs = false;
                    for (auto& action : actions)
                    {
                        if (action.kind == DispatchAction::Kind::Specialize)
                        {
                            allSpecArgs.addRange(action.specArgs);
                            hasSpecArgs = true;
                        }
                    }

                    if (hasSpecArgs)
                        reportSpecializedDispatchLocation(
                            module,
                            sink,
                            inst->getCalleeUse(),
                            tableSet,
                            allSpecArgs);
                    else
                        reportDispatchLocation(module, sink, inst->getCalleeUse(), tableSet);
                }

                callArgs.add(tableTag);
            }
            else if (isSetSpecializedGeneric(calleeSet->getElement(0)))
            {
                IRBuilder builder(module);
                builder.setInsertInto(module);

                // Check both the specialize instruction and the selected callee for
                // disallowed existential specialization.
                //
                auto selectedCallee = setTag->getSet()->getElement(0);
                if (isInvalidExistentialSpecialization(callee) ||
                    isInvalidExistentialSpecialization(selectedCallee))
                {
                    emitExistentialSpecializationDiagnostic(callee, inst->sourceLoc, inst);
                    module->getContainerPool().free(&callArgs);
                    return false;
                }
                else
                {
                    // Otherwise, we have a single element which is a set specialized generic.
                    // Add in the arguments for the set specialization.
                    //
                    addArgsForSetSpecializedGeneric(cast<IRSpecialize>(callee), callArgs);
                    callee = selectedCallee;
                    effectiveFuncType = getEffectiveFuncType(callee);
                    globalsWorkList.enqueue(callee);
                }
            }
            else if (auto specCallee = as<IRSpecialize>(callee))
            {
                // The callee is an IRSpecialize that escaped both the dispatch-action
                // and set-specialized-generic resolution paths. This happens when an
                // existential type flows into an unconstrained generic (no interface
                // constraint), so the typeflow pass cannot generate dispatch code.
                emitExistentialSpecializationDiagnostic(specCallee, inst->sourceLoc, inst);
                module->getContainerPool().free(&callArgs);
                return false;
            }
            else
            {
                SLANG_UNEXPECTED(
                    "Unexpected operand type for type-flow specialization of Call inst");
            }
        }
        else if (as<IRPoison>(callee))
        {
            // Occasionally, we will determine that there are absolutely no possible callees
            // for a call site. This typically happens to impossible branches.
            //
            // If this happens, the inst representing the callee would have been replaced
            // with a poison value. In this case, we're simply going to replace the entire call
            // with a default-constructed value of the appropriate type.
            //
            // Note that it doesn't matter what we replace it with since this code should be
            // effectively unreachable.
            //
            IRBuilder builder(context);
            builder.setInsertBefore(inst);
            auto defaultVal = builder.emitDefaultConstruct(inst->getDataType());
            inst->replaceUsesWith(defaultVal);
            inst->removeAndDeallocate();
            module->getContainerPool().free(&callArgs);
            return true;
        }
        else if (isGlobalInst(callee))
        {
            auto callSiteInfoPtr = this->callSiteInfo.tryGetValue(InstWithContext(context, inst));
            if (!callSiteInfoPtr || !*callSiteInfoPtr)
            {
                module->getContainerPool().free(&callArgs);
                return false;
            }
            auto calleeSet = as<IRElementOfSetType>(*callSiteInfoPtr)->getSet();
            SLANG_ASSERT(calleeSet->isSingleton());

            if (isIntrinsic(callee))
                effectiveFuncType = as<IRFuncType>(callee->getDataType());
            else
                effectiveFuncType = getEffectiveFuncType(calleeSet->getElement(0));

            // If we're dealing with bindings, materialize a new function now.
            if (as<IRSpecializeExistentialsInFunc>(calleeSet->getElement(0)))
            {
                // If our callee is a SpecializeExistentialsInFunc, we need to lower it to get a
                // concrete
                // function.
                callee = lowerSpecializeExistentialsInFunc(
                    as<IRSpecializeExistentialsInFunc>(calleeSet->getElement(0)));
            }
            else
            {
                callee = calleeSet->getElement(0);
            }

            globalsWorkList.enqueue(callee);
        }

        auto contextFuncTypePtr =
            this->callSiteFuncType.tryGetValue(InstWithContext(context, inst));
        if (!contextFuncTypePtr || !isGlobalInst(callee))
        {

            module->getContainerPool().free(&callArgs);
            return false;
        }

        SLANG_ASSERT(effectiveFuncType);
        auto funcTypeUnionMask = maybeExpandFuncType(*contextFuncTypePtr);

        // First, we'll legalize all operands by upcasting if necessary.
        // This needs to be done even if the callee is not a set.
        //
        UCount extraArgCount = callArgs.getCount();
        for (UInt i = 0; i < inst->getArgCount(); i++)
        {
            auto arg = inst->getArg(i);
            const auto [paramDirection, paramType] =
                splitParameterDirectionAndType(effectiveFuncType->getParamType(i + extraArgCount));
            if (isConcreteType(funcTypeUnionMask->getParamType(i)))
            {
                callArgs.add(arg);
                continue;
            }

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

            // Upcasting of out-parameters is the responsibility of the callee.
            case ParameterDirectionInfo::Kind::Out:

            // For all other modes, sets must match ('subtyping' is not allowed)
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

        if (changed)
        {
            IRBuilderSourceLocRAII builderSourceLocRAII(&builder, inst->sourceLoc);
            auto newCall =
                builder.emitCallInst(effectiveFuncType->getResultType(), callee, callArgs);
            inst->replaceUsesWith(newCall);
            inst->removeAndDeallocate();
        }
        else if (effectiveFuncType->getResultType() != inst->getFullType())
        {
            // If we didn't change the callee or the arguments, we still might
            // need to update the result type.
            //
            inst->setFullType(effectiveFuncType->getResultType());
            changed = true;
        }

        module->getContainerPool().free(&callArgs);

        return changed;
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

    bool specializeMakeArray(IRInst* context, IRMakeArray* inst)
    {
        // The main thing to handle here is that we might have specialized
        // the element type of the array, so we need to upcast the elements
        // if necessary.
        //
        auto arrayInfo = tryGetInfo(context, inst);
        if (!arrayInfo)
            return false;

        auto arrayType = as<IRArrayType>(arrayInfo);
        auto elementType = arrayType->getElementType();

        // Reinterpret any of the arguments as necessary.
        bool changed = false;
        for (UIndex i = 0; i < inst->getOperandCount(); i++)
        {
            auto arg = inst->getOperand(i);
            IRBuilder builder(context);
            builder.setInsertBefore(inst);
            auto newArg = upcastSet(&builder, arg, elementType);

            if (arg != newArg)
            {
                changed = true;
                inst->setOperand(i, newArg);
            }
        }

        IRBuilder builder(module);
        builder.replaceOperand(&inst->typeUse, getLoweredType(arrayInfo));
        return changed;
    }

    bool specializeMakeArrayFromElement(IRInst* context, IRMakeArrayFromElement* inst)
    {
        // Similar to MakeArray, but only a single element value is provided
        // that will be replicated to all array positions.
        //
        auto arrayInfo = tryGetInfo(context, inst);
        if (!arrayInfo)
            return false;

        auto arrayType = as<IRArrayType>(arrayInfo);
        auto elementType = arrayType->getElementType();

        // Reinterpret the single element argument as necessary.
        auto arg = inst->getOperand(0);
        IRBuilder builder(context);
        builder.setInsertBefore(inst);
        auto newArg = upcastSet(&builder, arg, elementType);

        bool changed = false;
        if (arg != newArg)
        {
            changed = true;
            inst->setOperand(0, newArg);
        }

        IRBuilder moduleBuilder(module);
        moduleBuilder.replaceOperand(&inst->typeUse, getLoweredType(arrayInfo));
        return changed;
    }

    bool specializeMakeTuple(IRInst* context, IRInst* inst)
    {
        // The main thing to handle here is that we might have specialized
        // the element types of the tuple, so we need to upcast the elements
        // if necessary.
        //
        auto tupleInfo = tryGetInfo(context, inst);
        if (!tupleInfo)
            return false;

        auto tupleType = as<IRTupleType>(tupleInfo);
        if (!tupleType)
            return false;

        UInt elementCount = tupleType->getOperandCount();

        // Reinterpret any of the arguments as necessary.
        bool changed = false;
        for (UInt i = 0; i < elementCount && i < inst->getOperandCount(); i++)
        {
            auto arg = inst->getOperand(i);
            auto elementType = (IRType*)tupleType->getOperand(i);
            IRBuilder builder(context);
            builder.setInsertBefore(inst);
            auto newArg = upcastSet(&builder, arg, elementType);

            if (arg != newArg)
            {
                changed = true;
                inst->setOperand(i, newArg);
            }
        }

        IRBuilder builder(module);
        builder.replaceOperand(&inst->typeUse, getLoweredType(tupleInfo));
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
            witnessTableTag = builder.emitGetTagOfElementInSet(
                (IRType*)makeTagType(tableSet),
                witnessTable,
                tableSet);
        }
        else if (as<IRSetTagType>(inst->getWitnessTable()->getDataType()))
        {
            witnessTableTag = upcastSet(&builder, inst->getWitnessTable(), makeTagType(tableSet));
        }

        // Create the appropriate any-value type
        auto effectiveType = typeSet->isSingleton()
                                 ? (IRType*)typeSet->getElement(0)
                                 : builder.getUntaggedUnionType((IRType*)typeSet);

        // Pack the value
        auto packedValue = as<IRUntaggedUnionType>(effectiveType)
                               ? builder.emitPackAnyValue(effectiveType, inst->getWrappedValue())
                               : inst->getWrappedValue();

        auto taggedUnionType = getLoweredType(taggedUnion);

        inst->replaceUsesWith(builder.emitMakeTaggedUnion(
            taggedUnionType,
            builder.emitPoison(makeTagType(typeSet)),
            witnessTableTag,
            packedValue));
        inst->removeAndDeallocate();
        return true;
    }

    bool specializeWrapExistential(IRInst* context, IRWrapExistential* inst)
    {
        SLANG_UNUSED(context);
        inst->replaceUsesWith(inst->getWrappedValue());
        inst->removeAndDeallocate();
        return true;
    }

    bool specializeMakeDifferentialPair(IRInst* context, IRMakeDifferentialPair* inst)
    {
        if (auto tupleType = as<IRTupleType>(tryGetInfo(context, inst)); tupleType)
        {
            IRBuilder builder(inst);
            builder.setInsertBefore(inst);
            auto newInst = builder.emitMakeTuple(inst->getPrimal(), inst->getDifferential());
            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();
            return true;
        }

        return false;
    }

    bool specializeDifferentialPairGetDifferential(
        IRInst* context,
        IRDifferentialPairGetDifferential* inst)
    {
        SLANG_UNUSED(context);
        if (auto tupleType = as<IRTupleType>(inst->getBase()->getDataType()))
        {
            IRBuilder builder(inst);
            builder.setInsertBefore(inst);
            auto newInst = builder.emitGetTupleElement(
                (IRType*)getLoweredType(tupleType->getOperand(1)),
                inst->getBase(),
                1);
            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();
            return true;
        }

        return false;
    }

    bool specializeDifferentialPairGetPrimal(IRInst* context, IRDifferentialPairGetPrimal* inst)
    {
        SLANG_UNUSED(context);
        if (auto tupleType = as<IRTupleType>(inst->getBase()->getDataType()))
        {
            IRBuilder builder(inst);
            builder.setInsertBefore(inst);
            auto newInst = builder.emitGetTupleElement(
                (IRType*)getLoweredType(tupleType->getOperand(0)),
                inst->getBase(),
                0);
            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();
            return true;
        }

        return false;
    }

    bool specializeCreateExistentialObject(IRInst* context, IRCreateExistentialObject* inst)
    {
        // A CreateExistentialObject uses an user-provided ID to create an object.
        // Note that this ID is not the same as the tags we use. The user-provided ID must be
        // compared against the SequentialID, which is a globally consistent & public ID present
        // on the witness tables.
        //
        // The tags are a locally consistent ID whose semantics are only meaningful within the
        // function. We use a special op `GetTagFromSequentialID` to convert from the
        // user-provided global ID to a local tag ID.
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
        auto set = taggedUnionType->getTypeSet();
        if (!set->isSingleton())
        {
            packedValue = builder.emitPackAnyValue(
                (IRType*)builder.getUntaggedUnionType(set),
                inst->getValue());
        }
        else
        {
            packedValue = builder.emitReinterpret(
                (IRType*)builder.getUntaggedUnionType(set),
                inst->getValue());
        }

        auto newInst = builder.emitMakeTaggedUnion(
            (IRType*)taggedUnionType,
            builder.emitPoison(makeTagType(taggedUnionType->getTypeSet())),
            translatedTag,
            packedValue);

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
        // result into a set of all available implementations of this
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
                auto taggedUnionType = cast<IRTaggedUnionType>(specializedValType);
                IRInst* castArgs[] = {
                    bufferHandle,
                    taggedUnionType->getWitnessTableSet(),
                    taggedUnionType->getTypeSet()};
                auto newHandle = builder.emitIntrinsicInst(
                    builder.getPtrType(specializedValType),
                    kIROp_CastInterfaceToTaggedUnionPtr,
                    3,
                    castArgs);
                IRInst* newLoadOperands[] = {newHandle, inst->getOperand(1)};
                auto newLoad = builder.emitIntrinsicInst(
                    specializedValType,
                    inst->getOp(),
                    2,
                    newLoadOperands);

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
        // operands with their set variants.
        //
        // If we're dealing with a function, there are two cases:
        // - A single function when dynamic specialization arguments.
        //      Removing the dynamic tag information will result in the eventual 'call'
        //      inst not have access to these insts.
        //
        //      Instead, we'll just replace the type, and retain the `Specialize` inst with
        //      the dynamic args. It will be specialized out in `specializeCall` instead.
        //
        // - A set of functions with concrete specialization arguments.
        //     In this case, we will emit an instruction to map from the input generic set
        //     to the output specialized set via `GetTagForSpecializedSet`.
        //     This inst encodes the key-value mapping in its operands:
        //          e.g.(input_tag, key0, value0, key1, value1, ...)
        //
        // - The case where there is a set of functions with dynamic specialization arguments
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

        // We'll emit a dynamic tag inst if the result is a func set with multiple elements
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
                        // If the result is a singleton set, we can just
                        // replace the type (if necessary) and be done with it.
                        return replaceType(context, inst);
                    }
                    else
                    {
                        // Otherwise, we'll emit a tag mapping instruction.
                        IRBuilder builder(inst);
                        setInsertBeforeOrdinaryInst(&builder, inst);

                        List<IRInst*>& specOperands = *module->getContainerPool().getList<IRInst>();
                        specOperands.add(inst->getBase());

                        for (UInt ii = 0; ii < inst->getArgCount(); ii++)
                            specOperands.add(inst->getArg(ii));

                        auto newInst = builder.emitIntrinsicInst(
                            (IRType*)makeTagType(elementOfSetType->getSet()),
                            kIROp_GetTagForSpecializedSet,
                            specOperands.getCount(),
                            specOperands.getBuffer());
                        module->getContainerPool().free(&specOperands);

                        inst->replaceUsesWith(newInst);
                        inst->removeAndDeallocate();
                        return true;
                    }
                }
                else
                {
                    SLANG_UNEXPECTED("Expected element-of-set type for function specialization");
                }
            }
        }

        // For all other specializations, we'll 'drop' the dynamic tag information.
        bool changed = false;
        List<IRInst*>& args = *module->getContainerPool().getList<IRInst>();
        for (UIndex i = 0; i < inst->getArgCount(); i++)
        {
            auto arg = inst->getArg(i);
            auto argDataType = arg->getDataType();
            if (auto setTagType = as<IRSetTagType>(argDataType))
            {
                // If this is a tag type, replace with set.
                changed = true;
                if (as<IRWitnessTableSet>(setTagType->getSet()))
                {
                    args.add(setTagType->getSet());
                }
                else if (auto typeSet = as<IRTypeSet>(setTagType->getSet()))
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

        // Is our base operand a set-tag-type?
        // This needs a slight change..
        //
        if (as<IRSetTagType>(inst->getBase()->getDataType()) &&
            as<IRWitnessTableType>(inst->getDataType()))
        {
            if (auto info = tryGetInfo(context, inst))
            {
                auto thisInstInfo = cast<IRElementOfSetType>(info);

                List<IRInst*> operands = {inst->getBase()};
                for (UInt i = 0; i < inst->getArgCount(); i++)
                    operands.add(inst->getArg(i));

                IRBuilder builder(inst);
                builder.setInsertBefore(inst);
                auto newInst = builder.emitIntrinsicInst(
                    (IRType*)makeTagType(thisInstInfo->getSet()),
                    kIROp_GetTagForSpecializedSet,
                    operands.getCount(),
                    operands.getBuffer());
                inst->replaceUsesWith(newInst);
                inst->removeAndDeallocate();
                module->getContainerPool().free(&args);
                return true;
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
        }
        module->getContainerPool().free(&args);
        return changed;
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
        // During specialization, we convert all "element-of" types into
        // run-time tag types so that we can obtain and use this information.
        //
        // Thus, `GetElementFromTag` simply becomes a no-op. Any instructions using
        // the result of `GetElementFromTag` should be specialized accordingly to
        // use the tag operand.
        //
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
            if ((as<IRInterfaceType>(ptrValType)) && !isComInterfaceType(ptrValType) &&
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
                auto taggedUnionType = cast<IRTaggedUnionType>(specializedType);
                IRInst* castArgs[] = {
                    loadPtr,
                    taggedUnionType->getWitnessTableSet(),
                    taggedUnionType->getTypeSet()};
                auto newLoadPtr = builder.emitIntrinsicInst(
                    builder.getPtrTypeWithAddressSpace(specializedType, loadPtrType),
                    kIROp_CastInterfaceToTaggedUnionPtr,
                    3,
                    castArgs);
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

        if (as<IRInterfaceType>(ptrInfo) && as<IRTaggedUnionType>(inst->getVal()->getDataType()))
        {
            // Cast the interface pointer to a tagged-union pointer, and then emit the store.
            auto taggedUnionType = cast<IRTaggedUnionType>(inst->getVal()->getDataType());
            IRInst* castArgs[] = {
                ptr,
                taggedUnionType->getWitnessTableSet(),
                taggedUnionType->getTypeSet()};
            auto newPtr = builder.emitIntrinsicInst(
                builder.getPtrTypeWithAddressSpace(
                    inst->getVal()->getDataType(),
                    as<IRPtrTypeBase>(ptr->getDataType())),
                kIROp_CastInterfaceToTaggedUnionPtr,
                3,
                castArgs);
            builder.replaceOperand(inst->getPtrUse(), newPtr);
            return true;
        }

        auto specializedVal = upcastSet(&builder, inst->getVal(), ptrInfo);

        if (specializedVal != inst->getVal())
        {
            // If the value was changed, we need to update the store instruction.
            builder.replaceOperand(inst->getValUse(), specializedVal);
            return true;
        }

        return false;
    }

    bool specializeSwizzledStore(IRInst* context, IRSwizzledStore* inst)
    {
        // Similar to `specializeStore`, we upcast the source elements
        // to match the destination pointer's element types before storing.
        //
        auto dest = inst->getDest();
        auto source = inst->getSource();
        auto destPtrType = as<IRPtrTypeBase>(dest->getDataType());
        if (!destPtrType)
            return false;

        auto destValueType = as<IRTupleType>(destPtrType->getValueType());
        if (!destValueType)
            return false;

        auto sourceType = as<IRTupleType>(source->getDataType());

        IRBuilder builder(context);
        builder.setInsertBefore(inst);

        bool hasChanges = false;

        // Build the new source value with upcasted elements
        List<IRInst*> newSourceElements;
        for (UInt i = 0; i < inst->getElementCount(); i++)
        {
            auto elementIndex = inst->getElementIndex(i);
            if (auto intLit = as<IRIntLit>(elementIndex))
            {
                auto destIndex = (Index)intLit->getValue();
                auto destElemType = (IRType*)destValueType->getOperand(destIndex);

                // Extract source element
                IRInst* sourceElement = nullptr;
                if (sourceType)
                {
                    auto sourceElemType = (IRType*)sourceType->getOperand(i);
                    sourceElement = builder.emitGetTupleElement(sourceElemType, source, i);
                }
                else
                {
                    // Source is a single value
                    sourceElement = source;
                }

                // Upcast the element to match destination type
                auto upcastedElement = upcastSet(&builder, sourceElement, destElemType);
                if (upcastedElement != sourceElement)
                    hasChanges = true;

                newSourceElements.add(upcastedElement);
            }
            else
            {
                return false;
            }
        }

        if (hasChanges)
        {
            // Build a new source tuple if needed
            IRInst* newSource = nullptr;
            if (sourceType)
            {
                // Build new tuple type for the upcasted elements
                List<IRType*> newElemTypes;
                for (auto elem : newSourceElements)
                    newElemTypes.add(elem->getFullType());
                auto newSourceType = builder.getTupleType(newElemTypes);
                newSource = builder.emitMakeTuple(newSourceType, newSourceElements);
            }
            else
            {
                // Single element case
                newSource = newSourceElements[0];
            }

            builder.replaceOperand(inst->getOperands() + 1, newSource);
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
            IRInst* args[] = {interfaceType, arg};
            auto newInst = builder.emitIntrinsicInst(
                (IRType*)builder.getUIntType(),
                kIROp_GetSequentialIDFromTag,
                2,
                args);

            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();
            return true;
        }

        return false;
    }

    bool specializeIsType(IRInst* context, IRIsType* inst)
    {
        // The is-type checks equality between two witness tables
        //
        // We'll turn this into a tag comparison by extracting the tag
        // for a specific element in the set, and comparing that to the
        // dynamic witness table tag.
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

            auto singletonWitnessTableTagType =
                makeTagType(builder.getSingletonSet(kIROp_WitnessTableSet, noneWitnessTable));
            IRInst* tableTag = builder.emitGetTagOfElementInSet(
                (IRType*)singletonWitnessTableTagType,
                noneWitnessTable,
                taggedUnionType->getWitnessTableSet());

            auto singletonTypeTagType =
                makeTagType(builder.getSingletonSet(kIROp_TypeSet, builder.getNoneTypeElement()));
            IRInst* typeTag = builder.emitGetTagOfElementInSet(
                (IRType*)singletonTypeTagType,
                builder.getNoneTypeElement(),
                taggedUnionType->getTypeSet());

            auto newTuple = builder.emitMakeTaggedUnion(
                (IRType*)taggedUnionType,
                typeTag,
                tableTag,
                builder.emitDefaultConstruct(makeUntaggedUnionType(taggedUnionType->getTypeSet())));

            inst->replaceUsesWith(newTuple);
            propagationMap[InstWithContext(context, newTuple)] =
                builder.getWeakUse(taggedUnionType);
            inst->removeAndDeallocate();

            return true;
        }

        return replaceType(context, inst);
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

        return replaceType(context, inst);
    }

    bool specializeGetOptionalValue(IRInst* context, IRGetOptionalValue* inst)
    {
        SLANG_UNUSED(context);
        if (auto srcTaggedUnionType =
                as<IRTaggedUnionType>(inst->getOptionalOperand()->getDataType()))
        {
            // How we handle `GetOptionalValue` depends on whether our analysis
            // shows that there can be a 'none' type in the input.
            //
            // If not, we can do a simple replace with the operand since the
            // input and output are equivalent (this is a no-op)
            //
            // If so, then we will cast the tagged-union's tag to a sub-set (without the none),
            // and unpack the untagged union-type'd value into the smaller union type.
            //
            // Note that the union is "smaller" only in the sense that it doesn't have the
            // 'none' type. Since a 'none' value takes up 0 space, the two union types will end
            // up having the same size in the end.
            //


            IRBuilder builder(inst);
            auto destTaggedUnionType = cast<IRTaggedUnionType>(tryGetInfo(context, inst));
            if (destTaggedUnionType != srcTaggedUnionType)
            {
                // If the source and destination tagged-union types are different,
                // we need to emit a cast.
                builder.setInsertBefore(inst);
                IRInst* tag = builder.emitGetTagFromTaggedUnion(inst->getOptionalOperand());
                auto downcastedTag = builder.emitIntrinsicInst(
                    (IRType*)makeTagType(destTaggedUnionType->getWitnessTableSet()),
                    kIROp_GetTagForSubSet,
                    1,
                    &tag);

                auto unpackedValue = builder.emitUnpackAnyValue(
                    getLoweredType(makeUntaggedUnionType(destTaggedUnionType->getTypeSet())),
                    builder.emitGetValueFromTaggedUnion(inst->getOptionalOperand()));

                auto newTaggedUnion = builder.emitMakeTaggedUnion(
                    getLoweredType(destTaggedUnionType),
                    builder.emitPoison(makeTagType(destTaggedUnionType->getTypeSet())),
                    downcastedTag,
                    unpackedValue);


                inst->replaceUsesWith(newTaggedUnion);
                inst->removeAndDeallocate();
            }
            else
            {
                inst->replaceUsesWith(inst->getOptionalOperand());
                inst->removeAndDeallocate();
            }

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
            //    input's set type doesn't have a 'none'. In this case
            //    we just return a true.
            //
            // 2. 'none' is a possibility. In this case, we'll get the
            //    tag of 'none' in the set by emitting a `GetTagOfElementInSet`
            //    compare those to determine if we have a value.
            //

            IRBuilder builder(inst);

            bool containsNone = false;
            forEachInSet(
                module,
                taggedUnionType->getWitnessTableSet(),
                [&](IRInst* wt)
                {
                    if (wt == getNoneWitness())
                        containsNone = true;
                });

            if (!containsNone)
            {
                // If 'none' isn't a part of the set, statically set
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
                // the value for 'none' (in the context of the tag's set)
                //
                builder.setInsertBefore(inst);

                auto dynTag = builder.emitGetTagFromTaggedUnion(inst->getOptionalOperand());

                // Cast the singleton tag to the target set tag (will convert the
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

    void collectExistentialTables(IRInterfaceType* interfaceType, HashSet<IRInst*>& outTables)
    {
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
                        outTables.add(witnessTable);
                    }
                }
            }
        }
    }

    bool processModule()
    {
        bool hasChanges = false;

        // Part 1: Information Propagation
        //    This phase propagates type information through the module
        //    and records them into different maps in the current context.
        //
        performInformationPropagation();

        if (sink->getErrorCount() > 0)
        {
            // If there are any diagnostics after propagation, don't continue into
            // the mutating lowering phase.
            return false;
        }

        // Part 2: Dynamic Instruction Specialization
        //    Re-write dynamic instructions into specialized versions based on the
        //    type information in the previous phase.
        //
        hasChanges |= performDynamicInstLowering();

        // If lowering reported its own diagnostics, the IR is in a partially
        // lowered state; don't pile cascade errors from the walker below.
        if (sink->getErrorCount() > 0)
            return hasChanges;

        // Part 3: Diagnose unresolved dispatch sites.
        //    Any `lookupWitnessMethod` that survived specialization with a
        //    non-concrete witness-table operand would ICE in codegen.  If the
        //    corresponding interface has no conformances registered, report
        //    E50100 pointing the user at the real fix.
        diagnoseUnresolvedLookupWitnesses();

        return hasChanges;
    }

    TypeFlowSpecializationContext(
        IRModule* module,
        TargetProgram* target,
        DiagnosticSink* sink,
        SpecializationContext* specContext,
        bool shouldReportDynamicDispatchSites)
        : module(module)
        , sink(sink)
        , shouldReportDynamicDispatchSites(shouldReportDynamicDispatchSites)
        , specContext(specContext)
        , translationContext(target, module, specContext, sink)
    {
    }


    // Basic context
    IRModule* module;
    DiagnosticSink* sink;
    bool shouldReportDynamicDispatchSites;

    // Set of parameters already diagnosed for ref/constref interface issues,
    // to avoid emitting duplicate diagnostics per call edge.
    HashSet<IRInst*> diagnosedRefParams;

    // Set of entry-point params already diagnosed for missing conformances,
    // to avoid emitting duplicate E50100 from multiple ExtractExistential* ops.
    HashSet<IRInst*> diagnosedEntryPointInterfaceParams;

    // Set of call sites/contexts already diagnosed for invalid existential specialization,
    // to avoid duplicate diagnostics when instructions are revisited during propagation.
    HashSet<IRInst*> diagnosedExistentialSpecializationSites;

    // Set of bit-cast instructions already diagnosed for unsupported
    // non-concrete result types during propagation.
    HashSet<IRInst*> diagnosedBitCasts;

    // Set of interface types already diagnosed with E50100 "no type conformances"
    // inside `diagnoseUnresolvedLookupWitnesses`, so a single missing conformance
    // surfacing at multiple surviving `lookupWitnessMethod` sites fires at most
    // one diagnostic.  Local to the post-specialization walker — `processModule`
    // bails out on any Part-1 error before that walker runs, so there is no
    // cross-phase deduplication to coordinate.
    HashSet<IRInst*> diagnosedNoTypeConformancesInterfaces;

    // Emit error 33180 for an invalid existential specialization, deduplicating by `dedupKey`.
    // Returns true if the diagnostic was emitted (first time for this key).
    bool emitExistentialSpecializationDiagnostic(
        IRInst* specializedValue,
        SourceLoc location,
        IRInst* dedupKey)
    {
        if (!diagnosedExistentialSpecializationSites.add(dedupKey))
            return false;

        String genericName;
        auto diagnosticTarget = getInvalidExistentialSpecializationTarget(specializedValue);
        if (auto nameHint = diagnosticTarget->findDecoration<IRNameHintDecoration>())
            genericName = nameHint->getName();
        if (genericName.getLength() == 0)
            genericName = "<generic>";

        sink->diagnose(Diagnostics::CannotSpecializeGenericWithExistential{
            .generic = genericName,
            .location = location});
        return true;
    }

    // Mapping from (context, inst) --> propagated info
    Dictionary<InstWithContext, IRWeakUse*> propagationMap;

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

    // Cache for SpecializeExistentialsInFunc: maps from base function to all
    // SpecializeExistentialsInFunc contexts created for it. This is used to
    // oppourtunistically merge variants of the same function depending on policy
    // (i.e. as-few-variants-as-possible vs. aggressive specialization).
    //
    Dictionary<IRInst*, List<IRSpecializeExistentialsInFunc*>> existentialSpecializedFuncCache;

    // Information on the call-site. Note that this may be different from the information
    // on the inst used by the call-site, since it may carry bindings from call arguments.
    //
    Dictionary<InstWithContext, IRInst*> callSiteInfo;

    // The declared func type of the callee at each call site, recorded during analysis
    // before the callee's type is replaced by tag/set types during specialization.
    // Used as the context type for structural union operations when computing effective
    // func types for dispatch.
    //
    Dictionary<InstWithContext, IRFuncType*> callSiteFuncType;

    // Incoming edge counts for each callee.
    Dictionary<IRInst*, Int> calleeRefCounts;

    // Translation context.
    TranslationContext translationContext;

    // Unique definitions (independent of bindings/specializations)
    //
    // Used when applying simplifications to function bodies, specifically to avoid
    // re-processing the same definition.
    //
    HashSet<IRFunc*> uniqueDefs;

    SpecializationContext* specContext;
};

// Main entry point
bool specializeDynamicInsts(
    IRModule* module,
    TargetProgram* target,
    DiagnosticSink* sink,
    SpecializationContext* outerContext,
    bool shouldReportDynamicDispatchSites)
{
    TypeFlowSpecializationContext
        context(module, target, sink, outerContext, shouldReportDynamicDispatchSites);
    return context.processModule();
}

} // namespace Slang
