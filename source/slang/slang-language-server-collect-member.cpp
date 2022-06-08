// slang-language-server-collect-member.cpp

// This file implements the logic to collect all members from a parsed type.]
// The flow is mostly the same as `lookupMemberInType`, but instead of looking for a specific name,
// we collect all members we see.

#include "slang-language-server-collect-member.h"

namespace Slang
{
void collectMembersInType(MemberCollectingContext* context, Type* type)
{
    if (auto pointerLikeType = as<PointerLikeType>(type))
    {
        collectMembersInType(context, pointerLikeType->elementType);
        return;
    }

    if (auto declRefType = as<DeclRefType>(type))
    {
        auto declRef = declRefType->declRef;

        collectMembersInTypeDeclImpl(
            context,
            declRef);
    }
    else if (auto nsType = as<NamespaceType>(type))
    {
        auto declRef = nsType->declRef;

        collectMembersInTypeDeclImpl(context, declRef);
    }
    else if (auto extractExistentialType = as<ExtractExistentialType>(type))
    {
        // We want lookup to be performed on the underlying interface type of the existential,
        // but we need to have a this-type substitution applied to ensure that the result of
        // lookup will have a comparable substitution applied (allowing things like associated
        // types, etc. used in the signature of a method to resolve correctly).
        //
        auto interfaceDeclRef = extractExistentialType->getSpecializedInterfaceDeclRef();
        collectMembersInTypeDeclImpl(context, interfaceDeclRef);
    }
    else if (auto thisType = as<ThisType>(type))
    {
        auto interfaceType = DeclRefType::create(context->astBuilder, thisType->interfaceDeclRef);
        collectMembersInType(context, interfaceType);
    }
    else if (auto andType = as<AndType>(type))
    {
        auto leftType = andType->left;
        auto rightType = andType->right;
        collectMembersInType(context, leftType);
        collectMembersInType(context, rightType);
    }
}

void collectMembersInTypeDeclImpl(
    MemberCollectingContext* context,
    DeclRef<Decl> declRef)
{
    if (declRef.getDecl()->checkState.getState() < DeclCheckState::ReadyForLookup)
        return;

    if (auto genericTypeParamDeclRef = declRef.as<GenericTypeParamDecl>())
    {
        // If the type we are doing lookup in is a generic type parameter,
        // then the members it provides can only be discovered by looking
        // at the constraints that are placed on that type.
        auto genericDeclRef = genericTypeParamDeclRef.getParent().as<GenericDecl>();
        assert(genericDeclRef);

        for (auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(genericDeclRef))
        {
            if (constraintDeclRef.decl->checkState.getState() < DeclCheckState::ReadyForLookup)
            {
                continue;
            }

            collectMembersInType(
                context,
                getSup(context->astBuilder, constraintDeclRef));
        }
    }
    else if (declRef.as<AssocTypeDecl>() || declRef.as<GlobalGenericParamDecl>())
    {
        for (auto constraintDeclRef :
             getMembersOfType<TypeConstraintDecl>(declRef.as<ContainerDecl>()))
        {
            if (constraintDeclRef.decl->checkState.getState() < DeclCheckState::ReadyForLookup)
            {
                continue;
            }
            collectMembersInType(context, getSup(context->astBuilder, constraintDeclRef));
        }
    }
    else if (auto namespaceDecl = declRef.as<NamespaceDecl>())
    {
        for (auto member : namespaceDecl.getDecl()->members)
        {
            if (member->getName())
            {
                context->members.add(member);
            }
        }
    }
    else if (auto aggTypeDeclBaseRef = declRef.as<AggTypeDeclBase>())
    {
        // In this case we are peforming lookup in the context of an aggregate
        // type or an `extension`, so the first thing to do is to look for
        // matching members declared directly in the body of the type/`extension`.
        //
        for (auto member : aggTypeDeclBaseRef.getDecl()->members)
        {
            if (member->getName())
            {
                context->members.add(member);
            }
        }
        
        if (auto aggTypeDeclRef = aggTypeDeclBaseRef.as<AggTypeDecl>())
        {
            auto extensions =
                context->semanticsContext.getCandidateExtensionsForTypeDecl(aggTypeDeclRef);
            for (auto extDecl : extensions)
            {
                // TODO: check if the extension can be applied before including its members.
                // TODO: eventually we need to insert a breadcrumb here so that
                // the constructed result can somehow indicate that a member
                // was found through an extension.
                //
                collectMembersInTypeDeclImpl(
                    context,
                    DeclRef<Decl>(extDecl, nullptr));
            }
        }

        // For both aggregate types and their `extension`s, we want lookup to follow
        // through the declared inheritance relationships on each declaration.
        //
        for (auto inheritanceDeclRef : getMembersOfType<InheritanceDecl>(aggTypeDeclBaseRef))
        {
            // Some things that are syntactically `InheritanceDecl`s don't actually
            // represent a subtype/supertype relationship, and thus we shouldn't
            // include members from the base type when doing lookup in the
            // derived type.
            //
            if (inheritanceDeclRef.getDecl()->hasModifier<IgnoreForLookupModifier>())
                continue;

            collectMembersInType(
                context, getSup(context->astBuilder, inheritanceDeclRef));
        }
    }
}

} // namespace Slang
