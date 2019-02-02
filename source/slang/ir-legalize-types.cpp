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
#include "mangle.h"
#include "name.h"

namespace Slang
{

LegalVal LegalVal::tuple(RefPtr<TuplePseudoVal> tupleVal)
{
    SLANG_ASSERT(tupleVal->elements.Count());

    LegalVal result;
    result.flavor = LegalVal::Flavor::tuple;
    result.obj = tupleVal;
    return result;
}

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
    SLANG_ASSERT(flavor == Flavor::implicitDeref);
    return as<ImplicitDerefVal>(obj)->val;
}


struct IRTypeLegalizationContext
{
    Session*    session;
    IRModule*   module;
    IRBuilder*  builder;

    /// Context to use for underlying (non-IR) type legalization.
    TypeLegalizationContext* typeLegalizationContext;

    // When inserting new globals, put them before this one.
    IRInst* insertBeforeGlobal = nullptr;

    // When inserting new parameters, put them before this one.
    IRParam* insertBeforeParam = nullptr;

    Dictionary<IRInst*, LegalVal> mapValToLegalVal;

    IRVar* insertBeforeLocalVar = nullptr;

    // store instructions that have been replaced here, so we can free them
    // when legalization has done
    List<IRInst*> replacedInstructions;
};

static void registerLegalizedValue(
    IRTypeLegalizationContext*  context,
    IRInst*                     irValue,
    LegalVal const&             legalVal)
{
    context->mapValToLegalVal[irValue] = legalVal;
}

struct IRGlobalNameInfo
{
    IRInst*         globalVar;
    UInt            counter;
};

static LegalVal declareVars(
    IRTypeLegalizationContext*  context,
    IROp                        op,
    LegalType                   type,
    TypeLayout*                 typeLayout,
    LegalVarChain*              varChain,
    UnownedStringSlice          nameHint,
    IRGlobalNameInfo*           globalNameInfo);

static LegalType legalizeType(
    IRTypeLegalizationContext*  context,
    IRType*                     type)
{
    return legalizeType(context->typeLegalizationContext, type);
}

// Take a value that is being used as an operand,
// and turn it into the equivalent legalized value.
static LegalVal legalizeOperand(
    IRTypeLegalizationContext*    context,
    IRInst*                    irValue)
{
    LegalVal legalVal;
    if (context->mapValToLegalVal.TryGetValue(irValue, legalVal))
        return legalVal;

    // For now, assume that anything not covered
    // by the mapping is legal as-is.

    return LegalVal::simple(irValue);
}

static void getArgumentValues(
    List<IRInst*> & instArgs,
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
    auto retType = legalizeType(context, callInst->getFullType());
    IRType* retIRType = nullptr;
    switch (retType.flavor)
    {
    case LegalType::Flavor::simple:
        retIRType = retType.getSimple();
        break;
    case LegalType::Flavor::none:
        retIRType = context->builder->getVoidType();
        break;
    default:
        // TODO: implement legalization of non-simple return types
        SLANG_UNEXPECTED("unimplemented legalized return type for IRInstCall.");
    }

    List<IRInst*> instArgs;
    for (auto i = 1u; i < callInst->getOperandCount(); i++)
        getArgumentValues(instArgs, legalizeOperand(context, callInst->getOperand(i)));

    return LegalVal::simple(context->builder->emitCallInst(
        retIRType,
        callInst->getCallee(),
        instArgs.Count(),
        instArgs.Buffer()));
}

static LegalVal legalizeRetVal(IRTypeLegalizationContext*    context,
    LegalVal retVal)
{
    switch (retVal.flavor)
    {
    case LegalVal::Flavor::simple:
        return LegalVal::simple(context->builder->emitReturn(retVal.getSimple()));
    case LegalVal::Flavor::none:
        return LegalVal::simple(context->builder->emitReturn());
    default:
        // TODO: implement legalization of non-simple return types
        SLANG_UNEXPECTED("unimplemented legalized return type for IRReturnVal.");
    }
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
                element.key = ee.key;
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

static LegalVal legalizeFieldExtract(
    IRTypeLegalizationContext*  context,
    LegalType                   type,
    LegalVal                    legalStructOperand,
    IRStructKey*                fieldKey)
{
    auto builder = context->builder;

    if (type.flavor == LegalType::Flavor::none)
        return LegalVal();

    switch (legalStructOperand.flavor)
    {
    case LegalVal::Flavor::none:
        return LegalVal();

    case LegalVal::Flavor::simple:
        return LegalVal::simple(
            builder->emitFieldExtract(
                type.getSimple(),
                legalStructOperand.getSimple(),
                fieldKey));

    case LegalVal::Flavor::pair:
        {
            // There are two sides, the ordinary and the special,
            // and we basically just dispatch to both of them.
            auto pairVal = legalStructOperand.getPair();
            auto pairInfo = pairVal->pairInfo;
            auto pairElement = pairInfo->findElement(fieldKey);
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
                ordinaryVal = legalizeFieldExtract(
                    context,
                    ordinaryType,
                    pairVal->ordinaryVal,
                    fieldKey);
            }

            if (pairElement->flags & PairInfo::kFlag_hasSpecial)
            {
                specialVal = legalizeFieldExtract(
                    context,
                    specialType,
                    pairVal->specialVal,
                    fieldKey);
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
            auto ptrTupleInfo = legalStructOperand.getTuple();
            for (auto ee : ptrTupleInfo->elements)
            {
                if (ee.key == fieldKey)
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

static LegalVal legalizeFieldExtract(
    IRTypeLegalizationContext*    context,
    LegalType                   type,
    LegalVal                    legalPtrOperand,
    LegalVal                    legalFieldOperand)
{
    // We don't expect any legalization to affect
    // the "field" argument.
    auto fieldKey = legalFieldOperand.getSimple();

    return legalizeFieldExtract(
        context,
        type,
        legalPtrOperand,
        (IRStructKey*) fieldKey);
}

static LegalVal legalizeFieldAddress(
    IRTypeLegalizationContext*    context,
    LegalType                   type,
    LegalVal                    legalPtrOperand,
    IRStructKey*                fieldKey)
{
    auto builder = context->builder;
    if (type.flavor == LegalType::Flavor::none)
        return LegalVal();

    switch (legalPtrOperand.flavor)
    {
    case LegalVal::Flavor::none:
        return LegalVal();

    case LegalVal::Flavor::simple:
        return LegalVal::simple(
            builder->emitFieldAddress(
                type.getSimple(),
                legalPtrOperand.getSimple(),
                fieldKey));

    case LegalVal::Flavor::pair:
        {
            // There are two sides, the ordinary and the special,
            // and we basically just dispatch to both of them.
            auto pairVal = legalPtrOperand.getPair();
            auto pairInfo = pairVal->pairInfo;
            auto pairElement = pairInfo->findElement(fieldKey);
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
                ordinaryVal = legalizeFieldAddress(
                    context,
                    ordinaryType,
                    pairVal->ordinaryVal,
                    fieldKey);
            }

            if (pairElement->flags & PairInfo::kFlag_hasSpecial)
            {
                specialVal = legalizeFieldAddress(
                    context,
                    specialType,
                    pairVal->specialVal,
                    fieldKey);
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
                if (ee.key == fieldKey)
                {
                    return ee.val;
                }
            }

            // TODO: we can legally reach this case now
            // when the field is "ordinary".

            SLANG_UNEXPECTED("didn't find tuple element");
            UNREACHABLE_RETURN(LegalVal());
        }

    case LegalVal::Flavor::implicitDeref:
        {
            // The original value had a level of indirection
            // that is now being removed, so should not be
            // able to get at the *address* of the field any
            // more, and need to resign ourselves to just
            // getting at the field *value* and then
            // adding an implicit dereference on top of that.
            //
            auto implicitDerefVal = legalPtrOperand.getImplicitDeref();

            return LegalVal::implicitDeref(legalizeFieldExtract(context,type, implicitDerefVal, fieldKey));
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
    auto fieldKey = legalFieldOperand.getSimple();

    return legalizeFieldAddress(
        context,
        type,
        legalPtrOperand,
        (IRStructKey*) fieldKey);
}

static LegalVal legalizeGetElement(
    IRTypeLegalizationContext*  context,
    LegalType                   type,
    LegalVal                    legalPtrOperand,
    IRInst*                    indexOperand)
{
    auto builder = context->builder;

    switch (legalPtrOperand.flavor)
    {
    case LegalVal::Flavor::none:
        return LegalVal();

    case LegalVal::Flavor::simple:
        return LegalVal::simple(
            builder->emitElementExtract(
                type.getSimple(),
                legalPtrOperand.getSimple(),
                indexOperand));

    case LegalVal::Flavor::pair:
        {
            // There are two sides, the ordinary and the special,
            // and we basically just dispatch to both of them.
            auto pairVal = legalPtrOperand.getPair();
            auto pairInfo = pairVal->pairInfo;

            LegalType ordinaryType = type;
            LegalType specialType = type;
            if (type.flavor == LegalType::Flavor::pair)
            {
                auto pairType = type.getPair();
                ordinaryType = pairType->ordinaryType;
                specialType = pairType->specialType;
            }

            LegalVal ordinaryVal = legalizeGetElement(
                context,
                ordinaryType,
                pairVal->ordinaryVal,
                indexOperand);

            LegalVal specialVal = legalizeGetElement(
                context,
                specialType,
                pairVal->specialVal,
                indexOperand);

            return LegalVal::pair(ordinaryVal, specialVal, pairInfo);
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

            RefPtr<TuplePseudoVal> resTupleInfo = new TuplePseudoVal();

            auto tupleType = type.getTuple();
            SLANG_ASSERT(tupleType);

            auto elemCount = ptrTupleInfo->elements.Count();
            SLANG_ASSERT(elemCount == tupleType->elements.Count());

            for(UInt ee = 0; ee < elemCount; ++ee)
            {
                auto ptrElem = ptrTupleInfo->elements[ee];
                auto elemType = tupleType->elements[ee].type;

                TuplePseudoVal::Element resElem;
                resElem.key = ptrElem.key;
                resElem.val = legalizeGetElement(
                    context,
                    elemType,
                    ptrElem.val,
                    indexOperand);

                resTupleInfo->elements.Add(resElem);
            }

            return LegalVal::tuple(resTupleInfo);
        }

    default:
        SLANG_UNEXPECTED("unhandled");
        UNREACHABLE_RETURN(LegalVal());
    }
}

static LegalVal legalizeGetElement(
    IRTypeLegalizationContext*  context,
    LegalType                   type,
    LegalVal                    legalPtrOperand,
    LegalVal                    legalIndexOperand)
{
    // We don't expect any legalization to affect
    // the "index" argument.
    auto indexOperand = legalIndexOperand.getSimple();

    return legalizeGetElement(
        context,
        type,
        legalPtrOperand,
        indexOperand);
}


static LegalVal legalizeGetElementPtr(
    IRTypeLegalizationContext*  context,
    LegalType                   type,
    LegalVal                    legalPtrOperand,
    IRInst*                    indexOperand)
{
    auto builder = context->builder;

    switch (legalPtrOperand.flavor)
    {
    case LegalVal::Flavor::none:
        return LegalVal();

    case LegalVal::Flavor::simple:
        return LegalVal::simple(
            builder->emitElementAddress(
                type.getSimple(),
                legalPtrOperand.getSimple(),
                indexOperand));

    case LegalVal::Flavor::pair:
        {
            // There are two sides, the ordinary and the special,
            // and we basically just dispatch to both of them.
            auto pairVal = legalPtrOperand.getPair();
            auto pairInfo = pairVal->pairInfo;

            LegalType ordinaryType = type;
            LegalType specialType = type;
            if (type.flavor == LegalType::Flavor::pair)
            {
                auto pairType = type.getPair();
                ordinaryType = pairType->ordinaryType;
                specialType = pairType->specialType;
            }

            LegalVal ordinaryVal = legalizeGetElementPtr(
                context,
                ordinaryType,
                pairVal->ordinaryVal,
                indexOperand);

            LegalVal specialVal = legalizeGetElementPtr(
                context,
                specialType,
                pairVal->specialVal,
                indexOperand);

            return LegalVal::pair(ordinaryVal, specialVal, pairInfo);
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

            RefPtr<TuplePseudoVal> resTupleInfo = new TuplePseudoVal();

            auto tupleType = type.getTuple();
            SLANG_ASSERT(tupleType);

            auto elemCount = ptrTupleInfo->elements.Count();
            SLANG_ASSERT(elemCount == tupleType->elements.Count());

            for(UInt ee = 0; ee < elemCount; ++ee)
            {
                auto ptrElem = ptrTupleInfo->elements[ee];
                auto elemType = tupleType->elements[ee].type;

                TuplePseudoVal::Element resElem;
                resElem.key = ptrElem.key;
                resElem.val = legalizeGetElementPtr(
                    context,
                    elemType,
                    ptrElem.val,
                    indexOperand);

                resTupleInfo->elements.Add(resElem);
            }

            return LegalVal::tuple(resTupleInfo);
        }

    case LegalVal::Flavor::implicitDeref:
        {
            // The original value used to be a pointer to an array,
            // and somebody is trying to get at an element pointer.
            // Now we just have an array (wrapped with an implicit
            // dereference) and need to just fetch the chosen element
            // instead (and then wrapp the element value with an
            // implicit dereference).
            //
            auto implicitDerefVal = legalPtrOperand.getImplicitDeref();
            return LegalVal::implicitDeref(legalizeGetElement(
                context,
                type,
                implicitDerefVal,
                indexOperand));
        }

    default:
        SLANG_UNEXPECTED("unhandled");
        UNREACHABLE_RETURN(LegalVal());
    }
}

static LegalVal legalizeGetElementPtr(
    IRTypeLegalizationContext*  context,
    LegalType                   type,
    LegalVal                    legalPtrOperand,
    LegalVal                    legalIndexOperand)
{
    // We don't expect any legalization to affect
    // the "index" argument.
    auto indexOperand = legalIndexOperand.getSimple();

    return legalizeGetElementPtr(
        context,
        type,
        legalPtrOperand,
        indexOperand);
}

static LegalVal legalizeMakeStruct(
    IRTypeLegalizationContext*  context,
    LegalType                   legalType,
    LegalVal const*             legalArgs,
    UInt                        argCount)
{
    auto builder = context->builder;

    switch(legalType.flavor)
    {
    case LegalType::Flavor::none:
        return LegalVal();

    case LegalType::Flavor::simple:
        {
            List<IRInst*> args;
            for(UInt aa = 0; aa < argCount; ++aa)
            {
                // Note: we assume that all the arguments
                // must be simple here, because otherwise
                // the `struct` type with them as fields
                // would not be simple...
                //
                args.Add(legalArgs[aa].getSimple());
            }
            return LegalVal::simple(
                builder->emitMakeStruct(
                    legalType.getSimple(),
                    argCount,
                    args.Buffer()));
        }

    case LegalType::Flavor::pair:
        {
            // There are two sides, the ordinary and the special,
            // and we basically just dispatch to both of them.
            auto pairType = legalType.getPair();
            auto pairInfo = pairType->pairInfo;
            LegalType ordinaryType = pairType->ordinaryType;
            LegalType specialType = pairType->specialType;

            List<LegalVal> ordinaryArgs;
            List<LegalVal> specialArgs;
            UInt argCounter = 0;
            for(auto ee : pairInfo->elements)
            {
                UInt argIndex = argCounter++;
                LegalVal arg = legalArgs[argIndex];

                if((ee.flags & Slang::PairInfo::kFlag_hasOrdinaryAndSpecial) == Slang::PairInfo::kFlag_hasOrdinaryAndSpecial)
                {
                    // The field is itself a pair type, so we expect
                    // the argument value to be one too...
                    auto argPair = arg.getPair();
                    ordinaryArgs.Add(argPair->ordinaryVal);
                    specialArgs.Add(argPair->specialVal);
                }
                else if(ee.flags & Slang::PairInfo::kFlag_hasOrdinary)
                {
                    ordinaryArgs.Add(arg);
                }
                else if(ee.flags & Slang::PairInfo::kFlag_hasSpecial)
                {
                    specialArgs.Add(arg);
                }
            }

            LegalVal ordinaryVal = legalizeMakeStruct(
                context,
                ordinaryType,
                ordinaryArgs.Buffer(),
                ordinaryArgs.Count());

            LegalVal specialVal = legalizeMakeStruct(
                context,
                specialType,
                specialArgs.Buffer(),
                specialArgs.Count());

            return LegalVal::pair(ordinaryVal, specialVal, pairInfo);
        }
        break;

    case LegalType::Flavor::tuple:
        {
            // We are constructing a tuple of values from
            // the individual fields. We need to identify
            // for each tuple element what field it uses,
            // and then extract that field's value.

            auto tupleType = legalType.getTuple();

            RefPtr<TuplePseudoVal> resTupleInfo = new TuplePseudoVal();
            UInt argCounter = 0;
            for(auto typeElem : tupleType->elements)
            {
                auto elemKey = typeElem.key;
                UInt argIndex = argCounter++;
                SLANG_ASSERT(argIndex < argCount);

                LegalVal argVal = legalArgs[argIndex];

                TuplePseudoVal::Element resElem;
                resElem.key = elemKey;
                resElem.val = argVal;

                resTupleInfo->elements.Add(resElem);
            }
            return LegalVal::tuple(resTupleInfo);
        }

    default:
        SLANG_UNEXPECTED("unhandled");
        UNREACHABLE_RETURN(LegalVal());
    }
}

static LegalVal legalizeConstruct(IRTypeLegalizationContext*    context,
    LegalType                   type)
{
    switch (type.flavor)
    {
    case LegalType::Flavor::none:
        return LegalVal();
    case LegalType::Flavor::simple:
        return LegalVal::simple(context->builder->emitConstructorInst(type.getSimple(), 0, nullptr));
    default:
        SLANG_UNEXPECTED("unhandled legalization case for construct inst.");
        UNREACHABLE_RETURN(LegalVal());
    }
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

    case kIROp_FieldExtract:
        return legalizeFieldExtract(context, type, args[0], args[1]);

    case kIROp_getElement:
        return legalizeGetElement(context, type, args[0], args[1]);

    case kIROp_getElementPtr:
        return legalizeGetElementPtr(context, type, args[0], args[1]);

    case kIROp_Store:
        return legalizeStore(context, args[0], args[1]);

    case kIROp_Call:
        return legalizeCall(context, (IRCall*)inst);
    case kIROp_ReturnVal:
        return legalizeRetVal(context, args[0]);
    case kIROp_makeStruct:
        return legalizeMakeStruct(
            context,
            type,
            args,
            inst->getOperandCount());
    case kIROp_Construct:
        return legalizeConstruct(context, type);
    case kIROp_undefined:
        return LegalVal();
    default:
        // TODO: produce a user-visible diagnostic here
        SLANG_UNEXPECTED("non-simple operand(s)!");
        break;
    }
}

RefPtr<VarLayout> findVarLayout(IRInst* value)
{
    if (auto layoutDecoration = value->findDecoration<IRLayoutDecoration>())
        return as<VarLayout>(layoutDecoration->getLayout());
    return nullptr;
}

static UnownedStringSlice findNameHint(IRInst* inst)
{
    if( auto nameHintDecoration = inst->findDecoration<IRNameHintDecoration>() )
    {
        return nameHintDecoration->getName();
    }
    return UnownedStringSlice();
}

static LegalVal legalizeLocalVar(
    IRTypeLegalizationContext*    context,
    IRVar*                irLocalVar)
{
    // Legalize the type for the variable's value
    auto legalValueType = legalizeType(
        context,
        irLocalVar->getDataType()->getValueType());

    auto originalRate = irLocalVar->getRate();

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
        {
            // Easy case: the type is usable as-is, and we
            // should just do that.
            auto type = maybeSimpleType.getSimple();
            type = context->builder->getPtrType(type);
            if( originalRate )
            {
                type = context->builder->getRateQualifiedType(
                    originalRate,
                    type);
            }
            irLocalVar->setFullType(type);
            return LegalVal::simple(irLocalVar);
        }

    default:
    {
        // TODO: We don't handle rates in this path.

        context->insertBeforeLocalVar = irLocalVar;

        LegalVarChain* varChain = nullptr;
        LegalVarChain varChainStorage;
        if (varLayout)
        {
            varChainStorage.next = nullptr;
            varChainStorage.varLayout = varLayout;
            varChain = &varChainStorage;
        }

        UnownedStringSlice nameHint = findNameHint(irLocalVar);
        LegalVal newVal = declareVars(context, kIROp_Var, legalValueType, typeLayout, varChain, nameHint, nullptr);

        // Remove the old local var.
        irLocalVar->removeFromParent();
        // add old local var to list
        context->replacedInstructions.Add(irLocalVar);
        return newVal;
    }
    break;
    }
}

static LegalVal legalizeParam(
    IRTypeLegalizationContext*  context,
    IRParam*                    originalParam)
{
    auto legalParamType = legalizeType(context, originalParam->getFullType());
    if (legalParamType.flavor == LegalType::Flavor::simple)
    {
        // Simple case: things were legalized to a simple type,
        // so we can just use the original parameter as-is.
        originalParam->setFullType(legalParamType.getSimple());
        return LegalVal::simple(originalParam);
    }
    else
    {
        // Complex case: we need to insert zero or more new parameters,
        // which will replace the old ones.

        context->insertBeforeParam = originalParam;

        UnownedStringSlice nameHint = findNameHint(originalParam);
        auto newVal = declareVars(context, kIROp_Param, legalParamType, nullptr, nullptr, nameHint, nullptr);

        originalParam->removeFromParent();
        context->replacedInstructions.Add(originalParam);
        return newVal;
    }
}

static LegalVal legalizeFunc(
    IRTypeLegalizationContext*  context,
    IRFunc*                     irFunc);

static LegalVal legalizeGlobalVar(
    IRTypeLegalizationContext*    context,
    IRGlobalVar*                irGlobalVar);

static LegalVal legalizeGlobalConstant(
    IRTypeLegalizationContext*  context,
    IRGlobalConstant*           irGlobalConstant);

static LegalVal legalizeGlobalParam(
    IRTypeLegalizationContext*  context,
    IRGlobalParam*              irGlobalParam);

static LegalVal legalizeInst(
    IRTypeLegalizationContext*  context,
    IRInst*                     inst)
{
    // Special-case certain operations
    switch (inst->op)
    {
    case kIROp_Var:
        return legalizeLocalVar(context, cast<IRVar>(inst));

    case kIROp_Param:
        return legalizeParam(context, cast<IRParam>(inst));

    case kIROp_WitnessTable:
        // Just skip these.
        break;

    case kIROp_Func:
        return legalizeFunc(context, cast<IRFunc>(inst));

    case kIROp_GlobalVar:
        return legalizeGlobalVar(context, cast<IRGlobalVar>(inst));

    case kIROp_GlobalConstant:
        return legalizeGlobalConstant(context, cast<IRGlobalConstant>(inst));

    case kIROp_GlobalParam:
        return legalizeGlobalParam(context, cast<IRGlobalParam>(inst));

    default:
        break;
    }

    // Need to legalize all the operands.
    auto argCount = inst->getOperandCount();
    List<LegalVal> legalArgs;
    bool anyComplex = false;
    for (UInt aa = 0; aa < argCount; ++aa)
    {
        auto oldArg = inst->getOperand(aa);
        auto legalArg = legalizeOperand(context, oldArg);
        legalArgs.Add(legalArg);

        if (legalArg.flavor != LegalVal::Flavor::simple)
            anyComplex = true;
    }

    // Also legalize the type of the instruction
    LegalType legalType = legalizeType(context, inst->getFullType());

    if (!anyComplex && legalType.flavor == LegalType::Flavor::simple)
    {
        // Nothing interesting happened to the operands,
        // so we seem to be okay, right?

        for (UInt aa = 0; aa < argCount; ++aa)
        {
            auto legalArg = legalArgs[aa];
            inst->setOperand(aa, legalArg.getSimple());
        }

        inst->setFullType(legalType.getSimple());

        return LegalVal::simple(inst);
    }

    // We have at least one "complex" operand, and we
    // need to figure out what to do with it. The anwer
    // will, in general, depend on what we are doing.

    // We will set up the IR builder so that any new
    // instructions generated will be placed before
    // the location of the original instruction.
    auto builder = context->builder;
    builder->setInsertBefore(inst);

    LegalVal legalVal = legalizeInst(
        context,
        inst,
        legalType,
        legalArgs.Buffer());

    // After we are done, we will eliminate the
    // original instruction by removing it from
    // the IR.
    //
    inst->removeFromParent();
    context->replacedInstructions.Add(inst);

    // The value to be used when referencing
    // the original instruction will now be
    // whatever value(s) we created to replace it.
    return legalVal;
}

static void addParamType(List<IRType*>& ioParamTypes, LegalType t)
{
    switch (t.flavor)
    {
    case LegalType::Flavor::none:
        break;

    case LegalType::Flavor::simple:
        ioParamTypes.Add(t.getSimple());
        break;

    case LegalType::Flavor::implicitDeref:
    {
        auto imp = t.getImplicitDeref();
        addParamType(ioParamTypes, imp->valueType);
        break;
    }
    case LegalType::Flavor::pair:
        {
            auto pairInfo = t.getPair();
            addParamType(ioParamTypes, pairInfo->ordinaryType);
            addParamType(ioParamTypes, pairInfo->specialType);
        }
        break;
    case LegalType::Flavor::tuple:
    {
        auto tup = t.getTuple();
        for (auto & elem : tup->elements)
            addParamType(ioParamTypes, elem.type);
    }
    break;
    default:
        SLANG_UNEXPECTED("unknown legalized type flavor");
    }
}

static void legalizeInstsInParent(
    IRTypeLegalizationContext*  context,
    IRInst*                     parent)
{
    IRInst* nextChild = nullptr;
    for(auto child = parent->getFirstDecorationOrChild(); child; child = nextChild)
    {
        nextChild = child->getNextInst();

        if (auto block = as<IRBlock>(child))
        {
            legalizeInstsInParent(context, block);
        }
        else
        {
            LegalVal legalVal = legalizeInst(context, child);
            registerLegalizedValue(context, child, legalVal);
        }
    }
}

static LegalVal legalizeFunc(
    IRTypeLegalizationContext*  context,
    IRFunc*                     irFunc)
{
    // Overwrite the function's type with the result of legalization.

    IRFuncType* oldFuncType = irFunc->getDataType();
    UInt oldParamCount = oldFuncType->getParamCount();

    // TODO: we should give an error message when the result type of a function
    // can't be legalized (e.g., trying to return a texture, or a structue that
    // contains one).
    auto legalReturnType = legalizeType(context, oldFuncType->getResultType());
    IRType* newResultType = nullptr;
    switch (legalReturnType.flavor)
    {
    case LegalType::Flavor::simple:
        newResultType = legalReturnType.getSimple();
        break;
    case LegalType::Flavor::none:
        newResultType = context->builder->getVoidType();
        break;
    default:
        SLANG_UNEXPECTED("unknown legalized function return type.");
    }
    List<IRType*> newParamTypes;
    for (UInt pp = 0; pp < oldParamCount; ++pp)
    {
        auto legalParamType = legalizeType(context, oldFuncType->getParamType(pp));
        addParamType(newParamTypes, legalParamType);
    }

    auto newFuncType = context->builder->getFuncType(
        newParamTypes.Count(),
        newParamTypes.Buffer(),
        newResultType);

    context->builder->setDataType(irFunc, newFuncType);

    legalizeInstsInParent(context, irFunc);
    return LegalVal::simple(irFunc);
}

static LegalVal declareSimpleVar(
    IRTypeLegalizationContext*  context,
    IROp                        op,
    IRType*                     type,
    TypeLayout*                 typeLayout,
    LegalVarChain*              varChain,
    UnownedStringSlice          nameHint,
    IRGlobalNameInfo*           globalNameInfo)
{
    SLANG_UNUSED(globalNameInfo);

    RefPtr<VarLayout> varLayout = createVarLayout(varChain, typeLayout);

    DeclRef<VarDeclBase> varDeclRef;
    if (varChain)
    {
        varDeclRef = varChain->varLayout->varDecl;
    }

    IRBuilder* builder = context->builder;

    IRInst*    irVar = nullptr;
    LegalVal    legalVarVal;

    switch (op)
    {
    case kIROp_GlobalVar:
        {
            auto globalVar = builder->createGlobalVar(type);
            globalVar->removeFromParent();
            globalVar->insertBefore(context->insertBeforeGlobal);

            irVar = globalVar;
            legalVarVal = LegalVal::simple(irVar);
        }
        break;

    case kIROp_GlobalConstant:
        {
            auto globalConst = builder->createGlobalConstant(type);
            globalConst->removeFromParent();
            globalConst->insertBefore(context->insertBeforeGlobal);

            irVar = globalConst;
            legalVarVal = LegalVal::simple(globalConst);
        }
        break;

    case kIROp_GlobalParam:
        {
            auto globalParam = builder->createGlobalParam(type);
            globalParam->removeFromParent();
            globalParam->insertBefore(context->insertBeforeGlobal);

            irVar = globalParam;
            legalVarVal = LegalVal::simple(globalParam);
        }
        break;

    case kIROp_Var:
        {
            builder->setInsertBefore(context->insertBeforeLocalVar);
            auto localVar = builder->emitVar(type);
            
            irVar = localVar;
            legalVarVal = LegalVal::simple(irVar);

        }
        break;

    case kIROp_Param:
        {
            auto param = builder->emitParam(type);
            param->insertBefore(context->insertBeforeParam);

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

        if( nameHint.size() )
        {
            context->builder->addNameHintDecoration(irVar, nameHint);
        }
    }

    return legalVarVal;
}

static LegalVal declareVars(
    IRTypeLegalizationContext*  context,
    IROp                        op,
    LegalType                   type,
    TypeLayout*                 typeLayout,
    LegalVarChain*              varChain,
    UnownedStringSlice          nameHint,
    IRGlobalNameInfo*           globalNameInfo)
{
    switch (type.flavor)
    {
    case LegalType::Flavor::none:
        return LegalVal();

    case LegalType::Flavor::simple:
        return declareSimpleVar(context, op, type.getSimple(), typeLayout, varChain, nameHint, globalNameInfo);
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
                varChain,
                nameHint,
                globalNameInfo);
            return LegalVal::implicitDeref(val);
        }
        break;

    case LegalType::Flavor::pair:
        {
            auto pairType = type.getPair();
            auto ordinaryVal = declareVars(context, op, pairType->ordinaryType, typeLayout, varChain, nameHint, globalNameInfo);
            auto specialVal = declareVars(context, op, pairType->specialType, typeLayout, varChain, nameHint, globalNameInfo);
            return LegalVal::pair(ordinaryVal, specialVal, pairType->pairInfo);
        }

    case LegalType::Flavor::tuple:
        {
            // Declare one variable for each element of the tuple
            auto tupleType = type.getTuple();

            RefPtr<TuplePseudoVal> tupleVal = new TuplePseudoVal();

            for (auto ee : tupleType->elements)
            {
                // Fields are currently required to have linkage, since we use
                // their mangled name to look up field layout information.
                //
                auto fieldLinkage = ee.key->findDecoration<IRLinkageDecoration>();
                SLANG_ASSERT(fieldLinkage);

                auto fieldLayout = getFieldLayout(typeLayout, fieldLinkage->getMangledName());
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

                UnownedStringSlice fieldNameHint;
                String joinedNameHintStorage;
                if( nameHint.size() )
                {
                    if( auto fieldNameHintDecoration = ee.key->findDecoration<IRNameHintDecoration>() )
                    {
                        joinedNameHintStorage.append(nameHint);
                        joinedNameHintStorage.append(".");
                        joinedNameHintStorage.append(fieldNameHintDecoration->getName());

                        fieldNameHint = joinedNameHintStorage.getUnownedSlice();
                    }

                }

                LegalVal fieldVal = declareVars(
                    context,
                    op,
                    ee.type,
                    fieldTypeLayout,
                    newVarChain,
                    fieldNameHint,
                    globalNameInfo);

                TuplePseudoVal::Element element;
                element.key = ee.key;
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

static LegalVal legalizeGlobalVar(
    IRTypeLegalizationContext*    context,
    IRGlobalVar*                irGlobalVar)
{
    // Legalize the type for the variable's value
    auto legalValueType = legalizeType(
        context,
        irGlobalVar->getDataType()->getValueType());

    switch (legalValueType.flavor)
    {
    case LegalType::Flavor::simple:
        // Easy case: the type is usable as-is, and we
        // should just do that.
        context->builder->setDataType(
            irGlobalVar,
            context->builder->getPtrType(
                legalValueType.getSimple()));
        return LegalVal::simple(irGlobalVar);

    default:
        {
            context->insertBeforeGlobal = irGlobalVar->getNextInst();

            IRGlobalNameInfo globalNameInfo;
            globalNameInfo.globalVar = irGlobalVar;
            globalNameInfo.counter = 0;

            UnownedStringSlice nameHint = findNameHint(irGlobalVar);
            LegalVal newVal = declareVars(context, kIROp_GlobalVar, legalValueType, nullptr, nullptr, nameHint, &globalNameInfo);

            // Register the new value as the replacement for the old
            registerLegalizedValue(context, irGlobalVar, newVal);

            // Remove the old global from the module.
            irGlobalVar->removeFromParent();
            context->replacedInstructions.Add(irGlobalVar);

            return newVal;
        }
        break;
    }
}

static LegalVal legalizeGlobalConstant(
    IRTypeLegalizationContext*  context,
    IRGlobalConstant*           irGlobalConstant)
{
    // Legalize the type for the variable's value
    auto legalValueType = legalizeType(
        context,
        irGlobalConstant->getFullType());

    switch (legalValueType.flavor)
    {
    case LegalType::Flavor::simple:
        // Easy case: the type is usable as-is, and we
        // should just do that.
        irGlobalConstant->setFullType(legalValueType.getSimple());
        return LegalVal::simple(irGlobalConstant);

    default:
        {
            context->insertBeforeGlobal = irGlobalConstant->getNextInst();

            IRGlobalNameInfo globalNameInfo;
            globalNameInfo.globalVar = irGlobalConstant;
            globalNameInfo.counter = 0;

            // TODO: need to handle initializer here!

            UnownedStringSlice nameHint = findNameHint(irGlobalConstant);
            LegalVal newVal = declareVars(context, kIROp_GlobalConstant, legalValueType, nullptr, nullptr, nameHint, &globalNameInfo);

            // Register the new value as the replacement for the old
            registerLegalizedValue(context, irGlobalConstant, newVal);

            // Remove the old global from the module.
            irGlobalConstant->removeFromParent();
            context->replacedInstructions.Add(irGlobalConstant);

            return newVal;
        }
        break;
    }
}

static LegalVal legalizeGlobalParam(
    IRTypeLegalizationContext*  context,
    IRGlobalParam*              irGlobalParam)
{
    // Legalize the type for the variable's value
    auto legalValueType = legalizeType(
        context,
        irGlobalParam->getFullType());

    RefPtr<VarLayout> varLayout = findVarLayout(irGlobalParam);
    RefPtr<TypeLayout> typeLayout = varLayout ? varLayout->typeLayout : nullptr;

    switch (legalValueType.flavor)
    {
    case LegalType::Flavor::simple:
        // Easy case: the type is usable as-is, and we
        // should just do that.
        irGlobalParam->setFullType(legalValueType.getSimple());
        return LegalVal::simple(irGlobalParam);

    default:
        {
            context->insertBeforeGlobal = irGlobalParam->getNextInst();

            LegalVarChain* varChain = nullptr;
            LegalVarChain varChainStorage;
            if (varLayout)
            {
                varChainStorage.next = nullptr;
                varChainStorage.varLayout = varLayout;
                varChain = &varChainStorage;
            }

            IRGlobalNameInfo globalNameInfo;
            globalNameInfo.globalVar = irGlobalParam;
            globalNameInfo.counter = 0;

            // TODO: need to handle initializer here!

            UnownedStringSlice nameHint = findNameHint(irGlobalParam);
            LegalVal newVal = declareVars(context, kIROp_GlobalParam, legalValueType, typeLayout, varChain, nameHint, &globalNameInfo);

            // Register the new value as the replacement for the old
            registerLegalizedValue(context, irGlobalParam, newVal);

            // Remove the old global from the module.
            irGlobalParam->removeFromParent();
            context->replacedInstructions.Add(irGlobalParam);

            return newVal;
        }
        break;
    }
}


static void legalizeTypes(
    IRTypeLegalizationContext*    context)
{
    // Legalize all the top-level instructions in the module
    auto module = context->module;
    legalizeInstsInParent(context, module->moduleInst);

    // Clean up after any instructions we replaced along the way.
    for (auto& lv : context->replacedInstructions)
    {
        lv->removeAndDeallocate();
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

    // Clean up after any type instructions we removed (e.g.,
    // global `struct` types).
    //
    // TODO: this logic should probably get paired up with
    // the case for `IRTypeLegalizationContext::replacedInstructions`,
    // but we haven't yet folded all the legalization logic into
    // the IR legalization pass (since it used to apply to the AST too).
    //
    // TODO: This code has issues that can lead to IR validation
    // failure, because we might remove a `struct X` that has been
    // legalized away, but leave around a `ParameterBlock<X>` instruction
    // that is no longer valid.
    for (auto& oldInst : typeLegalizationContext->instsToRemove)
    {
        oldInst->removeAndDeallocate();
    }
}

}
