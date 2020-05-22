// slang-legalize-types.cpp
#include "slang-legalize-types.h"

#include "slang-ir-insts.h"
#include "slang-mangle.h"

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
    SLANG_ASSERT(tupleType->elements.getCount());

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

LegalType LegalType::makeWrappedBuffer(
    IRType*             simpleType,
    LegalElementWrapping const&   elementInfo)
{
    RefPtr<WrappedBufferPseudoType> obj = new WrappedBufferPseudoType();
    obj->simpleType = simpleType;
    obj->elementInfo = elementInfo;

    LegalType result;
    result.flavor = Flavor::wrappedBuffer;
    result.obj = obj;
    return result;
}

//

LegalElementWrapping LegalElementWrapping::makeVoid()
{
    LegalElementWrapping result;
    result.flavor = Flavor::none;
    return result;
}

LegalElementWrapping LegalElementWrapping::makeSimple(IRStructKey* key, IRType* type)
{
    RefPtr<SimpleLegalElementWrappingObj> obj = new SimpleLegalElementWrappingObj();
    obj->key = key;
    obj->type = type;

    LegalElementWrapping result;
    result.flavor = Flavor::simple;
    result.obj = obj;
    return result;
}

RefPtr<SimpleLegalElementWrappingObj> LegalElementWrapping::getSimple() const
{
    SLANG_ASSERT(flavor == Flavor::simple);
    return obj.as<SimpleLegalElementWrappingObj>();
}

LegalElementWrapping LegalElementWrapping::makeImplicitDeref(LegalElementWrapping const& field)
{
    RefPtr<ImplicitDerefLegalElementWrappingObj> obj = new ImplicitDerefLegalElementWrappingObj();
    obj->field = field;

    LegalElementWrapping result;
    result.flavor = Flavor::implicitDeref;
    result.obj = obj;
    return result;
}

RefPtr<ImplicitDerefLegalElementWrappingObj> LegalElementWrapping::getImplicitDeref() const
{
    SLANG_ASSERT(flavor == Flavor::implicitDeref);
    return obj.as<ImplicitDerefLegalElementWrappingObj>();
}

LegalElementWrapping LegalElementWrapping::makePair(
    LegalElementWrapping const&   ordinary,
    LegalElementWrapping const&   special,
    PairInfo*           pairInfo)
{
    RefPtr<PairLegalElementWrappingObj> obj = new PairLegalElementWrappingObj();
    obj->ordinary = ordinary;
    obj->special = special;
    obj->pairInfo = pairInfo;

    LegalElementWrapping result;
    result.flavor = Flavor::pair;
    result.obj = obj;
    return result;
}

RefPtr<PairLegalElementWrappingObj> LegalElementWrapping::getPair() const
{
    SLANG_ASSERT(flavor == Flavor::pair);
    return obj.as<PairLegalElementWrappingObj>();
}

LegalElementWrapping LegalElementWrapping::makeTuple(TupleLegalElementWrappingObj* obj)
{
    LegalElementWrapping result;
    result.flavor = Flavor::tuple;
    result.obj = obj;
    return result;
}

RefPtr<TupleLegalElementWrappingObj> LegalElementWrapping::getTuple() const
{
    SLANG_ASSERT(flavor == Flavor::tuple);
    return obj.as<TupleLegalElementWrappingObj>();
}

//

bool isResourceType(IRType* type)
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
    for (auto dd = decl; dd; dd = dd->parentDecl)
    {
        if (auto moduleDecl = as<ModuleDecl>(dd))
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
        bool            isSpecial)
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
                if (!isSpecial)
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
                    isSpecial);
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
                if(isSpecial)
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
        ordinaryElements.add(ordinaryElement);

        if (specialType.flavor != LegalType::Flavor::none)
        {
            anySpecial = true;
            anyComplex = true;
            pairElement.flags |= PairInfo::kFlag_hasSpecial;

            TuplePseudoType::Element specialElement;
            specialElement.key = fieldKey;
            specialElement.type = specialType;
            specialElements.add(specialElement);
        }

        pairElement.type = LegalType::pair(ordinaryType, specialType, elementPairInfo);
        pairElements.add(pairElement);
    }

    // Add a field to the (pseudo-)type we are building
    void addField(
        IRStructField*  field)
    {
        auto fieldType = field->getFieldType();

        bool isSpecialField = context->isSpecialType(fieldType);
        auto legalFieldType = legalizeType(context, fieldType);

        addField(
            field->getKey(),
            legalFieldType,
            legalFieldType,
            isSpecialField);
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
            context->replacedInstructions.add(originalStructType);

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
    // We will handle some of the easy/non-interesting
    // cases here in the main routine, but for all
    // the non-trivial cases we will dispatch to logic
    // on the `context` (which may differ depending
    // on what we are using legalization to accomplish).
    //
    switch (legalElementType.flavor)
    {
    default:
        return context->createLegalUniformBufferType(
            op,
            legalElementType);

    case LegalType::Flavor::none:
        return LegalType();

    case LegalType::Flavor::simple:
        {
            // Easy case: we just have a simple element type,
            // so we want to create a uniform buffer that wraps it.
            //
            // TODO: This isn't *quite* right, since it won't handle something
            // like a `ParameterBlock<Texture2D>`, but that seems like
            // an unlikely case in practice.
            //
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
    }
}

// Create a uniform buffer type with a given legalized element type,
// under the assumption that we are doing resource-based type legalization.
//
LegalType createLegalUniformBufferTypeForResources(
    TypeLegalizationContext*    context,
    IROp                        op,
    LegalType                   legalElementType)
{
    switch (legalElementType.flavor)
    {
    case LegalType::Flavor::simple:
        {
            // Seeing a simple type here means that it must be a
            // "special" type (a resource type or array thereof)
            // because otherwise the catch-all behavior in
            // `createLegalUniformBufferType()` would have handled it.
            //
            // This case is the same as what we do for tuple elements below.
            //
            return LegalType::implicitDeref(legalElementType);
        }

    case LegalType::Flavor::pair:
        {
            auto pairType = legalElementType.getPair();

            // The pair has both an "ordinary" and a "special"
            // side, where the ordinary side is just plain data
            // that we can put in a constant buffer type without
            // any problems. The special side will (recursively)
            // contain any resource-type fields that were nested
            // in the constant buffer, and we'll need to
            // treat those as resources that stand alongside
            // the original constant buffer.
            //
            // We can start with the ordinary side, which we
            // just want to wrap up in an ordinary uniform
            // buffer with the appropriate `op`, so that case
            // is easy:
            //
            auto ordinaryType = createLegalUniformBufferType(
                context,
                op,
                pairType->ordinaryType);

            // For the special side, we really just want to turn
            // a special field of type `R` into a value of type
            // `R`, and the main detail we have to be aware of
            // is that any use sites for the original buffer/block
            // will include a dereferencing step to get from
            // the block to this field, so we need to add
            // something to the type structure to account for
            // that step.
            //
            // We handle that issue by wrapping the special
            // part of the type in an `implicitDeref` wrapper,
            // which indicates that we logically have `SomePtr<R>`
            // but we actually just have `R`, and any attempt to
            // load from or otherwise indirect through that pointer
            // will turn into a plain old reference to the `R` value.
            //
            auto specialType = LegalType::implicitDeref(pairType->specialType);

            // Once we've wrapped up both the ordinary and special
            // sides suitably, we tie them back together in a pair
            // and make that be the legalized type of the result.
            //
            return LegalType::pair(ordinaryType, specialType, pairType->pairInfo);
        }

    case LegalType::Flavor::tuple:
        {
            // A tuple type always represents purely "special" data,
            // which in this case means resources.
            //
            // As in the `pair` case, the main thing we have to
            // take into account is that each of the entries in the
            // tuple itself (e.g., a value of type `R`) and the code
            // that uses the legalized buffer type will expect a
            // `ConstantBuffer<R>` or at least `SomePtrType<R>`.
            //
            // We will construct a new tuple type that wraps each
            // of the element types in an `implicitDeref` to
            // account for the different in levels of indirection.
            //
            // TODO: This seems odd, because we *should* be able to
            // just wrap the whole thing in an `implicitDeref` and
            // have done. We should investigate why this roundabout
            // way of doing things was ever necessary.

            auto elementPseudoTupleType = legalElementType.getTuple();
            RefPtr<TuplePseudoType> bufferPseudoTupleType = new TuplePseudoType();

            for (auto ee : elementPseudoTupleType->elements)
            {
                TuplePseudoType::Element newElement;

                newElement.key = ee.key;
                newElement.type = LegalType::implicitDeref(ee.type);

                bufferPseudoTupleType->elements.add(newElement);
            }

            return LegalType::tuple(bufferPseudoTupleType);
        }
        break;

    default:
        SLANG_UNEXPECTED("unhandled legal type flavor");
        UNREACHABLE_RETURN(LegalType());
        break;
    }
}

// Legalizing a uniform buffer/block type for existentials is
// more interesting, because we don't actually want to push
// the "special" fields out of the buffer entirely (as we
// do for resources), and instead we just want to place
// them in the buffer *after* all the ordinary data.
//
// In order to accomplish this we need a way to emit a
// constant buffer with a new element type, and then
// "wrap" that constant buffer so that it looks like
// something that matches the legalization of the original
// element type.
//
// As a concrete example, suppose we have:
//
//      struct Params { ExistentialBox<Foo> f; int x; ExistentialBox<Bar> b; };
//      ConstantBuffer<Params> gParams;
//
// The legalized form of `Params` will be something like:
//
//      Pair(
//          /* ordinary: */ struct Params_Ordinary { int x; },
//          /* special: */ Tuple(
//              f -> ImplicitDeref(Foo),
//              b -> ImplicitDeref(Bar)))
//
// We need to be able to splat that all out into a single
// structure declaration like:
//
//      struct Params_Reordered
//      {
//          Params_Ordinary ordinary;
//          Foo f;
//          Bar b;
//      }
//
// That allows us to declare:
//
//      ConstantBuffer<Params_Reordered> gParams;
//
// That gets the in-memory layout of things correct for the
// way we are defining existential value slots to work.
// The challenge is that elsewehere in the code there are
// operations like `gParams.x` need to now refer to
// `gParams.ordinary.x`. Furthermore, even for something like
// `f` that seems fine in the example above, we have lost
// a level of indirection, so that where we had `load(gParams.f)`
// we now want just `gParams.f`.
//
// The solution is to take `gParams` as soon as it is declared
// and wrap it up as a new value:
//
//      pair(
//          /* ordinary: */ gParams.ordinary,
//          /* special: */ tuple(
//              f -> implicitDeref(gParams.f),
//              b -> implicitDeref(gParams.b)))
//
//
// Let's begin by just defining a function that can take
// a `LegalType` and turn it into zero or more field
// declarations, and return enough tracking information
// for us to be able to reconstruct a value like the above.
//
LegalElementWrapping declareStructFields(
    TypeLegalizationContext*    context,
    IRStructType*               structType,
    LegalType                   fieldType)
{
    // TODO: We should eventually thread through some kind
    // of "name hint" that can be used to give the generated
    // fields more useful names.

    switch(fieldType.flavor)
    {
    case LegalType::Flavor::none:
        return LegalElementWrapping::makeVoid();

    case LegalType::Flavor::simple:
        {
            auto simpleFieldType = fieldType.getSimple();
            auto builder = context->getBuilder();
            auto fieldKey = builder->createStructKey();
            builder->createStructField(structType, fieldKey, simpleFieldType);
            return LegalElementWrapping::makeSimple(fieldKey, simpleFieldType);
        }

    case LegalType::Flavor::implicitDeref:
        {
            auto subField = declareStructFields(context, structType, fieldType.getImplicitDeref()->valueType);
            return LegalElementWrapping::makeImplicitDeref(subField);
        }

    case LegalType::Flavor::pair:
        {
            auto pairType = fieldType.getPair();
            auto ordinaryField = declareStructFields(context, structType, pairType->ordinaryType);
            auto specialField = declareStructFields(context, structType, pairType->specialType);
            return LegalElementWrapping::makePair(
                ordinaryField,
                specialField,
                pairType->pairInfo);
        }

    case LegalType::Flavor::tuple:
        {
            auto tupleType = fieldType.getTuple();

            RefPtr<TupleLegalElementWrappingObj> obj = new TupleLegalElementWrappingObj();
            for( auto ee : tupleType->elements )
            {
                TupleLegalElementWrappingObj::Element element;
                element.key = ee.key;
                element.field = declareStructFields(context, structType, ee.type);
                obj->elements.add(element);
            }
            return LegalElementWrapping::makeTuple(obj);
        }

    default:
        SLANG_UNEXPECTED("unhandled legal type flavor");
        UNREACHABLE_RETURN(LegalElementWrapping::makeVoid());
        break;
    }
}

LegalType createLegalUniformBufferTypeForExistentials(
    TypeLegalizationContext*    context,
    IROp                        op,
    LegalType                   legalElementType)
{
    auto builder = context->getBuilder();

    // In order to wrap up all the data in `legalElementType`,
    // will create a fresh `struct` type and then declare
    // fields in it that are sufficient to hold that data
    // in `legalElementType`.
    //
    auto structType = builder->createStructType();
    auto elementWrapping = declareStructFields(
        context, structType, legalElementType);

    // Because the `structType` is an ordinary IR type
    // (not a `LegalType`) we can go ahead and create an
    // IR uniform buffer type that wraps it.
    //
    auto bufferType = createBuiltinGenericType(
        context,
        op,
        structType);

    // The `elementWrapping` computed when we declared all
    // the `struct` fields tells us how to get from the
    // actual fields declared in the structure type to a
    // `LegalVal` with the right shape for what users of
    // the buffer will expect. We record both the underlying
    // IR buffer type and that wrapping information into
    // the resulting `LegalType` so that we can use it
    // when declaring variables of this type.
    //
    return LegalType::makeWrappedBuffer(bufferType, elementWrapping);
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

                ptrPseudoTupleType->elements.add(newElement);
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

                resultTupleType->elements.add(element);
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
    if(!type)
        return LegalType::simple(nullptr);

    // It might be that the type we are looking at is
    // an intrinsic type on our chosen target, in which
    // case we should never legalize it, figuring that
    // the target defines its semantics fully.
    //
    if(type->findDecoration<IRTargetIntrinsicDecoration>())
        return LegalType::simple(type);

    context->builder->setInsertBefore(type);

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

        auto originalElementType = uniformBufferType->getElementType();

        // Legalize the element type to see what we are working with.
        auto legalElementType = legalizeType(context,
            originalElementType);

        // As a bit of a corner case, if the user requested something
        // like `ConstantBuffer<Texture2D>` the element type would
        // legalize to a "simple" type, and that would be interpreted
        // as an *ordinary* type, but we really need to notice the
        // case when the element type is simple, but *special*.
        //
        if( context->isSpecialType(originalElementType) )
        {
            // Anything that has a special element type needs to
            // be handled by the pass-specific logic in the context.
            //
            return context->createLegalUniformBufferType(
                uniformBufferType->op,
                legalElementType);
        }

        // Note that even when legalElementType.flavor == Simple
        // we still need to create a new uniform buffer type
        // from `legalElementType` instead of `type`
        // because the `legalElementType` may still differ from `type`
        // if, e.g., `type` contains empty structs.
        return createLegalUniformBufferType(
                context,
                uniformBufferType,
                legalElementType);

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
    else if( auto existentialPtrType = as<IRExistentialBoxType>(type))
    {
        // We want to transform an `ExistentialBox<T>` into just
        // a `T`, with an `iplicitDeref` to make sure that any
        // pointer-related operations on the box Just Work.
        //
        // Note: the logic here doesn't have to deal with moving
        // existential-type fields to the end of their outer
        // type(s) because that is mostly dealt with in the 
        // case for struct types below.
        //
        auto legalValueType = legalizeType(context, existentialPtrType->getValueType());
        return LegalType::implicitDeref(legalValueType);
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

        ArrayLegalTypeWrapper wrapper;
        wrapper.arrayType = arrayType;

        return wrapLegalType(
            context,
            legalElementType,
            &wrapper,
            &wrapper);
    }

    return LegalType::simple(type);
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

IRTypeLayout* getDerefTypeLayout(
    IRTypeLayout* typeLayout)
{
    if (!typeLayout)
        return nullptr;

    if (auto parameterGroupTypeLayout = as<IRParameterGroupTypeLayout>(typeLayout))
    {
        return parameterGroupTypeLayout->getOffsetElementTypeLayout();
    }

    return typeLayout;
}

IRVarLayout* getFieldLayout(
    IRTypeLayout*     typeLayout,
    IRInst*         fieldKey)
{
    if (!typeLayout)
        return nullptr;

    for(;;)
    {
        if(auto arrayTypeLayout = as<IRArrayTypeLayout>(typeLayout))
        {
            typeLayout = arrayTypeLayout->getElementTypeLayout();
        }
        else if(auto parameterGroupTypeLayout = as<IRParameterGroupTypeLayout>(typeLayout))
        {
            typeLayout = parameterGroupTypeLayout->getOffsetElementTypeLayout();
        }
        else
        {
            break;
        }
    }


    if (auto structTypeLayout = as<IRStructTypeLayout>(typeLayout))
    {
        for(auto ff : structTypeLayout->getFieldLayoutAttrs())
        {
            if(ff->getFieldKey() == fieldKey)
            {
                return ff->getLayout();
            }
        }
    }

    return nullptr;
}

void buildSimpleVarLayout(
    IRVarLayout::Builder*   builder,
    SimpleLegalVarChain*    varChain,
    IRTypeLayout*           typeLayout)
{
    // We need to construct a layout for the new variable
    // that reflects both the type we have given it, as
    // well as all the offset information that has accumulated
    // along the chain of parent variables.

    // TODO: This logic doesn't currently handle semantics or
    // other attributes that might have been present on the
    // original variable layout. That is probably okay for now
    // as the legalization logic does not apply to varying
    // parameters (where resource types would be illegal anyway),
    // but it is probably worth addressing sooner or later.

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
    for (auto rr : typeLayout->getSizeAttrs())
    {
        auto kind = rr->getResourceKind();
        auto resInfo = builder->findOrAddResourceInfo(kind);

        for (auto vv = varChain; vv; vv = vv->next)
        {
            if (auto parentResInfo = vv->varLayout->findOffsetAttr(kind))
            {
                resInfo->offset += parentResInfo->getOffset();
                resInfo->space += parentResInfo->getSpace();
            }
        }
    }

    // As a special case, if the leaf variable doesn't hold an entry for
    // `RegisterSpace`, but at least one declaration in the chain *does*,
    // then we want to make sure that we add such an entry.
    //
    if (!builder->usesResourceKind(LayoutResourceKind::RegisterSpace))
    {
        // Sum up contributions from all parents.
        UInt space = 0;
        for (auto vv = varChain; vv; vv = vv->next)
        {
            if (auto parentResInfo = vv->varLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace))
            {
                space += parentResInfo->getOffset();
            }
        }

        // If there were non-zero contributions, then add an entry to represent them.
        if (space)
        {
            builder->findOrAddResourceInfo(LayoutResourceKind::RegisterSpace)->offset = space;
        }
    }
}

IRVarLayout* createSimpleVarLayout(
    IRBuilder*              irBuilder,
    SimpleLegalVarChain*    varChain,
    IRTypeLayout*           typeLayout)
{
    if (!typeLayout)
        return nullptr;
    IRVarLayout::Builder varLayoutBuilder(irBuilder, typeLayout);
    buildSimpleVarLayout(&varLayoutBuilder, varChain, typeLayout);
    return varLayoutBuilder.build();
}

IRVarLayout* createVarLayout(
    IRBuilder*              irBuilder,
    LegalVarChain const&    varChain,
    IRTypeLayout*           typeLayout)
{
    if(!typeLayout)
        return nullptr;

    IRVarLayout::Builder varLayoutBuilder(irBuilder, typeLayout);
    buildSimpleVarLayout(&varLayoutBuilder, varChain.primaryChain, typeLayout);

    if(auto pendingDataTypeLayout = typeLayout->getPendingDataTypeLayout())
    {
        varLayoutBuilder.setPendingVarLayout(
            createSimpleVarLayout(irBuilder, varChain.pendingChain, typeLayout));
    }

    return varLayoutBuilder.build();
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
