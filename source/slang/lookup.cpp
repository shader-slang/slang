// lookup.cpp
#include "lookup.h"

namespace Slang {

//

DeclRef<ExtensionDecl> ApplyExtensionToType(
    SemanticsVisitor*       semantics,
    ExtensionDecl*          extDecl,
    RefPtr<Type>  type);

//


// Helper for constructing breadcrumb trails during lookup, without unnecessary heap allocaiton
struct BreadcrumbInfo
{
    LookupResultItem::Breadcrumb::Kind kind;
    DeclRef<Decl> declRef;
    BreadcrumbInfo* prev = nullptr;
};

void DoLocalLookupImpl(
    Session*                session,
    Name*                   name,
    DeclRef<ContainerDecl>  containerDeclRef,
    LookupRequest const&    request,
    LookupResult&		    result,
    BreadcrumbInfo*		    inBreadcrumbs);

//

void buildMemberDictionary(ContainerDecl* decl)
{
    // Don't rebuild if already built
    if (decl->memberDictionaryIsValid)
        return;

    decl->memberDictionary.Clear();
    decl->transparentMembers.Clear();

    // are we a generic?
    GenericDecl* genericDecl = dynamic_cast<GenericDecl*>(decl);

    for (auto m : decl->Members)
    {
        auto name = m->getName();

        // Add any transparent members to a separate list for lookup
        if (m->HasModifier<TransparentModifier>())
        {
            TransparentMemberInfo info;
            info.decl = m.Ptr();
            decl->transparentMembers.Add(info);
        }

        // Ignore members with no name
        if (!name)
            continue;

        // Ignore the "inner" member of a generic declaration
        if (genericDecl && m == genericDecl->inner)
            continue;


        m->nextInContainerWithSameName = nullptr;

        Decl* next = nullptr;
        if (decl->memberDictionary.TryGetValue(name, next))
            m->nextInContainerWithSameName = next;

        decl->memberDictionary[name] = m.Ptr();

    }
    decl->memberDictionaryIsValid = true;
}


bool DeclPassesLookupMask(Decl* decl, LookupMask mask)
{
    // type declarations
    if(auto aggTypeDecl = dynamic_cast<AggTypeDecl*>(decl))
    {
        return int(mask) & int(LookupMask::type);
    }
    else if(auto simpleTypeDecl = dynamic_cast<SimpleTypeDecl*>(decl))
    {
        return int(mask) & int(LookupMask::type);
    }
    // function declarations
    else if(auto funcDecl = dynamic_cast<FunctionDeclBase*>(decl))
    {
        return (int(mask) & int(LookupMask::Function)) != 0;
    }

    // default behavior is to assume a value declaration
    // (no overloading allowed)

    return (int(mask) & int(LookupMask::Value)) != 0;
}

void AddToLookupResult(
    LookupResult&		result,
    LookupResultItem	item)
{
    if (!result.isValid())
    {
        // If we hadn't found a hit before, we have one now
        result.item = item;
    }
    else if (!result.isOverloaded())
    {
        // We are about to make this overloaded
        result.items.Add(result.item);
        result.items.Add(item);
    }
    else
    {
        // The result was already overloaded, so we pile on
        result.items.Add(item);
    }
}

LookupResult refineLookup(LookupResult const& inResult, LookupMask mask)
{
    if (!inResult.isValid()) return inResult;
    if (!inResult.isOverloaded()) return inResult;

    LookupResult result;
    for (auto item : inResult.items)
    {
        if (!DeclPassesLookupMask(item.declRef.getDecl(), mask))
            continue;

        AddToLookupResult(result, item);
    }
    return result;
}

LookupResultItem CreateLookupResultItem(
    DeclRef<Decl> declRef,
    BreadcrumbInfo* breadcrumbInfos)
{
    LookupResultItem item;
    item.declRef = declRef;

    // breadcrumbs were constructed "backwards" on the stack, so we
    // reverse them here by building a linked list the other way
    RefPtr<LookupResultItem::Breadcrumb> breadcrumbs;
    for (auto bb = breadcrumbInfos; bb; bb = bb->prev)
    {
        breadcrumbs = new LookupResultItem::Breadcrumb(
            bb->kind,
            bb->declRef,
            breadcrumbs);
    }
    item.breadcrumbs = breadcrumbs;
    return item;
}

void DoMemberLookupImpl(
    Session*                session,
    Name*                   name,
    RefPtr<Type>            baseType,
    LookupRequest const&    request,
    LookupResult&           ioResult,
    BreadcrumbInfo*         breadcrumbs)
{
    // If the type was pointer-like, then dereference it
    // automatically here.
    if (auto pointerLikeType = baseType->As<PointerLikeType>())
    {
        // Need to leave a breadcrumb to indicate that we
        // did an implicit dereference here
        BreadcrumbInfo derefBreacrumb;
        derefBreacrumb.kind = LookupResultItem::Breadcrumb::Kind::Deref;
        derefBreacrumb.prev = breadcrumbs;

        // Recursively perform lookup on the result of deref
        return DoMemberLookupImpl(
            session,
            name, pointerLikeType->elementType, request, ioResult, &derefBreacrumb);
    }

    // Default case: no dereference needed

    if (auto baseDeclRefType = baseType->As<DeclRefType>())
    {
        if (auto baseAggTypeDeclRef = baseDeclRefType->declRef.As<AggTypeDecl>())
        {
            DoLocalLookupImpl(
                session,
                name, baseAggTypeDeclRef, request, ioResult, breadcrumbs);
        }
    }

    // TODO(tfoley): any other cases to handle here?
}

void DoMemberLookupImpl(
    Session*                session,
    Name*                   name,
    DeclRef<Decl>           baseDeclRef,
    LookupRequest const&    request,
    LookupResult&	        ioResult,
    BreadcrumbInfo*	        breadcrumbs)
{
    auto baseType = getTypeForDeclRef(
        session,
        baseDeclRef);
    return DoMemberLookupImpl(
        session,
        name, baseType, request, ioResult, breadcrumbs);
}

// Look for members of the given name in the given container for declarations
void DoLocalLookupImpl(
    Session*                session,
    Name*                   name,
    DeclRef<ContainerDecl>	containerDeclRef,
    LookupRequest const&    request,
    LookupResult&		    result,
    BreadcrumbInfo*		    inBreadcrumbs)
{
    ContainerDecl* containerDecl = containerDeclRef.getDecl();

    // Ensure that the lookup dictionary in the container is up to date
    if (!containerDecl->memberDictionaryIsValid)
    {
        buildMemberDictionary(containerDecl);
    }

    // Look up the declarations with the chosen name in the container.
    Decl* firstDecl = nullptr;
    containerDecl->memberDictionary.TryGetValue(name, firstDecl);

    // Now iterate over those declarations (if any) and see if
    // we find any that meet our filtering criteria.
    // For example, we might be filtering so that we only consider
    // type declarations.
    for (auto m = firstDecl; m; m = m->nextInContainerWithSameName)
    {
        if (!DeclPassesLookupMask(m, request.mask))
            continue;

        // The declaration passed the test, so add it!
        AddToLookupResult(result, CreateLookupResultItem(DeclRef<Decl>(m, containerDeclRef.substitutions), inBreadcrumbs));
    }


    // TODO(tfoley): should we look up in the transparent decls
    // if we already has a hit in the current container?

    for(auto transparentInfo : containerDecl->transparentMembers)
    {
        // The reference to the transparent member should use whatever
        // substitutions we used in referring to its outer container
        DeclRef<Decl> transparentMemberDeclRef(transparentInfo.decl, containerDeclRef.substitutions);

        // We need to leave a breadcrumb so that we know that the result
        // of lookup involves a member lookup step here

        BreadcrumbInfo memberRefBreadcrumb;
        memberRefBreadcrumb.kind = LookupResultItem::Breadcrumb::Kind::Member;
        memberRefBreadcrumb.declRef = transparentMemberDeclRef;
        memberRefBreadcrumb.prev = inBreadcrumbs;

        DoMemberLookupImpl(
            session,
            name,
            transparentMemberDeclRef,
            request,
            result,
            &memberRefBreadcrumb);
    }

    // Consider lookup via extension
    if( auto aggTypeDeclRef = containerDeclRef.As<AggTypeDecl>() )
    {
        RefPtr<Type> type = DeclRefType::Create(
            session,
            aggTypeDeclRef);

        for (auto ext = GetCandidateExtensions(aggTypeDeclRef); ext; ext = ext->nextCandidateExtension)
        {
            auto extDeclRef = ApplyExtensionToType(request.semantics, ext, type);
            if (!extDeclRef)
                continue;

            // TODO: eventually we need to insert a breadcrumb here so that
            // the constructed result can somehow indicate that a member
            // was found through an extension.

            DoLocalLookupImpl(
                session,
                name, extDeclRef, request, result, inBreadcrumbs);
        }
    }
}

void DoLookupImpl(
    Session*                session,
    Name*                   name,
    LookupRequest const&    request,
    LookupResult&           result)
{
    auto scope      = request.scope;
    auto endScope   = request.endScope;
    for (;scope != endScope; scope = scope->parent)
    {
        // Note that we consider all "peer" scopes together,
        // so that a hit in one of them does not proclude
        // also finding a hit in another
        for(auto link = scope; link; link = link->nextSibling)
        {
            auto containerDecl = link->containerDecl;

            if(!containerDecl)
                continue;

            DeclRef<ContainerDecl> containerDeclRef =
                DeclRef<Decl>(containerDecl, nullptr).As<ContainerDecl>();

            BreadcrumbInfo breadcrumb;
            BreadcrumbInfo* breadcrumbs = nullptr;

            // Depending on the kind of container we are looking into,
            // we may need to insert something like a `this` expression
            // to resolve the lookup result.
            //
            // Note: We are checking for `AggTypeDeclBase` here, and not
            // just `AggTypeDecl`, because we want to catch `extension`
            // declarations as well.
            //
            if (auto aggTypeDeclRef = containerDeclRef.As<AggTypeDeclBase>())
            {
                breadcrumb.kind = LookupResultItem::Breadcrumb::Kind::This;
                breadcrumb.declRef = aggTypeDeclRef;
                breadcrumb.prev = nullptr;

                breadcrumbs = &breadcrumb;
            }

            // Now perform "local" lookup in the context of the container,
            // as if we were looking up a member directly.
            //
            DoLocalLookupImpl(
                session,
                name, containerDeclRef, request, result, breadcrumbs);
        }

        if (result.isValid())
        {
            // If we've found a result in this scope, then there
            // is no reason to look further up (for now).
            return;
        }
    }

    // If we run out of scopes, then we are done.
}

LookupResult DoLookup(
    Session*                session,
    Name*                   name,
    LookupRequest const&    request)
{
    LookupResult result;
    DoLookupImpl(session, name, request, result);
    return result;
}

LookupResult lookUp(
    Session*            session,
    SemanticsVisitor*   semantics,
    Name*               name,
    RefPtr<Scope>       scope)
{
    LookupRequest request;
    request.semantics = semantics;
    request.scope = scope;
    return DoLookup(session, name, request);
}

// perform lookup within the context of a particular container declaration,
// and do *not* look further up the chain
LookupResult lookUpLocal(
    Session*                session,
    SemanticsVisitor*       semantics,
    Name*                   name,
    DeclRef<ContainerDecl>  containerDeclRef)
{
    LookupRequest request;
    request.semantics = semantics;

    LookupResult result;
    DoLocalLookupImpl(session, name, containerDeclRef, request, result, nullptr);
    return result;
}

void lookUpMemberImpl(
    Session* session,
    SemanticsVisitor*   semantics,
    Name*               name,
    Type*               type,
    LookupResult&       ioResult,
    BreadcrumbInfo*     inBreadcrumbs)
{
    if (auto declRefType = type->As<DeclRefType>())
    {
        auto declRef = declRefType->declRef;
        if (auto aggTypeDeclRef = declRef.As<AggTypeDecl>())
        {
            LookupRequest request;
            request.semantics = semantics;

            DoLocalLookupImpl(session, name, aggTypeDeclRef, request, ioResult, inBreadcrumbs);
        }
        else if (auto genericTypeParamDeclRef = declRef.As<GenericTypeParamDecl>())
        {
            auto genericDeclRef = genericTypeParamDeclRef.GetParent().As<GenericDecl>();
            assert(genericDeclRef);

            for(auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(genericDeclRef))
            {
                // Does this constraint pertain to the type we are working on?
                //
                // We want constraints of the form `T : Foo` where `T` is the
                // generic parameter in question, and `Foo` is whatever we are
                // constraining it to.
                auto subType = GetSub(constraintDeclRef);
                auto subDeclRefType = subType->As<DeclRefType>();
                if(!subDeclRefType)
                    continue;
                if(!subDeclRefType->declRef.Equals(genericTypeParamDeclRef))
                    continue;

                // The super-type in the constraint (e.g., `Foo` in `T : Foo`)
                // will tell us a type we should use for lookup.
                auto bound = GetSup(constraintDeclRef);

                // Go ahead and use the target type, with an appropriate breadcrumb
                // to indicate that we indirected through a type constraint.

                BreadcrumbInfo breadcrumb;
                breadcrumb.prev = inBreadcrumbs;
                breadcrumb.kind = LookupResultItem::Breadcrumb::Kind::Constraint;
                breadcrumb.declRef = constraintDeclRef;

                // TODO: Need to consider case where this might recurse infinitely.
                lookUpMemberImpl(session, semantics, name, bound, ioResult, &breadcrumb);
            }
        }
    }
    else if (auto assocTypeDeclRefType = type->As<AssocTypeDeclRefType>())
    {
        auto assocTypeDeclRef = assocTypeDeclRefType->declRef;
        for (auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(assocTypeDeclRef))
        {
            // The super-type in the constraint (e.g., `Foo` in `T : Foo`)
            // will tell us a type we should use for lookup.
            auto bound = GetSup(constraintDeclRef);

            // Go ahead and use the target type, with an appropriate breadcrumb
            // to indicate that we indirected through a type constraint.

            BreadcrumbInfo breadcrumb;
            breadcrumb.prev = inBreadcrumbs;
            breadcrumb.kind = LookupResultItem::Breadcrumb::Kind::Constraint;
            breadcrumb.declRef = constraintDeclRef;

            // TODO: Need to consider case where this might recurse infinitely.
            lookUpMemberImpl(session, semantics, name, bound, ioResult, &breadcrumb);
        }
    }
}

LookupResult lookUpMember(
    Session*            session,
    SemanticsVisitor*   semantics,
    Name*               name,
    Type*               type)
{
    LookupResult result;
    lookUpMemberImpl(session, semantics, name, type, result, nullptr);
    return result;
}


}
