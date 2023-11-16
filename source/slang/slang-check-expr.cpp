// slang-check-expr.cpp
#include "slang-check-impl.h"

// This file contains semantic-checking logic for the various
// expression types in the AST.
//
// Note that some cases of expression checking are split
// of into their own files. Notably:
//
// * `slang-check-overload.cpp` is responsible for the logic of resolving overloaded calls
//
// * `slang-check-conversion.cpp` is responsible for the logic of handling type conversion/coercion

#include "slang-ast-natural-layout.h"

#include "slang-lookup.h"
#include "slang-lookup-spirv.h"
#include "slang-ast-print.h"

namespace Slang
{
    DeclRefType* SemanticsVisitor::getExprDeclRefType(Expr * expr)
    {
        if (auto typetype = as<TypeType>(expr->type))
            return dynamicCast<DeclRefType>(typetype->getType());
        else
            return as<DeclRefType>(expr->type);
    }

    void SemanticsContext::ExprLocalScope::addBinding(LetExpr* binding)
    {
        if (!m_innerMostBinding)
        {
            SLANG_ASSERT(!m_outerMostBinding);

            // If we haven't added any bindings, then `binding`
            // becomes both the inner-most and outer most.
            //
            m_innerMostBinding = binding;
            m_outerMostBinding = binding;
        }
        else
        {
            SLANG_ASSERT(m_outerMostBinding);

            // If we already have bindings, then `binding`
            // will become the new inner-most binding.
            //
            m_innerMostBinding->body = binding;
            m_innerMostBinding = binding;
        }
    }


        /// Move `expr` into a temporary variable and execute `func` on that variable.
        ///
        /// Returns an expression that wraps both the creation and initialization of
        /// the temporary, and the computation created by `func`.
        ///
    template<typename F>
    Expr* SemanticsVisitor::moveTemp(Expr* const& expr, F const& func)
    {
        VarDecl* varDecl = m_astBuilder->create<VarDecl>();
        varDecl->parentDecl = nullptr; // TODO: need to fill this in somehow!
        varDecl->checkState = DeclCheckState::Checked;
        varDecl->nameAndLoc.loc = expr->loc;
        varDecl->initExpr = expr;
        varDecl->type.type = expr->type.type;

        auto varDeclRef = makeDeclRef(varDecl);

        LetExpr* letExpr = m_astBuilder->create<LetExpr>();
        letExpr->decl = varDecl;

        auto body = func(varDeclRef);
        Expr* result = body;
        if (auto exprLocalScope = getExprLocalScope())
        {
            // We want to add the `LetExpr` to the set of such expressions
            // in the local scope, so that it can be emitted properly.
            //
            exprLocalScope->addBinding(letExpr);
        }
        else
        {
            // If we somehow got in here and there wasn't an expression-local
            // scope established yet, it almost certainly represents an error.
            //
            SLANG_ASSERT(exprLocalScope);

            // As a fallback, though, we will try to wire up the `letExpr`
            // to surround the body directly and return that.
            //
            letExpr->body = body;
            letExpr->type = body->type;

            result = letExpr;
        }
        return result;
    }

        /// Execute `func` on a variable with the value of `expr`.
        ///
        /// If `expr` is just a reference to an immutable (e.g., `let`) variable
        /// then this might use the existing variable. Otherwise it will create
        /// a new variable to hold `expr`, using `moveTemp()`.
        ///
    template<typename F>
    Expr* SemanticsVisitor::maybeMoveTemp(Expr* const& expr, F const& func)
    {
        // TODO: Eventually this operation could consider any case where the
        // input `expr` names an immutable "path": one that starts at an
        // immutable binding and follows a (possibly empty) chain of accesses
        // to immutable members.

        if(auto varExpr = as<VarExpr>(expr))
        {
            auto declRef = varExpr->declRef;
            if(auto varDeclRef = declRef.as<LetDecl>())
                return func(varDeclRef);
        }

        return moveTemp(expr, func);
    }

        /// Return an expression that represents "opening" the existential `expr`.
        ///
        /// The type of `expr` must be an interface type, matching `interfaceDeclRef`.
        ///
        /// If we scope down the PL theory to just the case that Slang cares about,
        /// a value of an existential type like `IMover` is a tuple of:
        ///
        ///  * a concrete type `X`
        ///  * a witness `w` of the fact that `X` implements `IMover`
        ///  * a value `v` of type `X`
        ///
        /// "Opening" an existential value is the process of decomposing a single
        /// value `e : IMover` into the pieces `X`, `w`, and `v`.
        ///
        /// Rather than return all those pieces individually, this operation
        /// returns an expression that logically corresponds to `v`: an expression
        /// of type `X`, where the type carries the knowledge that `X` implements `IMover`.
        ///
    Expr* SemanticsVisitor::openExistential(
        Expr*            expr,
        DeclRef<InterfaceDecl>  interfaceDeclRef)
    {
        // If `expr` refers to an immutable binding,
        // then we can use it directly. If it refers
        // to an arbitrary expression or a mutable
        // binding, we will move its value into an
        // immutable temporary so that we can use
        // it directly.
        //
        return maybeMoveTemp(expr, [&](DeclRef<VarDeclBase> varDeclRef)
        {
            ExtractExistentialType* openedType = m_astBuilder->getOrCreate<ExtractExistentialType>(
                varDeclRef, expr->type.type, interfaceDeclRef);

            ExtractExistentialValueExpr* openedValue = m_astBuilder->create<ExtractExistentialValueExpr>();
            openedValue->declRef = varDeclRef;
            openedValue->type = QualType(openedType);
            openedValue->originalExpr = expr;

            // The result of opening an existential is an l-value
            // if the original existential is an l-value.
            //
            if(expr->type.isLeftValue)
            {
                // Marking the opened value as an l-value is the easy part.
                //
                openedValue->type.isLeftValue = true;

                // The more challenging bit is that in this case the `maybeMoveTemp()`
                // operation will have copied the original existential value into
                // a temporary.
                //
                // If this expression is used in an l-value context, then we need
                // to be able to generate code to "write back" the modified value
                // (which will be of `openedType`) to the original location named
                // by `expr` (an existential for `interfaceDeclRef`).
                //
            }

            return openedValue;
        });
    }

        /// If `expr` has existential type, then open it.
        ///
        /// Returns an expression that opens `expr` if it had existential type.
        /// Otherwise returns `expr` itself.
        ///
        /// See `openExistential` for a discussion of what "opening" an
        /// existential-type value means.
        ///
    Expr* SemanticsVisitor::maybeOpenExistential(Expr* expr)
    {
        auto exprType = expr->type.type;

        if(auto declRefType = as<DeclRefType>(exprType))
        {
            if(auto interfaceDeclRef = declRefType->getDeclRef().as<InterfaceDecl>())
            {
                return openExistential(expr, interfaceDeclRef);
            }
        }

        // Default: apply the callback to the original expression;
        return expr;
    }

    Expr* SemanticsVisitor::maybeOpenRef(Expr* expr)
    {
        auto exprType = expr->type.type;

        if (auto refType = as<RefTypeBase>(exprType))
        {
            auto openRef = m_astBuilder->create<OpenRefExpr>();
            openRef->innerExpr = expr;
            openRef->type.isLeftValue = (as<RefType>(exprType) != nullptr);
            openRef->type.type = refType->getValueType();
            return openRef;
        }
        return expr;
    }

    static SourceLoc _getMemberOpLoc(Expr* expr)
    {
        if (auto m = as<MemberExpr>(expr))
            return m->memberOperatorLoc;
        if (auto m = as<StaticMemberExpr>(expr))
            return m->memberOperatorLoc;
        return SourceLoc();
    }

    void SemanticsVisitor::diagnoseDeprecatedDeclRefUsage(
        DeclRef<Decl> declRef,
        SourceLoc loc,
        Expr* originalExpr)
    {
        // This is slightly subtle, because we don't want to warn more than
        // once for the same occurrence, however in some cases this function is
        // called more than once for the same declref (specifically in the case
        // of a non-overloaded function, once when the function is identified at
        // first, and again when it's checked from
        // CheckInvokeExprWithCheckedOperands).
        //
        // The correct fix is probably to make
        // CheckInvokeExprWithCheckedOperands reuse the original declref,
        // however that doesn't appear to be a simple change.
        //
        // What we do instead is see if there's already been a declRef
        // constructed for this expression and rest assured that it's already
        // had a diagnostic emitted.
        auto originalAppExpr = as<AppExprBase>(originalExpr);
        auto originalAppFunDecl = originalAppExpr ? as<DeclRefExpr>(originalAppExpr->functionExpr) : nullptr;
        if(originalAppFunDecl && originalAppFunDecl->declRef)
        {
            return;
        }
        if (auto deprecatedAttr = declRef.getDecl()->findModifier<DeprecatedAttribute>())
        {
            getSink()->diagnose(
                loc,
                Diagnostics::deprecatedUsage,
                declRef.getName(),
                deprecatedAttr->message);
        }
    }

    static bool isMutableGLSLBufferBlockVarExpr(Expr* expr)
    {
        if(const auto varExpr = as<VarExpr>(expr))
        {
            if(const auto declRefType = as<DeclRefType>(varExpr->type->getCanonicalType()))
            {
                if(declRefType->getDeclRef().getDecl()->isDerivedFrom(ASTNodeType::GLSLInterfaceBlockDecl))
                {
                    const auto d = varExpr->declRef.getDecl();
                    if(d->hasModifier<GLSLBufferModifier>() && !d->hasModifier<GLSLReadOnlyModifier>())
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    DeclRefExpr* SemanticsVisitor::ConstructDeclRefExpr(
        DeclRef<Decl>   declRef,
        Expr*    baseExpr,
        SourceLoc loc,
        Expr*    originalExpr)
    {
        // Compute the type that this declaration reference will have in context.
        //
        auto type = GetTypeForDeclRef(declRef, loc);

        // This is the bottleneck for using declarations which might be
        // deprecated, diagnose here.
        diagnoseDeprecatedDeclRefUsage(declRef, loc, originalExpr);

        // Construct an appropriate expression based on the structured of
        // the declaration reference.
        //
        if (baseExpr)
        {
            // If there was a base expression, we will have some kind of
            // member expression.

            // We want to check for the case where the base "expression"
            // actually names a type, because in that case we are doing
            // a static member reference.
            //
            if (auto typeType = as<TypeType>(baseExpr->type->getCanonicalType()))
            {
                // Before forming the reference, we will check if the
                // member being referenced can even be used as a static
                // member, and if not we will diagnose an error.
                //
                // TODO: It is conceptually possible to allow static
                // references to many instance members, provided we
                // change the exposed type/signature.
                //
                // E.g., if we have:
                //
                //      struct Test { float getVal() { ... } }
                //
                // Then a reference to `Test.getVal` could be allowed,
                // and given a type of `(Test) -> float` to indicate
                // that it is an "unbound" instance method.
                //
                if( !isDeclUsableAsStaticMember(declRef.getDecl()) )
                {
                    getSink()->diagnose(
                        loc,
                        Diagnostics::staticRefToNonStaticMember,
                        typeType->getType(),
                        declRef.getName());
                }

                auto expr = m_astBuilder->create<StaticMemberExpr>();
                expr->loc = loc;
                expr->type = type;
                expr->baseExpression = baseExpr;
                expr->name = declRef.getName();
                expr->declRef = declRef;
                expr->memberOperatorLoc = _getMemberOpLoc(originalExpr);
                return expr;
            }
            else if(isEffectivelyStatic(declRef.getDecl()))
            {
                // Extract the type of the baseExpr
                auto baseExprType = baseExpr->type.type;
                SharedTypeExpr* baseTypeExpr = m_astBuilder->create<SharedTypeExpr>();
                baseTypeExpr->base.type = baseExprType;
                baseTypeExpr->type.type = m_astBuilder->getTypeType(baseExprType);

                auto expr = m_astBuilder->create<StaticMemberExpr>();
                expr->loc = loc;
                expr->type = type;
                expr->baseExpression = baseTypeExpr;
                expr->name = declRef.getName();
                expr->declRef = declRef;
                expr->memberOperatorLoc = _getMemberOpLoc(originalExpr);
                return expr;
            }
            else
            {
                // If the base expression wasn't a type, then this
                // is a normal member expression.
                //
                auto expr = m_astBuilder->create<MemberExpr>();
                expr->loc = loc;
                expr->type = type;
                expr->baseExpression = baseExpr;
                expr->name = declRef.getName();
                expr->declRef = declRef;
                expr->memberOperatorLoc = _getMemberOpLoc(originalExpr);

                // When referring to a member through an expression,
                // the result is only an l-value if both the base
                // expression and the member agree that it should be.
                //
                // We have already used the `QualType` from the member
                // above (that is `type`), so we need to take the
                // l-value status of the base expression into account now.
                if(!baseExpr->type.isLeftValue)
                {
                    // One exception to this is if we're reading the contents
                    // of a GLSL buffer interface block which isn't marked as
                    // read_only
                    expr->type.isLeftValue = isMutableGLSLBufferBlockVarExpr(baseExpr);
                }
                else
                {
                    // If we are accessing a readonly property, then the result
                    // is not an l-value.
                    if (auto propertyDecl = as<PropertyDecl>(declRef.getDecl()))
                    {
                        bool isLValue = false;
                        for (auto member : propertyDecl->members)
                        {
                            if (as<SetterDecl>(member) || as< RefAccessorDecl>(member))
                            {
                                isLValue = true;
                                break;
                            }
                        }
                        expr->type.isLeftValue = isLValue;
                    }
                }
                return expr;
            }
        }
        else
        {
            // If there is no base expression, then the result must
            // be an ordinary variable expression.
            //
            auto expr = m_astBuilder->create<VarExpr>();
            expr->loc = loc;
            expr->name = declRef.getName();
            expr->type = type;
            expr->declRef = declRef;
            // Keep a reference to the original expr if it was a genericApp/member.
            // This is needed by the language server to locate the original tokens.
            if (as<GenericAppExpr>(originalExpr) || as<MemberExpr>(originalExpr) || as<StaticMemberExpr>(originalExpr))
            {
                expr->originalExpr = originalExpr;
            }
            return expr;
        }
    }

    Expr* SemanticsVisitor::ConstructDerefExpr(
        Expr*    base,
        SourceLoc       loc)
    {
        auto elementType = getPointedToTypeIfCanImplicitDeref(base->type);
        SLANG_ASSERT(elementType);

        auto derefExpr = m_astBuilder->create<DerefExpr>();
        derefExpr->loc = loc;
        derefExpr->base = base;
        derefExpr->type = QualType(elementType);

        derefExpr->type.isLeftValue = base->type.isLeftValue;

        return derefExpr;
    }

    InvokeExpr* SemanticsVisitor::constructUncheckedInvokeExpr(Expr* callee, const List<Expr*>& arguments)
    {
        auto result = m_astBuilder->create<InvokeExpr>();
        result->loc = callee->loc;
        result->functionExpr = callee;
        result->arguments.addRange(arguments);
        return result;
    }

    Expr* SemanticsVisitor::maybeUseSynthesizedDeclForLookupResult(
        LookupResultItem const& item,
        Expr* originalExpr)
    {
        // If the only result from lookup is an entry in an interface decl, it could be that
        // the user is leaving out an explicit definition for the requirement and depending on
        // the compiler to synthesis the definition.
        // In this case, if the lookup is triggered from a location such that the satisfying
        // definition should be returned should it existed, we should create a placeholder decl for
        // the definition and return a reference to to newly created decl instead of the requirement
        // decl in the interface.
        switch (item.declRef.getDecl()->astNodeType)
        {
        case ASTNodeType::AssocTypeDecl:
            break;
        case ASTNodeType::FuncDecl:
            // We don't need to intercept lookup results with synthesized decls for methods,
            // because function lookups will only take place when we are checking the decl bodies.
            // At that point conformance check and synthesis is already done so they will always resolve
            // to the synthesized method.
            return nullptr;
        default:
            return nullptr;
        }

        // We need to check if the lookup should resolve to a definition in an implementation type
        // if it existed.
        // This will be the case when the lookup is initiated from the concrete implementation type instead of
        // directly from the Interface decl. The breadcrumbs of the lookup should provide this information.

        // If no breadcrumbs existed, then the lookup should just resolve to the interface requirement.

        if (!item.breadcrumbs)
            return nullptr;

        // We will only ever need to synthesis a type to satisfy an associatedtype requirement.
        // In this case the lookup should have resolved to a known associatedtype decl.
        auto builtinAssocTypeAttr = item.declRef.getDecl()->findModifier<BuiltinRequirementModifier>();
        if (!builtinAssocTypeAttr)
            return nullptr;

        DeclRefType* subType = nullptr;

        // Check if we are reaching the associated type decl through inheritance from a concrete type.
        for (auto breadcrumb = item.breadcrumbs; breadcrumb; breadcrumb = breadcrumb->next)
        {
            switch (breadcrumb->kind)
            {
            case LookupResultItem::Breadcrumb::Kind::SuperType:
            {
                auto witness = as<SubtypeWitness>(breadcrumb->val);
                if (auto subDeclRefType = as<DeclRefType>(witness->getSub()))
                {
                    if (!as<InterfaceDecl>(subDeclRefType->getDeclRef().getDecl()))
                    {
                        // Store the inner most concrete super type.
                        subType = subDeclRefType;
                    }
                }
            }
            break;
            default:
                break;
            }
        }
        if (!subType)
            return nullptr;

        subType = as<DeclRefType>(subType->getCanonicalType());
        if (!subType)
            return nullptr;

        // Don't synthesize for generic parameters.
        auto parent = as<AggTypeDecl>(subType->getDeclRef().getDecl());
        if (!parent)
            return nullptr;

        // Don't synthesize for ThisType.
        if (as<ThisTypeDecl>(subType->getDeclRef().getDecl()))
            return nullptr;
        
        // If the inner most subtype is itself an associated type, then we're dealing
        // with an abstract type. There's not need to synthesize anythin at this point.
        // 
        if (as<AssocTypeDecl>(subType->getDeclRef().getDecl()))
            return nullptr;

        // If we reach here, we are expecting a synthesized decl defined in `subType`.
        // Instead of returning a DeclRefExpr to the requirement decl, we synthesize a placeholder decl
        // in `subType` and return a DeclRefExpr to the synthesized decl.

        Decl* synthesizedDecl = nullptr;
        switch (builtinAssocTypeAttr->kind)
        {
        case BuiltinRequirementKind::DifferentialType:
            {
                auto structDecl = m_astBuilder->create<StructDecl>();
                auto conformanceDecl = m_astBuilder->create<InheritanceDecl>();
                conformanceDecl->base.type = m_astBuilder->getDiffInterfaceType();
                conformanceDecl->parentDecl = structDecl;
                structDecl->members.add(conformanceDecl);
                structDecl->parentDecl = parent;

                synthesizedDecl = structDecl;
                auto typeDef = m_astBuilder->create<TypeAliasDecl>();
                typeDef->nameAndLoc.name = getName("Differential");
                typeDef->parentDecl = structDecl;

                auto synthDeclRef = createDefaultSubstitutionsIfNeeded(m_astBuilder, this, makeDeclRef(structDecl));

                typeDef->type.type = DeclRefType::create(m_astBuilder, synthDeclRef);
                structDecl->members.add(typeDef);
            }
            break;
        default:
            return nullptr;
        }
        synthesizedDecl->parentDecl = parent;
        synthesizedDecl->nameAndLoc.name = item.declRef.getName();
        synthesizedDecl->loc = parent->loc;
        parent->members.add(synthesizedDecl);
        parent->invalidateMemberDictionary();

        // Mark the newly synthesized decl as `ToBeSynthesized` so future checking can differentiate it
        // from user-provided definitions, and proceed to fill in its definition.
        auto toBeSynthesized = m_astBuilder->create<ToBeSynthesizedModifier>();
        addModifier(synthesizedDecl, toBeSynthesized);

        auto synthDeclMemberRef = m_astBuilder->getMemberDeclRef(subType->getDeclRef(), synthesizedDecl);
        return ConstructDeclRefExpr(
            synthDeclMemberRef,
            nullptr,
            originalExpr ? originalExpr->loc : SourceLoc(),
            originalExpr);
    }

    Expr* SemanticsVisitor::ConstructLookupResultExpr(
        LookupResultItem const& item,
        Expr*            baseExpr,
        SourceLoc loc,
        Expr* originalExpr)
    {
        if (!item.declRef)
        {
            originalExpr->type = QualType(m_astBuilder->getErrorType());
            return originalExpr;
        }

        // We could be referencing a decl that will be synthesized. If so create a placeholder
        // and return a DeclRefExpr to it.
        if (auto lookupResultExpr = maybeUseSynthesizedDeclForLookupResult(item, originalExpr))
            return lookupResultExpr;

        // If we collected any breadcrumbs, then these represent
        // additional segments of the lookup path that we need
        // to expand here.
        auto bb = baseExpr;
        for (auto breadcrumb = item.breadcrumbs; breadcrumb; breadcrumb = breadcrumb->next)
        {
            switch (breadcrumb->kind)
            {
            case LookupResultItem::Breadcrumb::Kind::Member:
                bb = ConstructDeclRefExpr(breadcrumb->declRef, bb, loc, originalExpr);
                break;

            case LookupResultItem::Breadcrumb::Kind::Deref:
                bb = ConstructDerefExpr(bb, loc);
                break;

            case LookupResultItem::Breadcrumb::Kind::SuperType:
                {
                    // Note: a lookup through a super-type can
                    // occur even in the case of a `static` member,
                    // so we only modify the base expression here
                    // if there is one.
                    //
                    if( bb )
                    {
                        // We know that the breadcrumb reprsents a
                        // cast of the base expression to a super type,
                        // so we construct that cast explicitly here.
                        //
                        auto witness = as<SubtypeWitness>(breadcrumb->val);
                        SLANG_ASSERT(witness);
                        auto expr = createCastToSuperTypeExpr(witness->getSup(), bb, witness);

                        // Note that we allow a cast of an l-value to
                        // be used as an l-value here because it enables
                        // `[mutating]` methods to be called, and
                        // mutable properties to be modified, but this
                        // is probably not *technically* correct, since
                        // treating an l-value of type `Derived` as
                        // an l-value of type `Base` implies that we
                        // can assign an arbitrary value of type `Base`
                        // to that l-value (which would be an error).
                        //
                        // TODO: make sure we believe there are no
                        // issues here.
                        //
                        if(bb && bb->type.isLeftValue)
                        {
                            expr->type.isLeftValue = true;
                        }

                        bb = expr;
                    }
                }
                break;

            case LookupResultItem::Breadcrumb::Kind::This:
                {
                    // We expect a `this` to always come
                    // at the start of a chain.
                    SLANG_ASSERT(bb == nullptr);

                    // We will compute the type to use for `This` using
                    // the same logic that a direct reference to `This`
                    // uses.
                    //
                    auto thisType = calcThisType(breadcrumb->declRef);

                    // Next we construct an appropriate expression to
                    // stand in for the implicit `this` or `This` reference.
                    //
                    // The lookup process will have computed the appropriate
                    // "mode" to use for the implicit `this` or `This`.
                    //
                    auto thisParameterMode = breadcrumb->thisParameterMode;
                    if(thisParameterMode == LookupResultItem::Breadcrumb::ThisParameterMode::Type)
                    {
                        // If we are in a static context, then we do not
                        // have implicit `this` expression, and the expression
                        // we construct will need to start with the `This`
                        // type.
                        //
                        // Because we are constrained to yield an expression
                        // here, we must construct an expression that
                        // references `This`, and the *type* of that expression
                        // will be `typeof(This)`, which conceptually
                        // `typeof(typeof(this))`
                        //
                        auto thisTypeType = m_astBuilder->getTypeType(thisType);

                        auto typeExpr = m_astBuilder->create<SharedTypeExpr>();
                        typeExpr->type.type = thisTypeType;
                        typeExpr->base.type = thisType;

                        bb = typeExpr;
                    }
                    else
                    {
                        // In a context where both static and instance members can
                        // be referenced, we will construct a reference to `this`,
                        // and then rely on downstream logic to ensure that a
                        // refernece to `this.someStaticMember` will be translated
                        // over to `This.someStaticMember`.
                        //
                        ThisExpr* expr = m_astBuilder->create<ThisExpr>();
                        expr->type.type = thisType;
                        expr->loc = loc;
                        if (auto declRefExpr = as<DeclRefExpr>(originalExpr))
                            expr->scope = declRefExpr->scope;
                        else if (auto invokeExpr = as<InvokeExpr>(originalExpr))
                        {
                            if (auto calleeDeclRefExpr = as<DeclRefExpr>(invokeExpr->originalFunctionExpr))
                                expr->scope = calleeDeclRefExpr->scope;
                        }
                        // Whether or not the implicit `this` is mutable depends
                        // on the context in which it is used, and the lookup
                        // logic will have computed an appropriate "mode" based
                        // on the context during lookup.
                        //
                        expr->type.isLeftValue = thisParameterMode == LookupResultItem::Breadcrumb::ThisParameterMode::MutableValue;

                        bb = expr;
                    }
                }
                break;

            default:
                SLANG_UNREACHABLE("all cases handle");
            }
            if (getShared()->isInLanguageServer())
            {
                // Don't make breadcrumb nodes carry any source loc info,
                // as they may confuse language server functionalities.
                if (bb)
                {
                    bb->loc = SourceLoc();
                }
            }
        }

        return ConstructDeclRefExpr(item.declRef, bb, loc, originalExpr);
    }

    void SemanticsVisitor::suggestCompletionItems(
        CompletionSuggestions::ScopeKind scopeKind, LookupResult const& lookupResult)
    {
        auto& suggestions = getLinkage()->contentAssistInfo.completionSuggestions;
        suggestions.clear();
        suggestions.scopeKind = scopeKind;
        for (auto item : lookupResult)
        {
            suggestions.candidateItems.add(item);
        }
    }


    Expr* SemanticsVisitor::createLookupResultExpr(
        Name*                   name,
        LookupResult const&     lookupResult,
        Expr*            baseExpr,
        SourceLoc loc,
        Expr* originalExpr)
    {
        if (lookupResult.isOverloaded())
        {
            auto overloadedExpr = m_astBuilder->create<OverloadedExpr>();
            overloadedExpr->name = name;
            overloadedExpr->loc = loc;
            overloadedExpr->type = QualType(
                m_astBuilder->getOverloadedType());
            overloadedExpr->base = baseExpr;
            overloadedExpr->lookupResult2 = lookupResult;
            return overloadedExpr;
        }
        else
        {
            return ConstructLookupResultExpr(lookupResult.item, baseExpr, loc, originalExpr);
        }
    }

    LookupResult SemanticsVisitor::resolveOverloadedLookup(LookupResult const& inResult)
    {
        // If the result isn't actually overloaded, it is fine as-is
        if (!inResult.isValid()) return inResult;
        if (!inResult.isOverloaded()) return inResult;

        // We are going to build up a list of items to return.
        List<LookupResultItem> items;
        for( auto item : inResult.items )
        {
            // For each item we consider adding, we will compare it
            // to those items we've already added.
            //
            // If any of the existing items is "better" than `item`,
            // then we will skip adding `item`.
            //
            // If `item` is "better" than any of the existing items,
            // we will remove those from `items`.
            //
            bool shouldAdd = true;
            for( Index ii = 0; ii < items.getCount(); ++ii )
            {
                int cmp = CompareLookupResultItems(item, items[ii]);
                if( cmp < 0 )
                {
                    // The new `item` is strictly better
                    items.fastRemoveAt(ii);
                    --ii;
                }
                else if( cmp > 0 )
                {
                    // The existing item is strictly better
                    shouldAdd = false;
                }
            }
            if( shouldAdd )
            {
                items.add(item);
            }
        }

        // The resulting `items` list should be all those items
        // that were neither better nor worse than one another.
        //
        // There should always be at least one such item.
        //
        SLANG_ASSERT(items.getCount() != 0);

        LookupResult result;
        for( auto item : items )
        {
            AddToLookupResult(result, item);
        }
        return result;
    }

    void SemanticsVisitor::diagnoseAmbiguousReference(OverloadedExpr* overloadedExpr, LookupResult const& lookupResult)
    {
        getSink()->diagnose(overloadedExpr, Diagnostics::ambiguousReference, lookupResult.items[0].declRef.getName());

        for(auto item : lookupResult.items)
        {
            String declString = ASTPrinter::getDeclSignatureString(item, m_astBuilder);
            getSink()->diagnose(item.declRef, Diagnostics::overloadCandidate, declString);
        }
    }

    void SemanticsVisitor::diagnoseAmbiguousReference(Expr* expr)
    {
        if( auto overloadedExpr = as<OverloadedExpr>(expr) )
        {
            diagnoseAmbiguousReference(overloadedExpr, overloadedExpr->lookupResult2);
        }
        else
        {
            getSink()->diagnose(expr, Diagnostics::ambiguousExpression);
        }
    }

    Expr* SemanticsVisitor::_resolveOverloadedExprImpl(OverloadedExpr* overloadedExpr, LookupMask mask, DiagnosticSink* diagSink)
    {
        auto lookupResult = overloadedExpr->lookupResult2;
        SLANG_RELEASE_ASSERT(lookupResult.isValid() && lookupResult.isOverloaded());

        // Take the lookup result we had, and refine it based on what is expected in context.
        //
        // E.g., if there is both a type and a variable named `Foo`, but in context we know
        // that a type is expected, then we can disambiguate by assuming the type is intended.
        //
        lookupResult = refineLookup(lookupResult, mask);

        // Try to filter out overload candidates based on which ones are "better" than one another.
        lookupResult = resolveOverloadedLookup(lookupResult);

        if (!lookupResult.isValid())
        {
            // If we didn't find any symbols after filtering, then just
            // use the original and report errors that way
            return overloadedExpr;
        }

        if(!lookupResult.isOverloaded())
        {
            // If there is only a single item left in the lookup result,
            // then we can proceed to use that item alone as the resolved
            // expression.
            //
            return ConstructLookupResultExpr(
                lookupResult.item, overloadedExpr->base, overloadedExpr->loc, overloadedExpr);
        }

        // Otherwise, we weren't able to resolve the overloading given
        // the information available in context.
        //
        // If the client is asking for us to emit diagnostics about
        // this fact, we should do so here:
        //
        if( diagSink )
        {
            diagnoseAmbiguousReference(overloadedExpr, lookupResult);

            // TODO(tfoley): should we construct a new ErrorExpr here?
            return CreateErrorExpr(overloadedExpr);
        }
        else
        {
            // If the client isn't trying to *force* overload resolution
            // to complete just yet (e.g., they are just trying out one
            // candidate for an overloaded call site), then we return
            // the input expression as-is.
            //
            return overloadedExpr;
        }
    }

    Expr* SemanticsVisitor::maybeResolveOverloadedExpr(Expr* expr, LookupMask mask, DiagnosticSink* diagSink)
    {
        if (IsErrorExpr(expr))
            return expr;

        if( auto overloadedExpr = as<OverloadedExpr>(expr) )
        {
            return _resolveOverloadedExprImpl(overloadedExpr, mask, diagSink);
        }
        else
        {
            return expr;
        }
    }

    Expr* SemanticsVisitor::resolveOverloadedExpr(OverloadedExpr* overloadedExpr, LookupMask mask)
    {
        return _resolveOverloadedExprImpl(overloadedExpr, mask, getSink());
    }

    Type* SemanticsVisitor::tryGetDifferentialType(ASTBuilder* builder, Type* type)
    {
        if (auto ptrType = as<PtrTypeBase>(type))
        {
            auto baseDiffType = tryGetDifferentialType(builder, ptrType->getValueType());
            if (!baseDiffType) return nullptr;
            return builder->getPtrType(
                baseDiffType,
                ptrType->getClassInfo().m_name);
        }
        else if (auto arrayType = as<ArrayExpressionType>(type))
        {
            auto baseDiffType = tryGetDifferentialType(builder, arrayType->getElementType());
            if (!baseDiffType) return nullptr;
            return builder->getArrayType(
                baseDiffType,
                arrayType->getElementCount());
        }

        if (auto declRefType = as<DeclRefType>(type))
        {
            if (auto builtinRequirement = declRefType->getDeclRef().getDecl()->findModifier<BuiltinRequirementModifier>())
            {
                if (builtinRequirement->kind == BuiltinRequirementKind::DifferentialType)
                {
                    // We are trying to get differential type from a differential type.
                    // The result is itself.
                    return type;
                }
            }
            type = resolveType(type);
            if (const auto witness = as<SubtypeWitness>(tryGetInterfaceConformanceWitness(type, builder->getDifferentiableInterfaceType())))
            {
                auto diffTypeLookupResult = lookUpMember(
                    getASTBuilder(),
                    this,
                    getName("Differential"),
                    type,
                    Slang::LookupMask::type,
                    Slang::LookupOptions::None);

                diffTypeLookupResult = resolveOverloadedLookup(diffTypeLookupResult);

                if (!diffTypeLookupResult.isValid())
                {
                    return nullptr;
                }
                else if (diffTypeLookupResult.isOverloaded())
                {
                    return nullptr;
                }
                else
                {
                    SharedTypeExpr* baseTypeExpr = m_astBuilder->create<SharedTypeExpr>();
                    baseTypeExpr->base.type = type;
                    baseTypeExpr->type.type = m_astBuilder->getTypeType(type);

                    auto diffTypeExpr = ConstructLookupResultExpr(
                        diffTypeLookupResult.item,
                        baseTypeExpr,
                        declRefType->getDeclRef().getLoc(),
                        baseTypeExpr);

                    return resolveType(ExtractTypeFromTypeRepr(diffTypeExpr));
                }
            }
        }

        return nullptr;
    }

    Type* SemanticsVisitor::getDifferentialType(ASTBuilder* builder, Type* type, SourceLoc loc)
    {
        auto result = tryGetDifferentialType(builder, type);
        if (!result)
        {
            getSink()->diagnose(loc, Diagnostics::typeDoesntImplementInterfaceRequirement, type, getName("Differential"));
            return m_astBuilder->getErrorType();
        }
        return result;
    }

    void SemanticsVisitor::addDifferentiableTypeToDiffTypeRegistry(DeclRefType* type, SubtypeWitness* witness)
    {
        SLANG_RELEASE_ASSERT(m_parentDifferentiableAttr);
        if (witness)
        {
            m_parentDifferentiableAttr->addType(type->getDeclRef(), witness);
        }
    }

    void SemanticsVisitor::maybeRegisterDifferentiableType(ASTBuilder* builder, Type* type)
    {
        if (!builder->isDifferentiableInterfaceAvailable())
        {
            return;
        }

        if (!m_parentDifferentiableAttr)
        {
            return;
        }

        maybeRegisterDifferentiableTypeImplRecursive(builder, type);
    }

    void SemanticsVisitor::maybeRegisterDifferentiableTypeImplRecursive(ASTBuilder* builder, Type* type)
    {
        // Recursively visit the tree of type and register all differentiable types along the way.
        
        if (as<TypeType>(type))
            return;
        if (!type)
            return;

        // Have we already registered this type? If so we can exit now.
        if (m_parentDifferentiableAttr->m_typeRegistrationWorkingSet.contains(type))
            return;

        m_parentDifferentiableAttr->m_typeRegistrationWorkingSet.add(type);

        // Check for special cases such as PtrTypeBase<T> or Array<T>
        // This could potentially be handled later by simply defining extensions
        // for Ptr<T:IDifferentiable> etc..
        //
        if (auto ptrType = as<PtrTypeBase>(type))
        {
            maybeRegisterDifferentiableTypeImplRecursive(builder, ptrType->getValueType());
            return;
        }

        if (auto arrayType = as<ArrayExpressionType>(type))
        {
            maybeRegisterDifferentiableTypeImplRecursive(builder, arrayType->getElementType());
            // Fall through to register the array type itself.
        }

        if (auto declRefType = as<DeclRefType>(type))
        {
            if (auto subtypeWitness = as<SubtypeWitness>(
                tryGetInterfaceConformanceWitness(type, getASTBuilder()->getDifferentiableInterfaceType())))
            {
                addDifferentiableTypeToDiffTypeRegistry((DeclRefType*)type, subtypeWitness);
            }
            if (auto aggTypeDeclRef = declRefType->getDeclRef().as<AggTypeDecl>())
            {
                foreachDirectOrExtensionMemberOfType<InheritanceDecl>(this, aggTypeDeclRef, [&](DeclRef<InheritanceDecl> member)
                    {
                        auto subType = DeclRefType::create(m_astBuilder, member);
                        maybeRegisterDifferentiableTypeImplRecursive(m_astBuilder, subType);
                    });
                foreachDirectOrExtensionMemberOfType<VarDeclBase>(this, aggTypeDeclRef, [&](DeclRef<VarDeclBase> member)
                    {
                        auto fieldType = getType(m_astBuilder, member);
                        maybeRegisterDifferentiableTypeImplRecursive(m_astBuilder, fieldType);
                    });
            }
            SubstitutionSet(declRefType->getDeclRef()).forEachSubstitutionArg([&](Val* arg)
                {
                    if (auto typeArg = as<Type>(arg))
                    {
                        maybeRegisterDifferentiableTypeImplRecursive(m_astBuilder, typeArg);
                    }
                });
            return;
        }
    }


    Expr* SemanticsVisitor::CheckTerm(Expr* term)
    {
        auto checkedTerm = _CheckTerm(term);
        // Differentiable type checking.
        // TODO: This can be super slow.
        if (this->m_parentFunc &&
            this->m_parentFunc->findModifier<DifferentiableAttribute>())
        {
            maybeRegisterDifferentiableType(getASTBuilder(), checkedTerm->type.type);
        }
        return checkedTerm;
    }

    Expr* SemanticsVisitor::_CheckTerm(Expr* term)
    {
        if (!term) return nullptr;

        // The process of checking a term/expression can end up introducing
        // temporaries that need to be added to an outer scope. When jumping
        // into expression checking, we want to check if we already have such
        // a scope in place. If we do, we will re-use it for any sub-expressions.
        // If not, we need to create one.
        //
        if(getExprLocalScope())
        {
            return dispatchExpr(term, *this);
        }

        ExprLocalScope exprLocalScope;

        Expr* checkedTerm = dispatchExpr(term, withExprLocalScope(&exprLocalScope));

        if (IsErrorExpr(checkedTerm))
            return checkedTerm;

        LetExpr* outerMostBinding = exprLocalScope.getOuterMostBinding();
        if(!outerMostBinding)
        {
            return checkedTerm;
        }

        LetExpr* binding = outerMostBinding;
        auto type = checkedTerm->type;
        while (binding)
        {
            binding->type = type;

            if (const auto body = binding->body)
            {
                binding = as<LetExpr>(binding->body);
                SLANG_ASSERT(binding);
                continue;
            }
            else
            {
                binding->body = checkedTerm;
                break;
            }
        }

        return outerMostBinding;
    }

    Expr* SemanticsVisitor::CreateErrorExpr(Expr* expr)
    {
        if (!expr)
        {
            expr = m_astBuilder->create<IncompleteExpr>();
        }
        expr->type = QualType(m_astBuilder->getErrorType());
        return expr;
    }

    bool SemanticsVisitor::IsErrorExpr(Expr* expr)
    {
        // TODO: we may want other cases here...

        if (const auto errorType = as<ErrorType>(expr->type))
            return true;

        return false;
    }

    Expr* SemanticsVisitor::GetBaseExpr(Expr* expr)
    {
        if (auto memberExpr = as<MemberExpr>(expr))
        {
            return memberExpr->baseExpression;
        }
        else if(auto overloadedExpr = as<OverloadedExpr>(expr))
        {
            return overloadedExpr->base;
        }
        else if (auto overloadedExpr2 = as<OverloadedExpr2>(expr))
        {
            return overloadedExpr2->base;
        }
        else if (auto genApp = as<GenericAppExpr>(expr))
        {
            return GetBaseExpr(genApp->functionExpr);
        }
        else if (auto partiallyApplied = as<PartiallyAppliedGenericExpr>(expr))
        {
            return GetBaseExpr(partiallyApplied->originalExpr);
        }
        return nullptr;
    }

    Expr* SemanticsExprVisitor::visitIncompleteExpr(IncompleteExpr* expr)
    {
        expr->type = m_astBuilder->getErrorType();
        return expr;
    }

    Expr* SemanticsExprVisitor::visitBoolLiteralExpr(BoolLiteralExpr* expr)
    {
        expr->type = m_astBuilder->getBoolType();
        return expr;
    }

    Expr* SemanticsExprVisitor::visitNullPtrLiteralExpr(NullPtrLiteralExpr* expr)
    {
        expr->type = m_astBuilder->getNullPtrType();
        return expr;
    }

    Expr* SemanticsExprVisitor::visitNoneLiteralExpr(NoneLiteralExpr* expr)
    {
        expr->type = m_astBuilder->getNoneType();
        return expr;
    }

    Expr* SemanticsExprVisitor::visitIntegerLiteralExpr(IntegerLiteralExpr* expr)
    {
        // The expression might already have a type, determined by its suffix.
        // It it doesn't, we will give it a default type.
        //
        // TODO: We should be careful to pick a "big enough" type
        // based on the size of the value (e.g., don't try to stuff
        // a constant in an `int` if it requires 64 or more bits).
        //
        // The long-term solution here is to give a type to a literal
        // based on the context where it is used, but that requires
        // a more sophisticated type system than we have today.
        //
        if(!expr->type.type)
        {
            expr->type = m_astBuilder->getIntType();
        }
        return expr;
    }

    Expr* SemanticsExprVisitor::visitFloatingPointLiteralExpr(FloatingPointLiteralExpr* expr)
    {
        if(!expr->type.type)
        {
            expr->type = m_astBuilder->getFloatType();
        }
        return expr;
    }

    Expr* SemanticsExprVisitor::visitStringLiteralExpr(StringLiteralExpr* expr)
    {
        expr->type = m_astBuilder->getStringType();
        return expr;
    }

    IntVal* SemanticsVisitor::getIntVal(IntegerLiteralExpr* expr)
    {
        return m_astBuilder->getIntVal(expr->type.type, expr->value);
    }

    IntVal* SemanticsVisitor::tryConstantFoldExpr(
        SubstExpr<InvokeExpr>           invokeExpr,
        ConstantFoldingCircularityInfo* circularityInfo)
    {
        // We need all the operands to the expression

        // Check if the callee is an operation that is amenable to constant-folding.
        //
        // For right now we will look for calls to intrinsic functions, and then inspect
        // their names (this is bad and slow).
        auto funcDeclRefExpr = getBaseExpr(invokeExpr).as<DeclRefExpr>();
        if (!funcDeclRefExpr) return nullptr;

        auto funcDeclRef = getDeclRef(m_astBuilder, funcDeclRefExpr);
        auto intrinsicMod = funcDeclRef.getDecl()->findModifier<IntrinsicOpModifier>();
        auto implicitCast = funcDeclRef.getDecl()->findModifier<ImplicitConversionModifier>();
        if (!intrinsicMod && !implicitCast)
        {
            // We can't constant fold anything that doesn't map to a builtin
            // operation right now.
            //
            // TODO: we should really allow constant-folding for anything
            // that can be lowered to our bytecode...
            return nullptr;
        }

        // Let's not constant-fold operations with more than a certain number of arguments, for simplicity
        static const int kMaxArgs = 8;
        auto argCount = getArgCount(invokeExpr);
        if (argCount > kMaxArgs)
            return nullptr;

        // Before checking the operation name, let's look at the arguments
        IntVal* argVals[kMaxArgs];
        IntegerLiteralValue constArgVals[kMaxArgs];
        bool allConst = true;
        for(Index a = 0; a < argCount; ++a)
        {
            auto argExpr = getArg(invokeExpr, a);
            auto argVal = tryFoldIntegerConstantExpression(argExpr, circularityInfo);
            if (!argVal)
                return nullptr;

            argVals[a] = argVal;

            if (auto constArgVal = as<ConstantIntVal>(argVal))
            {
                constArgVals[a] = constArgVal->getValue();
            }
            else
            {
                allConst = false;
            }
        }

        if (!allConst)
        {
            // We support a very limited number of operations
            // on "constants" that aren't actually known, to be able to handle a generic
            // that takes an integer `N` but then constructs a vector of size `N+1`.
            //
            // The hard part there is implementing the rules for value unification in the
            // presence of more complicated `IntVal` subclasses, like `SumIntVal`. You'd
            // need inference to be smart enough to know that `2 + N` and `N + 2` are the
            // same value, as are `N + M + 1 + 1` and `M + 2 + N`.
            //
            // This is done by constructing a 'PolynomialIntVal' and rely on its
            // `canonicalize` operation.
            if (implicitCast)
            {
                // We cannot support casting in this case.
                return nullptr;
            }

            auto opName = funcDeclRef.getName();

            // handle binary operators
            if (opName == getName("-"))
            {
                if (argCount == 1)
                {
                    return PolynomialIntVal::neg(m_astBuilder, argVals[0]);
                }
                else if (argCount == 2)
                {
                    return PolynomialIntVal::sub(m_astBuilder, argVals[0], argVals[1]);
                }
            }
            else if (opName == getName("+"))
            {
                if (argCount == 1)
                {
                    return argVals[0];
                }
                else if (argCount == 2)
                {
                    return PolynomialIntVal::add(m_astBuilder, argVals[0], argVals[1]);
                }
            }
            else if (opName == getName("*"))
            {
                if (argCount == 2)
                {
                    return PolynomialIntVal::mul(m_astBuilder, argVals[0], argVals[1]);
                }
            }
            else if (opName == getName("/") || opName == getName("==") || opName == getName(">=") || opName == getName("<=") || opName == getName("!=")
                || opName == getName(">") || opName == getName("<") || opName == getName("&&") || opName == getName("||") || opName == getName("!")
                || opName == getName("|") || opName == getName("&") || opName == getName("^") || opName == getName("~") || opName == getName("%") ||
                opName == getName("?:") || opName == getName("<<") || opName == getName(">>"))
            {
                auto result = m_astBuilder->getOrCreate<FuncCallIntVal>(
                    invokeExpr.getExpr()->type.type,
                    funcDeclRef,
                    as<Type>(funcDeclRefExpr.getExpr()->type->substitute(
                        m_astBuilder, funcDeclRefExpr.getSubsts())),
                    makeArrayView(argVals, argCount));
                SLANG_RELEASE_ASSERT(result->getFuncType());
                return result;
            }
            return nullptr;
        }

        // At this point, all the operands had simple integer values, so we are golden.
        IntegerLiteralValue resultValue = 0;
        // If this is an implicit cast, we can try to fold.
        if (implicitCast)
        {
            auto targetBasicType = as<BasicExpressionType>(invokeExpr.getExpr()->type.type);
            if (!targetBasicType)
                return nullptr;
            auto foldVal = as<IntVal>(
                TypeCastIntVal::tryFoldImpl(m_astBuilder, targetBasicType, argVals[0], getSink()));
            if (foldVal)
                return foldVal;
            auto result = m_astBuilder->getTypeCastIntVal(targetBasicType, argVals[0]);
            return result;
        }
        else
        {
            auto opName = funcDeclRef.getName();

            // handle binary operators
            if (opName == getName("-"))
            {
                if (argCount == 1)
                {
                    resultValue = -constArgVals[0];
                }
                else if (argCount == 2)
                {
                    resultValue = constArgVals[0] - constArgVals[1];
                }
            }
            else if (opName == getName("!"))
            {
                resultValue = constArgVals[0] != 0;
            }
            else if (opName == getName("~"))
            {
                resultValue = ~constArgVals[0];
            }

            // simple binary operators
#define CASE(OP)                                                    \
            else if(opName == getName(#OP)) do {                    \
                if(argCount != 2) return nullptr;                   \
                resultValue = constArgVals[0] OP constArgVals[1];   \
            } while(0)

            CASE(+); // TODO: this can also be unary...
            CASE(*);
            CASE(<<);
            CASE(>>);
            CASE(&);
            CASE(|);
            CASE(^);
            CASE(!=);
            CASE(==);
            CASE(>=);
            CASE(<=);
            CASE(<);
            CASE(>);
#undef CASE
            // binary operators with chance of divide-by-zero
            // TODO: issue a suitable error in that case
#define CASE(OP)                                                    \
            else if(opName == getName(#OP)) do {                    \
                if(argCount != 2) return nullptr;                   \
                if(!constArgVals[1]) return nullptr;                \
                resultValue = constArgVals[0] OP constArgVals[1];   \
            } while(0)
            CASE(/);
            CASE(%);
#undef CASE
            else if (opName == getName("?:"))
            {
                if (argCount != 3)
                    return nullptr;
                if (constArgVals[0] != 0)
                    resultValue = constArgVals[1];
                else
                    resultValue = constArgVals[2];
            }
            // TODO(tfoley): more cases
            else
            {
                return nullptr;
            }
        }

        IntVal* result = m_astBuilder->getIntVal(invokeExpr.getExpr()->type.type, resultValue);
        return result;
    }

    bool SemanticsVisitor::_checkForCircularityInConstantFolding(
        Decl*                           decl,
        ConstantFoldingCircularityInfo* circularityInfo)
    {
        // TODO: If the `decl` is already on the chain of `circularityInfo`,
        // then we know that we are trying to recursively fold the
        // same declaration as part of its own definition, and we need
        // to diagnose that as an error.
        //
        for( auto info = circularityInfo; info; info = info->next )
        {
            if(decl == info->decl)
            {
                getSink()->diagnose(decl, Diagnostics::variableUsedInItsOwnDefinition, decl);
                return true;
            }
        }

        return false;
    }

    IntVal* SemanticsVisitor::tryConstantFoldDeclRef(
        DeclRef<VarDeclBase> const&     declRef,
        ConstantFoldingCircularityInfo* circularityInfo)
    {
        auto decl = declRef.getDecl();

        if(_checkForCircularityInConstantFolding(decl, circularityInfo))
            return nullptr;

        // In HLSL, `static const` is used to mark compile-time constant expressions
        if(!decl->hasModifier<HLSLStaticModifier>())
            return nullptr;
        if(!decl->hasModifier<ConstModifier>())
            return nullptr;

        if (isInterfaceRequirement(decl))
        {
            auto witness = findThisTypeWitness(SubstitutionSet(declRef), as<InterfaceDecl>(decl->parentDecl));

            auto val = WitnessLookupIntVal::tryFold(
                m_astBuilder,
                witness,
                decl,
                declRef.substitute(m_astBuilder, decl->type.type));
            return as<IntVal>(val);
        }

        if (!getInitExpr(m_astBuilder, declRef))
            return nullptr;

        ensureDecl(declRef.getDecl(), DeclCheckState::Checked);
        ConstantFoldingCircularityInfo newCircularityInfo(decl, circularityInfo);
        return tryConstantFoldExpr(getInitExpr(m_astBuilder, declRef), &newCircularityInfo);
    }

    IntVal* SemanticsVisitor::tryConstantFoldExpr(
        SubstExpr<Expr>                 expr,
        ConstantFoldingCircularityInfo* circularityInfo)
    {
        
        // Unwrap any "identity" expressions
        while (auto parenExpr = expr.as<ParenExpr>())
        {
            expr = getBaseExpr(parenExpr);
        }

        if (auto intLitExpr = expr.as<IntegerLiteralExpr>())
        {
            return getIntVal(intLitExpr);
        }

        if (auto boolLitExpr = expr.as<BoolLiteralExpr>())
        {
            // If it's a boolean, we allow promotion to int.
            const IntegerLiteralValue value = IntegerLiteralValue(boolLitExpr.getExpr()->value);
            return m_astBuilder->getIntVal(m_astBuilder->getBoolType(), value);
        }

        if (auto arrayLengthExpr = expr.as<GetArrayLengthExpr>())
        {
            if (arrayLengthExpr.getExpr()->arrayExpr && arrayLengthExpr.getExpr()->arrayExpr->type)
            {
                auto type = arrayLengthExpr.getExpr()->arrayExpr->type.type->substitute(m_astBuilder, expr.getSubsts());
                if (auto arrayType = as<ArrayExpressionType>(type))
                {
                    if (!arrayType->isUnsized())
                    {
                        if (auto val = as<IntVal>(arrayType->getElementCount()))
                            return val;
                    }
                }
            }
        }

        // it is possible that we are referring to a generic value param
        if (auto declRefExpr = expr.as<DeclRefExpr>())
        {
            auto declRef = getDeclRef(m_astBuilder, declRefExpr);

            if (auto genericValParamRef = declRef.as<GenericValueParamDecl>())
            {
                Val* valResult = m_astBuilder->getOrCreate<GenericParamIntVal>(
                    declRef.substitute(m_astBuilder, genericValParamRef.getDecl()->getType()),
                    genericValParamRef);
                valResult = valResult->substitute(m_astBuilder, expr.getSubsts());
                return as<IntVal>(valResult);
            }

            // We may also need to check for references to variables that
            // are defined in a way that can be used as a constant expression:
            if(auto varRef = declRef.as<VarDeclBase>())
            {
                return tryConstantFoldDeclRef(varRef, circularityInfo);
            }
            else if(auto enumRef = declRef.as<EnumCaseDecl>())
            {
                // The cases in an `enum` declaration can also be used as constant expressions,
                if(auto tagExpr = getTagExpr(m_astBuilder, enumRef))
                {
                    auto enumCaseDecl = enumRef.getDecl();
                    if(_checkForCircularityInConstantFolding(enumCaseDecl, circularityInfo))
                        return nullptr;

                    ConstantFoldingCircularityInfo newCircularityInfo(enumCaseDecl, circularityInfo);
                    return tryConstantFoldExpr(tagExpr, &newCircularityInfo);
                }
            }
        }

        if(auto castExpr = expr.as<TypeCastExpr>())
        {
            auto substType = getType(m_astBuilder, expr);
            if (!substType)
                return nullptr;
            if (!isScalarIntegerType(substType))
                return nullptr;
            auto val = tryConstantFoldExpr(getArg(castExpr, 0), circularityInfo);
            if (val)
            {
                if (!castExpr.getExpr()->type)
                    return nullptr;
                auto foldVal = as<IntVal>(
                    TypeCastIntVal::tryFoldImpl(m_astBuilder, substType, val, getSink()));
                if (foldVal)
                    return foldVal;
                auto result = m_astBuilder->getTypeCastIntVal(substType, val);
                return result;
            }
        }
        else if (auto invokeExpr = expr.as<InvokeExpr>())
        {
            auto val = tryConstantFoldExpr(invokeExpr, circularityInfo);
            if (val)
                return val;
        }
        else if (auto sizeOfLikeExpr = as<SizeOfLikeExpr>(expr.getExpr()))
        {
            ASTNaturalLayoutContext context(getASTBuilder(), nullptr);
            const auto size = context.calcSize(sizeOfLikeExpr->sizedType);
            if (!size)
            {
                return nullptr;
            }

            auto value = as<AlignOfExpr>(sizeOfLikeExpr) ? 
                size.alignment :
                size.size;
            
            // We can return as an IntVal
            return getASTBuilder()->getIntVal(expr.getExpr()->type, value);
        }
        
        return nullptr;
    }

    IntVal* SemanticsVisitor::tryFoldIntegerConstantExpression(
        SubstExpr<Expr>                 expr,
        ConstantFoldingCircularityInfo* circularityInfo)
    {
        // Check if type is acceptable for an integer constant expression
        //
        if(!isScalarIntegerType(getType(m_astBuilder, expr)))
            return nullptr;

        // Consider operations that we might be able to constant-fold...
        //
        return tryConstantFoldExpr(expr, circularityInfo);
    }

    IntVal* SemanticsVisitor::CheckIntegerConstantExpression(Expr* inExpr, IntegerConstantExpressionCoercionType coercionType, Type* expectedType, DiagnosticSink* sink)
    {
        // No need to issue further errors if the expression didn't even type-check.
        if(IsErrorExpr(inExpr)) return nullptr;

        // First coerce the expression to the expected type
        Expr* expr = nullptr;
        switch (coercionType)
        {
        case IntegerConstantExpressionCoercionType::SpecificType:
            expr = coerce(CoercionSite::General, expectedType, inExpr);
            break;
        case IntegerConstantExpressionCoercionType::AnyInteger:
            if (isScalarIntegerType(inExpr->type))
                expr = inExpr;
            else
                expr = coerce(CoercionSite::General, m_astBuilder->getIntType(), inExpr);
            break;
        default:
            break;
        }

        // No need to issue further errors if the type coercion failed.
        if(IsErrorExpr(expr)) return nullptr;

        auto result = tryFoldIntegerConstantExpression(expr, nullptr);
        if (!result && sink)
        {
            sink->diagnose(expr, Diagnostics::expectedIntegerConstantNotConstant);
        }
        return result;
    }

    IntVal* SemanticsVisitor::CheckIntegerConstantExpression(Expr* inExpr, IntegerConstantExpressionCoercionType coercionType, Type* expectedType)
    {
        return CheckIntegerConstantExpression(inExpr, coercionType, expectedType, getSink());
    }

    IntVal* SemanticsVisitor::CheckEnumConstantExpression(Expr* expr)
    {
        // No need to issue further errors if the expression didn't even type-check.
        if(IsErrorExpr(expr)) return nullptr;

        // No need to issue further errors if the type coercion failed.
        if(IsErrorExpr(expr)) return nullptr;

        auto result = tryConstantFoldExpr(expr, nullptr);
        if (!result)
        {
            getSink()->diagnose(expr, Diagnostics::expectedIntegerConstantNotConstant);
        }
        return result;
    }

    Expr* SemanticsVisitor::CheckSimpleSubscriptExpr(
        IndexExpr*   subscriptExpr,
        Type*              elementType)
    {
        auto baseExpr = subscriptExpr->baseExpression;
        if (subscriptExpr->indexExprs.getCount() < 1)
        {
            getSink()->diagnose(subscriptExpr, Diagnostics::notEnoughArguments, subscriptExpr->indexExprs.getCount(), 1);
            return CreateErrorExpr(subscriptExpr);
        }
        else if (subscriptExpr->indexExprs.getCount() > 1)
        {
            getSink()->diagnose(subscriptExpr, Diagnostics::tooManyArguments, subscriptExpr->indexExprs.getCount(), 1);
            return CreateErrorExpr(subscriptExpr);
        }

        auto indexExpr = subscriptExpr->indexExprs[0];

        if (!indexExpr->type->equals(m_astBuilder->getIntType()) &&
            !indexExpr->type->equals(m_astBuilder->getUIntType()))
        {
            getSink()->diagnose(indexExpr, Diagnostics::subscriptIndexNonInteger);
            return CreateErrorExpr(subscriptExpr);
        }

        subscriptExpr->type = QualType(elementType);

        // TODO(tfoley): need to be more careful about this stuff
        subscriptExpr->type.isLeftValue = baseExpr->type.isLeftValue;

        return subscriptExpr;
    }

    Expr* SemanticsExprVisitor::visitIndexExpr(IndexExpr* subscriptExpr)
    {
        auto baseExpr = checkBaseForMemberExpr(subscriptExpr->baseExpression);

        for (auto& arg : subscriptExpr->indexExprs)
        {
            arg = CheckTerm(arg);
        }

        // If anything went wrong in the base expression,
        // then just move along...
        if (IsErrorExpr(baseExpr))
            return CreateErrorExpr(subscriptExpr);

        subscriptExpr->baseExpression = baseExpr;

        // Otherwise, we need to look at the type of the base expression,
        // to figure out how subscripting should work.
        auto baseType = baseExpr->type.Ptr();
        if (auto baseTypeType = as<TypeType>(baseType))
        {
            // We are trying to "index" into a type, so we have an expression like `float[2]`
            // which should be interpreted as resolving to an array type.

            IntVal* elementCount = nullptr;
            if (subscriptExpr->indexExprs.getCount() == 1)
            {
                elementCount = CheckIntegerConstantExpression(subscriptExpr->indexExprs[0], IntegerConstantExpressionCoercionType::AnyInteger, nullptr);
            }
            else if (subscriptExpr->indexExprs.getCount() != 0)
            {
                getSink()->diagnose(subscriptExpr, Diagnostics::multiDimensionalArrayNotSupported);
            }

            auto elementType = CoerceToUsableType(TypeExp(baseExpr, baseTypeType->getType()));
            auto arrayType = getArrayType(
                m_astBuilder,
                elementType,
                elementCount);

            subscriptExpr->type = QualType(m_astBuilder->getTypeType(arrayType));
            return subscriptExpr;
        }
        else if (auto baseArrayType = as<ArrayExpressionType>(baseType))
        {
            return CheckSimpleSubscriptExpr(
                subscriptExpr,
                baseArrayType->getElementType());
        }
        else if (auto vecType = as<VectorExpressionType>(baseType))
        {
            return CheckSimpleSubscriptExpr(
                subscriptExpr,
                vecType->getElementType());
        }
        else if (auto matType = as<MatrixExpressionType>(baseType))
        {
            // TODO(tfoley): We shouldn't go and recompute
            // row types over and over like this... :(
            auto rowType = createVectorType(
                matType->getElementType(),
                matType->getColumnCount());

            return CheckSimpleSubscriptExpr(
                subscriptExpr,
                rowType);
        }

        // Default behavior is to look at all available `__subscript`
        // declarations on the type and try to call one of them.

        auto operatorName = getName("operator[]");

        LookupResult lookupResult = lookUpMember(
            m_astBuilder,
            this,
            operatorName,
            baseType,
            LookupMask::Default,
            LookupOptions::NoDeref);
        if (!lookupResult.isValid())
        {
            getSink()->diagnose(subscriptExpr, Diagnostics::subscriptNonArray, baseType);
            return CreateErrorExpr(subscriptExpr);
        }
        auto subscriptFuncExpr = createLookupResultExpr(
            operatorName,
            lookupResult,
            subscriptExpr->baseExpression,
            subscriptExpr->loc,
            subscriptExpr);

        InvokeExpr* subscriptCallExpr = m_astBuilder->create<InvokeExpr>();
        subscriptCallExpr->loc = subscriptExpr->loc;
        subscriptCallExpr->functionExpr = subscriptFuncExpr;
        subscriptCallExpr->arguments.addRange(subscriptExpr->indexExprs);
        subscriptCallExpr->argumentDelimeterLocs.addRange(subscriptExpr->argumentDelimeterLocs);

        return CheckInvokeExprWithCheckedOperands(subscriptCallExpr);
    }

    Expr* SemanticsExprVisitor::visitParenExpr(ParenExpr* expr)
    {
        auto base = expr->base;
        base = CheckTerm(base);

        expr->base = base;
        expr->type = base->type;
        return expr;
    }

    void SemanticsVisitor::maybeDiagnoseThisNotLValue(Expr* expr)
    {
        // We will try to handle expressions of the form:
        //
        //      e ::= "this"
        //          | e . name
        //          | e [ expr ]
        //
        // We will unwrap the `e.name` and `e[expr]` cases in a loop.
        Expr* e = expr;
        for(;;)
        {
            if(auto memberExpr = as<MemberExpr>(e))
            {
                e = memberExpr->baseExpression;
            }
            else if(auto subscriptExpr = as<IndexExpr>(e))
            {
                e = subscriptExpr->baseExpression;
            }
            else
            {
                break;
            }
        }
        //
        // Now we check to see if we have a `this` expression,
        // and if it is immutable.
        if(auto thisExpr = as<ThisExpr>(e))
        {
            if(!thisExpr->type.isLeftValue)
            {
                getSink()->diagnoseWithoutSourceView(thisExpr, Diagnostics::thisIsImmutableByDefault);
            }
        }
    }

    Expr* SemanticsVisitor::checkAssignWithCheckedOperands(AssignExpr* expr)
    {
        expr->left = maybeOpenRef(expr->left);
        auto type = expr->left->type;
        auto right = maybeOpenRef(expr->right);
        expr->right = coerce(CoercionSite::Assignment, type, right);

        if (!type.isLeftValue)
        {
            if (as<ErrorType>(type))
            {
                // Don't report an l-value issue on an erroneous expression
            }
            else
            {
                getSink()->diagnose(expr, Diagnostics::assignNonLValue);

                // As a special case, check if the LHS expression is derived
                // from a `this` parameter (implicitly or explicitly), which
                // is immutable. We can give the user a bit more context into
                // what is going on.
                //
                maybeDiagnoseThisNotLValue(expr->left);
            }
        }
        expr->type = type;
        return expr;
    }

    Expr* SemanticsExprVisitor::visitAssignExpr(AssignExpr* expr)
    {
        expr->left = CheckExpr(expr->left);
        expr->right = CheckTerm(expr->right);

        return checkAssignWithCheckedOperands(expr);
    }

    Expr* SemanticsVisitor::CheckExpr(Expr* uncheckedExpr)
    {
        auto checkedTerm = CheckTerm(uncheckedExpr);

        // First, we want to do any disambiguation that is needed in order
        // to turn the `term` into an expression that names a single
        // value (and not something overloaded).
        //
        auto checkedExpr = maybeResolveOverloadedExpr(checkedTerm, LookupMask::Default, getSink());

        // Next, we want to ensure that the `expr` actually has a type
        // that is allowable in an expression context (e.g., make sure
        // that `expr` names a value and not a type).
        //
        // TODO: Implement this step.

        return checkedExpr;
    }

    static bool _canLValueCoerceScalarType(Type* a, Type* b)
    {
        auto basicTypeA = as<BasicExpressionType>(a);
        auto basicTypeB = as<BasicExpressionType>(b);

        if (basicTypeA && basicTypeB)
        {
            const auto& infoA = BaseTypeInfo::getInfo(basicTypeA->getBaseType());
            const auto& infoB = BaseTypeInfo::getInfo(basicTypeB->getBaseType());

            // TODO(JS): Initially this tries to limit where LValueImplict casts happen.
            // We could in principal allow different sizes, as long as we converted to a temprorary
            // and back again. 
            // 
            // For now we just stick with the simple case. 
            // // We only allow on integer types for now. In effect just allowing any size uint/int conversions
            if (infoA.sizeInBytes == infoB.sizeInBytes && 
                (infoA.flags & infoB.flags & BaseTypeInfo::Flag::Integer))
            {
                return true;
            }

        }
        return false;
    }

    static bool _canLValueCoerce(Type* a, Type* b)
    {
        // We can *assume* here that if they are coercable, that dimensions of vectors
        // and matrices match. We might want to assert to be sure...
        SLANG_ASSERT(a != b);
        if (a->astNodeType == b->astNodeType)
        {
            if (auto matA = as<MatrixExpressionType>(a))
            {
                return _canLValueCoerceScalarType(matA->getElementType(), static_cast<MatrixExpressionType*>(b)->getElementType());   
            }
            else if (auto vecA = as<VectorExpressionType>(a))
            {
                return  _canLValueCoerceScalarType(vecA->getScalarType(), static_cast<VectorExpressionType*>(b)->getScalarType());   
            }
        }
        return _canLValueCoerceScalarType(a, b);
    }

    Expr* SemanticsVisitor::CheckInvokeExprWithCheckedOperands(InvokeExpr *expr)
    {
        auto rs = ResolveInvoke(expr);
        if (auto invoke = as<InvokeExpr>(rs))
        {
            // if this is still an invoke expression, test arguments passed to inout/out parameter are LValues
            if(auto funcType = as<FuncType>(invoke->functionExpr->type))
            {
                if (!funcType->getErrorType()->equals(m_astBuilder->getBottomType()))
                {
                    // If the callee throws, make sure we are inside a try clause.
                    if (m_enclosingTryClauseType == TryClauseType::None)
                    {
                        getSink()->diagnose(invoke, Diagnostics::mustUseTryClauseToCallAThrowFunc);
                    }
                }

                Index paramCount = funcType->getParamCount();
                for (Index pp = 0; pp < paramCount; ++pp)
                {
                    auto paramType = funcType->getParamType(pp);
                    if (as<OutTypeBase>(paramType) || as<RefType>(paramType))
                    {
                        // `out`, `inout`, and `ref` parameters currently require
                        // an *exact* match on the type of the argument.
                        //
                        // TODO: relax this requirement by allowing an argument
                        // for an `inout` parameter to be converted in both
                        // directions.
                        //
                        if( pp < expr->arguments.getCount() )
                        {
                            auto argExpr = expr->arguments[pp];
                            if( !argExpr->type.isLeftValue)
                            {
                                auto implicitCastExpr = as<ImplicitCastExpr>(argExpr);

                                // NOTE: 
                                // This is currently only enabled for in/inout based scenarios. Ie NOT ref.
                                // 
                                // Depending on the target there can be an issue around atomics.
                                // The fall back transformation with InOut/OutImplicitCast is to introduce 
                                // a temporary, and do the work on that and copy back.
                                // 
                                // This doesn't work with an atomic. So the work around is to not enable
                                // the transformation with ref types, which atomics are defined on.
                                // 
                                // An argument can be made that transformation shouldn't apply to the ref scenario in general.
                                if (implicitCastExpr && 
                                    as<OutTypeBase>(paramType) && 
                                    _canLValueCoerce(implicitCastExpr->arguments[0]->type, implicitCastExpr->type))
                                {
                                    // This is to work around issues like
                                    //
                                    // ```
                                    // int a = 0;
                                    // uint b = 1;
                                    // a += b;
                                    // ```
                                    // That strictly speaking it's not allowed, but we are going to allow it for now 
                                    // for situations were the types are uint/int and vector/matrix varieties of those types
                                    // 
                                    // Then in lowering we are going to insert code to do something like
                                    // ```
                                    // var OutType: tmp = arg;
                                    // f(... tmp);
                                    // arg = tmp;
                                    // ```

                                    TypeCastExpr* lValueImplicitCast;

                                    // We want to record if the cast is being used for `out` or `inout`/`ref` as 
                                    // if it's just `out` we won't need to convert before passing in.
                                    if (as<OutType>(paramType))
                                    {
                                        lValueImplicitCast = getASTBuilder()->create<OutImplicitCastExpr>(*implicitCastExpr);
                                    }
                                    else
                                    {
                                        lValueImplicitCast = getASTBuilder()->create<InOutImplicitCastExpr>(*implicitCastExpr);
                                    }

                                    // Replace the expression. This should make this situation easier to detect.
                                    expr->arguments[pp] = lValueImplicitCast;
                                }
                                else if (!as<ErrorType>(argExpr->type))
                                {
                                    getSink()->diagnose(
                                        argExpr,
                                        Diagnostics::argumentExpectedLValue,
                                        pp);

                                    
                                    if(implicitCastExpr)
                                    {
                                        const DiagnosticInfo* diagnostic = nullptr;

                                        // Try and determine reason for failure
                                        if (as<RefType>(paramType))
                                        {
                                            // Ref types are not allowed to use this mechanism because it breaks atomics 
                                            diagnostic = &Diagnostics::implicitCastUsedAsLValueRef;
                                        }
                                        else if (!_canLValueCoerce(implicitCastExpr->arguments[0]->type, implicitCastExpr->type))
                                        {
                                            // We restict what types can use this mechanism - currently int/uint and same sized matrix/vectors
                                            // of those types.
                                            diagnostic = &Diagnostics::implicitCastUsedAsLValueType;
                                        }
                                        else
                                        {
                                            // Fall back, in case there are other reasons...
                                            diagnostic = &Diagnostics::implicitCastUsedAsLValue;
                                        }
                                        getSink()->diagnoseWithoutSourceView(
                                            argExpr,
                                            *diagnostic,
                                            implicitCastExpr->arguments[0]->type,
                                            implicitCastExpr->type);
                                    }

                                    maybeDiagnoseThisNotLValue(argExpr);
                                }
                            }
                        }
                        else
                        {
                            // There are two ways we could get here, both involving
                            // a call where the number of argument expressions is
                            // less than the number of parameters on the callee:
                            //
                            // 1. There might be fewer arguments than parameters
                            // because the trailing parameters should be defaulted
                            //
                            // 2. There might be fewer arguments than parameters
                            // because the call is incorrect.
                            //
                            // In case (2) an error would have already been diagnosed,
                            // and we don't want to emit another cascading error here.
                            //
                            // In case (1) this implies the user declared an `out`
                            // or `inout` parameter with a default argument expression.
                            // That should be an error, but it should be detected
                            // on the declaration instead of here at the use site.
                            //
                            // Thus, it makes sense to ignore this case here.
                        }
                    }
                }

                if (auto higherOrderInvoke = as<DifferentiateExpr>(invoke->functionExpr))
                {
                    FunctionDifferentiableLevel requiredLevel;
                    if (auto funcDeclExpr = as<DeclRefExpr>(
                            getInnerMostExprFromHigherOrderExpr(higherOrderInvoke, requiredLevel)))
                    {
                        auto funcDecl = as<FunctionDeclBase>(funcDeclExpr->declRef.getDecl());
                        if (funcDecl)
                        {
                            if (requiredLevel == FunctionDifferentiableLevel::Forward &&
                                !getShared()->isDifferentiableFunc(funcDecl))
                            {
                                getSink()->diagnose(funcDeclExpr, Diagnostics::functionNotMarkedAsDifferentiable, funcDecl, "forward");
                            }
                            if (requiredLevel == FunctionDifferentiableLevel::Backward &&
                                !getShared()->isBackwardDifferentiableFunc(funcDecl))
                            {
                                getSink()->diagnose(funcDeclExpr, Diagnostics::functionNotMarkedAsDifferentiable, funcDecl, "backward");
                            }
                            if (!isEffectivelyStatic(funcDecl) && !isGlobalDecl(funcDecl))
                            {
                                getSink()->diagnose(invoke->functionExpr, Diagnostics::nonStaticMemberFunctionNotAllowedAsDiffOperand, funcDecl);
                            }
                        }
                    }
                }
            }
        }
        return rs;
    }


    Expr* SemanticsExprVisitor::visitSelectExpr(SelectExpr* expr)
    {
        auto result = visitInvokeExpr(expr);
        if (as<ErrorType>(result->type.type))
            return result;
        auto invokeExpr = as<InvokeExpr>(result);
        if (!result)
            return result;
        if (invokeExpr->arguments.getCount() != 3)
            return result;

        if (as<BasicExpressionType>(invokeExpr->arguments[0]->type.type))
        {
            auto newArgs = invokeExpr->arguments;
            expr->arguments.clear();
            expr->arguments = newArgs;
            expr->type = invokeExpr->type;
            return expr;
        }

        if (getParentDifferentiableAttribute())
        {
            // If we are in a differentiable func, issue
            // a diagnostic on use of non short-circuiting select.
            getSink()->diagnose(expr->loc, Diagnostics::useOfNonShortCircuitingOperatorInDiffFunc);
        }
        else
        {
            // For all other functions, we issue a warning for deprecation of vector-typed ?: operator.
            getSink()->diagnose(expr->loc, Diagnostics::useOfNonShortCircuitingOperator);
        }
        return result;
    }

    Expr* SemanticsExprVisitor::visitInvokeExpr(InvokeExpr* expr)
    {
        // check the base expression first
        if (!expr->originalFunctionExpr)
            expr->originalFunctionExpr = expr->functionExpr;
        expr->functionExpr = CheckTerm(expr->functionExpr);
        auto treatAsDifferentiableExpr = m_treatAsDifferentiableExpr;
        m_treatAsDifferentiableExpr = nullptr;
        // Next check the argument expressions
        for (auto & arg : expr->arguments)
        {
            arg = CheckTerm(arg);
        }
        m_treatAsDifferentiableExpr = treatAsDifferentiableExpr;

        // If we are in a differentiable function, register differential witness tables involved in
        // this call.
        if (m_parentFunc && m_parentFunc->hasModifier<DifferentiableAttribute>())
        {
            for (auto& arg : expr->arguments)
            {
                maybeRegisterDifferentiableType(m_astBuilder, arg->type.type);
            }
        }

        auto checkedExpr = CheckInvokeExprWithCheckedOperands(expr);

        if (m_parentDifferentiableAttr)
        {
            FunctionDifferentiableLevel callerDiffLevel = FunctionDifferentiableLevel::None;
            if (m_parentFunc)
                callerDiffLevel = getShared()->getFuncDifferentiableLevel(m_parentFunc);

            if (auto checkedInvokeExpr = as<InvokeExpr>(checkedExpr))
            {
                // Register types for final resolved invoke arguments again.
                for (auto& arg : expr->arguments)
                {
                    maybeRegisterDifferentiableType(m_astBuilder, arg->type.type);
                }

                if (auto calleeExpr = as<DeclRefExpr>(checkedInvokeExpr->functionExpr))
                {
                    if (auto calleeDecl = as<FunctionDeclBase>(calleeExpr->declRef.getDecl()))
                    {
                        auto calleeDiffLevel = getShared()->getFuncDifferentiableLevel(calleeDecl);
                        if (calleeDiffLevel >= callerDiffLevel)
                        {
                            if (!m_treatAsDifferentiableExpr)
                            {
                                auto newFuncExpr =
                                    getASTBuilder()->create<TreatAsDifferentiableExpr>();
                                newFuncExpr->type = checkedInvokeExpr->type;
                                newFuncExpr->innerExpr = checkedInvokeExpr;
                                newFuncExpr->loc = checkedInvokeExpr->loc;
                                newFuncExpr->flavor = TreatAsDifferentiableExpr::Flavor::Differentiable;
                                checkedExpr = newFuncExpr;
                            }
                            else
                            {
                                getSink()->diagnose(
                                    m_treatAsDifferentiableExpr,
                                    Diagnostics::useOfNoDiffOnDifferentiableFunc);
                            }
                        }
                    }
                }
            }
            maybeRegisterDifferentiableType(m_astBuilder, checkedExpr->type.type);
        }
        return checkedExpr;
    }

    Expr* SemanticsExprVisitor::visitVarExpr(VarExpr *expr)
    {
        // If we've already resolved this expression, don't try again.
        if (expr->declRef)
        {
            if (!expr->type)
                expr->type = GetTypeForDeclRef(expr->declRef, expr->loc);
            return expr;
        }
        expr->type = QualType(m_astBuilder->getErrorType());
        auto lookupResult = lookUp(
            m_astBuilder,
            this, expr->name, expr->scope);

        if (expr->name == getSession()->getCompletionRequestTokenName())
        {
            auto scopeKind = CompletionSuggestions::ScopeKind::Expr;
            if (!m_parentFunc)
                scopeKind = CompletionSuggestions::ScopeKind::Decl;
            suggestCompletionItems(scopeKind, lookupResult);
            return expr;
        }

        if (lookupResult.isValid())
        {
            return createLookupResultExpr(
                expr->name,
                lookupResult,
                nullptr,
                expr->loc,
                expr);
        }

        getSink()->diagnose(expr, Diagnostics::undefinedIdentifier2, expr->name);

        return expr;
    }

    Type* SemanticsVisitor::_toDifferentialParamType(Type* primalType)
    {
        // Check for type modifiers like 'out' and 'inout'. We need to differentiate the
        // nested type.
        //
        if (auto primalOutType = as<OutType>(primalType))
        {
            return m_astBuilder->getOutType(_toDifferentialParamType(primalOutType->getValueType()));
        }
        else if (auto primalInOutType = as<InOutType>(primalType))
        {
            return m_astBuilder->getInOutType(_toDifferentialParamType(primalInOutType->getValueType()));
        }
        return getDifferentialPairType(primalType);
    }

    Type* SemanticsVisitor::getDifferentialPairType(Type* primalType)
    {
        if (auto modifiedType = as<ModifiedType>(primalType))
        {
            if (modifiedType->findModifier<NoDiffModifierVal>())
                return modifiedType->getBase();
        }

        // Get a reference to the builtin 'IDifferentiable' interface
        auto differentiableInterface = getASTBuilder()->getDifferentiableInterfaceType();

        auto conformanceWitness = as<Witness>(isSubtype(primalType, differentiableInterface));
        // Check if the provided type inherits from IDifferentiable.
        // If not, return the original type.
        if (conformanceWitness)
        {
            return m_astBuilder->getDifferentialPairType(primalType, conformanceWitness);
        }
        else
            return primalType;
    }

    Type* SemanticsVisitor::getForwardDiffFuncType(FuncType* originalType)
    {
        // Resolve JVP type here. 
        // Note that this type checking needs to be in sync with
        // the auto-generation logic in slang-ir-jvp-diff.cpp
        List<Type*> paramTypes;

        // The JVP return type is float if primal return type is float
        // void otherwise.
        //
        auto resultType = getDifferentialPairType(originalType->getResultType());
        
        // No support for differentiating function that throw errors, for now.
        SLANG_ASSERT(originalType->getErrorType()->equals(m_astBuilder->getBottomType()));
        auto errorType = originalType->getErrorType();

        for (Index i = 0; i < originalType->getParamCount(); i++)
        {
            if(auto jvpParamType = _toDifferentialParamType(originalType->getParamType(i)))
                paramTypes.add(jvpParamType);
        }
        FuncType* jvpType = m_astBuilder->getOrCreate<FuncType>(paramTypes.getArrayView(), resultType, errorType);

        return jvpType;
    }

    Type* SemanticsVisitor::getBackwardDiffFuncType(FuncType* originalType)
    {
        // Resolve backward diff type here. 
        // Note that this type checking needs to be in sync with
        // the auto-generation logic in slang-ir-jvp-diff.cpp
        List<Type*> paramTypes;

        // The backward diff return type is void
        //
        auto resultType = m_astBuilder->getVoidType();

        // No support for differentiating function that throw errors, for now.
        SLANG_ASSERT(originalType->getErrorType()->equals(m_astBuilder->getBottomType()));
        auto errorType = originalType->getErrorType();

        for (Index i = 0; i < originalType->getParamCount(); i++)
        {
            if (auto outType = as<OutType>(originalType->getParamType(i)))
            {
                auto diffElementType =
                    tryGetDifferentialType(m_astBuilder, outType->getValueType());
                if (diffElementType)
                {
                    paramTypes.add(diffElementType);
                }
                else
                {
                    continue;
                }
            }
            else if (auto derivType = _toDifferentialParamType(originalType->getParamType(i)))
            {
                if (as<DifferentialPairType>(derivType))
                {
                    // An `in` differentiable parameter becomes an `inout` parameter.
                    derivType = m_astBuilder->getInOutType(derivType);
                }
                else if (auto inoutType = as<InOutType>(derivType))
                {
                    if (!as<DifferentialPairType>(inoutType->getValueType()))
                    {
                        // An `inout` non differentiable parameter becomes an `in` parameter
                        // (removing `out`).
                        derivType = inoutType->getValueType();
                    }
                }
                paramTypes.add(derivType);
            }
        }
        
        // Last parameter is the initial derivative of the original return type
        auto dOutType = tryGetDifferentialType(m_astBuilder, originalType->getResultType());
        if (dOutType)
            paramTypes.add(dOutType);

        return m_astBuilder->getOrCreate<FuncType>(paramTypes.getArrayView(), resultType, errorType);
    }

    struct HigherOrderInvokeExprCheckingActions
    {
        virtual HigherOrderInvokeExpr* createHigherOrderInvokeExpr(SemanticsVisitor* semantics) = 0;
        virtual void fillHigherOrderInvokeExpr(HigherOrderInvokeExpr* resultDiffExpr, SemanticsVisitor* semantics, Expr* funcExpr) = 0;
        FuncType* getBaseFunctionType(SemanticsVisitor* semantics, Expr* funcExpr)
        {
            if (auto funcType = as<FuncType>(funcExpr->type.type))
                return funcType;
            auto astBuilder = semantics->getASTBuilder();
            if (auto declRefExpr = as<DeclRefExpr>(funcExpr))
            {
                if (auto baseFuncGenericDeclRef = declRefExpr->declRef.as<GenericDecl>())
                {
                    // Get inner function
                    DeclRef<Decl> unspecializedInnerRef = createDefaultSubstitutionsIfNeeded(astBuilder, semantics,
                        astBuilder->getMemberDeclRef(baseFuncGenericDeclRef, getInner(baseFuncGenericDeclRef)));
                    auto callableDeclRef = unspecializedInnerRef.as<CallableDecl>();
                    if (!callableDeclRef)
                        return nullptr;
                    auto funcType = getFuncType(astBuilder, callableDeclRef);
                    return funcType;
                }
            }
            return nullptr;
        }
    };

    struct ForwardDifferentiateExprCheckingActions : HigherOrderInvokeExprCheckingActions
    {
        virtual HigherOrderInvokeExpr* createHigherOrderInvokeExpr(SemanticsVisitor* semantics) override
        {
            return semantics->getASTBuilder()->create<ForwardDifferentiateExpr>();
        }
        void fillHigherOrderInvokeExpr(HigherOrderInvokeExpr* resultDiffExpr, SemanticsVisitor* semantics, Expr* funcExpr) override
        {
            resultDiffExpr->baseFunction = funcExpr;
            auto baseFuncType = getBaseFunctionType(semantics, funcExpr);
            if (!baseFuncType)
            {
                resultDiffExpr->type = semantics->getASTBuilder()->getErrorType();
                semantics->getSink()->diagnose(funcExpr, Diagnostics::expectedFunction, funcExpr->type.type);
                return;
            }
            resultDiffExpr->type = semantics->getForwardDiffFuncType(baseFuncType);
            if (auto declRefExpr = as<DeclRefExpr>(getInnerMostExprFromHigherOrderExpr(funcExpr)))
            {
                auto funcDecl = declRefExpr->declRef.as<CallableDecl>().getDecl();
                if (auto genDecl = as<GenericDecl>(declRefExpr->declRef.getDecl()))
                {
                    funcDecl = as<CallableDecl>(genDecl->inner);
                }
                if (funcDecl)
                {
                    for (auto param : funcDecl->getParameters())
                    {
                        resultDiffExpr->newParameterNames.add(param->getName());
                    }
                }
            }
        }
    };

    struct BackwardDifferentiateExprCheckingActions : HigherOrderInvokeExprCheckingActions
    {
        virtual HigherOrderInvokeExpr* createHigherOrderInvokeExpr(SemanticsVisitor* semantics) override
        {
            return semantics->getASTBuilder()->create<BackwardDifferentiateExpr>();
        }
        void fillHigherOrderInvokeExpr(HigherOrderInvokeExpr* resultDiffExpr, SemanticsVisitor* semantics, Expr* funcExpr) override
        {
            resultDiffExpr->baseFunction = funcExpr;
            auto baseFuncType = getBaseFunctionType(semantics, funcExpr);
            if (!baseFuncType)
            {
                resultDiffExpr->type = semantics->getASTBuilder()->getErrorType();
                semantics->getSink()->diagnose(funcExpr, Diagnostics::expectedFunction, funcExpr->type.type);
                return;
            }
            resultDiffExpr->type = semantics->getBackwardDiffFuncType(baseFuncType);
            if (auto declRefExpr = as<DeclRefExpr>(getInnerMostExprFromHigherOrderExpr(funcExpr)))
            {
                auto funcDecl = declRefExpr->declRef.as<CallableDecl>().getDecl();
                if (auto genDecl = as<GenericDecl>(declRefExpr->declRef.getDecl()))
                {
                    funcDecl = as<CallableDecl>(genDecl->inner);
                }
                if (funcDecl)
                {
                    for (auto param : funcDecl->getParameters())
                    {
                        if (param->findModifier<NoDiffModifier>())
                        {
                            if (param->findModifier<OutModifier>() &&
                                !param->findModifier<InModifier>() &&
                                !param->findModifier<InOutModifier>())
                                continue;
                        }
                        resultDiffExpr->newParameterNames.add(param->getName());
                    }
                    resultDiffExpr->newParameterNames.add(semantics->getName("resultGradient"));
                }
            }
        }
    };

    template<typename ExprASTType>
    struct PassthroughHighOrderExprCheckingActionsBase : HigherOrderInvokeExprCheckingActions
    {
        virtual HigherOrderInvokeExpr* createHigherOrderInvokeExpr(SemanticsVisitor* semantics) override
        {
            return semantics->getASTBuilder()->create<ExprASTType>();
        }
        void fillHigherOrderInvokeExpr(HigherOrderInvokeExpr* resultDiffExpr, SemanticsVisitor* semantics, Expr* funcExpr) override
        {
            resultDiffExpr->baseFunction = funcExpr;
            auto baseFuncType = getBaseFunctionType(semantics, funcExpr);
            if (!baseFuncType)
            {
                resultDiffExpr->type = semantics->getASTBuilder()->getErrorType();
                semantics->getSink()->diagnose(funcExpr, Diagnostics::expectedFunction, funcExpr->type.type);
                return;
            }
            resultDiffExpr->type = baseFuncType;
            if (auto declRefExpr = as<DeclRefExpr>(getInnerMostExprFromHigherOrderExpr(funcExpr)))
            {
                auto funcDecl = declRefExpr->declRef.as<CallableDecl>().getDecl();
                if (auto genDecl = as<GenericDecl>(declRefExpr->declRef.getDecl()))
                {
                    funcDecl = as<CallableDecl>(genDecl->inner);
                }
                if (funcDecl)
                {
                    for (auto param : funcDecl->getParameters())
                    {
                        resultDiffExpr->newParameterNames.add(param->getName());
                    }
                }
            }
        }
    };

    static Expr* _checkHigherOrderInvokeExpr(
        SemanticsVisitor* semantics,
        HigherOrderInvokeExpr* expr,
        HigherOrderInvokeExprCheckingActions* actions)
    {
        // Check/Resolve inner function declaration.
        expr->baseFunction = semantics->CheckTerm(expr->baseFunction);

        auto astBuilder = semantics->getASTBuilder();

        // If base is overloaded expr, we want to return an overloaded expr as check result.
        // This is done by pushing the `differentiate` operator to each item in the overloaded expr.
        if (auto overloadedExpr = as<OverloadedExpr>(expr->baseFunction))
        {
            OverloadedExpr2* result = astBuilder->create<OverloadedExpr2>();
            for (auto item : overloadedExpr->lookupResult2)
            {
                auto lookupResultExpr = semantics->ConstructLookupResultExpr(item,
                    nullptr,
                    overloadedExpr->loc,
                    nullptr);
                auto candidateExpr = actions->createHigherOrderInvokeExpr(semantics);
                actions->fillHigherOrderInvokeExpr(candidateExpr, semantics, lookupResultExpr);
                candidateExpr->loc = expr->loc;
                result->candidiateExprs.add(candidateExpr);
            }
            result->type.type = astBuilder->getOverloadedType();
            result->loc = expr->loc;
            return result;
        }
        else if (auto overloadedExpr2 = as<OverloadedExpr2>(expr->baseFunction))
        {
            OverloadedExpr2* result = astBuilder->create<OverloadedExpr2>();
            for (auto item : overloadedExpr2->candidiateExprs)
            {
                auto candidateExpr = actions->createHigherOrderInvokeExpr(semantics);
                actions->fillHigherOrderInvokeExpr(candidateExpr, semantics, item);
                candidateExpr->loc = expr->loc;
                result->candidiateExprs.add(candidateExpr);
            }
            result->type.type = astBuilder->getOverloadedType();
            result->loc = expr->loc;
            return result;
        }

        actions->fillHigherOrderInvokeExpr(expr, semantics, expr->baseFunction);
        return expr;
    }

    Expr* SemanticsExprVisitor::visitForwardDifferentiateExpr(ForwardDifferentiateExpr* expr)
    {
        ForwardDifferentiateExprCheckingActions actions;
        return _checkHigherOrderInvokeExpr(this, expr, &actions);
    }

    Expr* SemanticsExprVisitor::visitBackwardDifferentiateExpr(BackwardDifferentiateExpr* expr)
    {
        BackwardDifferentiateExprCheckingActions actions;
        return _checkHigherOrderInvokeExpr(this, expr, &actions);
    }

    Expr* SemanticsExprVisitor::visitPrimalSubstituteExpr(PrimalSubstituteExpr* expr)
    {
        PassthroughHighOrderExprCheckingActionsBase<PrimalSubstituteExpr> actions;
        return _checkHigherOrderInvokeExpr(this, expr, &actions);
    }

    Expr* SemanticsExprVisitor::visitDispatchKernelExpr(DispatchKernelExpr* expr)
    {
        auto isInt3Type = [this](Type* type)
        {
            auto vectorType = as<VectorExpressionType>(type);
            if (!vectorType)
                return false;
            if (!isIntegerBaseType(getVectorBaseType(vectorType)))
                return false;
            auto constElementCount = as<ConstantIntVal>(vectorType->getElementCount());
            if (!constElementCount)
                return false;
            return constElementCount->getValue() == 3;
        };
        expr->threadGroupSize = dispatchExpr(expr->threadGroupSize, *this);
        if (!isInt3Type(expr->threadGroupSize->type.type))
        {
            getSink()->diagnose(
                expr->threadGroupSize,
                Diagnostics::typeMismatch,
                "uint3",
                expr->threadGroupSize->type);
        }
        expr->dispatchSize = dispatchExpr(expr->dispatchSize, *this);
        if (!isInt3Type(expr->dispatchSize->type.type))
        {
            getSink()->diagnose(
                expr->dispatchSize,
                Diagnostics::typeMismatch,
                "uint3",
                expr->dispatchSize->type);
        }
        PassthroughHighOrderExprCheckingActionsBase<DispatchKernelExpr> actions;
        return _checkHigherOrderInvokeExpr(this, expr, &actions);
    }

    Expr* SemanticsExprVisitor::visitTreatAsDifferentiableExpr(TreatAsDifferentiableExpr* expr)
    {
        auto subContext = withTreatAsDifferentiable(expr);
        expr->innerExpr = dispatchExpr(expr->innerExpr, subContext);
        expr->type = expr->innerExpr->type;
        auto innerExpr = expr->innerExpr;
        while (auto parenExpr = as<ParenExpr>(innerExpr))
        {
            innerExpr = parenExpr->base;
        }
        if (!as<InvokeExpr>(innerExpr) && !as<IndexExpr>(innerExpr))
        {
            getSink()->diagnose(expr, Diagnostics::invalidUseOfNoDiff);
        }
        else if (!m_parentDifferentiableAttr)
        {
            getSink()->diagnose(expr, Diagnostics::cannotUseNoDiffInNonDifferentiableFunc);
        }
        return expr;
    }

    Expr* SemanticsExprVisitor::visitGetArrayLengthExpr(GetArrayLengthExpr* expr)
    {
        expr->arrayExpr = CheckTerm(expr->arrayExpr);
        if (auto arrType = as<ArrayExpressionType>(expr->arrayExpr->type))
        {
            expr->type = m_astBuilder->getIntType();
            if (arrType->isUnsized())
            {
                getSink()->diagnose(expr, Diagnostics::invalidArraySize);
            }
        }
        else
        {
            if (!as<ErrorType>(expr->arrayExpr->type))
            {
                getSink()->diagnose(
                    expr, Diagnostics::typeMismatch, "array", expr->arrayExpr->type);
            }
            expr->type = m_astBuilder->getErrorType();
        }
        return expr;
    }

    static bool _isSizeOfType(Type* type)
    {
        if (!type)
        {
            return false;
        }

        if (as<ArithmeticExpressionType>(type) ||
            as<ArrayExpressionType>(type) ||
            as<PtrTypeBase>(type) ||
            as<TupleType>(type) ||
            as<GenericDeclRefType>(type))
        {
            return true;
        }

        if (as<DeclRefType>(type))
        {
            return true;
        }
        
        return false;
    }

    Expr* SemanticsExprVisitor::visitSizeOfLikeExpr(SizeOfLikeExpr* sizeOfLikeExpr)
    {
        auto valueExpr = dispatch(sizeOfLikeExpr->value);
        
        Type* type = nullptr;

        if (as<TypeType>(valueExpr->type))
        {
            TypeExp typeExp;
            typeExp.exp = valueExpr;

            auto properTypeExpr = CoerceToProperType(typeExp);

            type = properTypeExpr.type;
        }
        else
        {
            // Is this a proper type?
            TypeExp typeExp(valueExpr->type);
            TypeExp properType = tryCoerceToProperType(typeExp);

            type = properType.type;
        }

        if (!_isSizeOfType(type))
        {
            getSink()->diagnose(sizeOfLikeExpr, Diagnostics::sizeOfArgumentIsInvalid);

            sizeOfLikeExpr->type = m_astBuilder->getErrorType();
            return sizeOfLikeExpr;
        }

        sizeOfLikeExpr->sizedType = type;

        return sizeOfLikeExpr;
    }

    Expr* SemanticsExprVisitor::visitTypeCastExpr(TypeCastExpr * expr)
    {
        if (expr->type)
            return expr;

        // Check the term we are applying first
        auto funcExpr = expr->functionExpr;
        funcExpr = CheckTerm(funcExpr);

        // Now ensure that the term represents a (proper) type.
        TypeExp typeExp;
        typeExp.exp = funcExpr;
        typeExp = CheckProperType(typeExp);

        expr->functionExpr = typeExp.exp;
        expr->type.type = typeExp.type;

        // Next check the argument expression (there should be only one)
        for (auto & arg : expr->arguments)
        {
            arg = CheckTerm(arg);
        }

        // LEGACY FEATURE: As a backwards-compatibility feature
        // for HLSL, we will allow for a cast to a `struct` type
        // from a literal zero, with the semantics of default
        // initialization.
        //
        if( auto declRefType = as<DeclRefType>(typeExp.type) )
        {
            if(const auto structDeclRef = as<StructDecl>(declRefType->getDeclRef()))
            {
                if( expr->arguments.getCount() == 1 )
                {
                    auto arg = expr->arguments[0];
                    if( auto intLitArg = as<IntegerLiteralExpr>(arg) )
                    {
                        if(getIntegerLiteralValue(intLitArg->token) == 0)
                        {
                            // At this point we have confirmed that the cast
                            // has the right form, so we want to apply our special case.
                            //
                            // TODO: If/when we allow for user-defined initializer/constructor
                            // definitions we would have to be careful here because it is
                            // possible that the target type has defined an initializer/constructor
                            // that takes a single `int` parmaeter and means to call that instead.
                            //
                            // For now that should be a non-issue, and in a pinch such a user
                            // could use `T(0)` instead of `(T) 0` to get around this special
                            // HLSL legacy feature.

                            // We will type-check code like:
                            //
                            //      MyStruct s = (MyStruct) 0;
                            //
                            // the same as:
                            //
                            //      MyStruct s = {};
                            //
                            // That is, we construct an empty initializer list, and then coerce
                            // that initializer list expression to the desired type (letting
                            // the code for handling initializer lists work out all of the
                            // details of what is/isn't valid). This choice means we get
                            // to benefit from the existing codegen support for initializer
                            // lists, rather than needing the `(MyStruct) 0` idiom to be
                            // special-cased in later stages of the compiler.
                            //
                            // Note: we use an empty initializer list `{}` instead of an
                            // initializer list with a single zero `{0}`, which is semantically
                            // significant if the first field of `MyStruct` had its own
                            // default initializer defined as part of the `struct` definition.
                            // Basically we have chosen to interpret the "cast from zero" syntax
                            // as sugar for default initialization, and *not* specifically
                            // for zero-initialization. That choice could be revisited if
                            // users express displeasure. For now there isn't enough usage
                            // of explicit default initializers for `struct` fields to
                            // make this a major concern (since they aren't supported in HLSL).
                            //
                            InitializerListExpr* initListExpr = m_astBuilder->create<InitializerListExpr>();
                            initListExpr->loc = expr->loc;
                            auto checkedInitListExpr = visitInitializerListExpr(initListExpr);

                            return coerce(CoercionSite::General, typeExp.type, checkedInitListExpr);
                        }
                    }
                }
            }
        }


        // Now process this like any other explicit call (so casts
        // and constructor calls are semantically equivalent).
        return CheckInvokeExprWithCheckedOperands(expr);
    }

    Expr* SemanticsExprVisitor::visitTryExpr(TryExpr* expr)
    {
        auto prevTryClauseType = m_enclosingTryClauseType;
        m_enclosingTryClauseType = expr->tryClauseType;
        expr->base = CheckTerm(expr->base);
        m_enclosingTryClauseType = prevTryClauseType;
        expr->type = expr->base->type;
        if (as<ErrorType>(expr->type))
            return expr;
        
        auto parentFunc = this->m_parentFunc;
        // TODO: check if the try clause is caught.
        // For now we assume all `try`s are not caught (because we don't have catch yet).
        if (!parentFunc)
        {
            getSink()->diagnose(expr, Diagnostics::uncaughtTryCallInNonThrowFunc);
            return expr;
        }
        if (parentFunc->errorType->equals(m_astBuilder->getBottomType()))
        {
            getSink()->diagnose(expr, Diagnostics::uncaughtTryCallInNonThrowFunc);
            return expr;
        }
        if (!as<InvokeExpr>(expr->base))
        {
            getSink()->diagnose(expr, Diagnostics::tryClauseMustApplyToInvokeExpr);
            return expr;
        }
        auto base = as<InvokeExpr>(expr->base);
        if (auto callee = as<DeclRefExpr>(base->functionExpr))
        {
            if (auto funcCallee = as<FuncDecl>(callee->declRef.getDecl()))
            {
                if (funcCallee->errorType->equals(m_astBuilder->getBottomType()))
                {
                    getSink()->diagnose(expr, Diagnostics::tryInvokeCalleeShouldThrow, callee->declRef);
                }
                if (!parentFunc->errorType->equals(funcCallee->errorType))
                {
                    getSink()->diagnose(
                        expr,
                        Diagnostics::errorTypeOfCalleeIncompatibleWithCaller,
                        callee->declRef,
                        funcCallee->errorType,
                        parentFunc->errorType);
                }
                return expr;
            }
        }
        getSink()->diagnose(expr, Diagnostics::calleeOfTryCallMustBeFunc);
        return expr;
    }

    Expr* SemanticsExprVisitor::visitIsTypeExpr(IsTypeExpr* expr)
    {
        expr->typeExpr = CheckProperType(expr->typeExpr);
        auto originalVal = CheckTerm(expr->value);
        expr->type = m_astBuilder->getBoolType();
        expr->value = originalVal;

        // If value is a subtype of `type`, then this expr is always true.
        if(isSubtype(expr->value->type.type, expr->typeExpr.type))
        {
            // Instead of returning a BoolLiteralExpr, we use a field to indicate this scenario,
            // so that the language server can still see the original syntax tree.
            expr->constantVal = m_astBuilder->create<BoolLiteralExpr>();
            expr->constantVal->type = m_astBuilder->getBoolType();
            expr->constantVal->value = true;
            expr->constantVal->loc = expr->loc;
            return expr;
        }

        // Otherwise, we need to ensure the target type is a subtype of value->type.

        expr->value = maybeOpenExistential(originalVal);
        expr->witnessArg = tryGetSubtypeWitness(expr->typeExpr.type, originalVal->type.type);
        if (expr->witnessArg)
        {
            // For now we can only support the scenario where `expr->value` is an interface type.
            if (!isInterfaceType(originalVal->type))
            {
                getSink()->diagnose(expr, Diagnostics::isOperatorValueMustBeInterfaceType);
            }
            return expr;
        }

        if (!as<ErrorType>(expr->typeExpr.type) && !as<ErrorType>(expr->value->type.type))
        {
            // The type is not in the same hierarchy, so we evaluate to false.
            expr->constantVal = m_astBuilder->create<BoolLiteralExpr>();
            expr->constantVal->type = m_astBuilder->getBoolType();
            expr->constantVal->value = false;
            expr->constantVal->loc = expr->loc;
        }
        return expr;
    }

    Expr* SemanticsExprVisitor::visitAsTypeExpr(AsTypeExpr* expr)
    {
        TypeExp typeExpr;
        typeExpr.exp = expr->typeExpr;
        typeExpr = CheckProperType(typeExpr);
        expr->value = CheckTerm(expr->value);
        auto optType = m_astBuilder->getOptionalType(typeExpr.type);
        expr->type = optType;

        // If value is a subtype of `type`, then this expr is equivalent to a CastToSuperTypeExpr.
        if (auto witness = tryGetSubtypeWitness(expr->value->type.type, typeExpr.type))
        {
            auto castToSuperType = createCastToSuperTypeExpr(typeExpr.type, expr->value, witness);
            auto makeOptional = m_astBuilder->create<MakeOptionalExpr>();
            makeOptional->loc = expr->loc;
            makeOptional->type = optType;
            makeOptional->value = castToSuperType;
            makeOptional->typeExpr = typeExpr.exp;
            return makeOptional;
        }

        // For now we can only support the scenario where `expr->value` is an interface type.
        if (!isInterfaceType(expr->value->type))
        {
            getSink()->diagnose(expr, Diagnostics::isOperatorValueMustBeInterfaceType);
        }

        expr->typeExpr = typeExpr.exp;
        expr->witnessArg = tryGetSubtypeWitness(typeExpr.type, expr->value->type.type);
        if (expr->witnessArg)
        {
            expr->value = maybeOpenExistential(expr->value);
            return expr;
        }

        if (!as<ErrorType>(typeExpr.type) && !as<ErrorType>(expr->value->type.type))
        {
            getSink()->diagnose(expr, Diagnostics::typeNotInTheSameHierarchy, expr->value->type.type, typeExpr.type);
        }

        expr->type = m_astBuilder->getErrorType();
        
        return expr;
    }

    Expr* SemanticsVisitor::MaybeDereference(Expr* inExpr)
    {
        Expr* expr = inExpr;
        for (;;)
        {
            auto baseType = expr->type;
            if (auto pointerLikeType = as<PointerLikeType>(baseType))
            {
                auto elementType = QualType(pointerLikeType->getElementType());
                elementType.isLeftValue = baseType.isLeftValue;

                auto derefExpr = m_astBuilder->create<DerefExpr>();
                derefExpr->base = expr;
                derefExpr->type = elementType;

                expr = derefExpr;
                continue;
            }

            // Default case: just use the expression as-is
            return expr;
        }
    }

    Expr* SemanticsVisitor::CheckMatrixSwizzleExpr(
        MemberExpr* memberRefExpr,
        Type*      baseElementType,
        IntegerLiteralValue baseElementRowCount,
        IntegerLiteralValue baseElementColCount)
    {
        MatrixSwizzleExpr* swizExpr = m_astBuilder->create<MatrixSwizzleExpr>();
        swizExpr->loc = memberRefExpr->loc;
        swizExpr->base = memberRefExpr->baseExpression;
        swizExpr->memberOpLoc = memberRefExpr->memberOperatorLoc;

        // We can have up to 4 swizzles of two elements each
        MatrixCoord elementCoords[4];
        int elementCount = 0;

        bool anyDuplicates = false;
        int zeroIndexOffset = -1;

        if (memberRefExpr->name == getSession()->getCompletionRequestTokenName())
        {
            auto& suggestions = getLinkage()->contentAssistInfo.completionSuggestions;
            suggestions.clear();
            suggestions.scopeKind = CompletionSuggestions::ScopeKind::Swizzle;
            suggestions.swizzleBaseType =
                memberRefExpr->baseExpression ? memberRefExpr->baseExpression->type : nullptr;
            suggestions.elementCount[0] = baseElementRowCount;
            suggestions.elementCount[1] = baseElementColCount;
        }

        String swizzleText = getText(memberRefExpr->name);
        auto cursor = swizzleText.begin();

        // The contents of the string are 0-terminated
        // Every update to cursor corresponds to a check against 0-termination
        while (*cursor)
        {
            // Throw out swizzling with more than 4 output elements
            if (elementCount >= 4)
            {
                getSink()->diagnose(swizExpr, Diagnostics::invalidSwizzleExpr, swizzleText, baseElementType->toString());
                return CreateErrorExpr(memberRefExpr);
            }
            MatrixCoord elementCoord = { 0, 0 };

            // Check for the preceding underscore
            if (*cursor++ != '_')
            {
                getSink()->diagnose(swizExpr, Diagnostics::invalidSwizzleExpr, swizzleText, baseElementType->toString());
                return CreateErrorExpr(memberRefExpr);
            }

            // Check for one or zero indexing            
            if (*cursor == 'm')
            {
                // Can't mix one and zero indexing
                if (zeroIndexOffset == 1)
                {
                    getSink()->diagnose(swizExpr, Diagnostics::invalidSwizzleExpr, swizzleText, baseElementType->toString());
                    return CreateErrorExpr(memberRefExpr);
                }
                zeroIndexOffset = 0;
                // Increment the index since we saw 'm'
                cursor++;
            }
            else
            {
                // Can't mix one and zero indexing
                if (zeroIndexOffset == 0)
                {
                    getSink()->diagnose(swizExpr, Diagnostics::invalidSwizzleExpr, swizzleText, baseElementType->toString());
                    return CreateErrorExpr(memberRefExpr);
                }
                zeroIndexOffset = 1;
            }

            // Check for the ij components
            for (Index j = 0; j < 2; j++)
            {
                auto ch = *cursor++;
                
                if (ch < '0' || ch > '4')
                {
                    // An invalid character in the swizzle is an error
                    getSink()->diagnose(swizExpr, Diagnostics::invalidSwizzleExpr, swizzleText, baseElementType->toString());
                    return CreateErrorExpr(memberRefExpr);
                }
                const int subIndex = ch - '0' - zeroIndexOffset;

                // Check the limit for either the row or column, depending on the step
                IntegerLiteralValue elementLimit;
                if (j == 0)
                {
                    elementLimit = baseElementRowCount;
                    elementCoord.row = subIndex;
                }
                else
                {
                    elementLimit = baseElementColCount;
                    elementCoord.col = subIndex;
                }
                // Make sure the index is in range for the source type
                // Account for off-by-one and reject 0 if oneIndexed
                if (subIndex >= elementLimit || subIndex < 0)
                {
                    getSink()->diagnose(swizExpr, Diagnostics::invalidSwizzleExpr, swizzleText, baseElementType->toString());
                    return CreateErrorExpr(memberRefExpr);
                }
            }
            // Check if we've seen this index before
            for (int ee = 0; ee < elementCount; ee++)
            {
                if (elementCoords[ee] == elementCoord)
                    anyDuplicates = true;
            }

            // add to our list...
            elementCoords[elementCount] = elementCoord;
            elementCount++;
        }

        // Store our list in the actual AST node
        for (int ee = 0; ee < elementCount; ++ee)
        {
            swizExpr->elementCoords[ee] = elementCoords[ee];
        }
        swizExpr->elementCount = elementCount;

        if (elementCount == 1)
        {
            // single-component swizzle produces a scalar
            //
            // Note(tfoley): the official HLSL rules seem to be that it produces
            // a one-component vector, which is then implicitly convertible to
            // a scalar, but that seems like it just adds complexity.
            swizExpr->type = QualType(baseElementType);
        }
        else
        {
            // TODO(tfoley): would be nice to "re-sugar" type
            // here if the input type had a sugared name...
            swizExpr->type = QualType(createVectorType(
                baseElementType,
                m_astBuilder->getIntVal(m_astBuilder->getIntType(), elementCount)));
        }

        // A swizzle can be used as an l-value as long as there
        // were no duplicates in the list of components
        swizExpr->type.isLeftValue = !anyDuplicates;

        return swizExpr;
    }

    Expr* SemanticsVisitor::CheckMatrixSwizzleExpr(
        MemberExpr* memberRefExpr,
        Type*		baseElementType,
        IntVal*				baseRowCount,
        IntVal*				baseColCount)
    {
        if (auto constantRowCount = as<ConstantIntVal>(baseRowCount))
        {
            if (auto constantColCount = as<ConstantIntVal>(baseColCount))
            {
                return CheckMatrixSwizzleExpr(memberRefExpr, baseElementType,
                    constantRowCount->getValue(), constantColCount->getValue());
            }
        }
        getSink()->diagnose(memberRefExpr, Diagnostics::unimplemented, "swizzle on matrix of unknown size");
        return CreateErrorExpr(memberRefExpr);
    }

    Expr* SemanticsVisitor::CheckSwizzleExpr(
        MemberExpr* memberRefExpr,
        Type*      baseElementType,
        IntegerLiteralValue         baseElementCount)
    {
        SwizzleExpr* swizExpr = m_astBuilder->create<SwizzleExpr>();
        swizExpr->loc = memberRefExpr->loc;
        swizExpr->base = memberRefExpr->baseExpression;
        swizExpr->elementIndices[0] = 0;
        swizExpr->elementIndices[1] = 0;
        swizExpr->elementIndices[2] = 0;
        swizExpr->elementIndices[3] = 0;
        swizExpr->memberOpLoc = memberRefExpr->memberOperatorLoc;
        IntegerLiteralValue limitElement = baseElementCount;

        int elementIndices[4];
        int elementCount = 0;

        bool anyDuplicates = false;
        bool anyError = false;
        if (memberRefExpr->name == getSession()->getCompletionRequestTokenName())
        {
            auto& suggestions = getLinkage()->contentAssistInfo.completionSuggestions;
            suggestions.clear();
            suggestions.scopeKind = CompletionSuggestions::ScopeKind::Swizzle;
            suggestions.swizzleBaseType =
                memberRefExpr->baseExpression ? memberRefExpr->baseExpression->type : nullptr;
            suggestions.elementCount[0] = baseElementCount;
            suggestions.elementCount[1] = 0;
        }
        auto swizzleText = getText(memberRefExpr->name);

        for (Index i = 0; i < swizzleText.getLength(); i++)
        {
            auto ch = swizzleText[i];
            int elementIndex = -1;
            switch (ch)
            {
            case 'x': case 'r': elementIndex = 0; break;
            case 'y': case 'g': elementIndex = 1; break;
            case 'z': case 'b': elementIndex = 2; break;
            case 'w': case 'a': elementIndex = 3; break;
            default:
                // An invalid character in the swizzle is an error
                getSink()->diagnose(swizExpr, Diagnostics::invalidSwizzleExpr, swizzleText, baseElementType->toString());
                anyError = true;
                continue;
            }

            // TODO(tfoley): GLSL requires that all component names
            // come from the same "family"...

            // Make sure the index is in range for the source type
            if (elementIndex >= limitElement)
            {
                getSink()->diagnose(swizExpr, Diagnostics::invalidSwizzleExpr, swizzleText, baseElementType->toString());
                anyError = true;
                continue;
            }

            // Check if we've seen this index before
            for (int ee = 0; ee < elementCount; ee++)
            {
                if (elementIndices[ee] == elementIndex)
                    anyDuplicates = true;
            }

            // add to our list...
            elementIndices[elementCount++] = elementIndex;
        }

        for (int ee = 0; ee < elementCount; ++ee)
        {
            swizExpr->elementIndices[ee] = elementIndices[ee];
        }
        swizExpr->elementCount = elementCount;

        if (anyError)
        {
            return CreateErrorExpr(memberRefExpr);
        }
        else if (elementCount == 1)
        {
            // single-component swizzle produces a scalar
            //
            // Note(tfoley): the official HLSL rules seem to be that it produces
            // a one-component vector, which is then implicitly convertible to
            // a scalar, but that seems like it just adds complexity.
            swizExpr->type = QualType(baseElementType);
        }
        else
        {
            // TODO(tfoley): would be nice to "re-sugar" type
            // here if the input type had a sugared name...
            swizExpr->type = QualType(createVectorType(
                baseElementType,
                m_astBuilder->getIntVal(m_astBuilder->getIntType(), elementCount)));
        }

        // A swizzle can be used as an l-value as long as there
        // were no duplicates in the list of components
        swizExpr->type.isLeftValue = !anyDuplicates &&
            swizExpr->base &&
            swizExpr->base->type &&
            swizExpr->base->type.isLeftValue;

        return swizExpr;
    }

    Expr* SemanticsVisitor::CheckSwizzleExpr(
        MemberExpr*	memberRefExpr,
        Type*		baseElementType,
        IntVal*				baseElementCount)
    {
        if (auto constantElementCount = as<ConstantIntVal>(baseElementCount))
        {
            return CheckSwizzleExpr(memberRefExpr, baseElementType, constantElementCount->getValue());
        }
        else
        {
            getSink()->diagnose(memberRefExpr, Diagnostics::unimplemented, "swizzle on vector of unknown size");
            return CreateErrorExpr(memberRefExpr);
        }
    }

    Expr* SemanticsVisitor::_lookupStaticMember(DeclRefExpr* expr, Expr* baseExpression)
    {
        auto& baseType = baseExpression->type;

        // TODO: Need to handle overloaded case (in case we
        // have multiple visible types and/or namespaces
        // with the same name).

        if (auto namespaceType = as<NamespaceType>(baseType))
        {
            // We are looking up a namespace member.
            //
            auto namespaceDeclRef = namespaceType->getDeclRef();

            // This ought to be the easy case, because
            // there are no restrictions on whether
            // we can reference the declaration here.
            //
            LookupResult lookupResult = lookUpDirectAndTransparentMembers(
                m_astBuilder,
                this,
                expr->name,
                namespaceDeclRef.getDecl(),
                namespaceDeclRef);
            if (!lookupResult.isValid())
            {
                return lookupMemberResultFailure(expr, baseType);
            }

            if (expr->name == getSession()->getCompletionRequestTokenName())
            {
                suggestCompletionItems(CompletionSuggestions::ScopeKind::Member, lookupResult);
            }
            return createLookupResultExpr(
                expr->name,
                lookupResult,
                nullptr,
                expr->loc,
                expr);
        }
        else if (auto typeType = as<TypeType>(baseType))
        {
            // We are looking up a member inside a type.
            // We want to be careful here because we should only find members
            // that are implicitly or explicitly `static`.
            //
            // TODO: this duplicates a *lot* of logic with the case below.
            // We need to fix that.
            auto type = typeType->getType();

            if (as<ErrorType>(type))
            {
                return CreateErrorExpr(expr);
            }

            LookupResult lookupResult = lookUpMember(
                m_astBuilder,
                this,
                expr->name,
                type);
            if (!lookupResult.isValid())
            {
                return lookupMemberResultFailure(expr, baseType);
            }

            // We need to confirm that whatever member we
            // are trying to refer to is usable via static reference.
            //
            // TODO: eventually we might allow a non-static
            // member to be adapted by turning it into something
            // like a closure that takes the missing `this` parameter.
            //
            // E.g., a static reference to a method could be treated
            // as a value with a function type, where the first parameter
            // is `type`.
            //
            // The biggest challenge there is that we'd need to arrange
            // to generate "dispatcher" functions that could be used
            // to implement that function, in the case where we are
            // making a static reference to some kind of polymorphic declaration.
            //
            // (Also, static references to fields/properties would get even
            // harder, because you'd have to know whether a getter/setter/ref-er
            // is needed).
            //
            // For now let's just be expedient and disallow all of that, because
            // we can always add it back in later.

            // If the lookup result is valid, then we want to filter
            // it to just those candidates that can be referenced statically,
            // and ignore any that would only be allowed as instance members.
            //
            if(lookupResult.isValid())
            {
                // We track both the usable items, and whether or
                // not there were any non-static items that need
                // to be ignored.
                //
                bool anyNonStatic = false;
                List<LookupResultItem> staticItems;
                for (auto item : lookupResult)
                {
                    // Is this item usable as a static member?
                    if (isUsableAsStaticMember(item))
                    {
                        // If yes, then it will be part of the output.
                        staticItems.add(item);
                    }
                    else
                    {
                        // If no, then we might need to output an error.
                        anyNonStatic = true;
                    }
                }

                // Was there anything non-static in the list?
                if (anyNonStatic)
                {
                    // If we had some static items, then that's okay,
                    // we just want to use our newly-filtered list.
                    if (staticItems.getCount())
                    {
                        lookupResult.items = staticItems;
                        lookupResult.item = staticItems[0];
                    }
                    else
                    {
                        // Otherwise, it is time to report an error.
                        getSink()->diagnose(
                            expr->loc,
                            Diagnostics::staticRefToNonStaticMember,
                            type,
                            expr->name);
                        return CreateErrorExpr(expr);
                    }
                }
                // If there were no non-static items, then the `items`
                // array already represents what we'd get by filtering...
            }
            if (expr->name == getSession()->getCompletionRequestTokenName())
            {
                suggestCompletionItems(CompletionSuggestions::ScopeKind::Member, lookupResult);
            }
            return createLookupResultExpr(
                expr->name,
                lookupResult,
                baseExpression,
                expr->loc,
                expr);
        }
        else if (as<ErrorType>(baseType))
        {
            return CreateErrorExpr(expr);
        }

        // Failure
        return lookupMemberResultFailure(expr, baseType);
    }

    Expr* SemanticsExprVisitor::visitStaticMemberExpr(StaticMemberExpr* expr)
    {
        expr->baseExpression = CheckTerm(expr->baseExpression);

        // Not sure this is needed -> but guess someone could do 
        expr->baseExpression = MaybeDereference(expr->baseExpression);

        // If the base of the member lookup has an interface type
        // *without* a suitable this-type substitution, then we are
        // trying to perform lookup on a value of existential type,
        // and we should "open" the existential here so that we
        // can expose its structure.
        //

        expr->baseExpression = maybeOpenExistential(expr->baseExpression);
        // Do a static lookup
        return _lookupStaticMember(expr, expr->baseExpression);
    }

    Expr* SemanticsVisitor::lookupMemberResultFailure(
        DeclRefExpr*     expr,
        QualType const& baseType)
    {
        // Check it's a member expression
        SLANG_ASSERT(as<StaticMemberExpr>(expr) || as<MemberExpr>(expr));

        getSink()->diagnose(expr, Diagnostics::noMemberOfNameInType, expr->name, baseType);
        expr->type = QualType(m_astBuilder->getErrorType());
        return expr;
    }

    Expr* SemanticsVisitor::checkBaseForMemberExpr(Expr* inBaseExpr)
    {
        auto baseExpr = inBaseExpr;

        baseExpr = CheckTerm(baseExpr);

        baseExpr = MaybeDereference(baseExpr);

        // If the base of the member lookup has an interface type
        // *without* a suitable this-type substitution, then we are
        // trying to perform lookup on a value of existential type,
        // and we should "open" the existential here so that we
        // can expose its structure.
        //
        baseExpr = maybeOpenExistential(baseExpr);

        // Handle the case of an overloaded base expression
        // here, in case we can use the name of the member to
        // disambiguate which of the candidates is meant, or if
        // we can return an overloaded result.
        if (auto overloadedExpr = as<OverloadedExpr>(baseExpr))
        {
            // If a member (dynamic or static) lookup result contains both the actual definition
            // and the interface definition obtained from inheritance, we want to filter out
            // the interface definitions.
            LookupResult filteredLookupResult;
            for (auto lookupResult : overloadedExpr->lookupResult2)
            {
                bool shouldRemove = false;
                if (lookupResult.declRef.getParent().as<InterfaceDecl>())
                {
                    shouldRemove = true;
                }
                if (lookupResult.declRef.getDecl()->hasModifier<ExtensionExternVarModifier>())
                    shouldRemove = true;
                if (!shouldRemove)
                {
                    filteredLookupResult.items.add(lookupResult);
                }
            }
            if (filteredLookupResult.items.getCount() == 1)
                filteredLookupResult.item = filteredLookupResult.items.getFirst();
            baseExpr = createLookupResultExpr(
                overloadedExpr->name,
                filteredLookupResult,
                overloadedExpr->base,
                overloadedExpr->loc,
                overloadedExpr);
            // TODO: handle other cases of OverloadedExpr that need filtering.
        }

        return baseExpr;
    }

    Expr* SemanticsExprVisitor::visitMemberExpr(MemberExpr * expr)
    {
        expr->baseExpression = checkBaseForMemberExpr(expr->baseExpression);
        auto baseType = expr->baseExpression->type;

        // If we are looking up through a modified type, just pass straight
        // through the inner type.
        if (auto modifiedType = as<ModifiedType>(baseType))
            baseType = modifiedType->getBase();

        // Note: Checking for vector types before declaration-reference types,
        // because vectors are also declaration reference types...
        //
        // Also note: the way this is done right now means that the ability
        // to swizzle vectors interferes with any chance o<f looking up
        // members via extension, for vector or scalar types.
        //
        // TODO: Matrix swizzles probably need to be handled at some point.
        if (auto baseMatrixType = as<MatrixExpressionType>(baseType))
        {
            return CheckMatrixSwizzleExpr(
                expr,
                baseMatrixType->getElementType(),
                baseMatrixType->getRowCount(),
                baseMatrixType->getColumnCount());
        }
        if (auto baseVecType = as<VectorExpressionType>(baseType))
        {
            return CheckSwizzleExpr(
                expr,
                baseVecType->getElementType(),
                baseVecType->getElementCount());
        }
        else if(auto baseScalarType = as<BasicExpressionType>(baseType))
        {
            // Treat scalar like a 1-element vector when swizzling
            return CheckSwizzleExpr(
                expr,
                baseScalarType,
                1);
        }
        else if( as<NamespaceType>(baseType) )
        {
            return _lookupStaticMember(expr, expr->baseExpression);
        }
        else if(const auto typeType = as<TypeType>(baseType))
        {
            return _lookupStaticMember(expr, expr->baseExpression);
        }
        else if (as<ErrorType>(baseType))
        {
            return CreateErrorExpr(expr);
        }
        else
        {
            LookupResult lookupResult = lookUpMember(
                m_astBuilder,
                this,
                expr->name,
                baseType.Ptr());
            if (!lookupResult.isValid())
            {
                return lookupMemberResultFailure(expr, baseType);
            }
            if (expr->name == getSession()->getCompletionRequestTokenName())
            {
                suggestCompletionItems(CompletionSuggestions::ScopeKind::Member, lookupResult);
            }
            return createLookupResultExpr(
                expr->name,
                lookupResult,
                expr->baseExpression,
                expr->loc,
                expr);
        }
    }

    Expr* SemanticsExprVisitor::visitInitializerListExpr(InitializerListExpr* expr)
    {
        // When faced with an initializer list, we first just check the sub-expressions blindly.
        // Actually making them conform to a desired type will wait for when we know the desired
        // type based on context.

        for( auto& arg : expr->args )
        {
            arg = CheckTerm(arg);
        }

        expr->type = m_astBuilder->getInitializerListType();

        return expr;
    }

    // Perform semantic checking of an object-oriented `this`
    // expression.
    Expr* SemanticsExprVisitor::visitThisExpr(ThisExpr* expr)
    {
        // A `this` expression will default to immutable.
        expr->type.isLeftValue = false;

        // We will do an upwards search starting in the current
        // scope, looking for a surrounding type (or `extension`)
        // declaration that could be the referrant of the expression.
        auto scope = expr->scope;
        while (scope)
        {
            auto containerDecl = scope->containerDecl;

            if( const auto ctorDecl = as<ConstructorDecl>(containerDecl) )
            {
                expr->type.isLeftValue = true;
            }
            else if( const auto setterDecl = as<SetterDecl>(containerDecl) )
            {
                expr->type.isLeftValue = true;
            }
            else if( auto funcDeclBase = as<FunctionDeclBase>(containerDecl) )
            {
                if( funcDeclBase->hasModifier<MutatingAttribute>() )
                {
                    expr->type.isLeftValue = true;
                }
            }
            else if( auto typeOrExtensionDecl = as<AggTypeDeclBase>(containerDecl) )
            {
                expr->type.type = calcThisType(makeDeclRef(typeOrExtensionDecl));
                return expr;
            }
#if 0
            else if (auto aggTypeDecl = as<AggTypeDecl>(containerDecl))
            {
                ensureDecl(aggTypeDecl, DeclCheckState::CanUseAsType);

                // Okay, we are using `this` in the context of an
                // aggregate type, so the expression should be
                // of the corresponding type.
                expr->type.type = DeclRefType::Create(
                    getSession(),
                    makeDeclRef(aggTypeDecl));
                return expr;
            }
            else if (auto extensionDecl = as<ExtensionDecl>(containerDecl))
            {
                ensureDecl(extensionDecl, DeclCheckState::CanUseExtensionTargetType);

                // When `this` is used in the context of an `extension`
                // declaration, then it should refer to an instance of
                // the type being extended.
                //
                // TODO: There is potentially a small gotcha here that
                // lookup through such a `this` expression should probably
                // prioritize members declared in the current extension
                // if there are multiple extensions in scope that add
                // members with the same name...
                //
                expr->type.type = extensionDecl->targetType.type;
                return expr;
            }
#endif

            scope = scope->parent;
        }

        getSink()->diagnose(expr, Diagnostics::thisExpressionOutsideOfTypeDecl);
        return CreateErrorExpr(expr);
    }

    Expr* SemanticsExprVisitor::visitThisTypeExpr(ThisTypeExpr* expr)
    {
        auto scope = expr->scope;
        while (scope)
        {
            auto containerDecl = scope->containerDecl;
            if( auto typeOrExtensionDecl = as<AggTypeDeclBase>(containerDecl) )
            {
                auto thisType = calcThisType(makeDeclRef(typeOrExtensionDecl));
                auto thisTypeType = m_astBuilder->getTypeType(thisType);

                expr->type.type = thisTypeType;
                return expr;
            }

            scope = scope->parent;
        }

        getSink()->diagnose(expr, Diagnostics::thisTypeOutsideOfTypeDecl);
        return CreateErrorExpr(expr);
    }

    Expr* SemanticsExprVisitor::visitReturnValExpr(ReturnValExpr* expr)
    {
        auto scope = expr->scope;
        if (scope)
        {
            auto parentFunc = as<CallableDecl>(getParentFunc(scope->containerDecl));
            if (parentFunc)
            {
                if (as<ErrorType>(parentFunc->returnType.type))
                {
                    expr->type = parentFunc->returnType.type;
                    return expr;
                }
                if (isNonCopyableType(parentFunc->returnType.type))
                {
                    expr->type.isLeftValue = true;
                    expr->type.type = parentFunc->returnType.type;
                    return expr;
                }
            }
        }
        getSink()->diagnose(expr, Diagnostics::returnValNotAvailable);
        expr->type = getASTBuilder()->getErrorType();
        return expr;
    }

    Expr* SemanticsExprVisitor::visitAndTypeExpr(AndTypeExpr* expr)
    {
        // The left and right sides of an `&` for types must both be types.
        //
        expr->left = CheckProperType(expr->left);
        expr->right = CheckProperType(expr->right);

        // TODO: We should enforce some rules here about what is allowed
        // for the `left` and `right` types.
        //
        // For now, the right rule is that they probably need to either
        // be interfaces, or conjunctions thereof.
        //
        // Eventually it may be valuable to support more flexible
        // types in conjunctions, especialy in cases where inheritance
        // gets involved.

        // The result of this expression is an `AndType`, which we need
        // to wrap in a `TypeType` to indicate that the result is the type
        // itself and not a value of  that type.
        //
        auto andType = m_astBuilder->getAndType(expr->left.type, expr->right.type);
        expr->type = m_astBuilder->getTypeType(andType);

        return expr;
    }

    Expr* SemanticsExprVisitor::visitPointerTypeExpr(PointerTypeExpr* expr)
    {
        expr->base = CheckProperType(expr->base);
        if (as<ErrorType>(expr->base.type))
            expr->type = expr->base.type;
        auto ptrType = m_astBuilder->getPtrType(expr->base.type);
        expr->type = m_astBuilder->getTypeType(ptrType);
        return expr;
    }

    Expr* SemanticsExprVisitor::visitModifiedTypeExpr(ModifiedTypeExpr* expr)
    {
        // The base type should be a proper type (not an expression, generic, etc.)
        //
        expr->base = CheckProperType(expr->base);
        auto baseType = expr->base.type;

        // We will check the modifiers that were applied to the type expression
        // one by one, and collect a list of the ones that should modify the
        // resulting `Type`.
        //
        List<Val*> modifierVals;
        for( auto modifier : expr->modifiers )
        {
            auto modifierVal = checkTypeModifier(modifier, baseType);
            if(!modifierVal)
                continue;
            modifierVals.add(modifierVal);
        }

        auto modifiedType = m_astBuilder->getModifiedType(baseType, modifierVals);
        expr->type = m_astBuilder->getTypeType(modifiedType);

        return expr;
    }

    Val* SemanticsExprVisitor::checkTypeModifier(Modifier* modifier, Type* type)
    {
        SLANG_UNUSED(type);

        if( const auto unormModifier = as<UNormModifier>(modifier) )
        {
            // TODO: validate that `type` is either `float` or a vector of `float`s
            return m_astBuilder->getUNormModifierVal();

        }
        else if( const auto snormModifier = as<SNormModifier>(modifier) )
        {
            // TODO: validate that `type` is either `float` or a vector of `float`s
            return m_astBuilder->getSNormModifierVal();
        }
        else if (const auto noDiffModifier = as<NoDiffModifier>(modifier))
        {
            return m_astBuilder->getNoDiffModifierVal();
        }
        else
        {
            // TODO: more complete error message here
            getSink()->diagnose(modifier, Diagnostics::unexpected, "unknown type modifier in semantic checking");
            return nullptr;
        }
    }

    Expr* SemanticsExprVisitor::visitFuncTypeExpr(FuncTypeExpr* expr)
    {
        // The input and output to a function type must both be types
        for(auto& t : expr->parameters)
            t = CheckProperType(t);
        expr->result = CheckProperType(expr->result);

        // TODO: Kind checking? Where are we stopping someone passing
        // constraints around as value-inhabitable types

        // The result of this expression is a `FuncType`, which we need
        // to wrap in a `TypeType` to indicate that the result is the type
        // itself and not a value of that type.
        List<Type*> types;
        types.reserve(expr->parameters.getCount());
        for(const auto& t : expr->parameters)
            types.add(t.type);
        auto funcType = m_astBuilder->getFuncType(types.getArrayView(), expr->result.type);
        expr->type = m_astBuilder->getTypeType(funcType);

        return expr;
    }

    Expr* SemanticsExprVisitor::visitTupleTypeExpr(TupleTypeExpr* expr)
    {
        // All tuple members must be types
        for(auto& t : expr->members)
            t = CheckProperType(t);

        // As in the other cases above, wrap in TypeType
        List<Type*> types;
        types.reserve(expr->members.getCount());
        for(auto t : expr->members)
            types.add(t.type);
        auto tupleType = m_astBuilder->getTupleType(types);
        expr->type = m_astBuilder->getTypeType(tupleType);

        return expr;
    }

    Expr* SemanticsExprVisitor::visitSPIRVAsmExpr(SPIRVAsmExpr* expr)
    {
        //
        // Firstly, get the info for this op, the opcode has already been
        // discovered by the parser
        //
        const auto& spirvInfo = getSession()->spirvCoreGrammarInfo;

        // We will iterate over all the operands in all the insts and check
        // them
        bool failed = false;
        for(auto& inst : expr->insts)
        {
            // It's not automatically a failure to not have info, we just won't
            // be able to deduce types for operands
            const auto opInfo = spirvInfo->opInfos.lookup(SpvOp(inst.opcode.knownValue));

            if(opInfo && opInfo->numOperandTypes == 0 && inst.operands.getCount())
            {
                failed = true;
                getSink()->diagnose(inst.opcode.token, Diagnostics::spirvInstructionWithTooManyOperands, inst.opcode.token, 0);
                continue;
            }

            const bool isLast = &inst == &expr->insts.getLast();
            for(Index operandIndex = 0; operandIndex < inst.operands.getCount(); ++operandIndex)
            {
                // Clamp to the end of the type info array, because the last one will be any variable operands
                const auto invalidOperandKind = SPIRVCoreGrammarInfo::OperandKind{0xff};
                const auto operandType
                    = opInfo.has_value()
                    ? opInfo->operandTypes[std::min(operandIndex, Index(opInfo->numOperandTypes)-1)]
                    : invalidOperandKind;
                const auto baseOperandType
                    = spirvInfo->operandKindUnderneathIds.lookup(operandType).value_or(operandType);
                const auto needsIdWrapper = baseOperandType != operandType;

                const auto check = [&](const auto& go, auto& operand) -> void {
                    if(operand.flavor == SPIRVAsmOperand::SlangType
                        || operand.flavor == SPIRVAsmOperand::SampledType)
                    {
                        // This is a $$type operand or __sampledType(T)
                        // operand, fill in its TypeExp member.
                        TypeExp& typeExpr = operand.type;
                        typeExpr.exp = operand.expr;
                        typeExpr = CheckProperType(typeExpr);
                        operand.expr = typeExpr.exp;
                    }
                    else if(operand.flavor == SPIRVAsmOperand::SlangValue
                        || operand.flavor == SPIRVAsmOperand::SlangValueAddr
                        || operand.flavor == SPIRVAsmOperand::ImageType
                        || operand.flavor == SPIRVAsmOperand::SampledImageType)
                    {
                        // This is a $expr operand, check the expr
                        operand.expr = dispatch(operand.expr);
                    }
                    else if(operand.flavor == SPIRVAsmOperand::ResultMarker)
                    {
                        // This is the <result-id> marker, check that it only
                        // appears in the last instruction.

                        // TODO: We could consider relaxing this, because SPIR-V
                        // does have forward references for decorations and such
                        if (!isLast)
                        {
                            getSink()->diagnose(operand.token, Diagnostics::misplacedResultIdMarker);
                            getSink()->diagnoseWithoutSourceView(expr, Diagnostics::considerOpCopyObject);
                        }
                    }
                    else if(operand.flavor == SPIRVAsmOperand::NamedValue)
                    {
                        // First try and look it up with the knowledge of this operand's type
                        auto enumValue
                            = spirvInfo->allEnums.lookup({baseOperandType, operand.token.getContent()});
                        // Then fall back to with the type prefix
                        if(!enumValue)
                            enumValue = spirvInfo->allEnumsWithTypePrefix.lookup(operand.token.getContent());
                        // Then see if it's an opcode (for OpSpecialize)
                        if(!enumValue)
                            enumValue = spirvInfo->opcodes.lookup(operand.token.getContent());
                        if (inst.opcode.knownValue == SpvOpExtInst)
                        {
                            if (!enumValue)
                            {
                                GLSLstd450 val;
                                if (lookupGLSLstd450(operand.token.getContent(), val))
                                {
                                    enumValue = (SpvWord)val;
                                }
                            }
                        }
                        if(!enumValue)
                        {
                            failed = true;
                            getSink()->diagnose(operand.token, Diagnostics::spirvUnableToResolveName, operand.token.getContent());
                            return;
                        }

                        operand.knownValue = *enumValue;
                        operand.wrapInId = needsIdWrapper;
                    }
                    else if (operand.flavor == SPIRVAsmOperand::BuiltinVar)
                    {
                        operand.type = CheckProperType(operand.type);
                        auto builtinVarKind = spirvInfo->allEnums.lookup(
                            SPIRVCoreGrammarInfo::QualifiedEnumName{spirvInfo->operandKinds.lookup(UnownedStringSlice("BuiltIn")).value(), operand.token.getContent()});
                        if (!builtinVarKind)
                        {
                            failed = true;
                            getSink()->diagnose(operand.token, Diagnostics::spirvUnableToResolveName, operand.token.getContent());
                            return;
                        }
                        operand.knownValue = builtinVarKind.value();
                    }
                    if(operand.bitwiseOrWith.getCount()
                        && operand.flavor != SPIRVAsmOperand::Literal
                        && operand.flavor != SPIRVAsmOperand::NamedValue)
                    {
                        failed = true;
                        getSink()->diagnose(operand.token, Diagnostics::spirvNonConstantBitwiseOr);
                    }
                    for(auto& o : operand.bitwiseOrWith)
                    {
                        if(o.flavor != SPIRVAsmOperand::Literal && o.flavor != SPIRVAsmOperand::NamedValue)
                        {
                            failed = true;
                            getSink()->diagnose(operand.token, Diagnostics::spirvNonConstantBitwiseOr);
                        }
                        go(go, o);
                        operand.knownValue |= o.knownValue;
                    }
                };

                check(check, inst.operands[operandIndex]);
            }
        }

        if(failed)
            return CreateErrorExpr(expr);

        // Assign the type of this expression from the type of the last
        // instruction, otherwise void
        if(expr->insts.getCount())
        {
            // TODO: we trust that this is correct, but could should verify
            const auto lastOperands = expr->insts.getLast().operands;
            if(lastOperands.getCount() >= 2
                && lastOperands[0].flavor == SPIRVAsmOperand::SlangType
                && lastOperands[1].flavor == SPIRVAsmOperand::ResultMarker)
            {
                expr->type = lastOperands[0].type.type;
            }
        }
        if(!expr->type)
            expr->type = m_astBuilder->getVoidType();

        return expr;
    }
}
