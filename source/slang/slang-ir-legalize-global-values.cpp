#include "slang-ir-legalize-global-values.h"

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
    // Return true if a value of `type` can live in target device-global storage
    // as an immutable constant, and therefore need not be reconstructed once per
    // invocation: a basic scalar, vector, matrix, an array of such, or a struct
    // whose every field is itself a simple constant.
    //
    // The struct case recurses through the fields, which is what admits a POD
    // `static const` table of structs (e.g. `static const Record records[2]`).
    // It still (correctly) rejects any struct that directly or transitively
    // contains a resource, pointer, or other opaque/handle field, because such a
    // field type is not basic/vector/matrix/array/struct and so falls through to
    // `return false`. Returning false is the conservative answer here: it forces
    // the value to be inlined to its use sites, which is always legal, so a
    // non-simple struct is handled safely rather than wrongly placed in global
    // storage.
    static bool isSimpleConstantType(IRType* type)
    {
        if (!type)
            return true;
        if (as<IRBasicType>(type))
            return true;
        if (as<IRVectorType>(type))
            return true;
        if (as<IRMatrixType>(type))
            return true;
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
        // A call is a runtime computation, never a compile-time constant, even
        // when its result type is a simple constant type. This matters because
        // the struct case of `isSimpleConstantType` above now accepts POD
        // structs: a synthesized member-wise constructor call is folded to a
        // `makeStruct` by `legalizeConstantConstructorCallsForGlobalScope`
        // before this pass, but any constructor call it does not fold (a
        // base-initializing derived constructor, or a user-defined constructor)
        // must stay illegal here so it is inlined into its use sites. Leaving
        // such a call at module scope would otherwise emit an illegal dynamic
        // global initializer — e.g. a `__device__` variable initialized by a
        // constructor call, which NVRTC rejects.
        if (as<IRCall>(inst))
            return false;
        return isSimpleConstantType(inst->getDataType());
    }

    bool isInlinableGlobalInstForTarget(IRInst* /* inst */) override { return false; }

    bool shouldBeInlinedForTarget(IRInst* /* user */) override { return false; }

    IRInst* getOutsideASM(IRInst* beforeInst) override { return beforeInst; }
};

// A struct's synthesized constructor is, by construction, member-wise: it
// allocates a temporary, stores each argument into the corresponding field in
// declaration order, then returns the loaded value. That is semantically
// identical to a `makeStruct` of the arguments. Inside a function body the
// normal inline + SSA-promotion pipeline already collapses such a call into a
// `makeStruct`, but at module scope (a `static const` initializer, e.g.
// `static const Record records[2] = { {1,2}, {3,4} }`) that pipeline never
// runs, so the call survives as the initializer's value:
//
//     let %r = globalConstant(makeArray(call Record.$init(1,2),
//                                        call Record.$init(3,4)))
//
// A bare call cannot be a legal global constant on any target, so leaving it
// there forces the whole table to be reconstructed per-invocation in every
// using function. This helper completes the same lowering the function-body
// pipeline would have done: given a call to a synthesized member-wise
// constructor with the exact var / field-store / load / return shape, it
// returns the equivalent `makeStruct`; otherwise it returns nullptr. The body
// is verified field-by-field (rather than trusting the constructor decoration
// alone) so that anything that is not a plain member-wise store of the
// parameters — a base-struct initializer, a default field value, a conversion,
// any control flow — causes a conservative bail, leaving the call to be inlined
// as before.
static IRInst* tryReplaceSynthesizedConstructorCallWithMakeStruct(IRCall* call)
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

    // Only fold when the struct is itself a simple constant (POD). A struct with a
    // resource, pointer, or other opaque field is not a legal global constant, so
    // folding its constructor would produce a `makeStruct` that must be inlined
    // anyway and would feed a non-constant aggregate into legalization; leave such
    // calls for the normal inlining path.
    if (!GlobalInstLegalizationInliningContext::isSimpleConstantType(structType))
        return nullptr;

    // A member-wise constructor has no control flow, so it is a single block.
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

    // Symbolically evaluate the body, recording the value stored into each
    // field. Bail on anything outside the recognized member-wise shape.
    IRVar* localVar = nullptr;
    IRInst* loadedResult = nullptr;
    Dictionary<IRInst*, IRInst*> fieldKeyToValue;
    HashSet<IRInst*> consumedParams;
    for (auto inst = block->getFirstOrdinaryInst(); inst; inst = inst->getNextInst())
    {
        switch (inst->getOp())
        {
        case kIROp_Var:
            if (localVar) // more than one temporary is not the simple shape
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
                // A store after the value has been loaded would not be reflected
                // in the returned struct, so it is not the member-wise shape.
                if (loadedResult)
                    return nullptr;
                auto store = as<IRStore>(inst);
                auto fieldAddr = as<IRFieldAddress>(store->getPtr());
                if (!fieldAddr || fieldAddr->getBase() != localVar)
                    return nullptr;
                IRInst* arg = nullptr;
                if (!paramToArg.tryGetValue(store->getVal(), arg))
                    return nullptr; // stored value is not a plain parameter
                if (consumedParams.contains(store->getVal()))
                    return nullptr; // a parameter used for two fields is not 1:1 member-wise
                consumedParams.add(store->getVal());
                if (fieldKeyToValue.containsKey(fieldAddr->getField()))
                    return nullptr; // a field written twice is not member-wise
                fieldKeyToValue[fieldAddr->getField()] = arg;
                break;
            }
        case kIROp_Load:
            if (as<IRLoad>(inst)->getPtr() != localVar)
                return nullptr;
            loadedResult = inst;
            break;
        case kIROp_Return:
            if (inst->getOperand(0) != loadedResult)
                return nullptr;
            break;
        default:
            return nullptr; // any other instruction: bail conservatively
        }
    }

    // Assemble the makeStruct operands in field-declaration order, requiring
    // every field to have been initialized exactly once from a parameter.
    List<IRInst*> args;
    UInt fieldCount = 0;
    for (auto field : structType->getFields())
    {
        IRInst* value = nullptr;
        if (!fieldKeyToValue.tryGetValue(field->getKey(), value))
            return nullptr; // a field was never initialized
        args.add(value);
        fieldCount++;
    }

    // A member-wise constructor initializes one field per parameter, one-to-one.
    // Require exactly that: with the per-field/per-parameter uniqueness enforced
    // above and the earlier `paramCount == call->getArgCount()` check, this makes
    // the reconstruction a 1:1 field/parameter/argument mapping, so the rebuilt
    // `makeStruct` faithfully evaluates every argument (none dropped or reused) and
    // satisfies `IRMakeStruct`'s one-operand-per-field contract.
    if (fieldCount != paramCount)
        return nullptr;
    SLANG_ASSERT((UInt)args.getCount() == fieldCount && fieldCount == call->getArgCount());

    IRBuilder builder(call->getModule());
    builder.setInsertBefore(call);
    return builder.emitMakeStruct(structType, args);
}

// Restore the canonical `makeStruct` representation for module-scope constant
// initializers that were lowered as synthesized member-wise constructor calls.
// This runs just before `inlineGlobalConstantsForLegalization` so the resulting
// `makeStruct` (whose type the extended `isSimpleConstantType` now recognizes)
// stays a legal global constant instead of being reconstructed per-invocation.
// See `tryReplaceSynthesizedConstructorCallWithMakeStruct` for why the call
// shape appears only at module scope and why folding it is the principled fix.
void legalizeConstantConstructorCallsForGlobalScope(IRModule* module)
{
    List<IRCall*> globalCalls;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto call = as<IRCall>(inst))
            globalCalls.add(call);
    }
    for (auto call : globalCalls)
    {
        if (auto makeStruct = tryReplaceSynthesizedConstructorCallWithMakeStruct(call))
        {
            call->replaceUsesWith(makeStruct);
            call->removeAndDeallocate();
        }
    }
}

void inlineGlobalConstantsForLegalization(IRModule* module)
{
    GlobalInstLegalizationInliningContext context;

    context.wrapReferences = false;
    context.inlineGlobalValuesAndRemoveIfUnused(module);
}

} // namespace Slang
