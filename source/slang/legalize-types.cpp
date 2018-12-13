// legalize-types.cpp
#include "legalize-types.h"

#include "ir-insts.h"
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
    SLANG_ASSERT(tupleType->elements.Count());

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

static bool isResourceType(IRType* type)
{
    while (auto arrayType = as<IRArrayTypeBase>(type))
    {
        type = arrayType->getElementType();
    }

    if (auto resourceTypeBase = as<IRResourceTypeBase>(type))
    {
        return true;
    }
    else if (auto builtinGenericType = as<IRBuiltinGenericType>(type))
    {
        return true;
    }
    else if (auto pointerLikeType = as<IRPointerLikeType>(type))
    {
        return true;
    }
    else if (auto samplerType = as<IRSamplerStateTypeBase>(type))
    {
        return true;
    }
    else if(auto untypedBufferType = as<IRUntypedBufferResourceType>(type))
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
    IRType*                     type;
    IRStructType*               originalStructType;

    struct OrdinaryElement
    {
        IRStructKey*            fieldKey = nullptr;
        IRType*                 type = nullptr;
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
        IRStructKey*    fieldKey,
        LegalType       legalFieldType,
        LegalType       legalLeafType,
        bool            isResource)
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

        case LegalType::Flavor::none:
            anyComplex = true;
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
                    fieldKey,
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

//        String mangledFieldName = getMangledName(fieldDeclRef.getDecl());

        PairInfo::Element pairElement;
        pairElement.flags = 0;
        pairElement.key = fieldKey;
        pairElement.fieldPairInfo = elementPairInfo;

        // We will always add a field to the "ordinary"
        // side of things, even if it has no ordinary
        // data, just to keep the list of fields aligned
        // with the original type.
        OrdinaryElement ordinaryElement;
        ordinaryElement.fieldKey = fieldKey;
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
            specialElement.key = fieldKey;
            specialElement.type = specialType;
            specialElements.Add(specialElement);
        }

        pairElement.type = LegalType::pair(ordinaryType, specialType, elementPairInfo);
        pairElements.Add(pairElement);
    }

    // Add a field to the (pseudo-)type we are building
    void addField(
        IRStructField*  field)
    {
        auto fieldType = field->getFieldType();

        bool isResourceField = isResourceType(fieldType);
        auto legalFieldType = legalizeType(context, fieldType);

        addField(
            field->getKey(),
            legalFieldType,
            legalFieldType,
            isResourceField);
    }

    LegalType getResult()
    {
        // If this is an empty struct, return a none type
        // This helps get rid of emtpy structs that often trips up the 
        // downstream compiler
        if (!anyOrdinary && !anySpecial && !anyComplex)
            return LegalType();

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
            // We are going to create an new IR `struct` type that contains
            // the "ordinary" fields from the original type. Note that these
            // fields may have different types from what they did before,
            // because the fields themselves might have been legalized.
            //
            // The new type will have the same mangled name as the old one, so
            // downstream code is going to need to be careful not to emit declarations
            // for both of them. This should be okay, though, because the original
            // type was illegal (that was the whole point) and so it shouldn't be
            // referenced in the output anyway.
            //
            IRBuilder* builder = context->getBuilder();
            IRStructType* ordinaryStructType = builder->createStructType();
            ordinaryStructType->sourceLoc = originalStructType->sourceLoc;

            if(auto nameHintDecoration = originalStructType->findDecoration<IRNameHintDecoration>())
            {
                builder->addNameHintDecoration(ordinaryStructType, nameHintDecoration->getNameOperand());
            }

            // The new struct type will appear right after the original in the IR,
            // so that we can be sure any instruction that could reference the
            // original can also reference the new one.
            ordinaryStructType->insertAfter(originalStructType);

            // Mark the original type for removal once all the other legalization
            // activity is completed. This is necessary because both the original
            // and replacement type have the same mangled name, so they would
            // collide.
            //
            // (Also, the original type wasn't legal - that was the whole point...)
            context->instsToRemove.Add(originalStructType);

            for(auto ee : ordinaryElements)
            {
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
                //
                // TODO: we should scrap that, and layout lookup should just
                // be based on mangled field names in all cases.
                //
                IRType* fieldType = ee.type;
                if(!fieldType)
                    fieldType = context->getBuilder()->getVoidType();

                // TODO: shallow clone of modifiers, etc.

                builder->createStructField(
                    ordinaryStructType,
                    ee.fieldKey,
                    fieldType);
            }

            ordinaryType = LegalType::simple((IRType*) ordinaryStructType);
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

static IRType* createBuiltinGenericType(
    TypeLegalizationContext*    context,
    IROp                        op,
    IRType*                     elementType)
{
    IRInst* operands[] = { elementType };
    return context->getBuilder()->getType(
        op,
        1,
        operands);
}

// Create a uniform buffer type with a given legalized
// element type.
static LegalType createLegalUniformBufferType(
    TypeLegalizationContext*    context,
    IROp                        op,
    LegalType                   legalElementType)
{
    switch (legalElementType.flavor)
    {
    case LegalType::Flavor::none:
        return LegalType();

    case LegalType::Flavor::simple:
        {
            // Easy case: we just have a simple element type,
            // so we want to create a uniform buffer that wraps it.
            return LegalType::simple(createBuiltinGenericType(
                context,
                op,
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
                op,
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
                op,
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

                newElement.key = ee.key;
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
    TypeLegalizationContext*        context,
    IRUniformParameterGroupType*    uniformBufferType,
    LegalType                       legalElementType)
{
    return createLegalUniformBufferType(
        context,
        uniformBufferType->op,
        legalElementType);
}

// Create a pointer type with a given legalized value type.
static LegalType createLegalPtrType(
    TypeLegalizationContext*    context,
    IROp                        op,
    LegalType                   legalValueType)
{
    switch (legalValueType.flavor)
    {
    case LegalType::Flavor::none:
        return LegalType();

    case LegalType::Flavor::simple:
        {
            // Easy case: we just have a simple element type,
            // so we want to create a uniform buffer that wraps it.
            return LegalType::simple(createBuiltinGenericType(
                context,
                op,
                legalValueType.getSimple()));
        }
        break;

    case LegalType::Flavor::implicitDeref:
        {
            // We are being asked to create a pointer type to something
            // that is implicitly dereferenced, meaning we had:
            //
            //      Ptr(PtrLike(T))
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
            //
            // TODO: invetigate whether there are situations where this
            // will matter.
            return LegalType::implicitDeref(createLegalPtrType(
                context,
                op,
                legalValueType.getImplicitDeref()->valueType));
        }
        break;

    case LegalType::Flavor::pair:
        {
            // We just need to pointer-ify both sides of the pair.
            auto pairType = legalValueType.getPair();

            auto ordinaryType = createLegalPtrType(
                context,
                op,
                pairType->ordinaryType);
            auto specialType = createLegalPtrType(
                context,
                op,
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

                newElement.key = ee.key;
                newElement.type = createLegalPtrType(
                    context,
                    op,
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
    virtual LegalType wrap(TypeLegalizationContext* context, IRType* type) = 0;
};

struct ArrayLegalTypeWrapper : LegalTypeWrapper
{
    IRArrayTypeBase*    arrayType;

    LegalType wrap(TypeLegalizationContext* context, IRType* type)
    {
        return LegalType::simple(context->getBuilder()->getArrayTypeBase(
            arrayType->op,
            type,
            arrayType->getElementCount()));
    }
};

struct BuiltinGenericLegalTypeWrapper : LegalTypeWrapper
{
    IROp op;

    LegalType wrap(TypeLegalizationContext* context, IRType* type)
    {
        return LegalType::simple(createBuiltinGenericType(
            context,
            op,
            type));
    }
};


struct ImplicitDerefLegalTypeWrapper : LegalTypeWrapper
{
    LegalType wrap(TypeLegalizationContext*, IRType* type)
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
    case LegalType::Flavor::none:
        return LegalType();

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

                element.key = ee.key;
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
LegalType legalizeTypeImpl(
    TypeLegalizationContext*    context,
    IRType*                     type)
{

    if (auto uniformBufferType = as<IRUniformParameterGroupType>(type))
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
    else if (as<IRBasicType>(type))
    {
        return LegalType::simple(type);
    }
    else if (as<IRVectorType>(type))
    {
        return LegalType::simple(type);
    }
    else if (as<IRMatrixType>(type))
    {
        return LegalType::simple(type);
    }
    else if (auto ptrType = as<IRPtrTypeBase>(type))
    {
        auto legalValueType = legalizeType(context, ptrType->getValueType());
        return createLegalPtrType(context, ptrType->op, legalValueType);
    }
    else if(auto structType = as<IRStructType>(type))
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

        // TODO: there is a risk here that we might recursively
        // invole `legalizeType` on the type that we are
        // currently trying to legalize. We need to detect that
        // situation somehow, by inserting a sentinel value
        // into `mapTypeToLegalType` during the per-field
        // legalization process, and then if we ever see that
        // sentinel in a call to `legalizeType`, we need
        // to construct some kind of proxy type to help resolve
        // the problem.

        TupleTypeBuilder builder;
        builder.context = context;
        builder.type = type;
        builder.originalStructType = structType;

        for (auto ff : structType->getFields())
        {
            builder.addField(ff);
        }

        return builder.getResult();
    }
    else if(auto arrayType = as<IRArrayTypeBase>(type))
    {
        auto legalElementType = legalizeType(
            context,
            arrayType->getElementType());

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

void initialize(
    TypeLegalizationContext*    context,
    Session*                    session,
    IRModule*                   module)
{
    context->session = session;
    context->irModule = module;

    context->sharedBuilder.session = session;
    context->sharedBuilder.module = module;

    context->builder.sharedBuilder = &context->sharedBuilder;
    context->builder.setInsertInto(module->moduleInst);
}

LegalType legalizeType(
    TypeLegalizationContext*    context,
    IRType*                     type)
{
    LegalType legalType;
    if(context->mapTypeToLegalType.TryGetValue(type, legalType))
        return legalType;

    legalType = legalizeTypeImpl(context, type);
    context->mapTypeToLegalType[type] = legalType;
    return legalType;
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
            if(mangledFieldName == getMangledName(ff->varDecl.getDecl()) )
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
