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
struct TuplePseudoType;
struct PairPseudoType;
struct PairInfo;

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

        // A compound type was broken apart into its constituent fields,
        // so a tuple "pseduo-type" is being used to collect
        // those fields together.
        tuple,

        // A type has to get split into "ordinary" and "special" parts,
        // each of which will be represented with its own `LegalType`.
        pair,
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
        RefPtr<TuplePseudoType>   tupleType);

    RefPtr<TuplePseudoType> getTuple()
    {
        assert(flavor == Flavor::tuple);
        return obj.As<TuplePseudoType>();
    }

    static LegalType pair(
        RefPtr<PairPseudoType>   pairType);

    static LegalType pair(
        RefPtr<Type>        ordinaryType,
        LegalType const&    specialType,
        RefPtr<PairInfo>    pairInfo);

    RefPtr<PairPseudoType> getPair()
    {
        assert(flavor == Flavor::pair);
        return obj.As<PairPseudoType>();
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

// Represents the pseudo-type for a compound type
// that had to be broken apart because it contained
// one or more fields of types that shouldn't be
// allowed in aggregates.
//
// A tuple pseduo-type will have an element for
// each field of the original type, that represents
// the legalization of that field's type.
//
// It optionally also contains an "ordinary" type
// that packs together any per-field data that
// itself has (or contains) an ordinary type.
struct TuplePseudoType : LegalTypeImpl
{
    // Represents one element of the tuple pseudo-type
    struct Element
    {
        // The field that this element replaces
        DeclRef<VarDeclBase>    fieldDeclRef;

        // The legalized type of the element
        LegalType               type;
    };

    // All of the elements of the tuple pseduo-type.
    List<Element>   elements;
};

LegalType LegalType::tuple(
    RefPtr<TuplePseudoType>   tupleType)
{
    LegalType result;
    result.flavor = Flavor::tuple;
    result.obj = tupleType;
    return result;
}

struct PairInfo : RefObject
{
    typedef unsigned int Flags;
    enum
    {
        kFlag_hasOrdinary = 0x1,
        kFlag_hasSpecial  = 0x2,
    };

    struct Element
    {
        // The field the element represents
        DeclRef<Decl> fieldDeclRef;

        // The conceptual type of the field.
        // If both the `hasOrdinary` and
        // `hasSpecial` bits are set, then
        // this is expected to be a
        // `LegalType::Flavor::pair`
        LegalType   type;

        // Is the value represented on
        // the ordinary side, the special
        // side, or both?
        Flags       flags;
    };

    // For a pair type or value, we need to track
    // which fields are on which side(s).
    List<Element> elements;

    Element* findElement(DeclRef<Decl> const& fieldDeclRef)
    {
        for (auto& ee : elements)
        {
            if(ee.fieldDeclRef.Equals(fieldDeclRef))
                return &ee;
        }
        return nullptr;
    }
};

struct PairPseudoType : LegalTypeImpl
{
    // Any field(s) with ordinary types will
    // get captured here, as a completely
    // standard AST-level type.
    RefPtr<Type> ordinaryType;

    // Any fields with "special" (not ordinary)
    // types will get captured here (usually
    // with a tuple).
    LegalType specialType;

    RefPtr<PairInfo> pairInfo;
};

LegalType LegalType::pair(
    RefPtr<PairPseudoType>   pairType)
{
    LegalType result;
    result.flavor = Flavor::pair;
    result.obj = pairType;
    return result;
}

LegalType LegalType::pair(
    RefPtr<Type>        ordinaryType,
    LegalType const&    specialType,
    RefPtr<PairInfo>    pairInfo)
{
    // Handle some special cases for when
    // one or the other of the types isn't
    // actually used.

    if (!ordinaryType)
    {
        // There was nothing ordinary.
        return specialType;
    }

    if (specialType.flavor == LegalType::Flavor::none)
    {
        return LegalType::simple(ordinaryType);
    }

    // There were both ordinary and special fields,
    // and so we need to handle them here.

    RefPtr<PairPseudoType> obj = new PairPseudoType();
    obj->ordinaryType = ordinaryType;
    obj->specialType = specialType;
    obj->pairInfo = pairInfo;
    return LegalType::pair(obj);
}


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


struct TypeLegalizationContext
{
    Session*    session;
    IRModule*   module;
    IRBuilder*  builder;

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

static LegalType legalizeType(
    TypeLegalizationContext*    context,
    Type*                       type);

// Helper type for legalization of aggregate types
// that might need to be turned into tuple pseudo-types.
struct TupleTypeBuilder
{
    TypeLegalizationContext*    context;
    RefPtr<Type>                type;

    List<FilteredTupleType::Element> ordinaryElements;
    List<TuplePseudoType::Element> specialElements;

    List<PairInfo::Element> pairElements;

    // Did we have any fields that forced us to change
    // the actual type away from the declared type?
    bool anyComplex = false;

    // Did we have any fields that actually required
    // storage in the "special" part of things?
    bool anySpecial = false;

    // Did we have any fields that actually used ordinary storage?
    bool anyOrdinary = false;

    // Add a field to the (pseudo-)type we are building
    void addField(
        DeclRef<VarDeclBase>    fieldDeclRef,
        LegalType               legalFieldType,
        LegalType               legalLeafType,
        bool                    isResource)
    {
        RefPtr<Type> ordinaryType;
        LegalType specialType;
        RefPtr<PairInfo> elementPairInfo;
        switch (legalLeafType.flavor)
        {
        case LegalType::Flavor::simple:
            {
                // We need to add an actual field, but we need
                // to check if it is a resource type to know
                // whether it should go in the "ordinary" list or not.
                if (!isResource)
                {
                    ordinaryType = legalLeafType.getSimple();
                }
                else
                {
                    specialType = legalFieldType;
                }
            }
            break;

        case LegalType::Flavor::implicitDeref:
            {
                // TODO: we may want to say that any use
                // of `implicitDeref` puts the entire thing
                // into the "special" category, rather than
                // try to look under the hood...

                anyComplex = true;

                // We want to recursively add data
                // based on the unwrapped type.
                //
                // Note: this assumes we can't have a tuple
                // or a pair "under" an `implicitDeref`, so
                // we'll need to ensure that elsewhere.
                addField(
                    fieldDeclRef,
                    legalFieldType,
                    legalLeafType.getImplicitDeref()->valueType,
                    isResource);
                return;
            }
            break;

        case LegalType::Flavor::pair:
            {
                // The field's type had both special and non-special parts
                auto pairType = legalLeafType.getPair();
                ordinaryType = pairType->ordinaryType;
                specialType = pairType->specialType;
                elementPairInfo = pairType->pairInfo;
            }
            break;

        case LegalType::Flavor::tuple:
            {
                // A tuple always represents "special" data
                specialType = legalFieldType;
            }
            break;

        default:
            SLANG_UNEXPECTED("unknown legal type flavor");
            break;
        }


        PairInfo::Element pairElement;
        pairElement.flags = 0;
        pairElement.fieldDeclRef = fieldDeclRef;

        if (ordinaryType)
        {
            anyOrdinary = true;
            pairElement.flags |= PairInfo::kFlag_hasOrdinary;

            FilteredTupleType::Element ordinaryElement;
            ordinaryElement.fieldDeclRef = fieldDeclRef;
            ordinaryElement.type = ordinaryType;
            ordinaryElements.Add(ordinaryElement);
        }

        if (specialType.flavor != LegalType::Flavor::none)
        {
            anySpecial = true;
            anyComplex = true;
            pairElement.flags |= PairInfo::kFlag_hasSpecial;

            TuplePseudoType::Element specialElement;
            specialElement.fieldDeclRef = fieldDeclRef;
            specialElement.type = specialType;
            specialElements.Add(specialElement);
        }

        pairElement.type = LegalType::pair(ordinaryType, specialType, elementPairInfo);
        pairElements.Add(pairElement);
    }

    // Add a field to the (pseudo-)type we are building
    void addField(
        DeclRef<VarDeclBase>    fieldDeclRef)
    {
        // Skip `static` fields.
        if (fieldDeclRef.getDecl()->HasModifier<HLSLStaticModifier>())
            return;

        auto fieldType = GetType(fieldDeclRef);

        bool isResourceField = isResourceType(fieldType);

        auto legalFieldType = legalizeType(context, fieldType);
        addField(
            fieldDeclRef,
            legalFieldType,
            legalFieldType,
            isResourceField);
    }

    LegalType getResult()
    {
        // If we didn't see anything "special"
        // then we can use the type as-is.
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
        if (!anyComplex)
        {
            return LegalType::simple(type);
        }

        // If there were any "ordinary" fields along the way,
        // then we need to collect them into a type to
        // represent the ordinary part of things.
        //
        RefPtr<Type> ordinaryType;
        if (anyOrdinary)
        {
            RefPtr<FilteredTupleType> ordinaryTypeImpl = new FilteredTupleType();
            ordinaryTypeImpl->setSession(context->session);
            ordinaryTypeImpl->originalType = type;
            ordinaryTypeImpl->elements = ordinaryElements;
            ordinaryType = ordinaryTypeImpl;
        }

        LegalType specialType;
        if (anySpecial)
        {
            RefPtr<TuplePseudoType> specialTuple = new TuplePseudoType();
            specialTuple->elements = specialElements;
            specialType = LegalType::tuple(specialTuple);
        }

        RefPtr<PairInfo> pairInfo;
        if (anyOrdinary && anySpecial)
        {
            pairInfo = new PairInfo();
            pairInfo->elements = pairElements;
        }

        return LegalType::pair(ordinaryType, specialType, pairInfo);
    }

};

static RefPtr<Type> createBuiltinGenericType(
    TypeLegalizationContext*    context,
    DeclRef<Decl> const&        typeDeclRef,
    RefPtr<Type>                elementType)
{
    // We are going to take the type for the original
    // decl-ref and construct a new one that uses
    // our new element type as its parameter.
    //
    // TODO: we should have library code to make
    // manipulations like this way easier.

    RefPtr<GenericSubstitution> oldGenericSubst = getGenericSubstitution(
        typeDeclRef.substitutions);
    SLANG_ASSERT(oldGenericSubst);

    RefPtr<GenericSubstitution> newGenericSubst = new GenericSubstitution();

    newGenericSubst->outer = oldGenericSubst->outer;
    newGenericSubst->genericDecl = oldGenericSubst->genericDecl;
    newGenericSubst->args = oldGenericSubst->args;
    newGenericSubst->args[0] = elementType;

    auto newDeclRef = DeclRef<Decl>(
        typeDeclRef.getDecl(),
        newGenericSubst);

    auto newType = DeclRefType::Create(
        context->session,
        newDeclRef);

    return newType;
}

// Create a uniform buffer type with a given legalized
// element type.
static LegalType createLegalUniformBufferType(
    TypeLegalizationContext*    context,
    DeclRef<Decl> const&        typeDeclRef,
    LegalType                   legalElementType)
{
    switch (legalElementType.flavor)
    {
    case LegalType::Flavor::simple:
        {
            // Easy case: we just have a simple element type,
            // so we want to create a uniform buffer that wraps it.
            return LegalType::simple(createBuiltinGenericType(
                context,
                typeDeclRef,
                legalElementType.getSimple()));
        }
        break;

    case LegalType::Flavor::implicitDeref:
        {
            // This is actually an annoying case, because
            // we are being asked to convert, e.g.,:
            //
            //      cbuffer Foo { ParameterBlock<Bar> bar; }
            //
            // into the equivalent of:
            //
            //      cbuffer Foo { Bar bar; }
            //
            // Which would really require a new `LegalType` that
            // would reprerent a resource type with a modified
            // element type.
            //
            // I'm going to attempt to hack this for now.
            return LegalType::implicitDeref(createLegalUniformBufferType(
                context,
                typeDeclRef,
                legalElementType.getImplicitDeref()->valueType));
        }
        break;

    case LegalType::Flavor::pair:
        {
            // We assume that the "ordinary" part of things
            // will get wrapped in a constant-buffer type,
            // and the "special" part needs to be wrapped
            // with an `implicitDeref`.
            auto pairType = legalElementType.getPair();

            auto ordinaryType = createBuiltinGenericType(
                context,
                typeDeclRef,
                pairType->ordinaryType);
            auto specialType = LegalType::implicitDeref(pairType->specialType);

            return LegalType::pair(ordinaryType, specialType, pairType->pairInfo);
        }

    case LegalType::Flavor::tuple:
        {
            // if we have a tuple type, then it must be representing
            // the fields that can't be stored in a buffer anyway,
            // so we just need to wrap each of them in an `implicitDeref`

            auto elementPseudoTupleType = legalElementType.getTuple();

            RefPtr<TuplePseudoType> bufferPseudoTupleType = new TuplePseudoType();

            // Wrap all the pseudo-tuple elements with `implicitDeref`,
            // since they used to be inside a tuple, but aren't any more.
            for (auto ee : elementPseudoTupleType->elements)
            {
                TuplePseudoType::Element newElement;

                newElement.fieldDeclRef = ee.fieldDeclRef;
                newElement.type = LegalType::implicitDeref(ee.type);

                bufferPseudoTupleType->elements.Add(newElement);
            }

            return LegalType::tuple(bufferPseudoTupleType);
        }
        break;

    default:
        SLANG_UNEXPECTED("unknown legal type flavor");
        UNREACHABLE_RETURN(LegalType());
        break;
    }
}

static LegalType createLegalUniformBufferType(
    TypeLegalizationContext*    context,
    UniformParameterGroupType*  uniformBufferType,
    LegalType                   legalElementType)
{
    return createLegalUniformBufferType(
        context,
        uniformBufferType->declRef,
        legalElementType);
}

// Create a pointer type with a given legalized value type.
static LegalType createLegalPtrType(
    TypeLegalizationContext*    context,
    DeclRef<Decl> const&        typeDeclRef,
    LegalType                   legalValueType)
{
    switch (legalValueType.flavor)
    {
    case LegalType::Flavor::simple:
        {
            // Easy case: we just have a simple element type,
            // so we want to create a uniform buffer that wraps it.
            return LegalType::simple(createBuiltinGenericType(
                context,
                typeDeclRef,
                legalValueType.getSimple()));
        }
        break;

    case LegalType::Flavor::implicitDeref:
        {
            // We are being asked to create a pointer type to something
            // that is implicitly dereferenced, meaning we had:
            //
            //      Ptr(PtrLink(T))
            //
            // and now are being asked to make:
            //
            //      Ptr(implicitDeref(LegalT))
            //
            // So it seems like we can just create:
            //
            //      implicitDeref(Ptr(LegalT))
            //
            // and nobody should really be able to tell the difference, right?
            return LegalType::implicitDeref(createLegalPtrType(
                context,
                typeDeclRef,
                legalValueType.getImplicitDeref()->valueType));
        }
        break;

    case LegalType::Flavor::pair:
        {
            // We just need to pointer-ify both sides of the pair.
            auto pairType = legalValueType.getPair();

            auto ordinaryType = createBuiltinGenericType(
                context,
                typeDeclRef,
                pairType->ordinaryType);
            auto specialType = createLegalPtrType(
                context,
                typeDeclRef,
                pairType->specialType);

            return LegalType::pair(ordinaryType, specialType, pairType->pairInfo);
        }

    case LegalType::Flavor::tuple:
        {
            // Wrap each of the tuple elements up as a pointer.
            auto valuePseudoTupleType = legalValueType.getTuple();

            RefPtr<TuplePseudoType> ptrPseudoTupleType = new TuplePseudoType();

            // Wrap all the pseudo-tuple elements with `implicitDeref`,
            // since they used to be inside a tuple, but aren't any more.
            for (auto ee : valuePseudoTupleType->elements)
            {
                TuplePseudoType::Element newElement;

                newElement.fieldDeclRef = ee.fieldDeclRef;
                newElement.type = createLegalPtrType(
                    context,
                    typeDeclRef,
                    ee.type);

                ptrPseudoTupleType->elements.Add(newElement);
            }

            return LegalType::tuple(ptrPseudoTupleType);
        }
        break;

    default:
        SLANG_UNEXPECTED("unknown legal type flavor");
        UNREACHABLE_RETURN(LegalType());
        break;
    }
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
    else if (auto uniformBufferType = type->As<UniformParameterGroupType>())
    {
        // We have a `ConstantBuffer<T>` or `TextureBuffer<T>` or
        // other pointer-like type that represents uniform parameters.
        // We need to pull any resource-type fields out of it, but
        // leave the non-resource fields where they are.

        // Legalize the element type to see what we are working with.
        auto legalElementType = legalizeType(context,
            uniformBufferType->getElementType());

        switch (legalElementType.flavor)
        {
        case LegalType::Flavor::simple:
            return LegalType::simple(type);

        default:
            return createLegalUniformBufferType(
                context,
                uniformBufferType,
                legalElementType);
        }

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
    else if (auto ptrType = type->As<PtrTypeBase>())
    {
        auto legalValueType = legalizeType(context, ptrType->getValueType());
        return createLegalPtrType(context, ptrType->declRef, legalValueType);
    }
    else if (auto declRefType = type->As<DeclRefType>())
    {
        auto declRef = declRefType->declRef;
        if (auto aggTypeDeclRef = declRef.As<AggTypeDecl>())
        {
            // Look at the (non-static) fields, and
            // see if anything needs to be cleaned up.
            // The things that need to be "cleaned up" for
            // our purposes are:
            //
            // - Fields of resource type, or any other future
            //   type we run into that isn't allowed in
            //   aggregates for at least some targets
            //
            // - Fields with types that themselves had to
            //   get legalized.
            //
            // If we don't run into any of these, we
            // can just use the type as-is. Hooray!
            //
            // Otherwise, we are effectively going to split
            // the type apart and create a `TuplePseudoType`.
            // Every field of the original type will be
            // represented as an element of this pseudo-type.
            // Each element will record its `LegalType`,
            // and the original field that it was created from.
            // An element will also track whether it contains
            // any "ordinary" data, and if so, it will remember
            // an element index in a real (AST-level, non-pseudo)
            // `TupleType` that is used to bundle together
            // such fields.
            //
            // Storing all the simple fields together like this
            // obviously adds complexity to the legalization
            // pass, but it has important benefits:
            //
            // - It avoids creating functions with a very large
            //   number of parameters (when passing a structure
            //   with many fields), which might confuse downstream
            //   compilers.
            //
            // - It avoids applying AOS->SOA conversion to fields
            //   that don't actually need it, which is basically
            //   required if we want type layout to work.
            //
            // - It ensures that we can actually construct a
            //   constant-buffer type that wraps a legalized
            //   aggregate type; the ordinary fields will get
            //   placed inside a new constant-buffer type,
            //   while the special ones will get left outside.
            // 

            TupleTypeBuilder builder;
            builder.context = context;
            builder.type = type;


            for (auto ff : getMembersOfType<StructField>(aggTypeDeclRef))
            {
                builder.addField(ff);
            }

            return builder.getResult();
        }

        // TODO: for other declaration-reference types, we really
        // need to legalize the types used in substitutions, and
        // signal an error if any of them turn out to be non-simple.
        //
        // The limited cases of types that can handle having non-simple
        // types as generic arguments all need to be special-cased here.
        // (For example, we can't handle `Texture2D<SomeStructWithTexturesInIt>`.
        // 
    }

    return LegalType::simple(type);
}

// Represents the "chain" of declarations that
// were followed to get to a variable that we
// are now declaring as a leaf variable.
struct LegalVarChain
{
    LegalVarChain*  next;
    VarLayout*      varLayout;
};

static LegalVal declareVars(
    TypeLegalizationContext*    context,
    IROp                        op,
    LegalType                   type,
    TypeLayout*                 typeLayout,
    LegalVarChain*              varChain);

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
    TypeLegalizationContext*    context,
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
    TypeLegalizationContext*    context,
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
    TypeLegalizationContext*    context,
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
                ordinaryType = LegalType::simple(fieldPairType->ordinaryType);
                specialType = fieldPairType->specialType;
            }

            LegalVal ordinaryVal;
            LegalVal specialVal;

            if (pairElement->flags & PairInfo::kFlag_hasOrdinary)
            {
                ordinaryVal = legalizeFieldAddress(context, ordinaryType, pairVal->ordinaryVal, legalFieldOperand);
            }

            if (pairElement->flags & PairInfo::kFlag_hasSpecial)
            {
                specialVal = legalizeFieldAddress(context, specialType, pairVal->specialVal, legalFieldOperand);
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
    TypeLegalizationContext*    context,
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
    TypeLegalizationContext*    context,
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
            addParamType(ftype, LegalType::simple(pairInfo->ordinaryType));
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
    TypeLegalizationContext*    context,
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
            auto parentSpaceInfo = vv->varLayout->FindResourceInfo(LayoutResourceKind::RegisterSpace);
            if (!parentSpaceInfo)
                continue;

            for (auto& rr : varLayout->resourceInfos)
            {
                if (rr.kind == LayoutResourceKind::RegisterSpace)
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
            auto ordinaryVal = declareVars(context, op, LegalType::simple(pairType->ordinaryType), typeLayout, varChain);
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
    TypeLegalizationContext*    context,
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

}
