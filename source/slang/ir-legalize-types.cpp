// ir-legalize-types.cpp

// This file implements a pass that takes IR
// that has been fully specialized (no more
// generics/interfaces needing to be specialized
// away) and replaces any types that can't actually
// be used as-is on the target.
//
// The particular case we are focused on is
// aggregate types (e.g., `struct` types) that
// contain resources (textures, samplers, etc.)
// or that mix resources and ordinary "uniform"
// data.

#include "ir.h"
#include "ir-insts.h"

namespace Slang
{

struct LegalTypeImpl : RefObject
{
};
struct ImplicitDerefType;
struct TupleType;

struct LegalType
{
    enum class Flavor
    {
        // Nothing: a NULL type
        none,

        // A simple type that can be represented directly as a `Type`
        simple,

        // Logically, we have a pointer-like type, but we are
        // going to represnet it as the pointed-to type
        implicitDeref,

        tuple,
    };

    Flavor              flavor = Flavor::none;
    RefPtr<RefObject>   obj;

    static LegalType simple(Type* type)
    {
        LegalType result;
        result.flavor = Flavor::simple;
        result.obj = type;
        return result;
    }

    RefPtr<Type> getSimple()
    {
        assert(flavor == Flavor::simple);
        return obj.As<Type>();
    }

    static LegalType implicitDeref(
        LegalType const& valueType);

    RefPtr<ImplicitDerefType> getImplicitDeref()
    {
        assert(flavor == Flavor::implicitDeref);
        return obj.As<ImplicitDerefType>();
    }

    static LegalType tuple(
        RefPtr<TupleType>   tupleType);

    RefPtr<TupleType> getTuple()
    {
        assert(flavor == Flavor::tuple);
        return obj.As<TupleType>();
    }
};

struct ImplicitDerefType : LegalTypeImpl
{
    LegalType valueType;
};

LegalType LegalType::implicitDeref(
    LegalType const& valueType)
{
    RefPtr<ImplicitDerefType> obj = new ImplicitDerefType();
    obj->valueType = valueType;

    LegalType result;
    result.flavor = Flavor::implicitDeref;
    result.obj = obj;
    return result;
}

struct TupleType : LegalTypeImpl
{
    struct Element
    {
        DeclRef<VarDeclBase>    fieldDeclRef;
        LegalType               type;
    };

    List<Element> elements;
};

LegalType LegalType::tuple(
    RefPtr<TupleType>   tupleType)
{
    LegalType result;
    result.flavor = Flavor::tuple;
    result.obj = tupleType;
    return result;
}

struct LegalValImpl : RefObject
{
};
struct TupleVal;

struct LegalVal
{
    enum class Flavor
    {
        none,
        simple,
        implicitDeref,
        tuple,
    };

    Flavor              flavor;
    RefPtr<RefObject>   obj;
    IRValue*            irValue;

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

    static LegalVal tuple(RefPtr<TupleVal> tupleVal);

    RefPtr<TupleVal> getTuple()
    {
        assert(flavor == Flavor::tuple);
        return obj.As<TupleVal>();
    }

    static LegalVal implicitDeref(LegalVal const& val);
    LegalVal getImplicitDeref();
};

struct TupleVal : LegalValImpl
{
    struct Element
    {
        DeclRef<VarDeclBase>    fieldDeclRef;
        LegalVal                val;
    };

    List<Element> elements;
};

LegalVal LegalVal::tuple(RefPtr<TupleVal> tupleVal)
{
    LegalVal result;
    result.flavor = LegalVal::Flavor::tuple;
    result.obj = tupleVal;
    return result;
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


struct TypeLegalizationContext
{
    Session*    session;
    IRModule*   module;
    IRBuilder*  builder;

    // When inserting new globals, put them before this one.
    IRGlobalValue* insertBeforeGlobal = nullptr;

    Dictionary<IRValue*, LegalVal> mapValToLegalVal;
};

static void registerLegalizedValue(
    TypeLegalizationContext*    context,
    IRValue*                    irValue,
    LegalVal const&             legalVal)
{
    context->mapValToLegalVal.Add(irValue, legalVal);
}


static bool isResourceType(Type* type)
{
    while (auto arrayType = type->As<ArrayExpressionType>())
    {
        type = arrayType->baseType;
    }

    if (auto textureTypeBase = type->As<TextureTypeBase>())
    {
        return true;
    }
    else if (auto samplerType = type->As<SamplerStateType>())
    {
        return true;
    }

    // TODO: need more comprehensive coverage here

    return false;
}

// Legalize a type, including any nested types
// that it transitively contains.
static LegalType legalizeType(
    TypeLegalizationContext*    context,
    Type*                       type)
{
    if (auto parameterBlockType = type->As<ParameterBlockType>())
    {
        // We basically legalize the `ParameterBlock<T>` type
        // over to `T`. In order to represent this preoperly,
        // we need to be careful to wrap it up in a way that
        // tells us to eliminate downstream deferences...

        auto legalElementType = legalizeType(context,
            parameterBlockType->getElementType());
        return LegalType::implicitDeref(legalElementType);
    }
    else if (isResourceType(type))
    {
        // We assume that any resource types not handled above
        // are legal as-is.
        return LegalType::simple(type);
    }
    else if (type->As<BasicExpressionType>())
    {
        return LegalType::simple(type);
    }
    else if (type->As<VectorExpressionType>())
    {
        return LegalType::simple(type);
    }
    else if (type->As<MatrixExpressionType>())
    {
        return LegalType::simple(type);
    }
    else if (auto declRefType = type->As<DeclRefType>())
    {
        auto declRef = declRefType->declRef;
        if (auto aggTypeDeclRef = declRef.As<AggTypeDecl>())
        {
            // Look at the (non-static) fields, and
            // see if anything needs to be cleaned up.

            // We collect the legalized types for the fields,
            // along with whether we've seen anything non-simple.
            List<TupleType::Element> legalizedElements;
            bool anyComplex = false;
            bool anyResource = false;

            for (auto ff : getMembersOfType<StructField>(aggTypeDeclRef))
            {
                if (ff.getDecl()->HasModifier<HLSLStaticModifier>())
                    continue;

                auto fieldType = GetType(ff);
                if (isResourceType(fieldType))
                {
                    anyResource = true;
                }

                auto legalFieldType = legalizeType(context, fieldType);

                TupleType::Element element;
                element.fieldDeclRef = ff;
                element.type = legalFieldType;
                legalizedElements.Add(element);

                switch (legalFieldType.flavor)
                {
                case LegalType::Flavor::simple:
                    break;

                default:
                    anyComplex = true;
                    break;
                }
            }

            // If we didn't see anything that requires work,
            // we can conceivably just use the type as-is
            //
            // TODO: this might be a good place to turn
            // a reference to a generic `struct` type into
            // a concrete non-generic type so that downstream
            // codegen doesn't have to deal with generics...
            //
            // TODO: In fact, why not just fully replace
            // all aggregate types here with some structural
            // types defined in the IR?
            if (!anyComplex && !anyResource)
            {
                return LegalType::simple(type);
            }

            // Okay, we are going to have to generate a
            // "tuple" type.
            //
            // TODO: split out the "simple" fields into
            // their own sub-type?

            RefPtr<TupleType> tupleType = new TupleType();
            tupleType->elements = legalizedElements;

            return LegalType::tuple(tupleType);
        }
    }

    return LegalType::simple(type);
}

// Legalize a type, and then expect it to
// result in a simple type.
static RefPtr<Type> legalizeSimpleType(
    TypeLegalizationContext*    context,
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
    TypeLegalizationContext*    context,
    IRValue*                    irValue)
{
    LegalVal legalVal;
    if (context->mapValToLegalVal.TryGetValue(irValue, legalVal))
        return legalVal;

    // For now, assume that anything not covered
    // by the mapping is legal as-is.

    return LegalVal::simple(irValue);
}

static LegalVal legalizeLoad(
    TypeLegalizationContext*    context,
    LegalVal                    legalPtrVal)
{
    switch (legalPtrVal.flavor)
    {
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

    case LegalVal::Flavor::tuple:
        {
            // We need to emit a load for each element of
            // the tuple.
            RefPtr<TupleVal> tupleVal = new TupleVal();
            for (auto ee : legalPtrVal.getTuple()->elements)
            {
                TupleVal::Element element;
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

static LegalVal legalizeFieldAddress(
    TypeLegalizationContext*    context,
    LegalType                   type,
    LegalVal                    legalPtrOperand,
    LegalVal                    legalFieldOperand)
{
    auto builder = context->builder;

    // We don't expect any legalization to affect
    // the "field" argument.
    auto fieldOperand = legalFieldOperand.getSimple();
    assert(fieldOperand->op == kIROp_decl_ref);
    auto fieldDeclRef = ((IRDeclRef*)fieldOperand)->declRef;

    switch (legalPtrOperand.flavor)
    {
    case LegalVal::Flavor::simple:
        return LegalVal::simple(
            builder->emitFieldAddress(
                type.getSimple(),
                legalPtrOperand.getSimple(),
                fieldOperand));

    case LegalVal::Flavor::tuple:
        {
            // The operand is a tuple of pointer-like
            // values, we want to extract the element
            // corresponding to a field. We will handle
            // this by simply returning the corresponding
            // element from the operand.
            for (auto ee : legalPtrOperand.getTuple()->elements)
            {
                if (ee.fieldDeclRef.Equals(fieldDeclRef))
                {
                    return ee.val;
                }
            }
            SLANG_UNEXPECTED("didn't find tuple element");
            return LegalVal();
        }

    default:
        SLANG_UNEXPECTED("unhandled");
        return LegalVal();
    }
}

static LegalVal legalizeInst(
    TypeLegalizationContext*    context,
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

    default:
        // TODO: produce a user-visible diagnostic here
        SLANG_UNEXPECTED("non-simple operand(s)!");
        break;
    }
}

static LegalVal legalizeInst(
    TypeLegalizationContext*    context,
    IRInst*                     inst)
{
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

static void legalizeFunc(
    TypeLegalizationContext*    context,
    IRFunc*                     irFunc)
{
    // Overwrite the function's type with
    // the result of legalization.
    irFunc->type = legalizeSimpleType(context, irFunc->type);

    // Go through the blocks of the function
    for (auto bb = irFunc->getFirstBlock(); bb; bb = bb->getNextBlock())
    {
        // Legalize the parameters of the block, which may
        // involve increasing the number of parameters
        for (auto pp = bb->getFirstParam(); pp; pp = pp->getNextParam())
        {
            auto legalParamType = legalizeType(context, pp->getType());

            switch (legalParamType.flavor)
            {
            case LegalType::Flavor::simple:
                // The type is simple, so we can just rewrite it in place
                pp->type = legalParamType.getSimple();
                break;

            default:
                // We have something like a tuple, and will need
                // to expand into multiple parameters now.
                SLANG_UNEXPECTED("need to handle it!");
                break;
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
}

// Represents the "chain" of declarations that
// were followed to get to a variable that we
// are now declaring as a leaf variable.
struct LegalVarChain
{
    LegalVarChain*  next;
    VarLayout*      varLayout;
};

static LegalVal declareSimpleVar(
    TypeLegalizationContext*    context,
    IROp                        op,
    Type*                       type,
    TypeLayout*                 typeLayout,
    LegalVarChain*              varChain)
{
    RefPtr<VarLayout> varLayout;
    if (typeLayout)
    {
        // We need to construct a layout for the new variable
        // that reflects both the type we have given it, as
        // well as all the offset information that has accumulated
        // along the chain of parent variables.

        varLayout = new VarLayout();
        varLayout->typeLayout = typeLayout;

        for (auto rr : typeLayout->resourceInfos)
        {
            auto resInfo = varLayout->findOrAddResourceInfo(rr.kind);

            for (auto vv = varChain; vv; vv = vv->next)
            {
                if (auto parentResInfo = vv->varLayout->FindResourceInfo(rr.kind))
                {
                    resInfo->index += parentResInfo->index;
                    resInfo->space += parentResInfo->space;
                }
            }
        }

        // Some of the parent variables might actually contain offsets
        // to the `space` or `set` of the field, and we need to apply
        // those to all the nested resource infos.
        for (auto vv = varChain; vv; vv = vv->next)
        {
            auto parentSpaceInfo = vv->varLayout->findOrAddResourceInfo(LayoutResourceKind::ParameterBlock);
            if (!parentSpaceInfo)
                continue;

            for (auto& rr : varLayout->resourceInfos)
            {
                if (rr.kind == LayoutResourceKind::ParameterBlock)
                {
                    rr.index += parentSpaceInfo->index;
                }
                else
                {
                    rr.space += parentSpaceInfo->index;
                }
            }
        }
    }

    switch (op)
    {
    case kIROp_global_var:
        {
            IRBuilder* builder = context->builder;

            auto globalVar = builder->createGlobalVar(type);
            globalVar->removeFromParent();
            globalVar->insertBefore(context->insertBeforeGlobal);

            if (varLayout)
            {
                builder->addLayoutDecoration(globalVar, varLayout);
            }

            return LegalVal::simple(globalVar);
        }
        break;

    default:
        SLANG_UNEXPECTED("unexpected IR opcode");
        break;
    }
}

static RefPtr<TypeLayout> getDerefTypeLayout(
    TypeLayout* typeLayout)
{
    if (!typeLayout)
        return nullptr;

    if (auto parameterGroupTypeLayout = dynamic_cast<ParameterGroupTypeLayout*>(typeLayout))
    {
        return parameterGroupTypeLayout->elementTypeLayout;
    }

    return typeLayout;
}

static RefPtr<VarLayout> getFieldLayout(
    TypeLayout*             typeLayout,
    DeclRef<VarDeclBase>    fieldDeclRef)
{
    if (!typeLayout)
        return nullptr;

    if (auto structTypeLayout = dynamic_cast<StructTypeLayout*>(typeLayout))
    {
        RefPtr<VarLayout> fieldLayout;
        if (structTypeLayout->mapVarToLayout.TryGetValue(fieldDeclRef.getDecl(), fieldLayout))
            return fieldLayout;
    }

    return nullptr;
}

static LegalVal declareVars(
    TypeLegalizationContext*    context,
    IROp                        op,
    LegalType                   type,
    TypeLayout*                 typeLayout,
    LegalVarChain*              varChain)
{
    switch (type.flavor)
    {
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

    case LegalType::Flavor::tuple:
        {
            // Declare one variable for each element of the tuple
            auto tupleType = type.getTuple();

            RefPtr<TupleVal> tupleVal = new TupleVal();

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

                TupleVal::Element element;
                element.fieldDeclRef = ee.fieldDeclRef;
                element.val = declareVars(
                    context,
                    op,
                    ee.type,
                    fieldTypeLayout,
                    newVarChain);
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

RefPtr<VarLayout> findVarLayout(IRValue* value)
{
    if (auto layoutDecoration = value->findDecoration<IRLayoutDecoration>())
        return layoutDecoration->layout.As<VarLayout>();
    return nullptr;
}

static void legalizeGlobalVar(
    TypeLegalizationContext*    context,
    IRGlobalVar*                irGlobalVar)
{
    // Legalize the type for the variable's value
    auto legalValueType = legalizeType(
        context,
        irGlobalVar->getType()->getValueType());

    RefPtr<VarLayout> varLayout = findVarLayout(irGlobalVar);
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
        irGlobalVar->type = context->session->getPtrType(
            maybeSimpleType.getSimple());
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
    TypeLegalizationContext*    context,
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
    TypeLegalizationContext*    context)
{
    auto module = context->module;
    for (auto gv = module->getFirstGlobalValue(); gv; gv = gv->getNextValue())
    {
        legalizeGlobalValue(context, gv);
    }
}


void legalizeTypes(
    IRModule*   module)
{
    auto session = module->session;

    SharedIRBuilder sharedBuilderStorage;
    auto sharedBuilder = &sharedBuilderStorage;

    sharedBuilder->session = session;
    sharedBuilder->module = module;

    IRBuilder builderStorage;
    auto builder = &builderStorage;

    builder->sharedBuilder = sharedBuilder;


    TypeLegalizationContext contextStorage;
    auto context = &contextStorage;

    context->session = session;
    context->module = module;
    context->builder = builder;

    legalizeTypes(context);

}

#if 0
    typedef unsigned int TypeScalarizationFlags;
    enum TypeScalarizationFlag
    {
        anyResource     = 0x1,
        anyNonResource  = 0x2,
        anyAggregate    = 0x4,
    };

    bool isResourceType(Type* type)
    {
        while (auto arrayType = type->As<ArrayExpressionType>())
        {
            type = arrayType->baseType;
        }

        if (auto textureTypeBase = type->As<TextureTypeBase>())
        {
            return true;
        }
        else if (auto samplerType = type->As<SamplerStateType>())
        {
            return true;
        }

        // TODO: need more comprehensive coverage here

        return false;
    }

    TypeScalarizationFlags getTypeScalarizationFlags(
        Session*    session,
        Type*       type)
    {
        // TODO: we should probably cache flags once
        // they are computed, to avoid O(N^2) sorts
        // of behavior.

        if (isResourceType(type))
            return TypeScalarizationFlag::anyNonResource;

        if(type->As<BasicExpressionType>())
        {
            return TypeScalarizationFlag::anyNonResource;
        }
        if(type->As<VectorExpressionType>())
        {
            return TypeScalarizationFlag::anyNonResource;
        }
        if(type->As<MatrixExpressionType>())
        {
            return TypeScalarizationFlag::anyNonResource;
        }
        else if (auto declRefType = type->As<DeclRefType>())
        {
            auto declRef = declRefType->declRef;
            if (auto structDeclRef = declRef.As<StructDecl>())
            {
                TypeScalarizationFlags flags = TypeScalarizationFlag::anyAggregate;

                // For structure types, the basic rule will be
                // that if the type contains *any* resource-type
                // fields, then it needs to be scalarized.
                // If it contains any non-resource-type fields,
                // then we should aggregate these into a single
                // new `struct` type with just the non-resource
                // fields.
                for (auto fieldDeclRef : getMembersOfType<StructField>(structDeclRef))
                {
                    auto fieldType = GetType(fieldDeclRef);

                    // TODO: we are making a recursive call here, so
                    // this will break if/when we ever allowed a recursive type!
                    auto fieldFlags = getTypeScalarizationFlags(session, fieldType);
                    flags |= fieldFlags;

                }

                return flags;
            }
        }
        else if (auto arrayType = type->As<ArrayExpressionType>())
        {
            return getTypeScalarizationFlags(
                session,
                arrayType->baseType);
        }

        // Default behavior: assume we have a non-resource type
        return TypeScalarizationFlag::anyNonResource;
    }

    struct ArrayScalarizationInfo
    {
        ArrayScalarizationInfo* next;
        RefPtr<IntVal>          elementCount;
        RefPtr<ArrayTypeLayout> typeLayout;
    };

    struct SharedScalarizationContext
    {

    };

    struct ScalarizationContext
    {
        SharedScalarizationContext* shared;

        IRBuilder*              builder;
        IRGlobalVar*            globalVar;
        VarLayout*              globalVarLayout;

        IRGlobalValue*          valueToInsertAfter;
    };

    IRValue* emitSimpleScalarizedField(
        ScalarizationContext*   context,
        Type*                   inType,
        VarLayout*              fieldLayout,
        TypeLayout*             inTypeLayout,
        ArrayScalarizationInfo* arrayInfo)
    {
        auto builder = context->builder;
        auto globalVar = context->globalVar;
        auto globalVarLayout = context->globalVarLayout;
        auto valueToInsertAfter = context->valueToInsertAfter;

        RefPtr<Type> type = inType;
        RefPtr<TypeLayout> typeLayout = inTypeLayout;

        // If we are turning an array-of-structs into
        // a struct-of-arrays, then we need to apply
        // all the appropriate array dimensions here.
        for (auto aa = arrayInfo; aa; aa = aa->next)
        {
            type = builder->getSession()->getArrayType(type, aa->elementCount);

            if (typeLayout)
            {
                RefPtr<ArrayTypeLayout> arrayTypeLayout = new ArrayTypeLayout();
                arrayTypeLayout->elementTypeLayout = typeLayout;

                // TODO: fill in the other fields!

                typeLayout = arrayTypeLayout;
            }
        }

        RefPtr<VarLayout> newVarLayout;
        if (typeLayout)
        {
            newVarLayout = new VarLayout();
            newVarLayout->typeLayout = typeLayout;

            if (fieldLayout)
            {
                for (auto fieldResourceInfo : fieldLayout->resourceInfos)
                {
                    auto newResourceInfo = newVarLayout->findOrAddResourceInfo(fieldResourceInfo.kind);

                    if (globalVarLayout)
                    {
                        if (auto globalResourceInfo = globalVarLayout->FindResourceInfo(fieldResourceInfo.kind))
                        {
                            newResourceInfo->index += globalResourceInfo->index;
                            newResourceInfo->space += globalResourceInfo->space;
                        }
                    }

                    newResourceInfo->index += fieldResourceInfo.index;
                    newResourceInfo->space += fieldResourceInfo.space;
                }
            }
        }

        auto newGlobalVar = addGlobalVariable(builder->getModule(), type);
        builder->addLayoutDecoration(newGlobalVar, newVarLayout);

        newGlobalVar->removeFromParent();
        newGlobalVar->insertAfter(valueToInsertAfter);

        context->valueToInsertAfter = newGlobalVar;

        return newGlobalVar;
    }

    void scalarizeGlobalVariable(
        ScalarizationContext*   context,
        Type*                   valueType,
        TypeLayout*             valueTypeLayout,
        ArrayScalarizationInfo* arrayInfo)
    {
        if (auto arrayType = valueType->As<ArrayExpressionType>())
        {
            // Okay, we need to recurse down and scalarize the
            // array element type, wrapping up each field in
            // an array declarator as needed.

            ArrayScalarizationInfo newArrayInfo;
            newArrayInfo.next = arrayInfo;
            newArrayInfo.elementCount = arrayType->ArrayLength;

            RefPtr<TypeLayout> elementTypeLayout;
            if (auto arrayTypeLayout = dynamic_cast<ArrayTypeLayout*>(valueTypeLayout))
            {
                newArrayInfo.typeLayout = arrayTypeLayout;
                elementTypeLayout = arrayTypeLayout->elementTypeLayout;
            }

            scalarizeGlobalVariable(
                context,
                arrayType->baseType,
                elementTypeLayout,
                &newArrayInfo);

            // Now we need to look at all uses of the variable,
            // and properly rework element-index operations
            // to instead index into the sub-arrays...
        }
        else if (auto declRefType = valueType->As<DeclRefType>())
        {
            auto declRef = declRefType->declRef;
            if (auto aggTypeDeclRef = declRef.As<AggTypeDecl>())
            {
                RefPtr<StructTypeLayout> structTypeLayout = dynamic_cast<StructTypeLayout*>(valueTypeLayout);

                // Okay, we need to look through the fields, and
                // create a new variable for each of them.
                Dictionary<Decl*, IRValue*> fieldMap;
                UInt fieldCounter = 0;
                for (auto fieldDeclRef : getMembersOfType<StructField>(aggTypeDeclRef))
                {
                    UInt fieldIndex = fieldCounter++;

                    RefPtr<VarLayout> fieldLayout;
                    RefPtr<TypeLayout> fieldTypeLayout;
                    if (structTypeLayout)
                    {
                        fieldLayout = structTypeLayout->fields[fieldIndex];
                        fieldTypeLayout = fieldLayout->typeLayout;
                    }

                    // Note: we do *not* try to deal with recursive
                    // expansion of the fields here, and instead
                    // prefer to handle those in further
                    // simplification passes.

                    auto fieldGlobalVar = emitSimpleScalarizedField(
                        context,
                        GetType(fieldDeclRef),
                        fieldLayout,
                        fieldTypeLayout,
                        arrayInfo);

                    fieldMap.Add(fieldDeclRef.getDecl(), fieldGlobalVar);
                }

                // Now we need to scan for uses of the original variable,
                // and replace them with uses of the individual fields.
                auto globalVar = context->globalVar;
                IRUse* nextUse = nullptr;
                for (IRUse* use = globalVar->firstUse; use; use = nextUse)
                {
                    nextUse = use->nextUse;

                    IRUser* user = use->user;
                    switch (user->op)
                    {
                    case kIROp_FieldAddress:
                        {
                            // This should be the easy case: we are taking
                            // the address of a field inside this global
                            // value, so we can just return the adress
                            // of the global value that replaced that field.
                            IRFieldAddress* fieldAddressInst = (IRFieldAddress*)user;

                            IRValue* fieldOperand = fieldAddressInst->getField();
                            assert(fieldOperand->op == kIROp_decl_ref);
                            auto fieldDeclRef = ((IRDeclRef*)fieldOperand)->declRef;
                            auto fieldDecl = fieldDeclRef.getDecl();

                            IRValue* fieldVar = *fieldMap.TryGetValue(fieldDecl);

                            fieldAddressInst->replaceUsesWith(fieldVar);
                        }
                        break;

                    default:
                        SLANG_UNEXPECTED("what to do?");
                        break;
                    }
                }
            }
            else
            {
                SLANG_UNEXPECTED("not handled");
            }
        }
        else
        {
            SLANG_UNEXPECTED("not handled");
        }
    }

    void scalarizeGlobalVariable(
        SharedScalarizationContext* sharedContext,
        IRBuilder*              builder,
        IRGlobalVar*            globalVar,
        VarLayout*              globalVarLayout,
        Type*                   valueType,
        TypeLayout*             valueTypeLayout)
    {
        ScalarizationContext contextStorage;
        auto context = &contextStorage;

        context->shared = sharedContext;
        context->builder = builder;
        context->globalVar = globalVar;
        context->globalVarLayout = globalVarLayout;
        context->valueToInsertAfter = globalVar;

        scalarizeGlobalVariable(
            context,
            valueType,
            valueTypeLayout,
            nullptr);
    }

    RefPtr<VarLayout> findVarLayout(IRValue* value)
    {
        if (auto layoutDecoration = value->findDecoration<IRLayoutDecoration>())
            return layoutDecoration->layout.As<VarLayout>();
        return nullptr;
    }

    void scalarizeMixedResourceTypes(
        Session*    session,
        IRModule*   module)
    {
        SharedIRBuilder sharedBuilderStorage;
        auto sharedBuilder = &sharedBuilderStorage;

        sharedBuilder->session = session;
        sharedBuilder->module = module;

        IRBuilder builderStorage;
        auto builder = &builderStorage;

        builder->shared = sharedBuilder;

        SharedScalarizationContext sharedContextStorage;
        auto sharedContext = &sharedContextStorage;


        List<IRValue*> workList;
        for (auto gv = module->getFirstGlobalValue(); gv; gv = gv->getNextValue())
        {
            workList.Add(gv);
        }

        while (workList.Count())
        {
            IRValue* value = workList[0];
            workList.FastRemoveAt(0);

            switch (value->op)
            {
            case kIROp_Func:
                {
                    // TODO: need to iterate over parameters of
                    // the function (and its blocks) to make
                    // sure that any types that need scalarization
                    // are properly handled.
                }
                break;

            case kIROp_global_var:
                {
                    IRGlobalVar* globalVar = (IRGlobalVar*)value;
                    auto valueType = globalVar->getType()->getValueType();

                    auto flags = getTypeScalarizationFlags(session, valueType);
                    if (!(flags & (TypeScalarizationFlag::anyNonResource | TypeScalarizationFlag::anyAggregate)))
                        continue;

                    auto varLayout = findVarLayout(globalVar);
                    RefPtr<TypeLayout> typeLayout = varLayout ? varLayout->typeLayout : nullptr;

                    // Okay, we have a variable of some composite type
                    // that we need to scalarize. Since this is a global,
                    // we also need to be careful to deal with any
                    // layout information that has been attached.

                    scalarizeGlobalVariable(
                        sharedContext,
                        builder,
                        globalVar,
                        varLayout,
                        valueType,
                        typeLayout);

                    globalVar->removeFromParent();
                    // TODO: need to destroy this global!
                }
                break;

            default:
                {
                    // TODO: look at the type of the value,
                    // and if it needs scalarization, replace
                    // it with a tuple here.
                }
                break;
            }
        }
    }


#endif

}
