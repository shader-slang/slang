// slang-check-conformance.cpp
#include "slang-check-impl.h"

// This file provides semantic checking services related
// to checking and representing the conformance of types
// to interfaces, as well as other subtype relationships.

namespace Slang
{
    RefPtr<DeclaredSubtypeWitness> SemanticsVisitor::createSimpleSubtypeWitness(
        TypeWitnessBreadcrumb*  breadcrumb)
    {
        RefPtr<DeclaredSubtypeWitness> witness = new DeclaredSubtypeWitness();
        witness->sub = breadcrumb->sub;
        witness->sup = breadcrumb->sup;
        witness->declRef = breadcrumb->declRef;
        return witness;
    }

    RefPtr<Val> SemanticsVisitor::createTypeWitness(
        RefPtr<Type>            type,
        DeclRef<InterfaceDecl>  interfaceDeclRef,
        TypeWitnessBreadcrumb*  inBreadcrumbs)
    {
        if(!inBreadcrumbs)
        {
            // We need to construct a witness to the fact
            // that `type` has been proven to be *equal*
            // to `interfaceDeclRef`.
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
        RefPtr<SubtypeWitness> witness;

        // `link` will point at the remaining "hole" in the
        // data structure, to be filled in.
        RefPtr<SubtypeWitness>* link = &witness;

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
            RefPtr<TransitiveSubtypeWitness> transitiveWitness = new TransitiveSubtypeWitness();
            transitiveWitness->sub = bb->sub;
            transitiveWitness->sup = bb->sup;
            transitiveWitness->midToSup = bb->declRef;

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
        RefPtr<DeclaredSubtypeWitness> declaredWitness = createSimpleSubtypeWitness(bb);
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

    bool SemanticsVisitor::doesTypeConformToInterfaceImpl(
        RefPtr<Type>            originalType,
        RefPtr<Type>            type,
        DeclRef<InterfaceDecl>  interfaceDeclRef,
        RefPtr<Val>*            outWitness,
        TypeWitnessBreadcrumb*  inBreadcrumbs)
    {
        // for now look up a conformance member...
        if(auto declRefType = as<DeclRefType>(type))
        {
            auto declRef = declRefType->declRef;

            // Easy case: a type conforms to itself.
            //
            // TODO: This is actually a bit more complicated, as
            // the interface needs to be "object-safe" for us to
            // really make this determination...
            if(declRef == interfaceDeclRef)
            {
                if(outWitness)
                {
                    *outWitness = createTypeWitness(originalType, interfaceDeclRef, inBreadcrumbs);
                }
                return true;
            }

            if( auto aggTypeDeclRef = declRef.as<AggTypeDecl>() )
            {
                ensureDecl(aggTypeDeclRef, DeclCheckState::CanEnumerateBases);

                for( auto inheritanceDeclRef : getMembersOfTypeWithExt<InheritanceDecl>(aggTypeDeclRef))
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

                    auto inheritedType = getBaseType(inheritanceDeclRef);

                    // We need to ensure that the witness that gets created
                    // is a composite one, reflecting lookup through
                    // the inheritance declaration.
                    TypeWitnessBreadcrumb breadcrumb;
                    breadcrumb.prev = inBreadcrumbs;

                    breadcrumb.sub = type;
                    breadcrumb.sup = inheritedType;
                    breadcrumb.declRef = inheritanceDeclRef;

                    if(doesTypeConformToInterfaceImpl(originalType, inheritedType, interfaceDeclRef, outWitness, &breadcrumb))
                    {
                        return true;
                    }
                }
                // if an inheritance decl is not found, try to find a GenericTypeConstraintDecl
                for (auto genConstraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(aggTypeDeclRef))
                {
                    ensureDecl(genConstraintDeclRef, DeclCheckState::CanUseBaseOfInheritanceDecl);
                    auto inheritedType = GetSup(genConstraintDeclRef);
                    TypeWitnessBreadcrumb breadcrumb;
                    breadcrumb.prev = inBreadcrumbs;
                    breadcrumb.sub = type;
                    breadcrumb.sup = inheritedType;
                    breadcrumb.declRef = genConstraintDeclRef;
                    if (doesTypeConformToInterfaceImpl(originalType, inheritedType, interfaceDeclRef, outWitness, &breadcrumb))
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
                auto genericDeclRef = genericTypeParamDeclRef.GetParent().as<GenericDecl>();
                SLANG_ASSERT(genericDeclRef);

                for( auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(genericDeclRef) )
                {
                    auto sub = GetSub(constraintDeclRef);
                    auto sup = GetSup(constraintDeclRef);

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

                    if(doesTypeConformToInterfaceImpl(originalType, sup, interfaceDeclRef, outWitness, &breadcrumb))
                    {
                        return true;
                    }
                }
            }
        }
        else if(auto taggedUnionType = as<TaggedUnionType>(type))
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
            List<RefPtr<Val>> caseWitnesses;
            for(auto caseType : taggedUnionType->caseTypes)
            {
                RefPtr<Val> caseWitness;

                if(!doesTypeConformToInterfaceImpl(
                    caseType,
                    caseType,
                    interfaceDeclRef,
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
            if(!isInterfaceSafeForTaggedUnion(interfaceDeclRef))
                return false;

            // If we reach this point then we have a concrete
            // witness for each of the case types, and that is
            // enough to build a witness for the tagged union.
            //
            if(outWitness)
            {
                RefPtr<TaggedUnionSubtypeWitness> taggedUnionWitness = new TaggedUnionSubtypeWitness();
                taggedUnionWitness->sub = taggedUnionType;
                taggedUnionWitness->sup = DeclRefType::Create(getSession(), interfaceDeclRef);
                taggedUnionWitness->caseWitnesses.swapWith(caseWitnesses);

                *outWitness = taggedUnionWitness;
            }
            return true;
        }

        // default is failure
        return false;
    }

    bool SemanticsVisitor::DoesTypeConformToInterface(
        RefPtr<Type>  type,
        DeclRef<InterfaceDecl>        interfaceDeclRef)
    {
        return doesTypeConformToInterfaceImpl(type, type, interfaceDeclRef, nullptr, nullptr);
    }

    RefPtr<Val> SemanticsVisitor::tryGetInterfaceConformanceWitness(
        RefPtr<Type>  type,
        DeclRef<InterfaceDecl>        interfaceDeclRef)
    {
        RefPtr<Val> result;
        doesTypeConformToInterfaceImpl(type, type, interfaceDeclRef, &result, nullptr);
        return result;
    }

    RefPtr<Val> SemanticsVisitor::createTypeEqualityWitness(
        Type*  type)
    {
        RefPtr<TypeEqualityWitness> rs = new TypeEqualityWitness();
        rs->sub = type;
        rs->sup = type;
        return rs;
    }

    RefPtr<Val> SemanticsVisitor::tryGetSubtypeWitness(
        RefPtr<Type>    sub,
        RefPtr<Type>    sup)
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

        return nullptr;
    }


}
