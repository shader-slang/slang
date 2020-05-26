// slang-lookup.cpp
#include "slang-lookup.h"
#include "slang-name.h"

namespace Slang {

void ensureDecl(SemanticsVisitor* visitor, Decl* decl, DeclCheckState state);

//

DeclRef<ExtensionDecl> ApplyExtensionToType(
    SemanticsVisitor*   semantics,
    ExtensionDecl*          extDecl,
    RefPtr<Type>  type);

//


// Helper for constructing breadcrumb trails during lookup, without unnecessary heap allocaiton
struct BreadcrumbInfo
{
    LookupResultItem::Breadcrumb::Kind kind;
    LookupResultItem::Breadcrumb::ThisParameterMode thisParameterMode = LookupResultItem::Breadcrumb::ThisParameterMode::Default;
    DeclRef<Decl> declRef;
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
            breadcrumbs,
            bb->thisParameterMode);
    }
    item.breadcrumbs = breadcrumbs;
    return item;
}

static void _lookUpMembersInValue(
    Session*                session,
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
    Session*                session,
    Name*                   name,
    DeclRef<ContainerDecl>  containerDeclRef,
    LookupRequest const&    request,
    LookupResult&           result,
    BreadcrumbInfo*         inBreadcrumbs)
{
    ContainerDecl* containerDecl = containerDeclRef.getDecl();

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
            session,
            name,
            transparentMemberDeclRef,
            request,
            result,
            &memberRefBreadcrumb);
    }
}

    /// Perform "direct" lookup in a container declaration
LookupResult lookUpDirectAndTransparentMembers(
    Session*                session,
    SemanticsVisitor*       semantics,
    Name*                   name,
    DeclRef<ContainerDecl>  containerDeclRef,
    LookupMask              mask)
{
    LookupRequest request;
    request.semantics = semantics;
    request.mask = mask;
    LookupResult result;
    _lookUpDirectAndTransparentMembers(
        session,
        name,
        containerDeclRef,
        request,
        result,
        nullptr);
    return result;
}


static RefPtr<SubtypeWitness> _makeSubtypeWitness(
    Type*                       subType,
    SubtypeWitness*             subToMidWitness,
    Type*                       superType,
    DeclRef<TypeConstraintDecl> midToSuperConstraint)
{
    if(subToMidWitness)
    {
        RefPtr<TransitiveSubtypeWitness> transitiveWitness = new TransitiveSubtypeWitness();
        transitiveWitness->subToMid = subToMidWitness;
        transitiveWitness->midToSup = midToSuperConstraint;
        transitiveWitness->sub = subType;
        transitiveWitness->sup = superType;
        return transitiveWitness;
    }
    else
    {
        RefPtr<DeclaredSubtypeWitness> declaredWitness = new DeclaredSubtypeWitness();
        declaredWitness->declRef = midToSuperConstraint;
        declaredWitness->sub = subType;
        declaredWitness->sup = superType;
        return declaredWitness;
    }
}

// Same as the above, but we are specializing a type instead of a decl-ref
static RefPtr<Type> _maybeSpecializeSuperType(
    Session*                    session,
    Type*                       superType,
    SubtypeWitness*             subIsSuperWitness)
{
    if (auto superDeclRefType = as<DeclRefType>(superType))
    {
        if (auto superInterfaceDeclRef = superDeclRefType->declRef.as<InterfaceDecl>())
        {
            RefPtr<ThisTypeSubstitution> thisTypeSubst = new ThisTypeSubstitution();
            thisTypeSubst->interfaceDecl = superInterfaceDeclRef.getDecl();
            thisTypeSubst->witness = subIsSuperWitness;
            thisTypeSubst->outer = superInterfaceDeclRef.substitutions.substitutions;

            auto specializedInterfaceDeclRef = DeclRef<Decl>(superInterfaceDeclRef.getDecl(), thisTypeSubst);

            auto specializedInterfaceType = DeclRefType::Create(session, specializedInterfaceDeclRef);
            return specializedInterfaceType;
        }
    }

    return superType;
}

static void _lookUpMembersInType(
    Session*                session,
    Name*                   name,
    RefPtr<Type>            type,
    LookupRequest const&    request,
    LookupResult&           ioResult,
    BreadcrumbInfo*         breadcrumbs);

static void _lookUpMembersInSuperTypeImpl(
    Session*                session,
    Name*                   name,
    Type*                   leafType,
    Type*                   superType,
    SubtypeWitness*         leafIsSuperWitness,
    LookupRequest const&    request,
    LookupResult&           ioResult,
    BreadcrumbInfo*         inBreadcrumbs);


static void _lookUpMembersInSuperType(
    Session*                    session,
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
    auto superType = GetSup(intermediateIsSuperConstraint);
    //
    // We will go ahead and perform lookup using `superType`,
    // after dealing with some details.

    auto leafIsSuperWitness = _makeSubtypeWitness(
        leafType,
        leafIsIntermediateWitness,
        superType,
        intermediateIsSuperConstraint);

    // If we are looking up through an interface type, then
    // we need to be sure that we add an appropriate
    // "this type" substitution here, since that needs to
    // be applied to any members we look up.
    //
    superType = _maybeSpecializeSuperType(
        session,
        superType,
        leafIsSuperWitness);

    // We need to track the indirection we took in lookup,
    // so that we can construct an appropriate AST on the other
    // side that includes the "upcast" from sub-type to super-type.
    //
    BreadcrumbInfo breadcrumb;
    breadcrumb.prev = inBreadcrumbs;
    breadcrumb.kind = LookupResultItem::Breadcrumb::Kind::Constraint;
    breadcrumb.declRef = intermediateIsSuperConstraint;
    breadcrumb.prev = inBreadcrumbs;

    // TODO: Need to consider case where this might recurse infinitely (e.g.,
    // if an inheritance clause does something like `Bad<T> : Bad<Bad<T>>`.
    //
    // TODO: The even simpler thing we need to worry about here is that if
    // there is ever a "diamond" relationship in the inheritance hierarchy,
    // we might end up seeing the same interface via different "paths" and
    // we wouldn't want that to lead to overload-resolution failure.
    //
    _lookUpMembersInSuperTypeImpl(session, name, leafType, superType, leafIsSuperWitness, request, ioResult, &breadcrumb);
}

static void _lookUpMembersInSuperTypeDeclImpl(
    Session*                session,
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

        auto genericDeclRef = genericTypeParamDeclRef.GetParent().as<GenericDecl>();
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
            auto subType = GetSub(constraintDeclRef);
            auto subDeclRefType = as<DeclRefType>(subType);
            if(!subDeclRefType)
                continue;
            if(!subDeclRefType->declRef.equals(genericTypeParamDeclRef))
                continue;

            _lookUpMembersInSuperType(
                session,
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
                session,
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
        _lookUpDirectAndTransparentMembers(session, name, aggTypeDeclBaseRef, request, ioResult, inBreadcrumbs);

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
                for(auto extDecl = GetCandidateExtensions(aggTypeDeclRef); extDecl; extDecl = extDecl->nextCandidateExtension)
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
                        session,
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
                _lookUpMembersInSuperType(session, name, leafType, leafIsSuperWitness, inheritanceDeclRef, request, ioResult, inBreadcrumbs);
            }
        }
    }
}

static void _lookUpMembersInSuperTypeImpl(
    Session*                session,
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
    if (auto pointerLikeType = as<PointerLikeType>(superType))
    {
        // Need to leave a breadcrumb to indicate that we
        // did an implicit dereference here
        BreadcrumbInfo derefBreacrumb;
        derefBreacrumb.kind = LookupResultItem::Breadcrumb::Kind::Deref;
        derefBreacrumb.prev = inBreadcrumbs;

        // Recursively perform lookup on the result of deref
        _lookUpMembersInType(
            session,
            name, pointerLikeType->elementType, request, ioResult, &derefBreacrumb);
        return;
    }

    // Default case: no dereference needed

    if(auto declRefType = as<DeclRefType>(superType))
    {
        auto declRef = declRefType->declRef;

        _lookUpMembersInSuperTypeDeclImpl(session, name, leafType, superType, leafIsSuperWitness, declRef, request, ioResult, inBreadcrumbs);
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
    Session*                session,
    Name*                   name,
    RefPtr<Type>            type,
    LookupRequest const&    request,
    LookupResult&           ioResult,
    BreadcrumbInfo*         breadcrumbs)
{
    if (!type)
    {
        return;
    }

    _lookUpMembersInSuperTypeImpl(session, name, type, type, nullptr, request, ioResult, breadcrumbs);
}

    /// Look up members by `name` in the given `valueDeclRef`.
    ///
    /// If `valueDeclRef` represents a reference to a variable
    /// or other named and typed value, then this performs the
    /// kind of lookup we'd expect for `valueDeclRef.<name>`.
    ///
static void _lookUpMembersInValue(
    Session*                session,
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
    auto valueType = getTypeForDeclRef(
        session,
        valueDeclRef,
        SourceLoc());
    return _lookUpMembersInType(
        session,
        name, valueType, request, ioResult, breadcrumbs);
}

static void _lookUpInScopes(
    Session*                session,
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
                DeclRef<Decl>(containerDecl, createDefaultSubstitutions(session, containerDecl)).as<ContainerDecl>();
            
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

                RefPtr<Type> type;
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
                    type = GetTargetType(extDeclRef);
                }
                else
                {
                    assert(aggTypeDeclBaseRef.as<AggTypeDecl>());
                    type = DeclRefType::Create(session, aggTypeDeclBaseRef);
                }

                _lookUpMembersInType(session, name, type, request, result, &breadcrumb);
            }
            else
            {
                // The default case is when the scope doesn't represent a
                // type or `extension` declaration, so we can look up members
                // in that scope much more simply.
                //
                _lookUpDirectAndTransparentMembers(session, name, containerDeclRef, request, result, nullptr);
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
            // If we've found a result in this scope, then there
            // is no reason to look further up (for now).
            return;
        }
    }

    // If we run out of scopes, then we are done.
}

LookupResult lookUp(
    Session*            session,
    SemanticsVisitor*   semantics,
    Name*               name,
    RefPtr<Scope>       scope,
    LookupMask          mask)
{
    LookupRequest request;
    request.semantics = semantics;
    request.scope = scope;
    request.mask = mask;

    LookupResult result;
    _lookUpInScopes(session, name, request, result);
    return result;
}

void lookUpMemberImpl(
    Session*            session,
    SemanticsVisitor*   semantics,
    Name*               name,
    Type*               type,
    LookupResult&       ioResult,
    BreadcrumbInfo*     inBreadcrumbs,
    LookupMask          mask);

LookupResult lookUpMember(
    Session*            session,
    SemanticsVisitor*   semantics,
    Name*               name,
    Type*               type,
    LookupMask          mask)
{
    LookupRequest request;
    request.semantics = semantics;
    request.mask = mask;

    LookupResult result;
    _lookUpMembersInType(session, name, type, request, result, nullptr);
    return result;
}

}
