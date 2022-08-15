// slang-lookup.cpp
#include "slang-lookup.h"

#include "../compiler-core/slang-name.h"
#include "slang-check-impl.h"

namespace Slang {

void ensureDecl(SemanticsVisitor* visitor, Decl* decl, DeclCheckState state);

//

DeclRef<ExtensionDecl> ApplyExtensionToType(
    SemanticsVisitor*   semantics,
    ExtensionDecl*          extDecl,
    Type*  type);

//


// Helper for constructing breadcrumb trails during lookup, without unnecessary heap allocaiton
struct BreadcrumbInfo
{
    LookupResultItem::Breadcrumb::Kind kind;
    LookupResultItem::Breadcrumb::ThisParameterMode thisParameterMode = LookupResultItem::Breadcrumb::ThisParameterMode::Default;
    DeclRef<Decl> declRef;
    Val* val = nullptr;
    BreadcrumbInfo* prev = nullptr;
};

//

void buildMemberDictionary(ContainerDecl* decl)
{
    // Don't rebuild if already built
    if (decl->isMemberDictionaryValid())
        return;

    // If it's < 0 it means that the dictionaries are entirely invalid
    if (decl->dictionaryLastCount < 0)
    {
        decl->dictionaryLastCount = 0;
        decl->memberDictionary.Clear();
        decl->transparentMembers.clear();
    }

    // are we a generic?
    GenericDecl* genericDecl = as<GenericDecl>(decl);

    const Index membersCount = decl->members.getCount();

    SLANG_ASSERT(decl->dictionaryLastCount >= 0 && decl->dictionaryLastCount <= membersCount);

    for (Index i = decl->dictionaryLastCount; i < membersCount; ++i)
    {
        Decl* m = decl->members[i];

        auto name = m->getName();

        // Add any transparent members to a separate list for lookup
        if (m->hasModifier<TransparentModifier>())
        {
            TransparentMemberInfo info;
            info.decl = m;
            decl->transparentMembers.add(info);
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

        decl->memberDictionary[name] = m;
    }

    decl->dictionaryLastCount = membersCount;
    SLANG_ASSERT(decl->isMemberDictionaryValid());
}


bool DeclPassesLookupMask(Decl* decl, LookupMask mask)
{
    // type declarations
    if(auto aggTypeDecl = as<AggTypeDecl>(decl))
    {
        return int(mask) & int(LookupMask::type);
    }
    else if(auto simpleTypeDecl = as<SimpleTypeDecl>(decl))
    {
        return int(mask) & int(LookupMask::type);
    }
    // function declarations
    else if(auto funcDecl = as<FunctionDeclBase>(decl))
    {
        return (int(mask) & int(LookupMask::Function)) != 0;
    }
    // attribute declaration
    else if( auto attrDecl = as<AttributeDecl>(decl) )
    {
        return (int(mask) & int(LookupMask::Attribute)) != 0;
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
        result.items.add(result.item);
        result.items.add(item);
    }
    else
    {
        // The result was already overloaded, so we pile on
        result.items.add(item);
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
            bb->val,
            breadcrumbs,
            bb->thisParameterMode);
    }
    item.breadcrumbs = breadcrumbs;
    return item;
}

static void _lookUpMembersInValue(
    ASTBuilder*             astBuilder,
    Name*                   name,
    DeclRef<Decl>           valueDeclRef,
    LookupRequest const&    request,
    LookupResult&	        ioResult,
    BreadcrumbInfo*	        breadcrumbs);

    /// Look up direct members (those declared in `containerDeclRef` itself, as well
    /// as transitively through any direct members that are marked "transparent."
    ///
    /// This function does *not* deal with looking up through `extension`s,
    /// inheritance clauses, etc.
    ///
static void _lookUpDirectAndTransparentMembers(
    ASTBuilder*             astBuilder,
    Name*                   name,
    DeclRef<ContainerDecl>  containerDeclRef,
    LookupRequest const&    request,
    LookupResult&           result,
    BreadcrumbInfo*         inBreadcrumbs)
{
    ContainerDecl* containerDecl = containerDeclRef.getDecl();


    if (request.isCompletionRequest())
    {
        // If we are looking up for completion suggestions,
        // return all the members that are available.
        for (auto member : containerDecl->members)
        {
            if (!DeclPassesLookupMask(member, request.mask))
                continue;
            AddToLookupResult(
                result,
                CreateLookupResultItem(
                    DeclRef<Decl>(member, containerDeclRef.substitutions), inBreadcrumbs));
        }
    }
    else
    {
        // Ensure that the lookup dictionary in the container is up to date
        if (!containerDecl->isMemberDictionaryValid())
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

        _lookUpMembersInValue(
            astBuilder,
            name,
            transparentMemberDeclRef,
            request,
            result,
            &memberRefBreadcrumb);
    }
}

LookupRequest initLookupRequest(
    SemanticsVisitor* semantics,
    Name* name,
    LookupMask mask,
    LookupOptions options,
    Scope* scope)
{
    LookupRequest request;
    request.semantics = semantics;
    request.mask = mask;
    request.options = options;
    request.scope = scope;

    if (semantics && semantics->getSession() &&
        name == semantics->getSession()->getCompletionRequestTokenName())
        request.options = (LookupOptions)((int)request.options | (int)LookupOptions::Completion);

    return request;
}

    /// Perform "direct" lookup in a container declaration
LookupResult lookUpDirectAndTransparentMembers(
    ASTBuilder*             astBuilder,
    SemanticsVisitor*       semantics,
    Name*                   name,
    DeclRef<ContainerDecl>  containerDeclRef,
    LookupMask              mask)
{
    LookupRequest request = initLookupRequest(semantics, name, mask, LookupOptions::None, nullptr);
    LookupResult result;
    _lookUpDirectAndTransparentMembers(
        astBuilder,
        name,
        containerDeclRef,
        request,
        result,
        nullptr);
    return result;
}

static SubtypeWitness* _makeSubtypeWitness(
    ASTBuilder*                 astBuilder,
    Type*                       subType,
    SubtypeWitness*             subToMidWitness,
    Type*                       superType,
    SubtypeWitness*             midtoSuperWitness)
{
    if(subToMidWitness)
    {
        TransitiveSubtypeWitness* transitiveWitness = astBuilder->create<TransitiveSubtypeWitness>();
        transitiveWitness->subToMid = subToMidWitness;
        transitiveWitness->midToSup = midtoSuperWitness;
        transitiveWitness->sub = subType;
        transitiveWitness->sup = superType;
        return transitiveWitness;
    }
    else
    {
        return midtoSuperWitness;
    }
}

static SubtypeWitness* _makeSubtypeWitness(
    ASTBuilder*                 astBuilder,
    Type*                       subType,
    SubtypeWitness*             subToMidWitness,
    Type*                       superType,
    DeclRef<TypeConstraintDecl> midToSuperConstraint)
{
    DeclaredSubtypeWitness* midToSuperWitness = astBuilder->create<DeclaredSubtypeWitness>();
    midToSuperWitness->declRef = midToSuperConstraint;
    midToSuperWitness->sub = subType;
    midToSuperWitness->sup = superType;
    return _makeSubtypeWitness(astBuilder, subType, subToMidWitness, superType, midToSuperWitness);
}

// Same as the above, but we are specializing a type instead of a decl-ref
static Type* _maybeSpecializeSuperType(
    ASTBuilder*                 astBuilder, 
    Type*                       superType,
    SubtypeWitness*             subIsSuperWitness)
{
    if (auto superDeclRefType = as<DeclRefType>(superType))
    {
        if (auto superInterfaceDeclRef = superDeclRefType->declRef.as<InterfaceDecl>())
        {
            ThisTypeSubstitution* thisTypeSubst = astBuilder->create<ThisTypeSubstitution>();
            thisTypeSubst->interfaceDecl = superInterfaceDeclRef.getDecl();
            thisTypeSubst->witness = subIsSuperWitness;
            thisTypeSubst->outer = superInterfaceDeclRef.substitutions.substitutions;

            auto specializedInterfaceDeclRef = DeclRef<Decl>(superInterfaceDeclRef.getDecl(), thisTypeSubst);

            auto specializedInterfaceType = DeclRefType::create(astBuilder, specializedInterfaceDeclRef);
            return specializedInterfaceType;
        }
    }

    return superType;
}

static void _lookUpMembersInType(
    ASTBuilder*             astBuilder, 
    Name*                   name,
    Type*            type,
    LookupRequest const&    request,
    LookupResult&           ioResult,
    BreadcrumbInfo*         breadcrumbs);

static void _lookUpMembersInSuperTypeImpl(
    ASTBuilder*             astBuilder, 
    Name*                   name,
    Type*                   leafType,
    Type*                   superType,
    SubtypeWitness*         leafIsSuperWitness,
    LookupRequest const&    request,
    LookupResult&           ioResult,
    BreadcrumbInfo*         inBreadcrumbs);

static void _lookUpMembersInSuperType(
    ASTBuilder*                 astBuilder,
    Name*                       name,
    Type*                       leafType,
    Type*                       superType,
    SubtypeWitness*             leafIsSuperWitness,
    LookupRequest const&        request,
    LookupResult&               ioResult,
    BreadcrumbInfo*             inBreadcrumbs)
{
    // If we are looking up through an interface type, then
    // we need to be sure that we add an appropriate
    // "this type" substitution here, since that needs to
    // be applied to any members we look up.
    //
    superType = _maybeSpecializeSuperType(
        astBuilder,
        superType,
        leafIsSuperWitness);

    // We need to track the indirection we took in lookup,
    // so that we can construct an appropriate AST on the other
    // side that includes the "upcast" from sub-type to super-type.
    //
    BreadcrumbInfo breadcrumb;
    breadcrumb.prev = inBreadcrumbs;
    breadcrumb.kind = LookupResultItem::Breadcrumb::Kind::SuperType;
    breadcrumb.val = leafIsSuperWitness;
    breadcrumb.prev = inBreadcrumbs;

    // TODO: Need to consider case where this might recurse infinitely (e.g.,
    // if an inheritance clause does something like `Bad<T> : Bad<Bad<T>>`.
    //
    // TODO: The even simpler thing we need to worry about here is that if
    // there is ever a "diamond" relationship in the inheritance hierarchy,
    // we might end up seeing the same interface via different "paths" and
    // we wouldn't want that to lead to overload-resolution failure.
    //
    _lookUpMembersInSuperTypeImpl(astBuilder, name, leafType, superType, leafIsSuperWitness, request, ioResult, &breadcrumb);
}

static void _lookUpMembersInSuperType(
    ASTBuilder*                 astBuilder, 
    Name*                       name,
    Type*                       leafType,
    SubtypeWitness*             leafIsIntermediateWitness,
    DeclRef<TypeConstraintDecl> intermediateIsSuperConstraint,
    LookupRequest const&        request,
    LookupResult&               ioResult,
    BreadcrumbInfo*             inBreadcrumbs)
{
    if( request.semantics )
    {
        ensureDecl(request.semantics, intermediateIsSuperConstraint, DeclCheckState::CanUseBaseOfInheritanceDecl);
    }

    // The super-type in the constraint (e.g., `Foo` in `T : Foo`)
    // will tell us a type we should use for lookup.
    //
    auto superType = getSup(astBuilder, intermediateIsSuperConstraint);
    //
    // We will go ahead and perform lookup using `superType`,
    // after dealing with some details.

    auto leafIsSuperWitness = _makeSubtypeWitness(
        astBuilder,
        leafType,
        leafIsIntermediateWitness,
        superType,
        intermediateIsSuperConstraint);

    return _lookUpMembersInSuperType(astBuilder, name, leafType, superType, leafIsSuperWitness, request, ioResult, inBreadcrumbs);
}

static void _lookUpMembersInSuperTypeDeclImpl(
    ASTBuilder*             astBuilder,
    Name*                   name,
    Type*                   leafType,
    Type*                   superType,
    SubtypeWitness*         leafIsSuperWitness,
    DeclRef<Decl>           declRef,
    LookupRequest const&    request,
    LookupResult&           ioResult,
    BreadcrumbInfo*         inBreadcrumbs)
{
    auto semantics = request.semantics;
    if( semantics )
    {
        ensureDecl(semantics, declRef.getDecl(), DeclCheckState::ReadyForLookup);
    }

    if (auto genericTypeParamDeclRef = declRef.as<GenericTypeParamDecl>())
    {
        // If the type we are doing lookup in is a generic type parameter,
        // then the members it provides can only be discovered by looking
        // at the constraints that are placed on that type.

        auto genericDeclRef = genericTypeParamDeclRef.getParent().as<GenericDecl>();
        assert(genericDeclRef);

        for(auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(genericDeclRef))
        {
            if( semantics )
            {
                ensureDecl(semantics, constraintDeclRef, DeclCheckState::CanUseBaseOfInheritanceDecl);
            }

            // Does this constraint pertain to the type we are working on?
            //
            // We want constraints of the form `T : Foo` where `T` is the
            // generic parameter in question, and `Foo` is whatever we are
            // constraining it to.
            auto subType = getSub(astBuilder, constraintDeclRef);
            auto subDeclRefType = as<DeclRefType>(subType);
            if(!subDeclRefType)
                continue;
            if(!subDeclRefType->declRef.equals(genericTypeParamDeclRef))
                continue;

            _lookUpMembersInSuperType(
                astBuilder,
                name,
                leafType,
                leafIsSuperWitness,
                constraintDeclRef,
                request,
                ioResult,
                inBreadcrumbs);
        }
    }
    else if (declRef.as<AssocTypeDecl>() || declRef.as<GlobalGenericParamDecl>())
    {
        for (auto constraintDeclRef : getMembersOfType<TypeConstraintDecl>(declRef.as<ContainerDecl>()))
        {
            _lookUpMembersInSuperType(
                astBuilder,
                name,
                leafType,
                leafIsSuperWitness,
                constraintDeclRef,
                request,
                ioResult,
                inBreadcrumbs);
        }
    }
    else if(auto aggTypeDeclBaseRef = declRef.as<AggTypeDeclBase>())
    {
        // In this case we are peforming lookup in the context of an aggregate
        // type or an `extension`, so the first thing to do is to look for
        // matching members declared directly in the body of the type/`extension`.
        //
        _lookUpDirectAndTransparentMembers(astBuilder, name, aggTypeDeclBaseRef, request, ioResult, inBreadcrumbs);

        // There are further lookup steps that we can only perform when a
        // semantic checking context is available to us. That means that
        // during parsing, lookup will fail to find members under `name`
        // if they required following these paths.
        //
        if(semantics)
        {
            if(auto aggTypeDeclRef = aggTypeDeclBaseRef.as<AggTypeDecl>())
            {
                // If the declaration we are looking at is a nominal type declaration,
                // then we want to consider any `extension`s that have been associated
                // directly with that type.
                //
                ensureDecl(request.semantics, aggTypeDeclRef.getDecl(), DeclCheckState::ReadyForLookup);
                for(auto extDecl : getCandidateExtensions(aggTypeDeclRef, semantics))
                {
                    // Note: In this case `extDecl` is an extension that was declared to apply
                    // (conditionally) to `aggTypeDeclRef`, which is the decl-ref part of
                    // `superType`. Thus when looking for a substitution to apply to the
                    // extension, we need to apply it to `superType` and not to `leafType`.
                    //
                    auto extDeclRef = ApplyExtensionToType(request.semantics, extDecl, superType);
                    if (!extDeclRef)
                        continue;

                    // TODO: eventually we need to insert a breadcrumb here so that
                    // the constructed result can somehow indicate that a member
                    // was found through an extension.
                    //
                    _lookUpMembersInSuperTypeDeclImpl(
                        astBuilder,
                        name,
                        leafType,
                        superType,
                        leafIsSuperWitness,
                        extDeclRef,
                        request,
                        ioResult,
                        inBreadcrumbs);
                }
            }

            // For both aggregate types and their `extension`s, we want lookup to follow
            // through the declared inheritance relationships on each declaration.
            //
            ensureDecl(semantics, aggTypeDeclBaseRef.getDecl(), DeclCheckState::CanEnumerateBases);
            for (auto inheritanceDeclRef : getMembersOfType<InheritanceDecl>(aggTypeDeclBaseRef))
            {
                ensureDecl(semantics, inheritanceDeclRef.getDecl(), DeclCheckState::CanUseBaseOfInheritanceDecl);

                // Some things that are syntactically `InheritanceDecl`s don't actually
                // represent a subtype/supertype relationship, and thus we shouldn't
                // include members from the base type when doing lookup in the
                // derived type.
                //
                if(inheritanceDeclRef.getDecl()->hasModifier<IgnoreForLookupModifier>())
                    continue;

                auto baseType = getSup(astBuilder, inheritanceDeclRef);
                if( auto baseDeclRefType = as<DeclRefType>(baseType) )
                {
                    if( auto baseInterfaceDeclRef = baseDeclRefType->declRef.as<InterfaceDecl>() )
                    {
                        if( int(request.options) & int(LookupOptions::IgnoreBaseInterfaces) )
                            continue;
                    }
                }

                _lookUpMembersInSuperType(astBuilder, name, leafType, leafIsSuperWitness, inheritanceDeclRef, request, ioResult, inBreadcrumbs);
            }
        }
    }
}

static void _lookUpMembersInSuperTypeImpl(
    ASTBuilder*             astBuilder,
    Name*                   name,
    Type*                   leafType,
    Type*                   superType,
    SubtypeWitness*         leafIsSuperWitness,
    LookupRequest const&    request,
    LookupResult&           ioResult,
    BreadcrumbInfo*         inBreadcrumbs)
{
    // If the type was pointer-like, then dereference it
    // automatically here.
    if (((uint32_t)request.options & (uint32_t)LookupOptions::NoDeref) == 0)
    {
        if (auto pointerElementType = getPointedToTypeIfCanImplicitDeref(superType))
        {
            // Need to leave a breadcrumb to indicate that we
            // did an implicit dereference here
            BreadcrumbInfo derefBreacrumb;
            derefBreacrumb.kind = LookupResultItem::Breadcrumb::Kind::Deref;
            derefBreacrumb.prev = inBreadcrumbs;

            // Recursively perform lookup on the result of deref
            _lookUpMembersInType(
                astBuilder,
                name, pointerElementType, request, ioResult, &derefBreacrumb);
            return;
        }
    }

    // Default case: no dereference needed

    if(auto declRefType = as<DeclRefType>(superType))
    {
        auto declRef = declRefType->declRef;

        _lookUpMembersInSuperTypeDeclImpl(astBuilder, name, leafType, superType, leafIsSuperWitness, declRef, request, ioResult, inBreadcrumbs);
    }
    else if (auto extractExistentialType = as<ExtractExistentialType>(superType))
    {
        // We want lookup to be performed on the underlying interface type of the existential,
        // but we need to have a this-type substitution applied to ensure that the result of
        // lookup will have a comparable substitution applied (allowing things like associated
        // types, etc. used in the signature of a method to resolve correctly).
        //
        auto interfaceDeclRef = extractExistentialType->getSpecializedInterfaceDeclRef();
        _lookUpMembersInSuperTypeDeclImpl(astBuilder, name, leafType, superType, leafIsSuperWitness, interfaceDeclRef, request, ioResult, inBreadcrumbs);
    }
    else if( auto thisType = as<ThisType>(superType) )
    {
        // We need to create a witness that represents the next link in the
        // chain. The `leafIsSuperWitness` represents the knowledge that `leafType : superType`
        // (and we know that `superType == thisType`,  but we now need to extend that
        // with the knowledge that `thisType : thisType->interfaceTypeDeclRef`.
        //
        auto interfaceType = DeclRefType::create(astBuilder, thisType->interfaceDeclRef);

        auto superIsInterfaceWitness = astBuilder->create<ThisTypeSubtypeWitness>();
        superIsInterfaceWitness->sub = superType;
        superIsInterfaceWitness->sup = interfaceType;

        auto leafIsInterfaceWitness = _makeSubtypeWitness(
            astBuilder,
            leafType,
            leafIsSuperWitness,
            interfaceType,
            superIsInterfaceWitness);

        _lookUpMembersInSuperType(astBuilder, name, leafType, interfaceType, leafIsInterfaceWitness, request, ioResult, inBreadcrumbs);
    }
    else if( auto andType = as<AndType>(superType) )
    {
        // We have a type of the form `leftType & rightType` and we need to perform
        // lookup in both `leftType` and `rightType`.
        //
        auto leftType = andType->left;
        auto rightType = andType->right;

        // Operationally, we are in a situation where we have a witness
        // that the `leafType` we are doing lookup on is an subtype
        // of `superType` (which is `leftType & rightType`) and now we need
        // to construct a witness that `leafType` is a subtype of
        // the `Left` type.
        //
        // Effectively, we have a witness that `T : X & Y` and we
        // need to extract from it a witness that `T : X`.
        // Fortunately, we have a class of subtype witness that does
        // *precisely* this:
        //
        auto leafIsLeftWitness = astBuilder->create<ExtractFromConjunctionSubtypeWitness>();
        //
        // Our witness will be to the fact that `leafType` is a subtype of `leftType`
        //
        leafIsLeftWitness->sub = leafType;
        leafIsLeftWitness->sup = leftType;
        //
        // The evidence for the subtype relationship will be a witness
        // proving that `leafType : leftType & rightType`:
        //
        leafIsLeftWitness->conunctionWitness = leafIsSuperWitness;
        //
        // ... along with the index of the desired super-type in
        // that conjunction. The index of `leftType` in `leftType & rightType`
        // is zero.
        //
        leafIsLeftWitness->indexInConjunction = 0;

        // The witness for the fact that `leafType : rightType` is the
        // same as for the left case, just with a different index into
        // the conjunction.
        //
        auto leafIsRightWitness = astBuilder->create<ExtractFromConjunctionSubtypeWitness>();
        leafIsRightWitness->conunctionWitness = leafIsSuperWitness;
        leafIsRightWitness->indexInConjunction = 1;
        leafIsRightWitness->sub = leafType;
        leafIsRightWitness->sup = rightType;

        // We then perform lookup on both sides of the conjunction, and
        // accumulate whatever items are found on either/both sides.
        //
        // For each recursive lookup, we pass the appropriate pair of
        // the type to look up in and the witness of the subtype
        // relationship.
        //
        _lookUpMembersInSuperType(astBuilder, name, leafType, leftType, leafIsLeftWitness, request, ioResult, inBreadcrumbs);
        _lookUpMembersInSuperType(astBuilder, name, leafType, rightType, leafIsRightWitness, request, ioResult, inBreadcrumbs);
    }
}

    /// Perform lookup for `name` in the context of `type`.
    ///
    /// This operation does the kind of lookup we'd expect if `name`
    /// was used inside of a member function on `type`, or if the
    /// user wrote `obj.<name>` for a variable `obj` of the given
    /// `type`.
    ///
    /// Looking up members in `type` includes lookup through any
    /// constraints or inheritance relationships that expand the
    /// set of members visible on `type`.
    ///
static void _lookUpMembersInType(
    ASTBuilder*             astBuilder,
    Name*                   name,
    Type*            type,
    LookupRequest const&    request,
    LookupResult&           ioResult,
    BreadcrumbInfo*         breadcrumbs)
{
    if (!type)
    {
        return;
    }

    _lookUpMembersInSuperTypeImpl(astBuilder, name, type, type, nullptr, request, ioResult, breadcrumbs);
}

    /// Look up members by `name` in the given `valueDeclRef`.
    ///
    /// If `valueDeclRef` represents a reference to a variable
    /// or other named and typed value, then this performs the
    /// kind of lookup we'd expect for `valueDeclRef.<name>`.
    ///
static void _lookUpMembersInValue(
    ASTBuilder*             astBuilder,
    Name*                   name,
    DeclRef<Decl>           valueDeclRef,
    LookupRequest const&    request,
    LookupResult&	        ioResult,
    BreadcrumbInfo*	        breadcrumbs)
{
    // Looking up `name` in the context of a value can
    // be reduced to the problem of looking up `name`
    // in the *type* of that value.
    //
    auto valueType = getTypeForDeclRef(astBuilder, valueDeclRef, SourceLoc());

    return _lookUpMembersInType(astBuilder, name, valueType, request, ioResult, breadcrumbs);
}

// True if the declaration is of an overloadable variety  
// (ie can have multiple definitions with the same name)
// 
// For example functions are overloadable, but variables are (typically) not.
static bool _isDeclOverloadable(Decl* decl)
{
    // If it's a generic strip off, to get to inner decl type
    while (auto genericDecl = as<GenericDecl>(decl))
    {
        decl = genericDecl->inner;
    }

    // TODO(JS): Do we need to special case around ConstructorDecl? or AccessorDecl?
    // It seems not as they are both function-like and potentially overloadable

    // If it's callable, it's a function-like and so overloadable 
    if (auto callableDecl = as<CallableDecl>(decl))
    {
        SLANG_UNUSED(callableDecl);
        return true;
    }

    return false;
}

static void _lookUpInScopes(
    ASTBuilder*             astBuilder,
    Name*                   name,
    LookupRequest const&    request,
    LookupResult&           result)
{
    auto thisParameterMode = LookupResultItem::Breadcrumb::ThisParameterMode::Default;

    auto scope      = request.scope;

    auto endScope   = request.endScope;
    for (;scope != endScope; scope = scope->parent)
    {
        // Note that we consider all "peer" scopes together,
        // so that a hit in one of them does not preclude
        // also finding a hit in another
        for(auto link = scope; link; link = link->nextSibling)
        {
            auto containerDecl = link->containerDecl;

            // It is possible for the first scope in a list of
            // siblings to be a "dummy" scope that only exists
            // to combine the siblings; in that case it will
            // have a null `containerDecl` and needs to be
            // skipped over.
            //
            if(!containerDecl)
                continue;

            // TODO: If we need default substitutions to be applied to
            // the `containerDecl`, then it might make sense to have
            // each `link` in the scope store a decl-ref instead of
            // just a decl.
            //
            DeclRef<ContainerDecl> containerDeclRef =
                DeclRef<Decl>(containerDecl, createDefaultSubstitutions(astBuilder, containerDecl)).as<ContainerDecl>();
            
            // If the container we are looking into represents a type
            // or an `extension` of a type, then we need to treat
            // this step as lookup into the `this` variable (or the
            // `This` type), which means including any `extension`s
            // or inheritance clauses in the lookup process.
            //
            // Note: The `AggTypeDeclBase` class is the common superclass
            // between `AggTypeDecl` and `ExtensionDecl`.
            //
            if (auto aggTypeDeclBaseRef = containerDeclRef.as<AggTypeDeclBase>())
            {
                // When reconstructing the final expression for a result
                // looked up through the type or extension, we will need
                // a `this` expression (or a `This` type expression) to
                // mark the base of the member reference, so we create
                // a "breadcrumb" here to track that fact.
                //
                BreadcrumbInfo breadcrumb;
                breadcrumb.kind = LookupResultItem::Breadcrumb::Kind::This;
                breadcrumb.thisParameterMode = thisParameterMode;
                breadcrumb.declRef = aggTypeDeclBaseRef;
                breadcrumb.prev = nullptr;

                Type* type = nullptr;
                if(auto extDeclRef = aggTypeDeclBaseRef.as<ExtensionDecl>())
                {
                    if( request.semantics )
                    {
                        ensureDecl(request.semantics, extDeclRef.getDecl(), DeclCheckState::CanUseExtensionTargetType);
                    }

                    // If we are doing lookup from inside an `extension`
                    // declaration, then the `this` expression will have
                    // a type that uses the "target type" of the `extension`.
                    //
                    type = getTargetType(astBuilder, extDeclRef);
                }
                else
                {
                    assert(aggTypeDeclBaseRef.as<AggTypeDecl>());
                    type = DeclRefType::create(astBuilder, aggTypeDeclBaseRef);
                }

                _lookUpMembersInType(astBuilder, name, type, request, result, &breadcrumb);
            }
            else
            {
                // The default case is when the scope doesn't represent a
                // type or `extension` declaration, so we can look up members
                // in that scope much more simply.
                //
                _lookUpDirectAndTransparentMembers(astBuilder, name, containerDeclRef, request, result, nullptr);
            }

            // Before we proceed up to the next outer scope to perform lookup
            // again, we need to consider what the current scope tells us
            // about how to interpret uses of implicit `this` or `This`. For
            // example, if we are inside a `[mutating]` method, then the implicit
            // `this` that we use for lookup should be an l-value.
            //
            // Similarly, if we look up a member in a type from the scope
            // of some nested type, then there shouldn't be an implicit `this`
            // expression for the outer type, but instead an implicit `This`.
            //
            if( containerDeclRef.is<ConstructorDecl>() )
            {
                // In the context of an `__init` declaration, the members of
                // the surrounding type are accessible through a mutable `this`.
                //
                thisParameterMode = LookupResultItem::Breadcrumb::ThisParameterMode::MutableValue;
            }
            else if( containerDeclRef.is<SetterDecl>() )
            {
                // In the context of a `set` accessor, the members of the
                // surrounding type are accessible through a mutable `this`.
                //
                // TODO: At some point we may want a way to opt out of this
                // behavior; it is possible to have a setter on a `struct`
                // that actually just sets data into a buffer that is
                // referenced by one of the `struct`'s fields.
                //
                thisParameterMode = LookupResultItem::Breadcrumb::ThisParameterMode::MutableValue;
            }
            else if( auto funcDeclRef = containerDeclRef.as<FunctionDeclBase>() )
            {
                // The implicit `this`/`This` for a function-like declaration
                // depends on modifiers attached to the declaration.
                //
                if( funcDeclRef.getDecl()->hasModifier<HLSLStaticModifier>() )
                {
                    // A `static` method only has access to an implicit `This`,
                    // and does not have a `this` expression available.
                    //
                    thisParameterMode = LookupResultItem::Breadcrumb::ThisParameterMode::Type;
                }
                else if( funcDeclRef.getDecl()->hasModifier<MutatingAttribute>() )
                {
                    // In a non-`static` method marked `[mutating]` there is
                    // an implicit `this` parameter that is mutable.
                    //
                    thisParameterMode = LookupResultItem::Breadcrumb::ThisParameterMode::MutableValue;
                }
                else
                {
                    // In all other cases, there is an implicit `this` parameter
                    // that is immutable.
                    //
                    thisParameterMode = LookupResultItem::Breadcrumb::ThisParameterMode::ImmutableValue;
                }
            }
            else if( containerDeclRef.as<AggTypeDeclBase>() )
            {
                // When lookup moves from a nested typed declaration to an
                // outer scope, there is no ability to use an implicit `this`
                // expression, and we have only the `This` type available.
                //
                thisParameterMode = LookupResultItem::Breadcrumb::ThisParameterMode::Type;
            }
            // TODO: What other cases need to be enumerated here?
        }

        if (result.isValid())
        {
            // If it's overloaded or the decl we have is of an overloadable type, or if we are
            // looking up for completion suggestions then we just keep going
            if (result.isOverloaded() || 
                _isDeclOverloadable(result.item.declRef.getDecl()) ||
                ((int32_t)request.options & (int32_t)LookupOptions::Completion) != 0)
            {
                continue;
            }

            // If we've found a result in this scope (and it's not overloadable), then there
            // is no reason to look further up (for now).
            break;
        }
    }

    // If we run out of scopes, then we are done.
}

LookupResult lookUp(
    ASTBuilder*         astBuilder, 
    SemanticsVisitor*   semantics,
    Name*               name,
    Scope*              scope,
    LookupMask          mask)
{
    LookupRequest request = initLookupRequest(semantics, name, mask, LookupOptions::None, scope);
    LookupResult result;
    _lookUpInScopes(astBuilder, name, request, result);
    return result;
}

LookupResult lookUpMember(
    ASTBuilder*         astBuilder, 
    SemanticsVisitor*   semantics,
    Name*               name,
    Type*               type,
    LookupMask          mask,
    LookupOptions       options)
{
    LookupRequest request = initLookupRequest(semantics, name, mask, options, nullptr);
    LookupResult result;
    _lookUpMembersInType(astBuilder, name, type, request, result, nullptr);
    return result;
}

}
