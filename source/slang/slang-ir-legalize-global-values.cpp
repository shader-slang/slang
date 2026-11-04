#include "slang-ir-legalize-global-values.h"

#include "slang-compiler.h"
#include "slang-ir-clone.h"
#include "slang-ir-util.h"

namespace Slang
{

void GlobalInstInliningContextGeneric::inlineGlobalValuesAndRemoveIfUnused(IRModule* module)
{
    List<IRUse*> globalInstUsesToInline;

    for (auto globalInst : module->getGlobalInsts())
    {
        if (isInlinableGlobalInst(globalInst))
        {
            for (auto use = globalInst->firstUse; use; use = use->nextUse)
            {
                if (getParentFunc(use->getUser()) != nullptr)
                    globalInstUsesToInline.add(use);
            }
        }
    }

    HashSet<IRInst*> globalInstsToConsiderDeleting;
    for (auto use : globalInstUsesToInline)
    {
        auto user = use->getUser();
        IRBuilder builder(user);
        builder.setInsertBefore(getOutsideASM(user));
        IRCloneEnv cloneEnv;
        auto val = maybeInlineGlobalValue(builder, use->getUser(), use->get(), cloneEnv);
        if (val != use->get())
        {
            // Since certain globals that appear in the IR are considered illegal for all targets,
            // e.g. calls to functions, we delete the globals we've inlined.
            // Note that the inlining is done such that none of the descendants of the global will
            // have any uses either.
            globalInstsToConsiderDeleting.add(use->usedValue);

            builder.replaceOperand(use, val);
        }
    }

    for (auto globalInst : globalInstsToConsiderDeleting)
    {
        if (!globalInst->hasUses())
            globalInst->removeAndDeallocate();
    }
}

bool GlobalInstInliningContextGeneric::isLegalGlobalInst(IRInst* inst)
{
    if (as<IRConstant>(inst))
        return true;
    if (isLegalGlobalInstForTarget(inst))
        return true;
    return false;
}

bool GlobalInstInliningContextGeneric::isInlinableGlobalInst(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_FRem:
    case kIROp_IRem:
    case kIROp_Lsh:
    case kIROp_Rsh:
    case kIROp_And:
    case kIROp_Or:
    case kIROp_Not:
    case kIROp_Neg:
    case kIROp_Div:
    case kIROp_FieldExtract:
    case kIROp_FieldAddress:
    case kIROp_GetElement:
    case kIROp_GetElementPtr:
    case kIROp_GetOffsetPtr:
    case kIROp_UpdateElement:
    case kIROp_MakeTuple:
    case kIROp_MakeValuePack:
    case kIROp_GetTupleElement:
    case kIROp_MakeStruct:
    case kIROp_MakeArray:
    case kIROp_MakeArrayFromElement:
    case kIROp_MakeVector:
    case kIROp_MakeMatrix:
    case kIROp_MakeMatrixFromScalar:
    case kIROp_MakeVectorFromScalar:
    case kIROp_Swizzle:
    case kIROp_SwizzleSet:
    case kIROp_MatrixReshape:
    case kIROp_MakeString:
    case kIROp_MakeResultError:
    case kIROp_MakeResultValue:
    case kIROp_GetResultError:
    case kIROp_GetResultValue:
    case kIROp_CastFloatToInt:
    case kIROp_CastIntToFloat:
    case kIROp_CastIntToPtr:
    case kIROp_PtrCast:
    case kIROp_CastPtrToBool:
    case kIROp_CastPtrToInt:
    case kIROp_BitAnd:
    case kIROp_BitNot:
    case kIROp_BitOr:
    case kIROp_BitXor:
    case kIROp_BitCast:
    case kIROp_IntCast:
    case kIROp_FloatCast:
    // A `DescriptorHandle`'s representation is `uint64` or `uint2` depending on the descriptor kind
    // and target, so an initializer may supply the other width. Reconciling the two is only
    // possible inside a block (by folding, or a `BitCast`), so these casts — and a `Select` that a
    // `?:` initializer can put between them — have to be inlinable for the chain around them to
    // sink into the function that uses the handle.
    case kIROp_CastUInt2ToDescriptorHandle:
    case kIROp_CastUInt64ToDescriptorHandle:
    case kIROp_CastDescriptorHandleToUInt2:
    case kIROp_CastDescriptorHandleToUInt64:
    case kIROp_Select:
    case kIROp_Greater:
    case kIROp_Less:
    case kIROp_Geq:
    case kIROp_Leq:
    case kIROp_Neq:
    case kIROp_Eql:
    case kIROp_Call:
    case kIROp_Load:
        return true;
    default:
        if (isInlinableGlobalInstForTarget(inst))
            return true;
        return false;
    }
}

bool GlobalInstInliningContextGeneric::shouldInlineInstImpl(IRInst* inst)
{
    // If 'inst' has an ancestor that is currently being inlined, then we
    // better inline it since we'll be removing the ancestor.
    bool ancestorShouldBeInlined = false;
    for (IRInst* ancestor = inst->parent; ancestor != nullptr; ancestor = ancestor->parent)
        if (m_mapGlobalInstToShouldInline.tryGetValue(inst, ancestorShouldBeInlined) &&
            ancestorShouldBeInlined)
            return true;

    if (!isInlinableGlobalInst(inst))
        return false;
    if (isLegalGlobalInst(inst))
    {
        for (UInt i = 0; i < inst->getOperandCount(); i++)
            if (shouldInlineInst(inst->getOperand(i)))
                return true;
        return false;
    }
    return true;
}

bool GlobalInstInliningContextGeneric::shouldInlineInst(IRInst* inst)
{
    bool result = false;
    if (m_mapGlobalInstToShouldInline.tryGetValue(inst, result))
        return result;
    result = shouldInlineInstImpl(inst);
    m_mapGlobalInstToShouldInline[inst] = result;
    return result;
}

IRInst* GlobalInstInliningContextGeneric::inlineInst(
    IRBuilder& builder,
    IRCloneEnv& cloneEnv,
    IRInst* inst)
{
    // We rely on this dictionary in order to force inlining of any nodes with that should be
    // inlined
    SLANG_ASSERT(m_mapGlobalInstToShouldInline[inst]);

    IRInst* result;
    if (cloneEnv.mapOldValToNew.tryGetValue(inst, result))
        return result;

    for (UInt i = 0; i < inst->getOperandCount(); i++)
    {
        auto operand = inst->getOperand(i);
        IRBuilder operandBuilder(builder);
        operandBuilder.setInsertBefore(getOutsideASM(builder.getInsertLoc().getInst()));
        maybeInlineGlobalValue(operandBuilder, inst, operand, cloneEnv);
    }
    result = cloneInstAndOperands(&cloneEnv, &builder, inst);
    cloneEnv.mapOldValToNew[inst] = result;
    IRBuilder subBuilder(builder);
    subBuilder.setInsertInto(result);
    for (auto child : inst->getDecorations())
    {
        cloneInst(&cloneEnv, &subBuilder, child);
    }
    for (auto child : inst->getChildren())
    {
        m_mapGlobalInstToShouldInline[child] = true;
        inlineInst(subBuilder, cloneEnv, child);
    }
    return result;
}

IRInst* GlobalInstInliningContextGeneric::maybeInlineGlobalValue(
    IRBuilder& builder,
    IRInst* user,
    IRInst* inst,
    IRCloneEnv& cloneEnv)
{
    if (!shouldInlineInst(inst))
    {
        switch (inst->getOp())
        {
        case kIROp_Func:
        case kIROp_Specialize:
        case kIROp_Generic:
        case kIROp_LookupWitnessMethod:
            return inst;
        }
        if (as<IRType>(inst))
            return inst;
        if (!wrapReferences)
            return inst;

        // If we encounter a global value that shouldn't be inlined, e.g. a const literal,
        // we should insert a GlobalValueRef() inst to wrap around it, so all the dependent
        // uses can be pinned to the function body.
        auto result = inst;
        bool shouldWrapGlobalRef = true;
        if (!isLegalGlobalInst(user) && !getIROpInfo(user->getOp()).isHoistable())
            shouldWrapGlobalRef = false;
        else if (shouldBeInlinedForTarget(user))
            shouldWrapGlobalRef = false;
        if (shouldWrapGlobalRef)
            result = builder.emitGlobalValueRef(inst);
        cloneEnv.mapOldValToNew[inst] = result;
        return result;
    }

    // If the global value is inlinable, we make all its operands avaialble locally, and
    // then copy it to the local scope.
    return inlineInst(builder, cloneEnv, inst);
}

struct GlobalInstLegalizationInliningContext : public GlobalInstInliningContextGeneric
{
    // True if the target can emit the initializer of a global constant of a type that
    // contains `half`. CUDA cannot: it emits a `half` literal as a `__half(...)`
    // constructor call, which NVRTC rejects as the dynamic initializer of a `__device__`
    // variable.
    bool targetCanEmitGlobalInitializerForHalf;

    explicit GlobalInstLegalizationInliningContext(CodeGenTarget target)
        : targetCanEmitGlobalInitializerForHalf(target != CodeGenTarget::CUDASource)
    {
    }

    // Return true if a value of `type` can stay a global constant. A simple type is a
    // scalar, a vector, a matrix, an array of a simple type, or a struct whose fields
    // are all simple types.
    //
    // A struct with a resource, pointer, or other opaque field is not simple. A type
    // that contains `half` is not simple when `targetCanEmitGlobalInitializerForHalf` is
    // false: CUDA emits a `half` literal as a `__half(...)` constructor call, which is a
    // dynamic initializer that NVRTC rejects. Returning false is always safe: the value
    // is then inlined into the functions that use it.
    bool isSimpleConstantType(IRType* type)
    {
        if (!type)
            return true;
        if (as<IRBasicType>(type))
            return targetCanEmitGlobalInitializerForHalf || type->getOp() != kIROp_HalfType;
        if (auto vectorType = as<IRVectorType>(type))
            return isSimpleConstantType(vectorType->getElementType());
        if (auto matrixType = as<IRMatrixType>(type))
            return isSimpleConstantType(matrixType->getElementType());
        if (auto arrayType = as<IRArrayTypeBase>(type))
            return isSimpleConstantType(arrayType->getElementType());
        if (auto structType = as<IRStructType>(type))
        {
            for (auto field : structType->getFields())
            {
                if (!isSimpleConstantType(field->getFieldType()))
                    return false;
            }
            return true;
        }
        return false;
    }
    bool isLegalGlobalInstForTarget(IRInst* inst) override
    {
        // A call is never a compile-time constant, even if its result type is simple.
        // `legalizeConstantConstructorCallsForGlobalScope` folds some constructor
        // calls. A call that it does not fold (for example a user-defined constructor)
        // must be inlined. If it stayed global, CUDA would get a dynamic initializer,
        // which NVRTC rejects.
        if (as<IRCall>(inst))
            return false;
        return isSimpleConstantType(inst->getDataType());
    }

    bool isInlinableGlobalInstForTarget(IRInst* /* inst */) override { return false; }

    bool shouldBeInlinedForTarget(IRInst* /* user */) override { return false; }

    IRInst* getOutsideASM(IRInst* beforeInst) override { return beforeInst; }
};

// Return a `makeStruct` that is equal to `call`, or nullptr if `call` is not a call to
// a member-wise synthesized constructor. The caller replaces the call.
//
// A member-wise constructor makes a temporary, stores each argument into its field,
// and returns the temporary. This is the same as `makeStruct` of the arguments. The
// function checks the constructor body, because the `synthesizedConstructor`
// decoration is not enough: the constructor of a derived struct also calls the base
// constructor, for example. Any other shape returns nullptr.
//
// Consider this example:
//
//     struct Record { uint a; uint b; }
//     static const Record records[2] = { {1, 2}, {3, 4} };
//
// The checker lowers each `{1, 2}` to a constructor call, so the module has:
//
//     %records = globalConstant(makeArray(call Record.$init(1,2),
//                                         call Record.$init(3,4)))
//
// A call is not a legal global constant, so the table would be inlined into every
// function that uses it. After this function folds the calls, the table is
// `makeArray(makeStruct(1,2), makeStruct(3,4))` and can stay global. Inside a
// function body the call stays a call, because a local struct is built at run time.
static IRInst* tryBuildMakeStructFromSynthesizedConstructorCall(
    GlobalInstLegalizationInliningContext& context,
    IRCall* call)
{
    auto callee = as<IRFunc>(getResolvedInstForDecorations(call->getCallee()));
    if (!callee)
        return nullptr;
    auto ctorDecor = callee->findDecoration<IRConstructorDecoration>();
    if (!ctorDecor || !ctorDecor->getSynthesizedStatus())
        return nullptr;

    auto structType = as<IRStructType>(call->getDataType());
    if (!structType)
        return nullptr;

    // Fold only a struct that is a simple constant. Other structs are inlined.
    if (!context.isSimpleConstantType(structType))
        return nullptr;

    // A member-wise constructor has one block.
    auto block = callee->getFirstBlock();
    if (!block || block->getNextBlock())
        return nullptr;

    // Map each parameter to the corresponding call argument.
    Dictionary<IRInst*, IRInst*> paramToArg;
    UInt paramCount = 0;
    for (auto param : block->getParams())
    {
        if (paramCount >= call->getArgCount())
            return nullptr;
        paramToArg[param] = call->getArg(paramCount);
        paramCount++;
    }
    if (paramCount != call->getArgCount())
        return nullptr;

    // Record the value stored into each field. Return nullptr on any other shape.
    IRVar* localVar = nullptr;
    IRInst* loadedResult = nullptr;
    Dictionary<IRInst*, IRInst*> fieldKeyToValue;
    HashSet<IRInst*> consumedParams;
    for (auto inst = block->getFirstOrdinaryInst(); inst; inst = inst->getNextInst())
    {
        switch (inst->getOp())
        {
        case kIROp_Var:
            if (localVar) // Only one temporary is allowed.
                return nullptr;
            localVar = as<IRVar>(inst);
            if (localVar->getDataType()->getValueType() != structType)
                return nullptr;
            break;
        case kIROp_FieldAddress:
            if (as<IRFieldAddress>(inst)->getBase() != localVar)
                return nullptr;
            break;
        case kIROp_Store:
            {
                // A store after the load is not in the returned value.
                if (loadedResult)
                    return nullptr;
                auto store = as<IRStore>(inst);
                auto fieldAddr = as<IRFieldAddress>(store->getPtr());
                if (!fieldAddr || fieldAddr->getBase() != localVar)
                    return nullptr;
                IRInst* arg = nullptr;
                if (!paramToArg.tryGetValue(store->getVal(), arg))
                    return nullptr; // The stored value is not a parameter.
                if (consumedParams.contains(store->getVal()))
                    return nullptr; // Each parameter must set one field.
                consumedParams.add(store->getVal());
                if (fieldKeyToValue.containsKey(fieldAddr->getField()))
                    return nullptr; // Each field must be set once.
                fieldKeyToValue[fieldAddr->getField()] = arg;
                break;
            }
        case kIROp_Load:
            if (as<IRLoad>(inst)->getPtr() != localVar)
                return nullptr;
            loadedResult = inst;
            break;
        case kIROp_Return:
            if (as<IRReturn>(inst)->getVal() != loadedResult)
                return nullptr;
            break;
        default:
            return nullptr; // Any other instruction.
        }
    }

    // Collect the operands in field order. Every field must be set.
    List<IRInst*> args;
    UInt fieldCount = 0;
    for (auto field : structType->getFields())
    {
        IRInst* value = nullptr;
        if (!fieldKeyToValue.tryGetValue(field->getKey(), value))
            return nullptr; // The field is never set.
        args.add(value);
        fieldCount++;
    }

    // Each field must match one parameter and one argument. This also makes sure
    // that no argument is dropped.
    if (fieldCount != paramCount)
        return nullptr;
    SLANG_ASSERT((UInt)args.getCount() == fieldCount && fieldCount == call->getArgCount());

    IRBuilder builder(call->getModule());
    builder.setInsertBefore(call);
    return builder.emitMakeStruct(structType, args);
}

// Fold module-scope calls to member-wise synthesized constructors into `makeStruct`.
// `inlineGlobalConstantsForLegalization` runs this first, so that a `static const`
// struct table can stay a global constant.
//
// Calls inside function bodies are not changed, because a local struct is built at
// run time. Only a module-scope value must be a compile-time constant. See
// `tryBuildMakeStructFromSynthesizedConstructorCall` for an example.
static void legalizeConstantConstructorCallsForGlobalScope(
    GlobalInstLegalizationInliningContext& context,
    IRModule* module)
{
    // The calls in a `static const` initializer are direct children of the module.
    // Collect them first, because the loop below changes the list of global insts.
    List<IRCall*> globalCalls;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto call = as<IRCall>(inst))
            globalCalls.add(call);
    }
    for (auto call : globalCalls)
    {
        if (auto makeStruct = tryBuildMakeStructFromSynthesizedConstructorCall(context, call))
        {
            call->replaceUsesWith(makeStruct);
            call->removeAndDeallocate();
        }
    }
}

void inlineGlobalConstantsForLegalization(IRModule* module, CodeGenTarget target)
{
    GlobalInstLegalizationInliningContext context(target);

    legalizeConstantConstructorCallsForGlobalScope(context, module);

    context.wrapReferences = false;
    context.inlineGlobalValuesAndRemoveIfUnused(module);
}

} // namespace Slang
