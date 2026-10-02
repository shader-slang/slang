#include "slang-ir-legalize-global-values.h"

#include "slang-ir-addr-inst-elimination.h"
#include "slang-ir-clone.h"
#include "slang-ir-inline.h"
#include "slang-ir-ssa-simplification.h"
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

// CUDA accepts aggregate initializers made entirely from constants. It does not
// accept calls or loads as device-variable initializers, even when their result type
// contains only numeric fields.
static bool isAggregateInitializerOperation(IRInst* value)
{
    if (as<IRConstant>(value))
        return true;
    switch (value->getOp())
    {
    case kIROp_MakeStruct:
    case kIROp_MakeArray:
    case kIROp_MakeArrayFromElement:
    case kIROp_MakeVector:
    case kIROp_MakeMatrix:
    case kIROp_MakeVectorFromScalar:
    case kIROp_MakeMatrixFromScalar:
        return true;
    default:
        return false;
    }
}

static bool isStaticAggregateInitializer(IRInst* value)
{
    if (!isAggregateInitializerOperation(value))
        return false;
    for (UInt i = 0; i < value->getOperandCount(); ++i)
        if (!isStaticAggregateInitializer(value->getOperand(i)))
            return false;
    return true;
}

// Return whether func is a synthesized value constructor used by a module initializer
// whose body can be canonicalized without escaping local addresses or executing calls.
// The resulting expression is checked separately after canonicalization.
static bool canFoldAggregateConstructor(IRModule* module, IRFunc* func)
{
    if (!func || !as<IRStructType>(func->getResultType()) ||
        as<IRClassType>(func->getResultType()) || func->findDecoration<IRNoInlineDecoration>())
        return false;
    auto ctor = func->findDecoration<IRConstructorDecoration>();
    auto block = func->getFirstBlock();
    if (!ctor || !ctor->getSynthesizedStatus() || !block || block->getNextBlock() ||
        !as<IRReturn>(block->getTerminator()))
        return false;

    bool usedByModuleInitializer = false;
    for (auto use = func->firstUse; use; use = use->nextUse)
    {
        if (auto call = as<IRCall>(use->getUser()))
            usedByModuleInitializer |=
                call->getCallee() == func && call->getParent() == module->getModuleInst();
    }
    if (!usedByModuleInitializer)
        return false;

    bool simple = true;
    for (auto op = block->getFirstInst(); op != block->getTerminator(); op = op->getNextInst())
    {
        if (as<IRParam>(op))
            continue;
        switch (op->getOp())
        {
        case kIROp_Var:
        case kIROp_FieldAddress:
        case kIROp_Load:
        case kIROp_Store:
            break;
        default:
            if (!getIROpInfo(op->getOp()).isHoistable() && !isAggregateInitializerOperation(op))
                simple = false;
            break;
        }
        if (op->getOp() == kIROp_Var || op->getOp() == kIROp_FieldAddress)
        {
            for (auto use = op->firstUse; use; use = use->nextUse)
            {
                auto user = use->getUser();
                if (as<IRLoad>(user) || as<IRFieldAddress>(user))
                    continue;
                if (auto store = as<IRStore>(user))
                    if (use == &store->ptr)
                        continue;
                simple = false;
            }
        }
    }
    return simple;
}

// Turn compiler-generated value constructors into aggregate expressions, then inline
// their module-scope calls. For example, Pair(a, b) becomes makeStruct(a, b), allowing
// a literal table to be initialized without a device call. Canonicalization updates
// the eligible constructor body, but function-local calls are not explicitly inlined.
// Reuse address elimination and SSA simplification instead of inferring field order
// from constructor arguments.
static void foldAggregateConstructors(IRModule* module, DiagnosticSink* sink)
{
    List<IRFunc*> constructors;
    for (auto inst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(inst);
        if (canFoldAggregateConstructor(module, func))
            constructors.add(func);
    }
    for (auto func : constructors)
    {
        if (SLANG_FAILED(eliminateAddressInsts(func, sink)))
            return;
        simplifyFunc(nullptr, func, IRSimplificationOptions::getDefault(nullptr), sink);
        auto block = func->getFirstBlock();
        bool expression = !block->getNextBlock() && as<IRReturn>(block->getTerminator());
        for (auto op = block->getFirstInst(); op != block->getTerminator(); op = op->getNextInst())
            if (!as<IRParam>(op) && !getIROpInfo(op->getOp()).isHoistable() &&
                !isAggregateInitializerOperation(op))
                expression = false;
        if (!expression)
            continue;
        List<IRCall*> calls;
        for (auto use = func->firstUse; use; use = use->nextUse)
            if (auto call = as<IRCall>(use->getUser()))
                // Only module-scope calls can contribute to a static global initializer.
                // Folding function-local calls changes ordinary executable code and can
                // greatly increase downstream compilation work.
                if (call->getCallee() == func && call->getParent() == module->getModuleInst())
                    calls.add(call);
        for (auto call : calls)
            inlineCall(call);
    }
}

struct GlobalInstLegalizationInliningContext : public GlobalInstInliningContextGeneric
{
    bool preserveStaticAggregates = false;

    static bool isSimpleConstantType(IRType* type)
    {
        for (;;)
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
            {
                type = arrayType->getElementType();
                continue;
            }
            return false;
        }
    }
    bool isLegalGlobalInstForTarget(IRInst* inst) override
    {
        auto type = inst->getDataType();
        return isSimpleConstantType(type) ||
               (preserveStaticAggregates && isStaticAggregateInitializer(inst));
    }

    bool isInlinableGlobalInstForTarget(IRInst* /* inst */) override { return false; }

    bool shouldBeInlinedForTarget(IRInst* /* user */) override { return false; }

    IRInst* getOutsideASM(IRInst* beforeInst) override { return beforeInst; }
};

void inlineGlobalConstantsForLegalization(
    IRModule* module,
    bool preserveStaticAggregates,
    DiagnosticSink* sink)
{
    if (preserveStaticAggregates)
        foldAggregateConstructors(module, sink);
    GlobalInstLegalizationInliningContext context;
    context.preserveStaticAggregates = preserveStaticAggregates;

    context.wrapReferences = false;
    context.inlineGlobalValuesAndRemoveIfUnused(module);
}

} // namespace Slang
