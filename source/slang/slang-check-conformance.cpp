// slang-check-conformance.cpp
#include "slang-check-impl.h"

// This file provides semantic checking services related
// to checking and representing the conformance of types
// to interfaces, as well as other subtype relationships.

namespace Slang
{
bool SemanticsVisitor::isInterfaceSafeForTaggedUnion(DeclRef<InterfaceDecl> interfaceDeclRef)
{
    for (auto memberDeclRef : getMembers(m_astBuilder, interfaceDeclRef))
    {
        if (!isInterfaceRequirementSafeForTaggedUnion(interfaceDeclRef, memberDeclRef))
            return false;
    }

    return true;
}

bool SemanticsVisitor::isInterfaceRequirementSafeForTaggedUnion(
    DeclRef<InterfaceDecl> interfaceDeclRef,
    DeclRef<Decl> requirementDeclRef)
{
    SLANG_UNUSED(interfaceDeclRef);

    if (auto callableDeclRef = requirementDeclRef.as<CallableDecl>())
    {
        // A `static` method requirement can't be satisfied by a
        // tagged union, because there is no tag to dispatch on.
        //
        if (requirementDeclRef.getDecl()->hasModifier<HLSLStaticModifier>())
            return false;

        // TODO: We will eventually want to check that any callable
        // requirements do not use the `This` type or any associated
        // types in ways that could lead to errors.
        //
        // For now we are disallowing interfaces that have associated
        // types completely, and we haven't implemented the `This`
        // type, so we should be safe.

        return true;
    }
    else
    {
        return false;
    }
}

SubtypeWitness* SemanticsVisitor::isSubtype(
    Type* subType,
    Type* superType,
    IsSubTypeOptions isSubTypeOptions)
{
    SubtypeWitness* result = nullptr;
    if (getShared()->tryGetSubtypeWitnessFromCache(subType, superType, result))
        return result;
    result = checkAndConstructSubtypeWitness(subType, superType, isSubTypeOptions);

    if (!result && (int(isSubTypeOptions) & int(IsSubTypeOptions::NoCaching)))
        return result;

    getShared()->cacheSubtypeWitness(subType, superType, result);
    return result;
}


Witness* SemanticsVisitor::getDiffTypeInfoWitness(Type* type)
{
    if (auto declRefType = as<DeclRefType>(type))
    {
        if (auto callableDeclRef = declRefType->getDeclRef().as<FunctionDeclBase>())
            return getDiffTypeInfoWitness(callableDeclRef);

        if (auto funcAliasDeclRef = declRefType->getDeclRef().as<FuncAliasDecl>())
        {
            auto targetDeclRef = substituteDeclRef(
                                     SubstitutionSet(funcAliasDeclRef),
                                     getCurrentASTBuilder(),
                                     funcAliasDeclRef.getDecl()->targetDeclRef)
                                     .as<FunctionDeclBase>();
            if (targetDeclRef)
                return getDiffTypeInfoWitness(targetDeclRef);
        }

        auto declRef = declRefType->getDeclRef();
        if (as<GenericTypeParamDeclBase>(declRef.getDecl()))
        {
            if (auto genericDecl = as<GenericDecl>(declRef.getDecl()->parentDecl))
            {
                for (auto constraintDecl :
                     genericDecl->getDirectMemberDeclsOfType<HasDiffTypeInfoConstraintDecl>())
                {
                    auto constraintDeclRef = substituteDeclRef(
                                                 SubstitutionSet(declRef),
                                                 getCurrentASTBuilder(),
                                                 constraintDecl->getDefaultDeclRef())
                                                 .as<HasDiffTypeInfoConstraintDecl>();
                    if (!constraintDeclRef)
                        continue;

                    auto constraintType = getBaseType(getCurrentASTBuilder(), constraintDeclRef);
                    if (constraintType && constraintType->equals(type))
                        return getCurrentASTBuilder()->getHasDiffTypeInfoWitness(constraintDeclRef);
                }
            }
        }
    }

    return nullptr;
}

Witness* SemanticsVisitor::getDiffTypeInfoWitness(DeclRef<FunctionDeclBase> callableDeclRef)
{
    List<SubtypeWitness*> paramWitnesses;

    auto astBuilder = getCurrentASTBuilder();
    FuncType* funcType = nullptr;
    auto rawDirectFuncType = callableDeclRef.getDecl()->funcType.type;
    Type* substitutedDirectFuncType = rawDirectFuncType
                                          ? dynamicCast<Type>(substituteType(
                                                                  SubstitutionSet(callableDeclRef),
                                                                  getCurrentASTBuilder(),
                                                                  rawDirectFuncType)
                                                                  ->resolve())
                                          : nullptr;

    if (auto rawFwdDiffFuncType = as<FwdDiffFuncType>(rawDirectFuncType); rawFwdDiffFuncType)
    {
        if (auto substFwdDiffFuncType = as<FwdDiffFuncType>(substitutedDirectFuncType))
        {
            auto diffTypeWitness =
                as<GenericAppDeclRef>(substFwdDiffFuncType->getDeclRefBase())->getArg(1);
            return astBuilder->getOrCreate<HigherOrderDiffTypeTranslationWitness>(
                as<Witness>(diffTypeWitness));
        }
        else if (auto directFuncType = dynamicCast<FuncType>(substitutedDirectFuncType))
        {
            funcType = directFuncType;
        }
        else
        {
            SLANG_UNEXPECTED("expected FuncType or FwdDiffFuncType after substitution");
            return nullptr;
        }
    }
    else if (auto directFuncType = dynamicCast<FuncType>(substitutedDirectFuncType))
    {
        funcType = directFuncType;
    }
    else
    {
        funcType = as<FuncType>(getFuncType(astBuilder, callableDeclRef));
    }

    if (!funcType)
    {
        SLANG_UNEXPECTED("expected FuncType or FwdDiffFuncType");
        return nullptr;
    }

    // Callable derivative signatures need the same boxed-payload metadata as differential pairs.
    // Ordinary subtype queries intentionally do not provide that metadata for boxes.
    for (auto paramType : funcType->getParamTypes())
    {
        auto paramInfo = getParamInfoFromTypeWithModeWrapper(paramType);
        auto witness = doesTypeHaveNoDiffModifier(paramInfo.type)
                           ? nullptr
                           : isTypeDifferentiable(paramInfo.type);
        paramWitnesses.add(witness);
    }

    SubtypeWitness* returnWitness =
        doesTypeHaveNoDiffModifier(funcType->getResultType()) ||
                callableDeclRef.getDecl()->findModifier<NoDiffModifier>()
            ? nullptr
            : isTypeDifferentiable(funcType->getResultType());

    Type* thisParamType = nullptr;
    SubtypeWitness* thisWitness = nullptr;

    if (auto thisParamInfo = findEffectiveThisParamInfo(callableDeclRef))
    {
        // Keep the witness faithful to the checked primal receiver ABI. Each derivative-function
        // consumer applies its own role-specific transformation to this declaration-derived mode;
        // specialization and the differentiated value type must not reselect the mode.
        thisParamType = getParamTypeWithModeWrapper(astBuilder, *thisParamInfo);
        if (!doesTypeHaveNoDiffModifier(thisParamInfo->type))
            thisWitness = isTypeDifferentiable(thisParamInfo->type);
    }

    return astBuilder->getOrCreate<DiffTypeInfoWitness>(
        thisParamType,
        thisWitness,
        returnWitness,
        paramWitnesses);
}

SubtypeWitness* SemanticsVisitor::checkAndConstructSubtypeWitness(
    Type* subType,
    Type* superType,
    IsSubTypeOptions isSubTypeOptions)
{
    // A box is a proper type that stores a value and its conformance witness. It does not
    // itself conform to the stored interface, and conformance cannot target a box type.
    if (subType->equals(superType))
        return m_astBuilder->getTypeEqualityWitness(subType);
    if (as<ExistentialBoxType>(subType) || as<ExistentialBoxType>(superType))
        return nullptr;

    // This query serves concrete inheritance, interface refinement, and conformance. For
    // `interface IFoo : IBar`, an `IFoo : IBar` witness describes refinement of constraints;
    // it does not make `IFoo` a data type that can satisfy a generic type parameter. Proper-type
    // checking makes that distinction before generic conformance checking. Packing a concrete
    // value into `dyn IFoo` is a conversion, handled separately by the coercion checker.

    SubtypeWitness* failureWitness = nullptr;

    // In the common case, we can use the pre-computed inheritance information for `subType`
    // to enumerate all the types it transitively inherits from.
    //
    auto inheritanceInfo = getShared()->getInheritanceInfo(subType);
    for (auto facet : inheritanceInfo.facets)
    {
        // The `subType` will have a `facet` for each type
        // that it transitively inherits from, as well as
        // for each `extension` that was found to apply to it.
        //
        // For subtype testing, we are only interested in
        // the facets that represent supertypes, and those
        // will be the ones that store a type on the facet.
        //
        auto rawFacetType = facet->getType();
        auto facetType = as<Type>(rawFacetType->resolve());
        if (!facetType)
            continue;

        // We will scan until we find a facet that corresponds
        // to `superType`, or fail to find such a facet.
        //
        if (!facetType->equals(superType))
            continue;

        // If the `superType` appears in the flattened inheritance list
        // for the `subType`, then we know that the subtype relationship
        // holds.

        // If the witness is optional, we should only return it if no certain
        // witness was found.
        auto declWitness = as<DeclaredSubtypeWitness>(facet->subtypeWitness);
        if (declWitness && declWitness->isOptional())
        {
            failureWitness = facet->subtypeWitness;
            continue;
        }

        // Conveniently, the `facet` stores a pre-computed witness for the
        // subtype relationship, which we can use here.
        auto witness = as<SubtypeWitness>(facet->subtypeWitness->resolve());

        // `getInheritanceInfo` for an interface *type* roots its facet witnesses at the
        // interface's `ThisType` (treating the type as a requirement template; see #11469).
        // When the query `subType` is the interface itself, the facet witness's `sub` is the
        // standalone `ThisType`, not the interface we were asked about.
        // We re-root the witness onto the queried `subType` so the returned witness satisfies
        // the invariant `result->getSub() == subType`; this yields a well-formed,
        // interface-rooted witness instead of one anchored at a free-floating `ThisType`.
        if (auto declaredWitness = as<DeclaredSubtypeWitness>(witness))
        {
            if (declaredWitness->getSub() != subType || declaredWitness->getSup() != superType)
                return m_astBuilder->getDeclaredSubtypeWitness(
                    subType,
                    superType,
                    declaredWitness->getDeclRef());
        }
        return witness;
    }

    //
    // TODO: We could expand upon the test using the facet list above
    // by taking the facet lists of both `subType` and `superType`
    // and then checking if all of the facets that appear in `superType`'s
    // linearization also appear in the linearization for `subType`
    // (and occur in the same order).
    //
    // That test could potentially handle certain cases of interface
    // conjunctions that the simpler algorithm above can't, but it wouldn't
    // seem to be a complete algorithm unless we ensured that interfaces
    // have a canonical sorting order for how they appear in linearizations.
    //
    // One of the main reasons why we don't implement such a test right now
    // is that it isn't obvious how to directly produce a witness value
    // as collateral from the test.

    // We expect the logic above to cover the vast majority of subtype
    // tests, but there are a few remaining cases of subtype testing
    // that cannot be folded into the type linearizations above.
    //
    // A few of these cases case if the `superType` is a `DeclRefType`
    // and, if so, want to compare its `DeclRef` against others. As
    // such, we will extract the `DeclRef` here, if it exists,
    // as a convienience.
    //
    DeclRef<Decl> superTypeDeclRef;
    if (auto superDeclRefType = as<DeclRefType>(superType))
    {
        superTypeDeclRef = superDeclRefType->getDeclRef();
    }

    if (as<DynamicType>(subType))
    {
        // A __Dynamic type always conforms to the interface via its witness table.
        auto witness = m_astBuilder->getOrCreate<DynamicSubtypeWitness>(subType, superType);
        return witness;
    }
    else if (as<AndType>(superType))
    {
        // AndType constraints should have been flattened into individual constraints
        // during visitGenericTypeConstraintDecl. If we get here, something is wrong.
        SLANG_UNEXPECTED("AndType should have been flattened before reaching isSubtype");
    }
    else if (auto eachSubType = as<EachType>(subType))
    {
        // `each T : U` is satisfied when every element of `T` satisfies `U`.
        if (auto patternWitness =
                isSubtype(eachSubType->getElementType(), superType, isSubTypeOptions))
        {
            return m_astBuilder->getEachSubtypeWitness(subType, superType, patternWitness);
        }
    }
    else if (auto subTypePack = as<ConcreteTypePack>(subType))
    {
        // An empty type pack vacuously satisfies any element-wise subtype constraint.
        if (subTypePack->getTypeCount() == 0)
        {
            return m_astBuilder->getSubtypeWitnessPack(
                subType,
                superType,
                ArrayView<SubtypeWitness*>());
        }

        List<SubtypeWitness*> elementWitnesses;
        for (Index i = 0; i < subTypePack->getTypeCount(); ++i)
        {
            auto elementWitness =
                isSubtype(subTypePack->getElementType(i), superType, isSubTypeOptions);
            if (!elementWitness)
                return failureWitness;

            elementWitnesses.add(elementWitness);
        }

        return m_astBuilder->getSubtypeWitnessPack(
            subType,
            superType,
            elementWitnesses.getArrayView());
    }
    // default is failure
    return failureWitness;
}

bool SemanticsVisitor::isValidGenericConstraintType(Type* type)
{
    if (auto andType = as<AndType>(type))
    {
        return isValidGenericConstraintType(andType->getLeft()) &&
               isValidGenericConstraintType(andType->getRight());
    }
    return isInterfaceType(type);
}

SubtypeWitness* SemanticsVisitor::isTypeDifferentiable(Type* type)
{
    if (auto valueWitness =
            tryGetDifferentiabilityWitness(type, m_astBuilder->getDiffInterfaceType()))
        return valueWitness;
    else if (
        auto ptrWitness =
            tryGetDifferentiabilityWitness(type, m_astBuilder->getDifferentiableRefInterfaceType()))
        return ptrWitness;

    return nullptr;
}

SubtypeWitness* SemanticsVisitor::tryGetDifferentiabilityWitness(Type* type, Type* interfaceType)
{
    auto isValueDifferentiability = interfaceType->equals(m_astBuilder->getDiffInterfaceType());
    auto isPointerDifferentiability =
        interfaceType->equals(m_astBuilder->getDifferentiableRefInterfaceType());
    SLANG_RELEASE_ASSERT(isValueDifferentiability || isPointerDifferentiability);

    if (auto boxType = as<ExistentialBoxType>(type))
    {
        // Autodiff represents the derivative of `dyn IFoo` by a boxed differential when
        // `IFoo` refines a differentiability interface. We expose that convention through a
        // dedicated witness here; ordinary `isSubtype` still rejects `dyn IFoo : IDifferentiable`.
        auto interfaceWitness = tryGetSubtypeWitness(boxType->getInterfaceType(), interfaceType);
        if (!interfaceWitness)
            return nullptr;
        return m_astBuilder->getExistentialBoxDifferentiabilityWitness(boxType, interfaceWitness);
    }
    return tryGetSubtypeWitness(type, interfaceType);
}

bool SemanticsVisitor::doesTypeHaveTag(Type* type, TypeTag tag)
{
    return (int(getTypeTags(type)) & int(tag)) != 0;
}

/// A context for computing the storage-related properties of a type and its instance fields.
///
/// The `getTags` method inspects checked field signatures rather than cached aggregate tags.
/// Header checking needs these properties to choose storage for uniform parameter shadows,
/// before body checking has accumulated tags on the aggregate declaration.
/// For example, `struct Box<T> { T value; };` has an opaque field in `Box<Texture2D>`, but not
/// in `Box<float>`. Caching tags on the unspecialized declaration cannot describe both cases.
/// Requires `visitor` to identify the semantic-checking context before `getTags` is called.
struct TypeTagContext
{
    SemanticsVisitor* visitor;
    HashSet<Type*> activeTypes;
    Dictionary<Type*, TypeTag> computedTags;
    UInt nestingDepth = 0;
    UInt interruptedInspections = 0;

    /// Compute the tags of `type` with query-local caching and bounded recursive inspection.
    ///
    /// Returns established flags, including `TypeTag::Incomplete` if inspection cannot finish.
    /// The nesting limit counts steps through fields, bases, aliases, and type wrappers.
    TypeTag getTags(Type* type)
    {
        // We need to bound inspection of fields such as `struct Box<T> { Box<Box<T>> next; }`.
        // Substitution creates a distinct type at each step, so cycle detection cannot stop it.
        // We mark the result incomplete at the compiler's nesting limit; ordinary type
        // validation diagnoses the invalid nesting at the declaration that uses the type.
        if (nestingDepth >= kMaxTypeNestingDepth)
        {
            interruptedInspections++;
            return TypeTag::Incomplete;
        }

        // We may encounter many fields with the same instantiated type. We reuse computed
        // tags so each encounter does not require traversing that type's nested structure again.
        TypeTag cachedTags;
        if (computedTags.tryGetValue(type, cachedTags))
            return cachedTags;

        // An active type encountered again indicates a cycle through fields, bases, or wrappers.
        // We return `TypeTag::Incomplete` and leave validity diagnostics to ordinary checking.
        if (!activeTypes.add(type))
        {
            interruptedInspections++;
            return TypeTag::Incomplete;
        }

        auto interruptedInspectionsBefore = interruptedInspections;
        nestingDepth++;
        auto tags = getTagsImpl(type);
        nestingDepth--;
        activeTypes.remove(type);

        // A result from interrupted inspection depends on the current path and remaining depth.
        // We do not cache it: a later, shallower visit may establish additional properties.
        // Fully inspected extern types can still be cached with `TypeTag::Incomplete`, which
        // records that linking may replace their definitions rather than interrupted inspection.
        if (interruptedInspections == interruptedInspectionsBefore)
            computedTags.add(type, tags);
        return tags;
    }

    /// Compute direct type properties and combine properties of instantiated fields and bases.
    ///
    /// Requires `getTags` to have registered `type` in the current recursion path.
    TypeTag getTagsImpl(Type* type)
    {
        // An array has the properties of its elements. Its bound additionally determines
        // whether storage has a known size, a link-time size, or no declared size.
        if (auto arrayType = as<ArrayExpressionType>(type))
        {
            auto tags = getTags(arrayType->getElementType());
            auto elementCount = arrayType->getElementCount();

            // An absent bound describes an unbounded and non-addressable array.
            if (!elementCount)
                return TypeTag(int(tags) | int(TypeTag::Unsized) | int(TypeTag::NonAddressable));

            // Linking or specialization resolves a count that is not yet a `ConstantIntVal`.
            auto constantCount = as<ConstantIntVal>(elementCount);
            if (!constantCount)
                return TypeTag(int(tags) | int(TypeTag::LinkTimeSized));

            // The compiler also represents an unbounded array with an internal sentinel count.
            if (constantCount->getValue() == kUnsizedArrayMagicLength)
                return TypeTag(int(tags) | int(TypeTag::Unsized) | int(TypeTag::NonAddressable));
            return tags;
        }

        // Type modifiers do not change the properties represented by `TypeTag`. We inspect
        // the underlying type so qualifiers such as `no_diff` do not hide a resource field.
        if (auto modifiedType = as<ModifiedType>(type))
            return getTags(modifiedType->getBase());

        // We classify parameter-group representations before inspecting aggregate declarations.
        // `ParameterBlock<T>` is a binding container rather than an addressable value of `T`.
        if (as<ParameterBlockType>(type))
            return TypeTag::NonAddressable;
        if (auto parameterGroupType = as<UniformParameterGroupType>(type))
        {
            // Other parameter groups, such as `ConstantBuffer<T>`, are opaque buffer values.
            // The buffer value has a fixed representation even when `T` has a trailing unsized
            // array. We clear only `TypeTag::Unsized`; the other element properties still apply
            // to validation of the buffer's contents.
            auto tags = getTags(parameterGroupType->getElementType());
            return TypeTag((int(tags) & ~int(TypeTag::Unsized)) | int(TypeTag::Opaque));
        }

        // The compiler's resource type classes describe opaque values directly. Their builtin
        // declarations do not describe the fields that a compiled resource value would store.
        if (as<UntypedBufferResourceType>(type))
            return TypeTag::Opaque;
        if (as<ResourceType>(type))
            return TypeTag::Opaque;

        // Samplers and structured buffers also use dedicated resource representations. Their
        // tags do not depend on the fields of the builtin declarations.
        if (as<SamplerStateType>(type))
            return TypeTag::Opaque;
        if (as<HLSLStructuredBufferTypeBase>(type))
            return TypeTag::Opaque;

        // A dynamic resource has an opaque representation even before specialization chooses
        // a particular resource type.
        if (as<DynamicResourceType>(type))
            return TypeTag::Opaque;

        // For types without an aggregate declaration, no additional flags are currently known.
        // This includes scalar types and generic parameters that have not been substituted.
        // `TypeTag::None` does not certify the validity of their eventual specializations.
        auto declRefType = as<DeclRefType>(type);
        if (!declRefType)
            return TypeTag::None;
        auto aggregateRef = declRefType->getDeclRef().as<AggTypeDecl>();
        if (!aggregateRef)
            return TypeTag::None;

        // A builtin aggregate may use a dedicated compiler representation rather than its
        // declared fields. Its declaration tags remain authoritative for that representation.
        auto aggregate = aggregateRef.getDecl();
        if (aggregate->hasModifier<MagicTypeModifier>())
            return aggregate->typeTags;

        // Header checking resolves a link-time alias's default type. An externally replaceable
        // declaration remains incomplete because the linker can select a different definition.
        visitor->ensureDecl(aggregate, DeclCheckState::ReadyForReference);
        TypeTag tags = TypeTag::None;
        if (aggregate->hasModifier<ExternModifier>())
            tags = TypeTag::Incomplete;
        if (aggregate->aliasedType.type)
        {
            // A link-time alias has a checked default type, such as `Data` in
            // `extern struct Alias : IData = Data;`. We apply the alias reference's
            // substitutions to that semantic type and resolve it before inspecting its fields.
            auto defaultType =
                as<Type>(aggregate->aliasedType.type
                             ->substitute(visitor->getASTBuilder(), SubstitutionSet(aggregateRef))
                             ->resolve());
            SLANG_RELEASE_ASSERT(defaultType);

            // `getTags` establishes properties of the default, but linking can select a different
            // definition. We include `TypeTag::Incomplete` even when the default is fully known.
            return TypeTag(int(getTags(defaultType)) | int(TypeTag::Incomplete));
        }

        // We now inspect instance fields. `getMemberDeclRef` constructs a field reference with
        // the aggregate's substitutions, and `getType` applies them to its checked field type.
        for (auto field : aggregate->getFields())
        {
            if (isEffectivelyStatic(field))
                continue;

            // We access `field` directly rather than through name lookup. `getType` requires
            // its checked signature, but not the redeclaration checks in `ReadyForReference`.
            // Header checking publishes `SignatureChecked` before array-element validation,
            // so we can inspect a recursive field without re-entering that validation.
            visitor->ensureDecl(field, DeclCheckState::SignatureChecked);
            auto fieldRef =
                visitor->getASTBuilder()->getMemberDeclRef(aggregateRef, field).as<VarDeclBase>();
            tags = TypeTag(int(tags) | int(getTags(getType(visitor->getASTBuilder(), fieldRef))));
        }

        // Concrete base types contribute stored fields, while interface conformance does not.
        // `getMemberDeclRef` records the substitutions; `getBaseType` applies them to the base.
        for (auto base : aggregate->getMembersOfType<InheritanceDecl>())
        {
            visitor->ensureDecl(base, DeclCheckState::CanUseBaseOfInheritanceDecl);
            auto baseRef = visitor->getASTBuilder()
                               ->getMemberDeclRef(aggregateRef, base)
                               .as<InheritanceDecl>();
            auto baseType = getBaseType(visitor->getASTBuilder(), baseRef);
            if (isDeclRefTypeOf<InterfaceDecl>(baseType))
                continue;
            tags = TypeTag(int(tags) | int(getTags(baseType)));
        }
        return tags;
    }
};

TypeTag SemanticsVisitor::getTypeTags(Type* type)
{
    // We start a fresh query so tags are computed from the checked type and its substitutions.
    // The cache lasts only for this call and is keyed by instantiated `Type*`, so different
    // specializations of an aggregate have separate results.
    TypeTagContext context;
    context.visitor = this;
    return context.getTags(type);
}


Type* SemanticsVisitor::getConstantBufferElementType(Type* type)
{
    if (auto arrType = as<ArrayExpressionType>(type))
        return getConstantBufferElementType(arrType->getElementType());
    if (auto modifiedType = as<ModifiedType>(type))
        return getConstantBufferElementType(modifiedType->getBase());
    if (auto constantBuffer = as<ConstantBufferType>(type))
        return constantBuffer->getElementType();
    if (auto parameterBlock = as<ParameterBlockType>(type))
        return parameterBlock->getElementType();
    return nullptr;
}


SubtypeWitness* SemanticsVisitor::tryGetInterfaceConformanceWitness(Type* type, Type* interfaceType)
{
    return isSubtype(type, interfaceType, IsSubTypeOptions::None);
}

TypeEqualityWitness* SemanticsVisitor::createTypeEqualityWitness(Type* type)
{
    return m_astBuilder->getTypeEqualityWitness(type);
}
} // namespace Slang
