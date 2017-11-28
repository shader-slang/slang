// ir-legalize-types.cpp

// This file implements type legalization for the IR.
// It uses the core legalization logic in
// `legalize-types.{h,cpp}` to decide what to do with
// the types, while this file handles the actual
// rewriting of the IR to use the new types.
//
// This pass should only be applied to IR that has been
// fully specialized (no more generics/interfaces), so
// that the concrete type of everything is known.

#include "ir.h"
#include "ir-insts.h"
#include "legalize-types.h"

namespace Slang
{



struct LegalValImpl : RefObject
{
};
struct TuplePseudoVal;
struct PairPseudoVal;

struct LegalVal
{
    enum class Flavor
    {
        none,
        simple,
        implicitDeref,
        tuple,
        pair,
    };

    Flavor              flavor = Flavor::none;
    RefPtr<RefObject>   obj;
    IRValue*            irValue = nullptr;

    static LegalVal simple(IRValue* irValue)
    {
        LegalVal result;
        result.flavor = Flavor::simple;
        result.irValue = irValue;
        return result;
    }

    IRValue* getSimple()
    {
        assert(flavor == Flavor::simple);
        return irValue;
    }

    static LegalVal tuple(RefPtr<TuplePseudoVal> tupleVal);

    RefPtr<TuplePseudoVal> getTuple()
    {
        assert(flavor == Flavor::tuple);
        return obj.As<TuplePseudoVal>();
    }

    static LegalVal implicitDeref(LegalVal const& val);
    LegalVal getImplicitDeref();

    static LegalVal pair(RefPtr<PairPseudoVal> pairInfo);
    static LegalVal pair(
        LegalVal const&     ordinaryVal,
        LegalVal const&     specialVal,
        RefPtr<PairInfo>    pairInfo);

    RefPtr<PairPseudoVal> getPair()
    {
        assert(flavor == Flavor::pair);
        return obj.As<PairPseudoVal>();
    }
};

struct TuplePseudoVal : LegalValImpl
{
    struct Element
    {
        DeclRef<VarDeclBase>            fieldDeclRef;
        LegalVal                        val;
    };

    List<Element>   elements;
};

LegalVal LegalVal::tuple(RefPtr<TuplePseudoVal> tupleVal)
{
    LegalVal result;
    result.flavor = LegalVal::Flavor::tuple;
    result.obj = tupleVal;
    return result;
}

struct PairPseudoVal : LegalValImpl
{
    LegalVal ordinaryVal;
    LegalVal specialVal;

    // The info to tell us which fields
    // are on which side(s)
    RefPtr<PairInfo>  pairInfo;
};

LegalVal LegalVal::pair(RefPtr<PairPseudoVal> pairInfo)
{
    LegalVal result;
    result.flavor = LegalVal::Flavor::pair;
    result.obj = pairInfo;
    return result;
}

LegalVal LegalVal::pair(
    LegalVal const&     ordinaryVal,
    LegalVal const&     specialVal,
    RefPtr<PairInfo>    pairInfo)
{
    if (ordinaryVal.flavor == LegalVal::Flavor::none)
        return specialVal;

    if (specialVal.flavor == LegalVal::Flavor::none)
        return ordinaryVal;


    RefPtr<PairPseudoVal> obj = new PairPseudoVal();
    obj->ordinaryVal = ordinaryVal;
    obj->specialVal = specialVal;
    obj->pairInfo = pairInfo;

    return LegalVal::pair(obj);
}

struct ImplicitDerefVal : LegalValImpl
{
    LegalVal val;
};

LegalVal LegalVal::implicitDeref(LegalVal const& val)
{
    RefPtr<ImplicitDerefVal> implicitDerefVal = new ImplicitDerefVal();
    implicitDerefVal->val = val;

    LegalVal result;
    result.flavor = LegalVal::Flavor::implicitDeref;
    result.obj = implicitDerefVal;
    return result;
}

LegalVal LegalVal::getImplicitDeref()
{
    assert(flavor == Flavor::implicitDeref);
    return obj.As<ImplicitDerefVal>()->val;
}


struct IRTypeLegalizationContext
{
    Session*    session;
    IRModule*   module;
    IRBuilder*  builder;

    /// Context to use for underlying (non-IR) type legalization.
    TypeLegalizationContext* typeLegalizationContext;

    // When inserting new globals, put them before this one.
    IRGlobalValue* insertBeforeGlobal = nullptr;

    // When inserting new parameters, put them before this one.
    IRParam* insertBeforeParam = nullptr;

    Dictionary<IRValue*, LegalVal> mapValToLegalVal;

    IRVar* insertBeforeLocalVar = nullptr;
    // store local var instructions that have been replaced here, so we can free them
    // when legalization has done
    List<IRInst*> oldLocalVars;
};

static void registerLegalizedValue(
    IRTypeLegalizationContext*    context,
    IRValue*                    irValue,
    LegalVal const&             legalVal)
{
    context->mapValToLegalVal.Add(irValue, legalVal);
}

static LegalVal declareVars(
    IRTypeLegalizationContext*    context,
    IROp                        op,
    LegalType                   type,
    TypeLayout*                 typeLayout,
    LegalVarChain*              varChain);

static LegalType legalizeType(
    IRTypeLegalizationContext*  context,
    Type*                       type)
{
    return legalizeType(context->typeLegalizationContext, type);
}

// Legalize a type, and then expect it to
// result in a simple type.
static RefPtr<Type> legalizeSimpleType(
    IRTypeLegalizationContext*    context,
    Type*                       type)
{
    auto legalType = legalizeType(context, type);
    switch (legalType.flavor)
    {
    case LegalType::Flavor::simple:
        return legalType.getSimple();

    default:
        // TODO: need to issue a diagnostic here.
        SLANG_UNEXPECTED("unexpected type case");
        break;
    }
}

// Take a value that is being used as an operand,
// and turn it into the equivalent legalized value.
static LegalVal legalizeOperand(
    IRTypeLegalizationContext*    context,
    IRValue*                    irValue)
{
    LegalVal legalVal;
    if (context->mapValToLegalVal.TryGetValue(irValue, legalVal))
        return legalVal;

    // For now, assume that anything not covered
    // by the mapping is legal as-is.

    return LegalVal::simple(irValue);
}

static void getArgumentValues(
    List<IRValue*> & instArgs, 
    LegalVal val)
{
    switch (val.flavor)
    {
    case LegalVal::Flavor::none:
        break;

    case LegalVal::Flavor::simple:
        instArgs.Add(val.getSimple());
        break;

    case LegalVal::Flavor::implicitDeref:
        getArgumentValues(instArgs, val.getImplicitDeref());
        break;

    case LegalVal::Flavor::pair:
        {
            auto pairVal = val.getPair();
            getArgumentValues(instArgs, pairVal->ordinaryVal);
            getArgumentValues(instArgs, pairVal->specialVal);
        }
        break;

    case LegalVal::Flavor::tuple:
        {
            auto tuplePsuedoVal = val.getTuple();
            for (auto elem : val.getTuple()->elements)
            {
                getArgumentValues(instArgs, elem.val);
            }
        }
        break;

    default:
        SLANG_UNEXPECTED("uhandled val flavor");
        break;
    }
}

static LegalVal legalizeCall(
    IRTypeLegalizationContext*    context,
    IRCall* callInst)
{
    // TODO: implement legalization of non-simple return types
    auto retType = legalizeType(context, callInst->type);
    SLANG_ASSERT(retType.flavor == LegalType::Flavor::simple);
    
    List<IRValue*> instArgs;
    for (auto i = 1u; i < callInst->argCount; i++)
        getArgumentValues(instArgs, legalizeOperand(context, callInst->getArg(i)));

    return LegalVal::simple(context->builder->emitCallInst(
        callInst->type,
        callInst->func.usedValue,
        instArgs.Count(),
        instArgs.Buffer()));
}

static LegalVal legalizeLoad(
    IRTypeLegalizationContext*    context,
    LegalVal                    legalPtrVal)
{
    switch (legalPtrVal.flavor)
    {
    case LegalVal::Flavor::none:
        return LegalVal();

    case LegalVal::Flavor::simple:
        {
            return LegalVal::simple(
                context->builder->emitLoad(legalPtrVal.getSimple()));
        }
        break;

    case LegalVal::Flavor::implicitDeref:
        // We have turne a pointer(-like) type into its pointed-to (value)
        // type, and so the operation of loading goes away; we just use
        // the underlying value.
        return legalPtrVal.getImplicitDeref();

    case LegalVal::Flavor::pair:
        {
            auto ptrPairVal = legalPtrVal.getPair();

            auto ordinaryVal = legalizeLoad(context, ptrPairVal->ordinaryVal);
            auto specialVal = legalizeLoad(context, ptrPairVal->specialVal);
            return LegalVal::pair(ordinaryVal, specialVal, ptrPairVal->pairInfo);
        }

    case LegalVal::Flavor::tuple:
        {
            // We need to emit a load for each element of
            // the tuple.
            auto ptrTupleVal = legalPtrVal.getTuple();
            RefPtr<TuplePseudoVal> tupleVal = new TuplePseudoVal();

            for (auto ee : legalPtrVal.getTuple()->elements)
            {
                TuplePseudoVal::Element element;
                element.fieldDeclRef = ee.fieldDeclRef;
                element.val = legalizeLoad(context, ee.val);

                tupleVal->elements.Add(element);
            }
            return LegalVal::tuple(tupleVal);
        }
        break;

    default:
        SLANG_UNEXPECTED("unhandled case");
        break;
    }
}

static LegalVal legalizeStore(
    IRTypeLegalizationContext*    context,
    LegalVal                    legalPtrVal,
    LegalVal                    legalVal)
{
    switch (legalPtrVal.flavor)
    {
    case LegalVal::Flavor::none:
        return LegalVal();

    case LegalVal::Flavor::simple:
    {
        context->builder->emitStore(legalPtrVal.getSimple(), legalVal.getSimple());
        return legalVal;
    }
    break;

    case LegalVal::Flavor::implicitDeref:
        // TODO: what is the right behavior here?
        if (legalVal.flavor == LegalVal::Flavor::implicitDeref)
            return legalizeStore(context, legalPtrVal.getImplicitDeref(), legalVal.getImplicitDeref());
        else
            return legalizeStore(context, legalPtrVal.getImplicitDeref(), legalVal);

    case LegalVal::Flavor::pair:
        {
            auto destPair = legalPtrVal.getPair();
            auto valPair = legalVal.getPair();
            legalizeStore(context, destPair->ordinaryVal, valPair->ordinaryVal);
            legalizeStore(context, destPair->specialVal, valPair->specialVal);
            return LegalVal();
        }

    case LegalVal::Flavor::tuple:
        {
            // We need to emit a store for each element of
            // the tuple.
            auto destTuple = legalPtrVal.getTuple();
            auto valTuple = legalVal.getTuple();
            SLANG_ASSERT(destTuple->elements.Count() == valTuple->elements.Count());

            for (UInt i = 0; i < valTuple->elements.Count(); i++)
            {
                legalizeStore(context, destTuple->elements[i].val, valTuple->elements[i].val);
            }
            return legalVal;
        }
        break;

    default:
        SLANG_UNEXPECTED("unhandled case");
        break;
    }
}

static LegalVal legalizeFieldAddress(
    IRTypeLegalizationContext*    context,
    LegalType                   type,
    LegalVal                    legalPtrOperand,
    DeclRef<Decl>               fieldDeclRef)
{
    auto builder = context->builder;

    switch (legalPtrOperand.flavor)
    {
    case LegalVal::Flavor::simple:
        return LegalVal::simple(
            builder->emitFieldAddress(
                type.getSimple(),
                legalPtrOperand.getSimple(),
                builder->getDeclRefVal(fieldDeclRef)));

    case LegalVal::Flavor::pair:
        {
            // There are two sides, the ordinary and the special,
            // and we basically just dispatch to both of them.
            auto pairVal = legalPtrOperand.getPair();
            auto pairInfo = pairVal->pairInfo;
            auto pairElement = pairInfo->findElement(fieldDeclRef);
            if (!pairElement)
            {
                SLANG_UNEXPECTED("didn't find tuple element");
                UNREACHABLE_RETURN(LegalVal());
            }

            // If the field we are extracting has a pair type,
            // that means it exists on both the ordinary and
            // special sides.
            RefPtr<PairInfo> fieldPairInfo;
            LegalType ordinaryType = type;
            LegalType specialType = type;
            if (type.flavor == LegalType::Flavor::pair)
            {
                auto fieldPairType = type.getPair();
                fieldPairInfo = fieldPairType->pairInfo;
                ordinaryType = fieldPairType->ordinaryType;
                specialType = fieldPairType->specialType;
            }

            LegalVal ordinaryVal;
            LegalVal specialVal;

            if (pairElement->flags & PairInfo::kFlag_hasOrdinary)
            {
                // Note: the ordinary side of the pair is expected
                // to be a filtered `struct` type, and so it will
                // have different field declarations than the
                // oridinal type. The element of the `PairInfo`
                // structure stores the correct field decl-ref to use
                // as `ordinaryFieldDeclRef`.

                ordinaryVal = legalizeFieldAddress(
                    context,
                    ordinaryType,
                    pairVal->ordinaryVal,
                    pairElement->ordinaryFieldDeclRef);
            }

            if (pairElement->flags & PairInfo::kFlag_hasSpecial)
            {
                specialVal = legalizeFieldAddress(
                    context,
                    specialType,
                    pairVal->specialVal,
                    fieldDeclRef);
            }
            return LegalVal::pair(ordinaryVal, specialVal, fieldPairInfo);
        }
        break;

    case LegalVal::Flavor::tuple:
        {
            // The operand is a tuple of pointer-like
            // values, we want to extract the element
            // corresponding to a field. We will handle
            // this by simply returning the corresponding
            // element from the operand.
            auto ptrTupleInfo = legalPtrOperand.getTuple();
            for (auto ee : ptrTupleInfo->elements)
            {
                if (ee.fieldDeclRef.Equals(fieldDeclRef))
                {
                    return ee.val;
                }
            }

            // TODO: we can legally reach this case now
            // when the field is "ordinary".

            SLANG_UNEXPECTED("didn't find tuple element");
            UNREACHABLE_RETURN(LegalVal());
        }

    default:
        SLANG_UNEXPECTED("unhandled");
        UNREACHABLE_RETURN(LegalVal());
    }
}

static LegalVal legalizeFieldAddress(
    IRTypeLegalizationContext*    context,
    LegalType                   type,
    LegalVal                    legalPtrOperand,
    LegalVal                    legalFieldOperand)
{
    // We don't expect any legalization to affect
    // the "field" argument.
    auto fieldOperand = legalFieldOperand.getSimple();
    assert(fieldOperand->op == kIROp_decl_ref);
    auto fieldDeclRef = ((IRDeclRef*)fieldOperand)->declRef;

    return legalizeFieldAddress(
        context,
        type,
        legalPtrOperand,
        fieldDeclRef);
}

static LegalVal legalizeInst(
    IRTypeLegalizationContext*    context,
    IRInst*                     inst,
    LegalType                   type,
    LegalVal const*             args)
{
    switch (inst->op)
    {
    case kIROp_Load:
        return legalizeLoad(context, args[0]);

    case kIROp_FieldAddress:
        return legalizeFieldAddress(context, type, args[0], args[1]);

    case kIROp_Store:
        return legalizeStore(context, args[0], args[1]);

    case kIROp_Call:
        return legalizeCall(context, (IRCall*)inst);

    default:
        // TODO: produce a user-visible diagnostic here
        SLANG_UNEXPECTED("non-simple operand(s)!");
        break;
    }
}

RefPtr<VarLayout> findVarLayout(IRValue* value)
{
    if (auto layoutDecoration = value->findDecoration<IRLayoutDecoration>())
        return layoutDecoration->layout.As<VarLayout>();
    return nullptr;
}

static LegalVal legalizeLocalVar(
    IRTypeLegalizationContext*    context,
    IRVar*                irLocalVar)
{
    // Legalize the type for the variable's value
    auto legalValueType = legalizeType(
        context,
        irLocalVar->getType()->getValueType());

    RefPtr<VarLayout> varLayout = findVarLayout(irLocalVar);
    RefPtr<TypeLayout> typeLayout = varLayout ? varLayout->typeLayout : nullptr;

    // If we've decided to do implicit deref on the type,
    // then go ahead and declare a value of the pointed-to type.
    LegalType maybeSimpleType = legalValueType;
    while (maybeSimpleType.flavor == LegalType::Flavor::implicitDeref)
    {
        maybeSimpleType = maybeSimpleType.getImplicitDeref()->valueType;
    }

    switch (maybeSimpleType.flavor)
    {
    case LegalType::Flavor::simple:
        // Easy case: the type is usable as-is, and we
        // should just do that.
        irLocalVar->type = context->session->getPtrType(
            maybeSimpleType.getSimple());
        return LegalVal::simple(irLocalVar);

    default:
    {
        context->insertBeforeLocalVar = irLocalVar;

        LegalVarChain* varChain = nullptr;
        LegalVarChain varChainStorage;
        if (varLayout)
        {
            varChainStorage.next = nullptr;
            varChainStorage.varLayout = varLayout;
            varChain = &varChainStorage;
        }

        LegalVal newVal = declareVars(context, kIROp_Var, legalValueType, typeLayout, varChain);

        // Remove the old local var.
        irLocalVar->removeFromParent();
        // add old local var to list
        context->oldLocalVars.Add(irLocalVar);
        return newVal;
    }
    break;
    }
}

static LegalVal legalizeInst(
    IRTypeLegalizationContext*    context,
    IRInst*                     inst)
{
    if (inst->op == kIROp_Var)
        return legalizeLocalVar(context, (IRVar*)inst);

    // Need to legalize all the operands.
    auto argCount = inst->getArgCount();
    List<LegalVal> legalArgs;
    bool anyComplex = false;
    for (UInt aa = 0; aa < argCount; ++aa)
    {
        auto oldArg = inst->getArg(aa);
        auto legalArg = legalizeOperand(context, oldArg);
        legalArgs.Add(legalArg);

        if (legalArg.flavor != LegalVal::Flavor::simple)
            anyComplex = true;
    }

    // Also legalize the type of the instruction
    LegalType legalType = legalizeType(context, inst->type);

    if (!anyComplex && legalType.flavor == LegalType::Flavor::simple)
    {
        // Nothing interesting happened to the operands,
        // so we seem to be okay, right?

        for (UInt aa = 0; aa < argCount; ++aa)
        {
            auto legalArg = legalArgs[aa];
            inst->setArg(aa, legalArg.getSimple());
        }

        inst->type = legalType.getSimple();

        return LegalVal::simple(inst);
    }

    // We have at least one "complex" operand, and we
    // need to figure out what to do with it. The anwer
    // will, in general, depend on what we are doing.

    // We will set up the IR builder so that any new
    // instructions generated will be placed after
    // the location of the original instruct.
    auto builder = context->builder;
    builder->curBlock = inst->getParentBlock();
    builder->insertBeforeInst = inst->getNextInst();

    LegalVal legalVal = legalizeInst(
        context,
        inst,
        legalType,
        legalArgs.Buffer());

    // After we are done, we will eliminate the
    // original instruction by removing it from
    // the IR.
    //
    // TODO: we need to add it to a list of
    // instructions to be cleaned up...
    inst->removeFromParent();

    // The value to be used when referencing
    // the original instruction will now be
    // whatever value(s) we created to replace it.
    return legalVal;
}

static void addParamType(IRFuncType * ftype, LegalType t)
{
    switch (t.flavor)
    {
    case LegalType::Flavor::none:
        break;
    case LegalType::Flavor::simple:
        ftype->paramTypes.Add(t.obj.As<Type>());
        break;
    case LegalType::Flavor::implicitDeref:
    {
        auto imp = t.obj.As<ImplicitDerefType>();
        addParamType(ftype, imp->valueType);
        break;
    }
    case LegalType::Flavor::pair:
        {
            auto pairInfo = t.getPair();
            addParamType(ftype, pairInfo->ordinaryType);
            addParamType(ftype, pairInfo->specialType);
        }
        break;
    case LegalType::Flavor::tuple:
    {
        auto tup = t.obj.As<TuplePseudoType>();
        for (auto & elem : tup->elements)
            addParamType(ftype, elem.type);
    }
    break;
    default:
        SLANG_ASSERT(false);
    }
}

static void legalizeFunc(
    IRTypeLegalizationContext*    context,
    IRFunc*                     irFunc)
{
    // Overwrite the function's type with
    // the result of legalization.
    auto newFuncType = new IRFuncType();
    newFuncType->setSession(context->session);
    auto oldFuncType = irFunc->type.As<IRFuncType>();
    newFuncType->resultType = legalizeSimpleType(context, oldFuncType->resultType);
    for (auto & paramType : oldFuncType->paramTypes)
    {
        auto legalParamType = legalizeType(context, paramType);
        addParamType(newFuncType, legalParamType);
    }
    irFunc->type = newFuncType;
    List<LegalVal> paramVals;
    List<IRValue*> oldParams;

    // we use this list to store replaced local var insts.
    // these old instructions will be freed when we are done.
    context->oldLocalVars.Clear();
    
    // Go through the blocks of the function
    for (auto bb = irFunc->getFirstBlock(); bb; bb = bb->getNextBlock())
    {
        // Legalize the parameters of the block, which may
        // involve increasing the number of parameters
        for (auto pp = bb->getFirstParam(); pp; pp = pp->nextParam)
        {
            auto legalParamType = legalizeType(context, pp->getType());
            if (legalParamType.flavor != LegalType::Flavor::simple)
            {
                context->insertBeforeParam = pp;
                context->builder->curBlock = nullptr;

                auto paramVal = declareVars(context, kIROp_Param, legalParamType, nullptr, nullptr);
                paramVals.Add(paramVal);
                if (pp == bb->getFirstParam())
                {
                    bb->firstParam = pp;
                    while (bb->firstParam->prevParam)
                        bb->firstParam = bb->firstParam->prevParam;
                }
                bb->lastParam = pp->prevParam;
                if (pp->prevParam)
                    pp->prevParam->nextParam = pp->nextParam;
                if (pp->nextParam)
                    pp->nextParam->prevParam = pp->prevParam;
                auto oldParam = pp;
                oldParams.Add(oldParam);
                registerLegalizedValue(context, oldParam, paramVal);
            }
           
        }

        // Now legalize the instructions inside the block
        IRInst* nextInst = nullptr;
        for (auto ii = bb->getFirstInst(); ii; ii = nextInst)
        {
            nextInst = ii->getNextInst();

            LegalVal legalVal = legalizeInst(context, ii);

            registerLegalizedValue(context, ii, legalVal);
        }

    }
    for (auto & op : oldParams)
    {
        SLANG_ASSERT(op->firstUse == nullptr || op->firstUse->nextUse == nullptr);
        op->deallocate();
    }
    for (auto & lv : context->oldLocalVars)
        lv->deallocate();
}

static LegalVal declareSimpleVar(
    IRTypeLegalizationContext*    context,
    IROp                        op,
    Type*                       type,
    TypeLayout*                 typeLayout,
    LegalVarChain*              varChain)
{
    RefPtr<VarLayout> varLayout = createVarLayout(varChain, typeLayout);

    DeclRef<VarDeclBase> varDeclRef;
    if (varChain)
    {
        varDeclRef = varChain->varLayout->varDecl;
    }

    IRBuilder* builder = context->builder;

    IRValue*    irVar = nullptr;
    LegalVal    legalVarVal;

    switch (op)
    {
    case kIROp_global_var:
        {
            auto globalVar = builder->createGlobalVar(type);
            globalVar->removeFromParent();
            globalVar->insertBefore(context->insertBeforeGlobal);

            irVar = globalVar;
            legalVarVal = LegalVal::simple(irVar);
        }
        break;

    case kIROp_Var:
        {
            auto localVar = builder->emitVar(type);
            localVar->removeFromParent();
            localVar->insertBefore(context->insertBeforeLocalVar);

            irVar = localVar;
            legalVarVal = LegalVal::simple(irVar);

        }
        break;

    case kIROp_Param:
        {
            auto param = builder->emitParam(type);
            if (context->insertBeforeParam->prevParam)
                context->insertBeforeParam->prevParam->nextParam = param;
            param->prevParam = context->insertBeforeParam->prevParam;
            param->nextParam = context->insertBeforeParam;
            context->insertBeforeParam->prevParam = param;

            irVar = param;
            legalVarVal = LegalVal::simple(irVar);
        }
        break;

    default:
        SLANG_UNEXPECTED("unexpected IR opcode");
        break;
    }

    if (irVar)
    {
        if (varLayout)
        {
            builder->addLayoutDecoration(irVar, varLayout);
        }

        if (varDeclRef)
        {
            builder->addHighLevelDeclDecoration(irVar, varDeclRef.getDecl());
        }
    }

    return legalVarVal;
}

static LegalVal declareVars(
    IRTypeLegalizationContext*    context,
    IROp                        op,
    LegalType                   type,
    TypeLayout*                 typeLayout,
    LegalVarChain*              varChain)
{
    switch (type.flavor)
    {
    case LegalType::Flavor::none:
        return LegalVal();

    case LegalType::Flavor::simple:
        return declareSimpleVar(context, op, type.getSimple(), typeLayout, varChain);
        break;

    case LegalType::Flavor::implicitDeref:
        {
            // Just declare a variable of the pointed-to type,
            // since we are removing the indirection.

            auto val = declareVars(
                context,
                op,
                type.getImplicitDeref()->valueType,
                getDerefTypeLayout(typeLayout),
                varChain);
            return LegalVal::implicitDeref(val);
        }
        break;

    case LegalType::Flavor::pair:
        {
            auto pairType = type.getPair();
            auto ordinaryVal = declareVars(context, op, pairType->ordinaryType, typeLayout, varChain);
            auto specialVal = declareVars(context, op, pairType->specialType, typeLayout, varChain);
            return LegalVal::pair(ordinaryVal, specialVal, pairType->pairInfo);
        }

    case LegalType::Flavor::tuple:
        {
            // Declare one variable for each element of the tuple
            auto tupleType = type.getTuple();

            RefPtr<TuplePseudoVal> tupleVal = new TuplePseudoVal();

            for (auto ee : tupleType->elements)
            {
                auto fieldLayout = getFieldLayout(typeLayout, ee.fieldDeclRef);
                RefPtr<TypeLayout> fieldTypeLayout = fieldLayout ? fieldLayout->typeLayout : nullptr;

                // If we are processing layout information, then
                // we need to create a new link in the chain
                // of variables that will determine offsets
                // for the eventual leaf fields...
                LegalVarChain newVarChainStorage;
                LegalVarChain* newVarChain = varChain;
                if (fieldLayout)
                {
                    newVarChainStorage.next = varChain;
                    newVarChainStorage.varLayout = fieldLayout;
                    newVarChain = &newVarChainStorage;
                }

                LegalVal fieldVal = declareVars(
                    context,
                    op,
                    ee.type,
                    fieldTypeLayout,
                    newVarChain);

                TuplePseudoVal::Element element;
                element.fieldDeclRef = ee.fieldDeclRef;
                element.val = fieldVal;
                tupleVal->elements.Add(element);
            }

            return LegalVal::tuple(tupleVal);
        }
        break;

    default:
        SLANG_UNEXPECTED("unhandled");
        break;
    }
}

static void legalizeGlobalVar(
    IRTypeLegalizationContext*    context,
    IRGlobalVar*                irGlobalVar)
{
    // Legalize the type for the variable's value
    auto legalValueType = legalizeType(
        context,
        irGlobalVar->getType()->getValueType());

    RefPtr<VarLayout> varLayout = findVarLayout(irGlobalVar);
    RefPtr<TypeLayout> typeLayout = varLayout ? varLayout->typeLayout : nullptr;

    switch (legalValueType.flavor)
    {
    case LegalType::Flavor::simple:
        // Easy case: the type is usable as-is, and we
        // should just do that.
        irGlobalVar->type = context->session->getPtrType(
            legalValueType.getSimple());
        break;

    default:
        {
            context->insertBeforeGlobal = irGlobalVar->getNextValue();

            LegalVarChain* varChain = nullptr;
            LegalVarChain varChainStorage;
            if (varLayout)
            {
                varChainStorage.next = nullptr;
                varChainStorage.varLayout = varLayout;
                varChain = &varChainStorage;
            }

            LegalVal newVal = declareVars(context, kIROp_global_var, legalValueType, typeLayout, varChain);

            // Register the new value as the replacement for the old
            registerLegalizedValue(context, irGlobalVar, newVal);

            // Remove the old global from the module.
            irGlobalVar->removeFromParent();
            // TODO: actually clean up the global!
        }
        break;
    }
}

static void legalizeGlobalValue(
    IRTypeLegalizationContext*    context,
    IRGlobalValue*              irValue)
{
    switch (irValue->op)
    {
    case kIROp_witness_table:
        // Just skip these.
        break;

    case kIROp_Func:
        legalizeFunc(context, (IRFunc*)irValue);
        break;

    case kIROp_global_var:
        legalizeGlobalVar(context, (IRGlobalVar*)irValue);
        break;

    default:
        SLANG_UNEXPECTED("unknown global value type");
        break;
    }
}

static void legalizeTypes(
    IRTypeLegalizationContext*    context)
{
    auto module = context->module;
    for (auto gv = module->getFirstGlobalValue(); gv; gv = gv->getNextValue())
    {
        legalizeGlobalValue(context, gv);
    }
}


void legalizeTypes(
    TypeLegalizationContext*    typeLegalizationContext,
    IRModule*                   module)
{
    auto session = module->session;

    SharedIRBuilder sharedBuilderStorage;
    auto sharedBuilder = &sharedBuilderStorage;

    sharedBuilder->session = session;
    sharedBuilder->module = module;

    IRBuilder builderStorage;
    auto builder = &builderStorage;

    builder->sharedBuilder = sharedBuilder;


    IRTypeLegalizationContext contextStorage;
    auto context = &contextStorage;

    context->session = session;
    context->module = module;
    context->builder = builder;

    context->typeLegalizationContext = typeLegalizationContext;

    legalizeTypes(context);

}

}
