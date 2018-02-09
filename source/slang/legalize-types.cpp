// legalize-types.cpp
#include "legalize-types.h"

#include "mangle.h"

namespace Slang
{

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

LegalType LegalType::tuple(
    RefPtr<TuplePseudoType>   tupleType)
{
    LegalType result;
    result.flavor = Flavor::tuple;
    result.obj = tupleType;
    return result;
}

LegalType LegalType::pair(
    RefPtr<PairPseudoType>   pairType)
{
    LegalType result;
    result.flavor = Flavor::pair;
    result.obj = pairType;
    return result;
}

LegalType LegalType::pair(
    LegalType const&    ordinaryType,
    LegalType const&    specialType,
    RefPtr<PairInfo>    pairInfo)
{
    // Handle some special cases for when
    // one or the other of the types isn't
    // actually used.

    if (ordinaryType.flavor == LegalType::Flavor::none)
    {
        // There was nothing ordinary.
        return specialType;
    }

    if (specialType.flavor == LegalType::Flavor::none)
    {
        return ordinaryType;
    }

    // There were both ordinary and special fields,
    // and so we need to handle them here.

    RefPtr<PairPseudoType> obj = new PairPseudoType();
    obj->ordinaryType = ordinaryType;
    obj->specialType = specialType;
    obj->pairInfo = pairInfo;
    return LegalType::pair(obj);
}

//

static bool isResourceType(Type* type)
{
    while (auto arrayType = type->As<ArrayExpressionType>())
    {
        type = arrayType->baseType;
    }

    if (auto resourceTypeBase = type->As<ResourceTypeBase>())
    {
        return true;
    }
    else if (auto builtinGenericType = type->As<BuiltinGenericType>())
    {
        return true;
    }
    else if (auto pointerLikeType = type->As<PointerLikeType>())
    {
        return true;
    }
    else if (auto samplerType = type->As<SamplerStateType>())
    {
        return true;
    }
    else if(auto untypedBufferType = type->As<UntypedBufferResourceType>())
    {
        return true;
    }

    // TODO: need more comprehensive coverage here

    return false;
}

ModuleDecl* findModuleForDecl(
    Decl*   decl)
{
    for (auto dd = decl; dd; dd = dd->ParentDecl)
    {
        if (auto moduleDecl = dynamic_cast<ModuleDecl*>(dd))
            return moduleDecl;
    }
    return nullptr;
}


// Helper type for legalization of aggregate types
// that might need to be turned into tuple pseudo-types.
struct TupleTypeBuilder
{
    TypeLegalizationContext*    context;
    RefPtr<Type>                type;
    DeclRef<AggTypeDecl>        typeDeclRef;

    struct OrdinaryElement
    {
        DeclRef<VarDeclBase>    fieldDeclRef;
        RefPtr<Type>            type;
    };


    List<OrdinaryElement> ordinaryElements;
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
        LegalType ordinaryType;
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
                    ordinaryType = legalLeafType;
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

                // If things originally started as a resource type, then
                // we want to externalize all the fields that arose, even
                // if there is (nominally) ordinary data.
                //
                // This is because the "ordinary" side of the legalization
                // of `ConstantBuffer<Foo>` will still be a resource type.
                if(isResource)
                {
                    specialType = legalFieldType;
                }
                else
                {
                    ordinaryType = pairType->ordinaryType;
                    specialType = pairType->specialType;
                    elementPairInfo = pairType->pairInfo;
                }
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

        String mangledFieldName = getMangledName(fieldDeclRef.getDecl());

        PairInfo::Element pairElement;
        pairElement.flags = 0;
        pairElement.mangledName = mangledFieldName;
        pairElement.fieldPairInfo = elementPairInfo;

        // We will always add a field to the "ordinary"
        // side of things, even if it has no ordinary
        // data, just to keep the list of fields aligned
        // with the original type.
        OrdinaryElement ordinaryElement;
        ordinaryElement.fieldDeclRef = fieldDeclRef;
        if (ordinaryType.flavor != LegalType::Flavor::none)
        {
            anyOrdinary = true;
            pairElement.flags |= PairInfo::kFlag_hasOrdinary;

            LegalType ot = ordinaryType;

            // TODO: any cases we should "unwrap" here?
            // E.g., `implicitDeref`?

            if(ot.flavor == LegalType::Flavor::simple)
            {
                ordinaryElement.type = ot.getSimple();
            }
            else
            {
                SLANG_UNEXPECTED("unexpected ordinary field type");
            }
        }
        ordinaryElements.Add(ordinaryElement);

        if (specialType.flavor != LegalType::Flavor::none)
        {
            anySpecial = true;
            anyComplex = true;
            pairElement.flags |= PairInfo::kFlag_hasSpecial;

            TuplePseudoType::Element specialElement;
            specialElement.mangledName = mangledFieldName;
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
        // then we need to collect them into a new `struct` type
        // that represents these fields.
        //
        LegalType ordinaryType;
        if (anyOrdinary)
        {
            // We are going to create a new `struct` type declaration that clones
            // the fields we care about from the original `struct` type. Note that
            // these fields may have different types from what they did before,
            // because the fields themselves might have been legalized.
            //
            // Our new declaration will have the same name as the old one, so
            // downstream code is going to need to be careful not to emit declarations
            // for both of them. This should be okay, though, because the original
            // type was illegal (that was the whole point) and so it shouldn't be
            // allowed in the output anyway.
            RefPtr<StructDecl> ordinaryStructDecl = new StructDecl();
            ordinaryStructDecl->loc = typeDeclRef.getDecl()->loc;
            ordinaryStructDecl->nameAndLoc = typeDeclRef.getDecl()->nameAndLoc;

            auto typeLegalizedModifier = new LegalizedModifier();
            typeLegalizedModifier->originalMangledName = getMangledName(typeDeclRef);
            addModifier(ordinaryStructDecl, typeLegalizedModifier);

            // We will do something a bit unsavory here, by setting the logical
            // parent of the new `struct` type to be the same as the orignal type
            // (All of this helps ensure it gets the same mangled name).
            //
            ordinaryStructDecl->ParentDecl = typeDeclRef.getDecl()->ParentDecl;

            if (context->mainModuleDecl)
            {
                // If the declaration we are lowering belongs to the AST-based
                // module being lowered (rather than translated to IR), then we
                // need to add any new declaration we create to that output.

                // If we are *not* outputting an IR module as well, then
                // everything needs to wind up in a single AST module.
                if (!context->irModule)
                {
                    context->outputModuleDecl->Members.Add(ordinaryStructDecl);
                }
                else
                {
                    // Otherwise, check if this declaration belongs to the main
                    // module (which is being lowered via the AST-to-AST pass),
                    // and add it to the output if needed.
                    //
                    // TODO: This won't work correctly if a type from the AST
                    // module is used to specialize a generic in the IR module,
                    // since the declaration would need to precede the specialized
                    // func...
                    auto parentModule = findModuleForDecl(typeDeclRef.getDecl());
                    if (parentModule && (parentModule == context->mainModuleDecl))
                    {
                        context->outputModuleDecl->Members.Add(ordinaryStructDecl);
                    }
                }
            }

            // For memory management reasons, we need to keep a reference to
            // the declaration live, no matter what.
            context->createdDecls.Add(ordinaryStructDecl);

            UInt elementCounter = 0;
            for(auto ee : ordinaryElements)
            {
                UInt elementIndex = elementCounter++;

                // We will ensure that all the original fields are represented,
                // although they may have different types (due to legalization).
                // For fields that have *no* ordinary data, we will give them
                // a dummy `void` type and rely on downstream passes to not
                // actually emit declarations for those fields.
                //
                // (This helps keeps things simple because both the original
                // and modified type will have the same number of fields, so
                // we can continue to look up field layouts by index in the
                // emit logic)
                RefPtr<Type> fieldType = ee.type;
                if(!fieldType)
                    fieldType = context->session->getVoidType();

                // TODO: shallow clone of modifiers, etc.

                RefPtr<StructField> fieldDecl = new StructField();
                fieldDecl->loc = ee.fieldDeclRef.getDecl()->loc;
                fieldDecl->nameAndLoc = ee.fieldDeclRef.getDecl()->nameAndLoc;
                fieldDecl->type.type = fieldType;

                fieldDecl->ParentDecl = ordinaryStructDecl;
                ordinaryStructDecl->Members.Add(fieldDecl);

                pairElements[elementIndex].ordinaryFieldDeclRef = makeDeclRef(fieldDecl.Ptr());

                auto fieldLegalizedModifier = new LegalizedModifier();
                fieldLegalizedModifier->originalMangledName = getMangledName(ee.fieldDeclRef);
                addModifier(fieldDecl, fieldLegalizedModifier);
            }

            RefPtr<Type> ordinaryStructType = DeclRefType::Create(
                context->session,
                makeDeclRef(ordinaryStructDecl.Ptr()));

            ordinaryType = LegalType::simple(ordinaryStructType);
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

    RefPtr<GenericSubstitution> oldGenericSubst = typeDeclRef.substitutions.genericSubstitutions;
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

            auto ordinaryType = createLegalUniformBufferType(
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

                newElement.mangledName = ee.mangledName;
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

            auto ordinaryType = createLegalPtrType(
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

                newElement.mangledName = ee.mangledName;
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

struct LegalTypeWrapper
{
    virtual LegalType wrap(TypeLegalizationContext* context, Type* type) = 0;
};

struct ArrayLegalTypeWrapper : LegalTypeWrapper
{
    ArrayExpressionType*    arrayType;

    LegalType wrap(TypeLegalizationContext* context, Type* type)
    {
        return LegalType::simple(context->session->getArrayType(
            type,
            arrayType->ArrayLength));
    }
};

struct BuiltinGenericLegalTypeWrapper : LegalTypeWrapper
{
    DeclRef<Decl>   declRef;

    LegalType wrap(TypeLegalizationContext* context, Type* type)
    {
        return LegalType::simple(createBuiltinGenericType(
            context,
            declRef,
            type));
    }
};


struct ImplicitDerefLegalTypeWrapper : LegalTypeWrapper
{
    LegalType wrap(TypeLegalizationContext*, Type* type)
    {
        return LegalType::implicitDeref(LegalType::simple(type));
    }
};

static LegalType wrapLegalType(
    TypeLegalizationContext*    context,
    LegalType                   legalType,
    LegalTypeWrapper*           ordinaryWrapper,
    LegalTypeWrapper*           specialWrapper)
{
    switch (legalType.flavor)
    {
    case LegalType::Flavor::simple:
        {
            return ordinaryWrapper->wrap(context, legalType.getSimple());
        }
        break;

    case LegalType::Flavor::implicitDeref:
        {
            return LegalType::implicitDeref(wrapLegalType(
                context,
                legalType,
                ordinaryWrapper,
                specialWrapper));
        }
        break;

    case LegalType::Flavor::pair:
        {
            // We just need to pointer-ify both sides of the pair.
            auto pairType = legalType.getPair();

            auto ordinaryType = wrapLegalType(
                context,
                pairType->ordinaryType,
                ordinaryWrapper,
                ordinaryWrapper);
            auto specialType = wrapLegalType(
                context,
                pairType->specialType,
                specialWrapper,
                specialWrapper);

            return LegalType::pair(ordinaryType, specialType, pairType->pairInfo);
        }

    case LegalType::Flavor::tuple:
        {
            // Wrap each of the tuple elements up as a pointer.
            auto tupleType = legalType.getTuple();

            RefPtr<TuplePseudoType> resultTupleType = new TuplePseudoType();

            // Wrap all the pseudo-tuple elements with `implicitDeref`,
            // since they used to be inside a tuple, but aren't any more.
            for (auto ee : tupleType->elements)
            {
                TuplePseudoType::Element element;

                element.mangledName = ee.mangledName;
                element.type = wrapLegalType(
                    context,
                    ee.type,
                    ordinaryWrapper,
                    specialWrapper);

                resultTupleType->elements.Add(element);
            }

            return LegalType::tuple(resultTupleType);
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
LegalType legalizeType(
    TypeLegalizationContext*    context,
    Type*                       type)
{
    if (auto uniformBufferType = type->As<UniformParameterGroupType>())
    {
        // We have one of:
        //
        //      ConstantBuffer<T>
        //      TextureBuffer<T>
        //      ParameterBlock<T>
        //
        // or some other pointer-like type that represents uniform
        // parameters. We need to pull any resource-type fields out
        // of it, but leave non-resource fields where they are.
        //
        // As a special case, if the type contains *no* uniform data,
        // we'll want to completely eliminate the uniform/ordinary
        // part.

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

        LegalType legalType;
        if(context->mapDeclRefToLegalType.TryGetValue(declRef, legalType))
            return legalType;


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
            builder.typeDeclRef = aggTypeDeclRef;


            for (auto ff : getMembersOfType<StructField>(aggTypeDeclRef))
            {
                builder.addField(ff);
            }

            legalType = builder.getResult();
            context->mapDeclRefToLegalType.AddIfNotExists(declRef, legalType);
            return legalType;
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
    else if(auto arrayType = type->As<ArrayExpressionType>())
    {
        auto legalElementType = legalizeType(
            context,
            arrayType->baseType);

        switch (legalElementType.flavor)
        {
        case LegalType::Flavor::simple:
            // Element type didn't need to be legalized, so
            // we can just use this type as-is.
            return LegalType::simple(type);

        default:
            {
                ArrayLegalTypeWrapper wrapper;
                wrapper.arrayType = arrayType;

                return wrapLegalType(
                    context,
                    legalElementType,
                    &wrapper,
                    &wrapper);
            }
            break;
        }

    }

    return LegalType::simple(type);
}

//

RefPtr<TypeLayout> getDerefTypeLayout(
    TypeLayout* typeLayout)
{
    if (!typeLayout)
        return nullptr;

    if (auto parameterGroupTypeLayout = dynamic_cast<ParameterGroupTypeLayout*>(typeLayout))
    {
        return parameterGroupTypeLayout->offsetElementTypeLayout;
    }

    return typeLayout;
}

RefPtr<VarLayout> getFieldLayout(
    TypeLayout*     typeLayout,
    String const&   mangledFieldName)
{
    if (!typeLayout)
        return nullptr;

    for(;;)
    {
        if(auto arrayTypeLayout = dynamic_cast<ArrayTypeLayout*>(typeLayout))
        {
            typeLayout = arrayTypeLayout->elementTypeLayout;
        }
        else if(auto parameterGroupTypeLayotu = dynamic_cast<ParameterGroupTypeLayout*>(typeLayout))
        {
            typeLayout = parameterGroupTypeLayotu->offsetElementTypeLayout;
        }
        else
        {
            break;
        }
    }


    if (auto structTypeLayout = dynamic_cast<StructTypeLayout*>(typeLayout))
    {
        for(auto ff : structTypeLayout->fields)
        {
            if(mangledFieldName == getMangledName(ff->varDecl) )
            {
                return ff;
            }
        }
    }

    return nullptr;
}

RefPtr<VarLayout> createVarLayout(
    LegalVarChain*  varChain,
    TypeLayout*     typeLayout)
{
    if (!typeLayout)
        return nullptr;

    // We need to construct a layout for the new variable
    // that reflects both the type we have given it, as
    // well as all the offset information that has accumulated
    // along the chain of parent variables.

    // TODO: this logic needs to propagate through semantics...

    RefPtr<VarLayout> varLayout = new VarLayout();
    varLayout->typeLayout = typeLayout;

    // For most resource kinds, the register index/space to use should
    // be the sum along the entire chain of variables.
    //
    // For example, if we had input:
    //
    //      struct S { Texture2D a; Texture2D b; };
    //      S s : register(t10);
    //
    // And we were generating a stand-alone variable for `s.b`, then
    // we'd need to add the offset for `b` (1 texture register), to
    // the offset for `s` (10 texture registers) to get the final
    // binding to apply.
    //
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

    // As a special case, if the leaf variable doesn't hold an entry for
    // `RegisterSpace`, but at least one declaration in the chain *does*,
    // then we want to make sure that we add such an entry.
    if (!varLayout->FindResourceInfo(LayoutResourceKind::RegisterSpace))
    {
        // Sum up contributions from all parents.
        UInt space = 0;
        for (auto vv = varChain; vv; vv = vv->next)
        {
            if (auto parentResInfo = vv->varLayout->FindResourceInfo(LayoutResourceKind::RegisterSpace))
            {
                space += parentResInfo->index;
            }
        }

        // If there were non-zero contributions, then add an entry to represent them.
        if (space)
        {
            varLayout->findOrAddResourceInfo(LayoutResourceKind::RegisterSpace)->index = space;
        }
    }



    return varLayout;
}

//

// TODO(tfoley): The code captured here is the logic that used to be
// applied to decide whether or not to desugar aggregate types that
// contain resources. Right now the implementation will *always* legalize
// away such types (since the IR always does this), while the AST-to-AST
// pass would only do it if required (according to the tests below).
//
// For right now this is an academic distinction, since the only project
// using Slang right now enables this tansformation unconditionally, but
// we probably need to re-parent this code back into the `TypeLegalizationContext`
// somewhere.
#if 0

bool shouldDesugarTupleTypes = false;
if (getTarget() == CodeGenTarget::GLSL)
{
    // Always desugar this stuff for GLSL, since it doesn't
    // support nesting of resources in structs.
    //
    // TODO: Need a way to make this more fine-grained to
    // handle cases where a nested member might be allowed
    // due to, e.g., bindless textures.
    shouldDesugarTupleTypes = true;
}
else if( shared->compileRequest->compileFlags & SLANG_COMPILE_FLAG_SPLIT_MIXED_TYPES )
{
    // If the user is directly asking us to do this transformation,
    // then obviously we need to do it.
    //
    // TODO: The way this is defined here means it will even apply to user
    // HLSL code (not just code written in Slang). We may want to
    // reconsider that choice, and only split things that originated in Slang.
    //
    shouldDesugarTupleTypes = true;
}

#endif

}
