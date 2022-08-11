// slang-check-conformance.cpp
#include "slang-check-impl.h"

// This file provides semantic checking services related
// to checking and representing the conformance of types
// to interfaces, as well as other subtype relationships.

namespace Slang
{
    DeclaredSubtypeWitness* SemanticsVisitor::createSimpleSubtypeWitness(
        TypeWitnessBreadcrumb*  breadcrumb)
    {
        DeclaredSubtypeWitness* witness = m_astBuilder->create<DeclaredSubtypeWitness>();
        witness->sub = breadcrumb->sub;
        witness->sup = breadcrumb->sup;
        witness->declRef = breadcrumb->declRef;
        return witness;
    }

    Val* SemanticsVisitor::createTypeWitness(
        Type*            subType,
        DeclRef<AggTypeDecl>    superTypeDeclRef,
        TypeWitnessBreadcrumb*  inBreadcrumbs)
    {
        SLANG_UNUSED(subType);
        SLANG_UNUSED(superTypeDeclRef);

        if(!inBreadcrumbs)
        {
            // We need to construct a witness to the fact
            // that `subType` has been proven to be *equal*
            // to `superTypeDeclRef`.
            //
            SLANG_UNEXPECTED("reflexive type witness");
            UNREACHABLE_RETURN(nullptr);
        }

        // We might have one or more steps in the breadcrumb trail, e.g.:
        //
        //      {A : B} {B : C} {C : D}
        //
        // The chain is stored as a reversed linked list, so that
        // the first entry would be the `(C : D)` relationship
        // above.
        //
        // We need to walk the list and build up a suitable witness,
        // which in the above case would look like:
        //
        //      Transitive(
        //          Transitive(
        //              Declared({A : B}),
        //              {B : C}),
        //          {C : D})
        //
        // Because of the ordering of the breadcrumb trail, along
        // with the way the `Transitive` case nests, we will be
        // building these objects outside-in, and keeping
        // track of the "hole" where the next step goes.
        //
        auto bb = inBreadcrumbs;

        // `witness` here will hold the first (outer-most) object
        // we create, which is the overall result.
        SubtypeWitness* witness = nullptr;

        // `link` will point at the remaining "hole" in the
        // data structure, to be filled in.
        SubtypeWitness** link = &witness;

        // As long as there is more than one breadcrumb, we
        // need to be creating transitive witnesses.
        while(bb->prev)
        {
            // On the first iteration when processing the list
            // above, the breadcrumb would be for `{ C : D }`,
            // and so we'd create:
            //
            //      Transitive(
            //          [...],
            //          { C : D})
            //
            // where `[...]` represents the "hole" we leave
            // open to fill in next.
            //
            DeclaredSubtypeWitness* declaredWitness = m_astBuilder->create<DeclaredSubtypeWitness>();
            declaredWitness->sub = bb->sub;
            declaredWitness->sup = bb->sup;
            declaredWitness->declRef = bb->declRef;

            TransitiveSubtypeWitness* transitiveWitness = m_astBuilder->create<TransitiveSubtypeWitness>();
            transitiveWitness->sub = subType;
            transitiveWitness->sup = bb->sup;
            transitiveWitness->midToSup = declaredWitness;

            // Fill in the current hole, and then set the
            // hole to point into the node we just created.
            *link = transitiveWitness;
            link = &transitiveWitness->subToMid;

            // Move on with the list.
            bb = bb->prev;
        }

        // If we exit the loop, then there is only one breadcrumb left.
        // In our running example this would be `{ A : B }`. We create
        // a simple (declared) subtype witness for it, and plug the
        // final hole, after which there shouldn't be a hole to deal with.
        DeclaredSubtypeWitness* declaredWitness = createSimpleSubtypeWitness(bb);
        *link = declaredWitness;

        // We now know that our original `witness` variable has been
        // filled in, and there are no other holes.
        return witness;
    }

    bool SemanticsVisitor::isInterfaceSafeForTaggedUnion(
        DeclRef<InterfaceDecl> interfaceDeclRef)
    {
        for( auto memberDeclRef : getMembers(interfaceDeclRef) )
        {
            if(!isInterfaceRequirementSafeForTaggedUnion(interfaceDeclRef, memberDeclRef))
                return false;
        }

        return true;
    }

    bool SemanticsVisitor::isInterfaceRequirementSafeForTaggedUnion(
        DeclRef<InterfaceDecl>  interfaceDeclRef,
        DeclRef<Decl>           requirementDeclRef)
    {
        SLANG_UNUSED(interfaceDeclRef);

        if(auto callableDeclRef = requirementDeclRef.as<CallableDecl>())
        {
            // A `static` method requirement can't be satisfied by a
            // tagged union, because there is no tag to dispatch on.
            //
            if(requirementDeclRef.getDecl()->hasModifier<HLSLStaticModifier>())
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

    bool SemanticsVisitor::_isDeclaredSubtype(
        Type*            originalSubType,
        Type*            subType,
        DeclRef<AggTypeDecl>    superTypeDeclRef,
        Val**            outWitness,
        TypeWitnessBreadcrumb*  inBreadcrumbs)
    {
        // for now look up a conformance member...
        if(auto declRefType = as<DeclRefType>(subType))
        {
            auto declRef = declRefType->declRef;

            // Easy case: a type conforms to itself.
            //
            // TODO: This is actually a bit more complicated, as
            // the interface needs to be "object-safe" for us to
            // really make this determination...
            if(declRef == superTypeDeclRef)
            {
                if(outWitness)
                {
                    *outWitness = createTypeWitness(originalSubType, superTypeDeclRef, inBreadcrumbs);
                }
                return true;
            }
            if (auto dynamicType = as<DynamicType>(subType))
            {
                // A __Dynamic type always conforms to the interface via its witness table.
                if (outWitness)
                {
                    *outWitness = m_astBuilder->create<DynamicSubtypeWitness>();
                }
                return true;
            }
            else if( auto aggTypeDeclRef = declRef.as<AggTypeDecl>() )
            {
                ensureDecl(aggTypeDeclRef, DeclCheckState::CanEnumerateBases);

                bool found = false;
                foreachDirectOrExtensionMemberOfType<InheritanceDecl>(this, aggTypeDeclRef, [&](DeclRef<InheritanceDecl> const& inheritanceDeclRef)
                {
                    ensureDecl(inheritanceDeclRef, DeclCheckState::CanUseBaseOfInheritanceDecl);

                    // Here we will recursively look up conformance on the type
                    // that is being inherited from. This is dangerous because
                    // it might lead to infinite loops.
                    //
                    // TODO: A better approach would be to create a linearized list
                    // of all the interfaces that a given type directly or indirectly
                    // inherits, and store it with the type, so that we don't have
                    // to recurse in places like this (and can maybe catch infinite
                    // loops better). This would also help avoid checking multiply-inherited
                    // conformances multiple times.

                    auto inheritedType = getBaseType(m_astBuilder, inheritanceDeclRef);


                    // There's one annoying corner case where something that *looks* like an inheritnace
                    // declaration isn't actually one, and that is when an `enum` type includes an explicit
                    // declaration of its "tag type."
                    //
                    if (auto enumDeclRef = declRef.as<EnumDecl>())
                    {
                        if (inheritedType->equals(getTagType(m_astBuilder, enumDeclRef)))
                        {
                            return;
                        }
                    }


                    // We need to ensure that the witness that gets created
                    // is a composite one, reflecting lookup through
                    // the inheritance declaration.
                    TypeWitnessBreadcrumb breadcrumb;
                    breadcrumb.prev = inBreadcrumbs;

                    breadcrumb.sub = subType;
                    breadcrumb.sup = inheritedType;
                    breadcrumb.declRef = inheritanceDeclRef;

                    if(_isDeclaredSubtype(originalSubType, inheritedType, superTypeDeclRef, outWitness, &breadcrumb))
                    {
                        found = true;
                    }
                });
                if(found)
                    return true;

                // if an inheritance decl is not found, try to find a GenericTypeConstraintDecl
                for (auto genConstraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(aggTypeDeclRef))
                {
                    ensureDecl(genConstraintDeclRef, DeclCheckState::CanUseBaseOfInheritanceDecl);
                    auto inheritedType = getSup(m_astBuilder, genConstraintDeclRef);
                    TypeWitnessBreadcrumb breadcrumb;
                    breadcrumb.prev = inBreadcrumbs;
                    breadcrumb.sub = subType;
                    breadcrumb.sup = inheritedType;
                    breadcrumb.declRef = genConstraintDeclRef;
                    if (_isDeclaredSubtype(originalSubType, inheritedType, superTypeDeclRef, outWitness, &breadcrumb))
                    {
                        return true;
                    }
                }
            }
            else if( auto genericTypeParamDeclRef = declRef.as<GenericTypeParamDecl>() )
            {
                // We need to enumerate the constraints placed on this type by its outer
                // generic declaration, and see if any of them guarantees that we
                // satisfy the given interface..
                auto genericDeclRef = genericTypeParamDeclRef.getParent().as<GenericDecl>();
                SLANG_ASSERT(genericDeclRef);

                for( auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(genericDeclRef) )
                {
                    auto sub = getSub(m_astBuilder, constraintDeclRef);
                    auto sup = getSup(m_astBuilder, constraintDeclRef);

                    auto subDeclRef = as<DeclRefType>(sub);
                    if(!subDeclRef)
                        continue;
                    if(subDeclRef->declRef != genericTypeParamDeclRef)
                        continue;

                    // The witness that we create needs to reflect that
                    // it found the needed conformance by lookup through
                    // a generic type constraint.

                    TypeWitnessBreadcrumb breadcrumb;
                    breadcrumb.prev = inBreadcrumbs;
                    breadcrumb.sub = sub;
                    breadcrumb.sup = sup;
                    breadcrumb.declRef = constraintDeclRef;

                    if(_isDeclaredSubtype(originalSubType, sup, superTypeDeclRef, outWitness, &breadcrumb))
                    {
                        return true;
                    }
                }
            }
        }
        else if (auto extractExistentialType = as<ExtractExistentialType>(subType))
        {
            // An ExtractExistentialType from an existential value of type I
            // is a subtype of I.
            // We need to check and make sure the interface type of the `ExtractExistentialType`
            // is equal to `superType`.
            //
            auto interfaceDeclRef = extractExistentialType->originalInterfaceDeclRef;
            if (interfaceDeclRef.equals(superTypeDeclRef))
            {
                if (outWitness)
                {
                    *outWitness = extractExistentialType->getSubtypeWitness();
                }
                return true;
            }
            return false;
        }
        else if(auto taggedUnionType = as<TaggedUnionType>(subType))
        {
            // A tagged union type conforms to an interface if all of
            // the constituent types in the tagged union conform.
            //
            // We will iterate over the "case" types in the tagged
            // union, and check if they conform to the interface.
            // Along the way we will collect the conformance witness
            // values *if* we are being asked to produce a witness
            // value for the tagged union itself (that is, if
            // `outWitness` is non-null).
            //
            List<Val*> caseWitnesses;
            for(auto caseType : taggedUnionType->caseTypes)
            {
                Val* caseWitness = nullptr;

                if(!_isDeclaredSubtype(
                    caseType,
                    caseType,
                    superTypeDeclRef,
                    outWitness ? &caseWitness : nullptr,
                    nullptr))
                {
                    return false;
                }

                if(outWitness)
                {
                    caseWitnesses.add(caseWitness);
                }
            }

            // We also need to validate the requirements on
            // the interface to make sure that they are suitable for
            // use with a tagged-union type.
            //
            // For example, if the interface includes a `static` method
            // (which can therefore be called without a particular instance),
            // then we wouldn't know what implementation of that method
            // to use because there is no tag value to dispatch on.
            //
            // We will start out being conservative about what we accept
            // here, just to keep things simple.
            //
            if( auto superInterfaceDeclRef = superTypeDeclRef.as<InterfaceDecl>() )
            {
                if(!isInterfaceSafeForTaggedUnion(superInterfaceDeclRef))
                    return false;
            }

            // If we reach this point then we have a concrete
            // witness for each of the case types, and that is
            // enough to build a witness for the tagged union.
            //
            if(outWitness)
            {
                TaggedUnionSubtypeWitness* taggedUnionWitness = m_astBuilder->create<TaggedUnionSubtypeWitness>();
                taggedUnionWitness->sub = taggedUnionType;
                taggedUnionWitness->sup = DeclRefType::create(m_astBuilder, superTypeDeclRef);
                taggedUnionWitness->caseWitnesses.swapWith(caseWitnesses);

                *outWitness = taggedUnionWitness;
            }
            return true;
        }
        // default is failure
        return false;
    }

    bool SemanticsVisitor::isDeclaredSubtype(
        Type*            subType,
        DeclRef<AggTypeDecl>    superTypeDeclRef)
    {
        return _isDeclaredSubtype(subType, subType, superTypeDeclRef, nullptr, nullptr);
    }

    bool SemanticsVisitor::isDeclaredSubtype(
        Type* subType,
        Type* superType)
    {
        if (auto declRefType = as<DeclRefType>(superType))
        {
            if (auto aggTypeDeclRef = declRefType->declRef.as<AggTypeDecl>())
                return _isDeclaredSubtype(subType, subType, aggTypeDeclRef, nullptr, nullptr);
        }
        return false;
    }

    bool SemanticsVisitor::isInterfaceType(Type* type)
    {
        if (auto declRefType = as<DeclRefType>(type))
        {
            if (auto interfaceDeclRef = declRefType->declRef.as<InterfaceDecl>())
                return true;
        }
        return false;
    }

    Val* SemanticsVisitor::tryGetSubtypeWitness(
        Type*            subType,
        DeclRef<AggTypeDecl>    superTypeDeclRef)
    {
        Val* result = nullptr;
        _isDeclaredSubtype(subType, subType, superTypeDeclRef, &result, nullptr);
        return result;
    }

    Val* SemanticsVisitor::tryGetInterfaceConformanceWitness(
        Type*            type,
        DeclRef<InterfaceDecl>  interfaceDeclRef)
    {
        return tryGetSubtypeWitness(type, interfaceDeclRef);
    }

    Val* SemanticsVisitor::createTypeEqualityWitness(
        Type*  type)
    {
        TypeEqualityWitness* rs = m_astBuilder->create<TypeEqualityWitness>();
        rs->sub = type;
        rs->sup = type;
        return rs;
    }

    Val* SemanticsVisitor::tryGetSubtypeWitness(
        Type*    sub,
        Type*    sup)
    {
        if(sub->equals(sup))
        {
            // They are the same type, so we just need a witness
            // for type equality.
            return createTypeEqualityWitness(sub);
        }

        if(auto supDeclRefType = as<DeclRefType>(sup))
        {
            auto supDeclRef = supDeclRefType->declRef;
            if(auto supInterfaceDeclRef = supDeclRef.as<InterfaceDecl>())
            {
                if(auto witness = tryGetInterfaceConformanceWitness(sub, supInterfaceDeclRef))
                {
                    return witness;
                }
            }
        }
        else if( auto andType = as<AndType>(sup) )
        {
            // A type `T` is a subtype of `A & B` if `T` is a
            // subtype of `A` and `T` is a subtype of `B`.
            //
            auto leftWitness = tryGetSubtypeWitness(sub, andType->left);
            if(!leftWitness) return nullptr;

            auto rightWitness = tryGetSubtypeWitness(sub, andType->right);
            if(!rightWitness) return nullptr;

            ConjunctionSubtypeWitness* w = m_astBuilder->create<ConjunctionSubtypeWitness>();
            w->leftWitness = leftWitness;
            w->rightWitness = rightWitness;
            w->sub = sub;
            w->sup = sup;
            return w;
        }

        return nullptr;
    }


}
