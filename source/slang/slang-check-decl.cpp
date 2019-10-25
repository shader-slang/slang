// slang-check-decl.cpp
#include "slang-check-impl.h"

// This file constaints the semantic checking logic and
// related queries for declarations.
//
// Because declarations are the top-level construct
// of the AST (in turn containing all the statements,
// types, and expressions), the declaration-checking
// logic also orchestrates the overall flow and how
// and when things get checked.

#include "slang-lookup.h"

namespace Slang
{

        /// Should the given `decl` nested in `parentDecl` be treated as a static rather than instance declaration?
    bool isEffectivelyStatic(
        Decl*           decl,
        ContainerDecl*  parentDecl)
    {
        // Things at the global scope are always "members" of their module.
        //
        if(as<ModuleDecl>(parentDecl))
            return false;

        // Anything explicitly marked `static` and not at module scope
        // counts as a static rather than instance declaration.
        //
        if(decl->HasModifier<HLSLStaticModifier>())
            return true;

        // Next we need to deal with cases where a declaration is
        // effectively `static` even if the language doesn't make
        // the user say so. Most languages make the default assumption
        // that nested types are `static` even if they don't say
        // so (Java is an exception here, perhaps due to some
        // influence from the Scandanavian OOP tradition of Beta/gbeta).
        //
        if(as<AggTypeDecl>(decl))
            return true;
        if(as<SimpleTypeDecl>(decl))
            return true;

        // Things nested inside functions may have dependencies
        // on values from the enclosing scope, but this needs to
        // be dealt with via "capture" so they are also effectively
        // `static`
        //
        if(as<FunctionDeclBase>(parentDecl))
            return true;

        // Type constraint declarations are used in member-reference
        // context as a form of casting operation, so we treat them
        // as if they are instance members. This is a bit of a hack,
        // but it achieves the result we want until we have an
        // explicit representation of up-cast operations in the
        // AST.
        //
        if(as<TypeConstraintDecl>(decl))
            return false;

        return false;
    }

    bool isEffectivelyStatic(
        Decl*           decl)
    {
        // For the purposes of an ordinary declaration, when determining if
        // it is static or per-instance, the "parent" declaration we really
        // care about is the next outer non-generic declaration.
        //
        // TODO: This idiom of getting the "next outer non-generic declaration"
        // comes up just enough that we should probably have a convenience
        // function for it.

        auto parentDecl = decl->ParentDecl;
        if(auto genericDecl = as<GenericDecl>(parentDecl))
            parentDecl = genericDecl->ParentDecl;

        return isEffectivelyStatic(decl, parentDecl);
    }

        /// Is `decl` a global shader parameter declaration?
    bool isGlobalShaderParameter(VarDeclBase* decl)
    {
        // A global shader parameter must be declared at global (module) scope.
        //
        if(!as<ModuleDecl>(decl->ParentDecl)) return false;

        // A global variable marked `static` indicates a traditional
        // global variable (albeit one that is implicitly local to
        // the translation unit)
        //
        if(decl->HasModifier<HLSLStaticModifier>()) return false;

        // The `groupshared` modifier indicates that a variable cannot
        // be a shader parameters, but is instead transient storage
        // allocated for the duration of a thread-group's execution.
        //
        if(decl->HasModifier<HLSLGroupSharedModifier>()) return false;

        return true;
    }

    // Get the type to use when referencing a declaration
    QualType getTypeForDeclRef(
        Session*                session,
        SemanticsVisitor*       sema,
        DiagnosticSink*         sink,
        DeclRef<Decl>           declRef,
        RefPtr<Type>* outTypeResult)
    {
        if( sema )
        {
            sema->checkDecl(declRef.getDecl());
        }

        // We need to insert an appropriate type for the expression, based on
        // what we found.
        if (auto varDeclRef = declRef.as<VarDeclBase>())
        {
            QualType qualType;
            qualType.type = GetType(varDeclRef);

            bool isLValue = true;
            if(varDeclRef.getDecl()->FindModifier<ConstModifier>())
                isLValue = false;

            // Global-scope shader parameters should not be writable,
            // since they are effectively program inputs.
            //
            // TODO: We could eventually treat a mutable global shader
            // parameter as a shorthand for an immutable parameter and
            // a global variable that gets initialized from that parameter,
            // but in order to do so we'd need to support global variables
            // with resource types better in the back-end.
            //
            if(isGlobalShaderParameter(varDeclRef.getDecl()))
                isLValue = false;

            // Variables declared with `let` are always immutable.
            if(varDeclRef.is<LetDecl>())
                isLValue = false;

            // Generic value parameters are always immutable
            if(varDeclRef.is<GenericValueParamDecl>())
                isLValue = false;

            // Function parameters declared in the "modern" style
            // are immutable unless they have an `out` or `inout` modifier.
            if(varDeclRef.is<ModernParamDecl>())
            {
                // Note: the `inout` modifier AST class inherits from
                // the class for the `out` modifier so that we can
                // make simple checks like this.
                //
                if( !varDeclRef.getDecl()->HasModifier<OutModifier>() )
                {
                    isLValue = false;
                }
            }

            qualType.IsLeftValue = isLValue;
            return qualType;
        }
        else if( auto enumCaseDeclRef = declRef.as<EnumCaseDecl>() )
        {
            QualType qualType;
            qualType.type = getType(enumCaseDeclRef);
            qualType.IsLeftValue = false;
            return qualType;
        }
        else if (auto typeAliasDeclRef = declRef.as<TypeDefDecl>())
        {
            auto type = getNamedType(session, typeAliasDeclRef);
            *outTypeResult = type;
            return QualType(getTypeType(type));
        }
        else if (auto aggTypeDeclRef = declRef.as<AggTypeDecl>())
        {
            auto type = DeclRefType::Create(session, aggTypeDeclRef);
            *outTypeResult = type;
            return QualType(getTypeType(type));
        }
        else if (auto simpleTypeDeclRef = declRef.as<SimpleTypeDecl>())
        {
            auto type = DeclRefType::Create(session, simpleTypeDeclRef);
            *outTypeResult = type;
            return QualType(getTypeType(type));
        }
        else if (auto genericDeclRef = declRef.as<GenericDecl>())
        {
            auto type = getGenericDeclRefType(session, genericDeclRef);
            *outTypeResult = type;
            return QualType(getTypeType(type));
        }
        else if (auto funcDeclRef = declRef.as<CallableDecl>())
        {
            auto type = getFuncType(session, funcDeclRef);
            return QualType(type);
        }
        else if (auto constraintDeclRef = declRef.as<TypeConstraintDecl>())
        {
            // When we access a constraint or an inheritance decl (as a member),
            // we are conceptually performing a "cast" to the given super-type,
            // with the declaration showing that such a cast is legal.
            auto type = GetSup(constraintDeclRef);
            return QualType(type);
        }
        if( sink )
        {
            sink->diagnose(declRef, Diagnostics::unimplemented, "cannot form reference to this kind of declaration");
        }
        return QualType(session->getErrorType());
    }

    QualType getTypeForDeclRef(
        Session*        session,
        DeclRef<Decl>   declRef)
    {
        RefPtr<Type> typeResult;
        return getTypeForDeclRef(session, nullptr, nullptr, declRef, &typeResult);
    }

    DeclRef<ExtensionDecl> ApplyExtensionToType(
        SemanticsVisitor*       semantics,
        ExtensionDecl*          extDecl,
        RefPtr<Type>  type)
    {
        if(!semantics)
            return DeclRef<ExtensionDecl>();

        return semantics->ApplyExtensionToType(extDecl, type);
    }

    RefPtr<GenericSubstitution> createDefaultSubsitutionsForGeneric(
        Session*                session,
        GenericDecl*            genericDecl,
        RefPtr<Substitutions>   outerSubst)
    {
        RefPtr<GenericSubstitution> genericSubst = new GenericSubstitution();
        genericSubst->genericDecl = genericDecl;
        genericSubst->outer = outerSubst;

        for( auto mm : genericDecl->Members )
        {
            if( auto genericTypeParamDecl = as<GenericTypeParamDecl>(mm) )
            {
                genericSubst->args.add(DeclRefType::Create(session, DeclRef<Decl>(genericTypeParamDecl, outerSubst)));
            }
            else if( auto genericValueParamDecl = as<GenericValueParamDecl>(mm) )
            {
                genericSubst->args.add(new GenericParamIntVal(DeclRef<GenericValueParamDecl>(genericValueParamDecl, outerSubst)));
            }
        }

        // create default substitution arguments for constraints
        for (auto mm : genericDecl->Members)
        {
            if (auto genericTypeConstraintDecl = as<GenericTypeConstraintDecl>(mm))
            {
                RefPtr<DeclaredSubtypeWitness> witness = new DeclaredSubtypeWitness();
                witness->declRef = DeclRef<Decl>(genericTypeConstraintDecl, outerSubst);
                witness->sub = genericTypeConstraintDecl->sub.type;
                witness->sup = genericTypeConstraintDecl->sup.type;
                genericSubst->args.add(witness);
            }
        }

        return genericSubst;
    }

    // Sometimes we need to refer to a declaration the way that it would be specialized
    // inside the context where it is declared (e.g., with generic parameters filled in
    // using their archetypes).
    //
    SubstitutionSet createDefaultSubstitutions(
        Session*        session,
        Decl*           decl,
        SubstitutionSet outerSubstSet)
    {
        auto dd = decl->ParentDecl;
        if( auto genericDecl = as<GenericDecl>(dd) )
        {
            // We don't want to specialize references to anything
            // other than the "inner" declaration itself.
            if(decl != genericDecl->inner)
                return outerSubstSet;

            RefPtr<GenericSubstitution> genericSubst = createDefaultSubsitutionsForGeneric(
                session,
                genericDecl,
                outerSubstSet.substitutions);

            return SubstitutionSet(genericSubst);
        }

        return outerSubstSet;
    }

    SubstitutionSet createDefaultSubstitutions(
        Session* session,
        Decl*   decl)
    {
        SubstitutionSet subst;
        if( auto parentDecl = decl->ParentDecl )
        {
            subst = createDefaultSubstitutions(session, parentDecl);
        }
        subst = createDefaultSubstitutions(session, decl, subst);
        return subst;
    }

    void checkDecl(SemanticsVisitor* visitor, Decl* decl)
    {
        visitor->checkDecl(decl);
    }

    bool SemanticsVisitor::isDeclUsableAsStaticMember(
        Decl*   decl)
    {
        if(decl->HasModifier<HLSLStaticModifier>())
            return true;

        if(as<ConstructorDecl>(decl))
            return true;

        if(as<EnumCaseDecl>(decl))
            return true;

        if(as<AggTypeDeclBase>(decl))
            return true;

        if(as<SimpleTypeDecl>(decl))
            return true;

        return false;
    }

    bool SemanticsVisitor::isUsableAsStaticMember(
        LookupResultItem const& item)
    {
        // There's a bit of a gotcha here, because a lookup result
        // item might include "breadcrumbs" that indicate more steps
        // along the lookup path. As a result it isn't always
        // valid to just check whether the final decl is usable
        // as a static member, because it might not even be a
        // member of the thing we are trying to work with.
        //

        Decl* decl = item.declRef.getDecl();
        for(auto bb = item.breadcrumbs; bb; bb = bb->next)
        {
            switch(bb->kind)
            {
            // In case lookup went through a `__transparent` member,
            // we are interested in the static-ness of that transparent
            // member, and *not* the static-ness of whatever was inside
            // of it.
            //
            // TODO: This would need some work if we ever had
            // transparent *type* members.
            //
            case LookupResultItem::Breadcrumb::Kind::Member:
                decl = bb->declRef.getDecl();
                break;

            // TODO: Are there any other cases that need special-case
            // handling here?

            default:
                break;
            }
        }

        // Okay, we've found the declaration we should actually
        // be checking, so lets validate that.

        return isDeclUsableAsStaticMember(decl);
    }

    // Make sure a declaration has been checked, so we can refer to it.
    // Note that this may lead to us recursively invoking checking,
    // so this may not be the best way to handle things.
    void SemanticsVisitor::EnsureDecl(RefPtr<Decl> decl, DeclCheckState state)
    {
        if (decl->IsChecked(state)) return;
        if (decl->checkState == DeclCheckState::CheckingHeader)
        {
            // We tried to reference the same declaration while checking it!
            //
            // TODO: we should ideally be tracking a "chain" of declarations
            // being checked on the stack, so that we can report the full
            // chain that leads from this declaration back to itself.
            //
            getSink()->diagnose(decl, Diagnostics::cyclicReference, decl);
            return;
        }

        // Hack: if we are somehow referencing a local variable declaration
        // before the line of code that defines it, then we need to diagnose
        // an error.
        //
        // TODO: The right answer is that lookup should have been performed in
        // the scope that was in place *before* the variable was declared, but
        // this is a quick fix that at least alerts the user to how we are
        // interpreting their code.
        //
        if (auto varDecl = as<VarDecl>(decl))
        {
            if (auto parenScope = as<ScopeDecl>(varDecl->ParentDecl))
            {
                // TODO: This diagnostic should be emitted on the line that is referencing
                // the declaration. That requires `EnsureDecl` to take the requesting
                // location as a parameter.
                getSink()->diagnose(decl, Diagnostics::localVariableUsedBeforeDeclared, decl);
                return;
            }
        }

        if (DeclCheckState::CheckingHeader > decl->checkState)
        {
            decl->SetCheckState(DeclCheckState::CheckingHeader);
        }

        // Check the modifiers on the declaration first, in case
        // semantics of the body itself will depend on them.
        checkModifiers(decl);

        // Use visitor pattern to dispatch to correct case
        dispatchDecl(decl);

        if(state > decl->checkState)
        {
            decl->SetCheckState(state);
        }
    }

    void SemanticsVisitor::EnusreAllDeclsRec(RefPtr<Decl> decl)
    {
        checkDecl(decl);
        if (auto containerDecl = as<ContainerDecl>(decl))
        {
            for (auto m : containerDecl->Members)
            {
                EnusreAllDeclsRec(m);
            }
        }
    }

    void SemanticsVisitor::CheckVarDeclCommon(RefPtr<VarDeclBase> varDecl)
    {
        // A variable that didn't have an explicit type written must
        // have its type inferred from the initial-value expression.
        //
        if(!varDecl->type.exp)
        {
            // In this case we need to perform all checking of the
            // variable (including semantic checking of the initial-value
            // expression) during the first phase of checking.

            auto initExpr = varDecl->initExpr;
            if(!initExpr)
            {
                getSink()->diagnose(varDecl, Diagnostics::varWithoutTypeMustHaveInitializer);
                varDecl->type.type = getSession()->getErrorType();
            }
            else
            {
                initExpr = CheckExpr(initExpr);

                // TODO: We might need some additional steps here to ensure
                // that the type of the expression is one we are okay with
                // inferring. E.g., if we ever decide that integer and floating-point
                // literals have a distinct type from the standard int/float types,
                // then we would need to "decay" a literal to an explicit type here.

                varDecl->initExpr = initExpr;
                varDecl->type.type = initExpr->type;
            }

            varDecl->SetCheckState(DeclCheckState::Checked);
        }
        else
        {
            if (getFunction() || getCheckingPhase() == CheckingPhase::Header)
            {
                TypeExp typeExp = CheckUsableType(varDecl->type);
                varDecl->type = typeExp;
                if (varDecl->type.Equals(getSession()->getVoidType()))
                {
                    getSink()->diagnose(varDecl, Diagnostics::invalidTypeVoid);
                }
            }

            if (getCheckingPhase() == CheckingPhase::Body)
            {
                if (auto initExpr = varDecl->initExpr)
                {
                    initExpr = CheckTerm(initExpr);
                    initExpr = coerce(varDecl->type.Ptr(), initExpr);
                    varDecl->initExpr = initExpr;

                    // If this is an array variable, then we first want to give
                    // it a chance to infer an array size from its initializer
                    //
                    // TODO(tfoley): May need to extend this to handle the
                    // multi-dimensional case...
                    //
                    maybeInferArraySizeForVariable(varDecl);
                    //
                    // Next we want to make sure that the declared (or inferred)
                    // size for the array meets whatever language-specific
                    // constraints we want to enforce (e.g., disallow empty
                    // arrays in specific cases)
                    //
                    validateArraySizeForVariable(varDecl);
                }
            }

        }
        varDecl->SetCheckState(getCheckedState());
    }

    // Fill in default substitutions for the 'subtype' part of a type constraint decl
    void SemanticsVisitor::CheckConstraintSubType(TypeExp& typeExp)
    {
        if (auto sharedTypeExpr = as<SharedTypeExpr>(typeExp.exp))
        {
            if (auto declRefType = as<DeclRefType>(sharedTypeExpr->base))
            {
                declRefType->declRef.substitutions = createDefaultSubstitutions(getSession(), declRefType->declRef.getDecl());

                if (auto typetype = as<TypeType>(typeExp.exp->type))
                    typetype->type = declRefType;
            }
        }
    }

    void SemanticsVisitor::CheckGenericConstraintDecl(GenericTypeConstraintDecl* decl)
    {
        // TODO: are there any other validations we can do at this point?
        //
        // There probably needs to be a kind of "occurs check" to make
        // sure that the constraint actually applies to at least one
        // of the parameters of the generic.
        if (decl->checkState == DeclCheckState::Unchecked)
        {
            decl->checkState = getCheckedState();
            CheckConstraintSubType(decl->sub);
            decl->sub = TranslateTypeNodeForced(decl->sub);
            decl->sup = TranslateTypeNodeForced(decl->sup);
        }
    }

    void SemanticsVisitor::checkDecl(Decl* decl)
    {
        EnsureDecl(decl, getCheckingPhase() == CheckingPhase::Header ? DeclCheckState::CheckedHeader : DeclCheckState::Checked);
    }

    void SemanticsVisitor::checkGenericDeclHeader(GenericDecl* genericDecl)
    {
        if (genericDecl->IsChecked(DeclCheckState::CheckedHeader))
            return;
        // check the parameters
        for (auto m : genericDecl->Members)
        {
            if (auto typeParam = as<GenericTypeParamDecl>(m))
            {
                typeParam->initType = CheckProperType(typeParam->initType);
            }
            else if (auto valParam = as<GenericValueParamDecl>(m))
            {
                // TODO: some real checking here...
                CheckVarDeclCommon(valParam);
            }
            else if (auto constraint = as<GenericTypeConstraintDecl>(m))
            {
                CheckGenericConstraintDecl(constraint);
            }
        }

        genericDecl->SetCheckState(DeclCheckState::CheckedHeader);
    }

    void SemanticsDeclVisitor::visitGenericDecl(GenericDecl* genericDecl)
    {
        checkGenericDeclHeader(genericDecl);

        // check the nested declaration
        // TODO: this needs to be done in an appropriate environment...
        checkDecl(genericDecl->inner);
        genericDecl->SetCheckState(getCheckedState());
    }

    void SemanticsDeclVisitor::visitGenericTypeConstraintDecl(GenericTypeConstraintDecl * genericConstraintDecl)
    {
        if (genericConstraintDecl->IsChecked(DeclCheckState::CheckedHeader))
            return;
        // check the type being inherited from
        auto base = genericConstraintDecl->sup;
        base = TranslateTypeNode(base);
        genericConstraintDecl->sup = base;
    }

    void SemanticsDeclVisitor::visitInheritanceDecl(InheritanceDecl* inheritanceDecl)
    {
        if (inheritanceDecl->IsChecked(DeclCheckState::CheckedHeader))
            return;
        // check the type being inherited from
        auto base = inheritanceDecl->base;
        CheckConstraintSubType(base);
        base = TranslateTypeNode(base);
        inheritanceDecl->base = base;

        // For now we only allow inheritance from interfaces, so
        // we will validate that the type expression names an interface

        if(auto declRefType = as<DeclRefType>(base.type))
        {
            if(auto interfaceDeclRef = declRefType->declRef.as<InterfaceDecl>())
            {
                return;
            }
        }
        else if(base.type.is<ErrorType>())
        {
            // If an error was already produced, don't emit a cascading error.
            return;
        }

        // If type expression didn't name an interface, we'll emit an error here
        // TODO: deal with the case of an error in the type expression (don't cascade)
        getSink()->diagnose( base.exp, Diagnostics::expectedAnInterfaceGot, base.type);
    }

    void SemanticsDeclVisitor::visitSyntaxDecl(SyntaxDecl*)
    {
        // These are only used in the stdlib, so no checking is needed
    }

    void SemanticsDeclVisitor::visitAttributeDecl(AttributeDecl*)
    {
        // These are only used in the stdlib, so no checking is needed
    }

    void SemanticsDeclVisitor::visitGenericTypeParamDecl(GenericTypeParamDecl*)
    {
        // These are only used in the stdlib, so no checking is needed for now
    }

    void SemanticsDeclVisitor::visitGenericValueParamDecl(GenericValueParamDecl*)
    {
        // These are only used in the stdlib, so no checking is needed for now
    }

    void SemanticsVisitor::checkInterfaceConformancesRec(Decl* decl)
    {
        // Any user-defined type may have declared interface conformances,
        // which we should check.
        //
        if( auto aggTypeDecl = as<AggTypeDecl>(decl) )
        {
            checkAggTypeConformance(aggTypeDecl);
        }
        // Conformances can also come via `extension` declarations, and
        // we should check them against the type(s) being extended.
        //
        else if(auto extensionDecl = as<ExtensionDecl>(decl))
        {
            checkExtensionConformance(extensionDecl);
        }

        // We need to handle the recursive cases here, the first
        // of which is a generic decl, where we want to recurivsely
        // check the inner declaration.
        //
        if(auto genericDecl = as<GenericDecl>(decl))
        {
            checkInterfaceConformancesRec(genericDecl->inner);
        }
        // For any other kind of container declaration, we will
        // recurse into all of its member declarations, so that
        // we can handle, e.g., nested `struct` types.
        //
        else if(auto containerDecl = as<ContainerDecl>(decl))
        {
            for(auto member : containerDecl->Members)
            {
                checkInterfaceConformancesRec(member);
            }
        }
    }

    void SemanticsDeclVisitor::visitModuleDecl(ModuleDecl* programNode)
    {
        // Try to register all the builtin decls
        for (auto decl : programNode->Members)
        {
            auto inner = decl;
            if (auto genericDecl = as<GenericDecl>(decl))
            {
                inner = genericDecl->inner;
            }

            if (auto builtinMod = inner->FindModifier<BuiltinTypeModifier>())
            {
                registerBuiltinDecl(getSession(), decl, builtinMod);
            }
            if (auto magicMod = inner->FindModifier<MagicTypeModifier>())
            {
                registerMagicDecl(getSession(), decl, magicMod);
            }
        }

        // We need/want to visit any `import` declarations before
        // anything else, to make sure that scoping works.
        for(auto& importDecl : programNode->getMembersOfType<ImportDecl>())
        {
            checkDecl(importDecl);
        }
        // register all extensions
        for (auto & s : programNode->getMembersOfType<ExtensionDecl>())
            registerExtension(s);
        for (auto & g : programNode->getMembersOfType<GenericDecl>())
        {
            if (auto extDecl = as<ExtensionDecl>(g->inner))
            {
                checkGenericDeclHeader(g);
                registerExtension(extDecl);
            }
        }
        // check user defined attribute classes first
        for (auto decl : programNode->Members)
        {
            if (auto typeMember = as<StructDecl>(decl))
            {
                bool isTypeAttributeClass = false;
                for (auto attrib : typeMember->GetModifiersOfType<UncheckedAttribute>())
                {
                    if (attrib->name == getSession()->getNameObj("AttributeUsageAttribute"))
                    {
                        isTypeAttributeClass = true;
                        break;
                    }
                }
                if (isTypeAttributeClass)
                    checkDecl(decl);
            }
        }
        // check types
        for (auto & s : programNode->getMembersOfType<TypeDefDecl>())
            checkDecl(s.Ptr());

        for (int pass = 0; pass < 2; pass++)
        {
            auto& checkingPhase = getShared()->checkingPhase;
            checkingPhase = pass == 0 ? CheckingPhase::Header : CheckingPhase::Body;

            for (auto & s : programNode->getMembersOfType<AggTypeDecl>())
            {
                checkDecl(s.Ptr());
            }
            // HACK(tfoley): Visiting all generic declarations here,
            // because otherwise they won't get visited.
            for (auto & g : programNode->getMembersOfType<GenericDecl>())
            {
                checkDecl(g.Ptr());
            }

            // before checking conformance, make sure we check all the extension bodies
            // generic extension decls are already checked by the loop above
            for (auto & s : programNode->getMembersOfType<ExtensionDecl>())
                checkDecl(s);

            for (auto & func : programNode->getMembersOfType<FuncDecl>())
            {
                if (!func->IsChecked(getCheckedState()))
                {
                    VisitFunctionDeclaration(func.Ptr());
                }
            }
            for (auto & func : programNode->getMembersOfType<FuncDecl>())
            {
                checkDecl(func);
            }

            if (getSink()->GetErrorCount() != 0)
                return;

            // Force everything to be fully checked, just in case
            // Note that we don't just call this on the program,
            // because we'd end up recursing into this very code path...
            for (auto d : programNode->Members)
            {
                EnusreAllDeclsRec(d);
            }

            if (pass == 0)
            {
                checkInterfaceConformancesRec(programNode);
            }
        }
    }

    bool SemanticsVisitor::doesSignatureMatchRequirement(
        DeclRef<CallableDecl>   satisfyingMemberDeclRef,
        DeclRef<CallableDecl>   requiredMemberDeclRef,
        RefPtr<WitnessTable>    witnessTable)
    {
        if(satisfyingMemberDeclRef.getDecl()->HasModifier<MutatingAttribute>()
            && !requiredMemberDeclRef.getDecl()->HasModifier<MutatingAttribute>())
        {
            // A `[mutating]` method can't satisfy a non-`[mutating]` requirement,
            // but vice-versa is okay.
            return false;
        }

        if(satisfyingMemberDeclRef.getDecl()->HasModifier<HLSLStaticModifier>()
            != requiredMemberDeclRef.getDecl()->HasModifier<HLSLStaticModifier>())
        {
            // A `static` method can't satisfy a non-`static` requirement and vice versa.
            return false;
        }

        // TODO: actually implement matching here. For now we'll
        // just pretend that things are satisfied in order to make progress..
        witnessTable->requirementDictionary.Add(
            requiredMemberDeclRef.getDecl(),
            RequirementWitness(satisfyingMemberDeclRef));
        return true;
    }

    bool SemanticsVisitor::doesGenericSignatureMatchRequirement(
        DeclRef<GenericDecl>        genDecl,
        DeclRef<GenericDecl>        requirementGenDecl,
        RefPtr<WitnessTable>        witnessTable)
    {
        if (genDecl.getDecl()->Members.getCount() != requirementGenDecl.getDecl()->Members.getCount())
            return false;
        for (Index i = 0; i < genDecl.getDecl()->Members.getCount(); i++)
        {
            auto genMbr = genDecl.getDecl()->Members[i];
            auto requiredGenMbr = genDecl.getDecl()->Members[i];
            if (auto genTypeMbr = as<GenericTypeParamDecl>(genMbr))
            {
                if (auto requiredGenTypeMbr = as<GenericTypeParamDecl>(requiredGenMbr))
                {
                }
                else
                    return false;
            }
            else if (auto genValMbr = as<GenericValueParamDecl>(genMbr))
            {
                if (auto requiredGenValMbr = as<GenericValueParamDecl>(requiredGenMbr))
                {
                    if (!genValMbr->type->Equals(requiredGenValMbr->type))
                        return false;
                }
                else
                    return false;
            }
            else if (auto genTypeConstraintMbr = as<GenericTypeConstraintDecl>(genMbr))
            {
                if (auto requiredTypeConstraintMbr = as<GenericTypeConstraintDecl>(requiredGenMbr))
                {
                    if (!genTypeConstraintMbr->sup->Equals(requiredTypeConstraintMbr->sup))
                    {
                        return false;
                    }
                }
                else
                    return false;
            }
        }

        // TODO: this isn't right, because we need to specialize the
        // declarations of the generics to a common set of substitutions,
        // so that their types are comparable (e.g., foo<T> and foo<U>
        // need to have substitutions applies so that they are both foo<X>,
        // after which uses of the type X in their parameter lists can
        // be compared).

        return doesMemberSatisfyRequirement(
            DeclRef<Decl>(genDecl.getDecl()->inner.Ptr(), genDecl.substitutions),
            DeclRef<Decl>(requirementGenDecl.getDecl()->inner.Ptr(), requirementGenDecl.substitutions),
            witnessTable);
    }

    bool SemanticsVisitor::doesTypeSatisfyAssociatedTypeRequirement(
        RefPtr<Type>            satisfyingType,
        DeclRef<AssocTypeDecl>  requiredAssociatedTypeDeclRef,
        RefPtr<WitnessTable>    witnessTable)
    {
        // We need to confirm that the chosen type `satisfyingType`,
        // meets all the constraints placed on the associated type
        // requirement `requiredAssociatedTypeDeclRef`.
        //
        // We will enumerate the type constraints placed on the
        // associated type and see if they can be satisfied.
        //
        bool conformance = true;
        for (auto requiredConstraintDeclRef : getMembersOfType<TypeConstraintDecl>(requiredAssociatedTypeDeclRef))
        {
            // Grab the type we expect to conform to from the constraint.
            auto requiredSuperType = GetSup(requiredConstraintDeclRef);

            // Perform a search for a witness to the subtype relationship.
            auto witness = tryGetSubtypeWitness(satisfyingType, requiredSuperType);
            if(witness)
            {
                // If a subtype witness was found, then the conformance
                // appears to hold, and we can satisfy that requirement.
                witnessTable->requirementDictionary.Add(requiredConstraintDeclRef, RequirementWitness(witness));
            }
            else
            {
                // If a witness couldn't be found, then the conformance
                // seems like it will fail.
                conformance = false;
            }
        }

        // TODO: if any conformance check failed, we should probably include
        // that in an error message produced about not satisfying the requirement.

        if(conformance)
        {
            // If all the constraints were satisfied, then the chosen
            // type can indeed satisfy the interface requirement.
            witnessTable->requirementDictionary.Add(
                requiredAssociatedTypeDeclRef.getDecl(),
                RequirementWitness(satisfyingType));
        }

        return conformance;
    }

    bool SemanticsVisitor::doesMemberSatisfyRequirement(
        DeclRef<Decl>               memberDeclRef,
        DeclRef<Decl>               requiredMemberDeclRef,
        RefPtr<WitnessTable>        witnessTable)
    {
        // At a high level, we want to check that the
        // `memberDecl` and the `requiredMemberDeclRef`
        // have the same AST node class, and then also
        // check that their signatures match.
        //
        // There are a bunch of detailed decisions that
        // have to be made, though, because we might, e.g.,
        // allow a function with more general parameter
        // types to satisfy a requirement with more
        // specific parameter types.
        //
        // If we ever allow for "property" declarations,
        // then we would probably need to allow an
        // ordinary field to satisfy a property requirement.
        //
        // An associated type requirement should be allowed
        // to be satisfied by any type declaration:
        // a typedef, a `struct`, etc.
        //
        if (auto memberFuncDecl = memberDeclRef.as<FuncDecl>())
        {
            if (auto requiredFuncDeclRef = requiredMemberDeclRef.as<FuncDecl>())
            {
                // Check signature match.
                return doesSignatureMatchRequirement(
                    memberFuncDecl,
                    requiredFuncDeclRef,
                    witnessTable);
            }
        }
        else if (auto memberInitDecl = memberDeclRef.as<ConstructorDecl>())
        {
            if (auto requiredInitDecl = requiredMemberDeclRef.as<ConstructorDecl>())
            {
                // Check signature match.
                return doesSignatureMatchRequirement(
                    memberInitDecl,
                    requiredInitDecl,
                    witnessTable);
            }
        }
        else if (auto genDecl = memberDeclRef.as<GenericDecl>())
        {
            // For a generic member, we will check if it can satisfy
            // a generic requirement in the interface.
            //
            // TODO: we could also conceivably check that the generic
            // could be *specialized* to satisfy the requirement,
            // and then install a specialization of the generic into
            // the witness table. Actually doing this would seem
            // to require performing something akin to overload
            // resolution as part of requirement satisfaction.
            //
            if (auto requiredGenDeclRef = requiredMemberDeclRef.as<GenericDecl>())
            {
                return doesGenericSignatureMatchRequirement(genDecl, requiredGenDeclRef, witnessTable);
            }
        }
        else if (auto subAggTypeDeclRef = memberDeclRef.as<AggTypeDecl>())
        {
            if(auto requiredTypeDeclRef = requiredMemberDeclRef.as<AssocTypeDecl>())
            {
                checkDecl(subAggTypeDeclRef.getDecl());

                auto satisfyingType = DeclRefType::Create(getSession(), subAggTypeDeclRef);
                return doesTypeSatisfyAssociatedTypeRequirement(satisfyingType, requiredTypeDeclRef, witnessTable);
            }
        }
        else if (auto typedefDeclRef = memberDeclRef.as<TypeDefDecl>())
        {
            // this is a type-def decl in an aggregate type
            // check if the specified type satisfies the constraints defined by the associated type
            if (auto requiredTypeDeclRef = requiredMemberDeclRef.as<AssocTypeDecl>())
            {
                checkDecl(typedefDeclRef.getDecl());

                auto satisfyingType = getNamedType(getSession(), typedefDeclRef);
                return doesTypeSatisfyAssociatedTypeRequirement(satisfyingType, requiredTypeDeclRef, witnessTable);
            }
        }
        // Default: just assume that thing aren't being satisfied.
        return false;
    }

    bool SemanticsVisitor::findWitnessForInterfaceRequirement(
        ConformanceCheckingContext* context,
        DeclRef<AggTypeDeclBase>    typeDeclRef,
        InheritanceDecl*            inheritanceDecl,
        DeclRef<InterfaceDecl>      interfaceDeclRef,
        DeclRef<Decl>               requiredMemberDeclRef,
        RefPtr<WitnessTable>        witnessTable)
    {
        // The goal of this function is to find a suitable
        // value to satisfy the requirement.
        //
        // The 99% case is that the requirement is a named member
        // of the interface, and we need to search for a member
        // with the same name in the type declaration and
        // its (known) extensions.

        // As a first pass, lets check if we already have a
        // witness in the table for the requirement, so
        // that we can bail out early.
        //
        if(witnessTable->requirementDictionary.ContainsKey(requiredMemberDeclRef.getDecl()))
        {
            return true;
        }


        // An important exception to the above is that an
        // inheritance declaration in the interface is not going
        // to be satisfied by an inheritance declaration in the
        // conforming type, but rather by a full "witness table"
        // full of the satisfying values for each requirement
        // in the inherited-from interface.
        //
        if( auto requiredInheritanceDeclRef = requiredMemberDeclRef.as<InheritanceDecl>() )
        {
            // Recursively check that the type conforms
            // to the inherited interface.
            //
            // TODO: we *really* need a linearization step here!!!!

            RefPtr<WitnessTable> satisfyingWitnessTable = checkConformanceToType(
                context,
                typeDeclRef,
                requiredInheritanceDeclRef.getDecl(),
                getBaseType(requiredInheritanceDeclRef));

            if(!satisfyingWitnessTable)
                return false;

            witnessTable->requirementDictionary.Add(
                requiredInheritanceDeclRef.getDecl(),
                RequirementWitness(satisfyingWitnessTable));
            return true;
        }

        // We will look up members with the same name,
        // since only same-name members will be able to
        // satisfy the requirement.
        //
        // TODO: this won't work right now for members that
        // don't have names, which right now includes
        // initializers/constructors.
        Name* name = requiredMemberDeclRef.GetName();

        // We are basically looking up members of the
        // given type, but we need to be a bit careful.
        // We *cannot* perfom lookup "through" inheritance
        // declarations for this or other interfaces,
        // since that would let us satisfy a requirement
        // with itself.
        //
        // There's also an interesting question of whether
        // we can/should support innterface requirements
        // being satisfied via `__transparent` members.
        // This seems like a "clever" idea rather than
        // a useful one, and IR generation would
        // need to construct real IR to trampoline over
        // to the implementation.
        //
        // The final case that can't be reduced to just
        // "a directly declared member with the same name"
        // is the case where the type inherits a member
        // that can satisfy the requirement from a base type.
        // We are ignoring implementation inheritance for
        // now, so we won't worry about this.

        // Make sure that by-name lookup is possible.
        buildMemberDictionary(typeDeclRef.getDecl());
        auto lookupResult = lookUpLocal(getSession(), this, name, typeDeclRef);

        if (!lookupResult.isValid())
        {
            getSink()->diagnose(inheritanceDecl, Diagnostics::typeDoesntImplementInterfaceRequirement, typeDeclRef, requiredMemberDeclRef);
            return false;
        }

        // Iterate over the members and look for one that matches
        // the expected signature for the requirement.
        for (auto member : lookupResult)
        {
            if (doesMemberSatisfyRequirement(member.declRef, requiredMemberDeclRef, witnessTable))
                return true;
        }

        // No suitable member found, although there were candidates.
        //
        // TODO: Eventually we might want something akin to the current
        // overload resolution logic, where we keep track of a list
        // of "candidates" for satisfaction of the requirement,
        // and if nothing is found we print the candidates

        getSink()->diagnose(inheritanceDecl, Diagnostics::typeDoesntImplementInterfaceRequirement, typeDeclRef, requiredMemberDeclRef);
        return false;
    }

    RefPtr<WitnessTable> SemanticsVisitor::checkInterfaceConformance(
        ConformanceCheckingContext* context,
        DeclRef<AggTypeDeclBase>    typeDeclRef,
        InheritanceDecl*            inheritanceDecl,
        DeclRef<InterfaceDecl>      interfaceDeclRef)
    {
        // Has somebody already checked this conformance,
        // and/or is in the middle of checking it?
        RefPtr<WitnessTable> witnessTable;
        if(context->mapInterfaceToWitnessTable.TryGetValue(interfaceDeclRef, witnessTable))
            return witnessTable;

        // We need to check the declaration of the interface
        // before we can check that we conform to it.
        checkDecl(interfaceDeclRef.getDecl());

        // We will construct the witness table, and register it
        // *before* we go about checking fine-grained requirements,
        // in order to short-circuit any potential for infinite recursion.

        // Note: we will re-use the witnes table attached to the inheritance decl,
        // if there is one. This catches cases where semantic checking might
        // have synthesized some of the conformance witnesses for us.
        //
        witnessTable = inheritanceDecl->witnessTable;
        if(!witnessTable)
        {
            witnessTable = new WitnessTable();
        }
        context->mapInterfaceToWitnessTable.Add(interfaceDeclRef, witnessTable);

        bool result = true;

        // TODO: If we ever allow for implementation inheritance,
        // then we will need to consider the case where a type
        // declares that it conforms to an interface, but one of
        // its (non-interface) base types already conforms to
        // that interface, so that all of the requirements are
        // already satisfied with inherited implementations...
        for(auto requiredMemberDeclRef : getMembers(interfaceDeclRef))
        {
            auto requirementSatisfied = findWitnessForInterfaceRequirement(
                context,
                typeDeclRef,
                inheritanceDecl,
                interfaceDeclRef,
                requiredMemberDeclRef,
                witnessTable);

            result = result && requirementSatisfied;
        }

        // Extensions that apply to the interface type can create new conformances
        // for the concrete types that inherit from the interface.
        //
        // These new conformances should not be able to introduce new *requirements*
        // for an implementing interface (although they currently can), but we
        // still need to go through this logic to find the appropriate value
        // that will satisfy the requirement in these cases, and also to put
        // the required entry into the witness table for the interface itself.
        //
        // TODO: This logic is a bit slippery, and we need to figure out what
        // it means in the context of separate compilation. If module A defines
        // an interface IA, module B defines a type C that conforms to IA, and then
        // module C defines an extension that makes IA conform to IC, then it is
        // unreasonable to expect the {B:IA} witness table to contain an entry
        // corresponding to {IA:IC}.
        //
        // The simple answer then would be that the {IA:IC} conformance should be
        // fixed, with a single witness table for {IA:IC}, but then what should
        // happen in B explicitly conformed to IC already?
        //
        // For now we will just walk through the extensions that are known at
        // the time we are compiling and handle those, and punt on the larger issue
        // for abit longer.
        for(auto candidateExt = interfaceDeclRef.getDecl()->candidateExtensions; candidateExt; candidateExt = candidateExt->nextCandidateExtension)
        {
            // We need to apply the extension to the interface type that our
            // concrete type is inheriting from.
            //
            // TODO: need to decide if a this-type substitution is needed here.
            // It probably it.
            RefPtr<Type> targetType = DeclRefType::Create(
                getSession(),
                interfaceDeclRef);
            auto extDeclRef = ApplyExtensionToType(candidateExt, targetType);
            if(!extDeclRef)
                continue;

            // Only inheritance clauses from the extension matter right now.
            for(auto requiredInheritanceDeclRef : getMembersOfType<InheritanceDecl>(extDeclRef))
            {
                auto requirementSatisfied = findWitnessForInterfaceRequirement(
                    context,
                    typeDeclRef,
                    inheritanceDecl,
                    interfaceDeclRef,
                    requiredInheritanceDeclRef,
                    witnessTable);

                result = result && requirementSatisfied;
            }
        }

        // If we failed to satisfy any requirements along the way,
        // then we don't actually want to keep the witness table
        // we've been constructing, because the whole thing was a failure.
        if(!result)
        {
            return nullptr;
        }

        return witnessTable;
    }

    RefPtr<WitnessTable> SemanticsVisitor::checkConformanceToType(
        ConformanceCheckingContext* context,
        DeclRef<AggTypeDeclBase>    typeDeclRef,
        InheritanceDecl*            inheritanceDecl,
        Type*                       baseType)
    {
        if (auto baseDeclRefType = as<DeclRefType>(baseType))
        {
            auto baseTypeDeclRef = baseDeclRefType->declRef;
            if (auto baseInterfaceDeclRef = baseTypeDeclRef.as<InterfaceDecl>())
            {
                // The type is stating that it conforms to an interface.
                // We need to check that it provides all of the members
                // required by that interface.
                return checkInterfaceConformance(
                    context,
                    typeDeclRef,
                    inheritanceDecl,
                    baseInterfaceDeclRef);
            }
        }

        getSink()->diagnose(inheritanceDecl, Diagnostics::unimplemented, "type not supported for inheritance");
        return nullptr;
    }

    bool SemanticsVisitor::checkConformance(
        DeclRef<AggTypeDeclBase>    declRef,
        InheritanceDecl*            inheritanceDecl)
    {
        declRef = createDefaultSubstitutionsIfNeeded(getSession(), declRef).as<AggTypeDeclBase>();

        // Don't check conformances for abstract types that
        // are being used to express *required* conformances.
        if (auto assocTypeDeclRef = declRef.as<AssocTypeDecl>())
        {
            // An associated type declaration represents a requirement
            // in an outer interface declaration, and its members
            // (type constraints) represent additional requirements.
            return true;
        }
        else if (auto interfaceDeclRef = declRef.as<InterfaceDecl>())
        {
            // HACK: Our semantics as they stand today are that an
            // `extension` of an interface that adds a new inheritance
            // clause acts *as if* that inheritnace clause had been
            // attached to the original `interface` decl: that is,
            // it adds additional requirements.
            //
            // This is *not* a reasonable semantic to keep long-term,
            // but it is required for some of our current example
            // code to work.
            return true;
        }


        // Look at the type being inherited from, and validate
        // appropriately.
        auto baseType = inheritanceDecl->base.type;

        ConformanceCheckingContext context;
        RefPtr<WitnessTable> witnessTable = checkConformanceToType(&context, declRef, inheritanceDecl, baseType);
        if(!witnessTable)
            return false;

        inheritanceDecl->witnessTable = witnessTable;
        return true;
    }

    void SemanticsVisitor::checkExtensionConformance(ExtensionDecl* decl)
    {
        if (auto targetDeclRefType = as<DeclRefType>(decl->targetType))
        {
            if (auto aggTypeDeclRef = targetDeclRefType->declRef.as<AggTypeDecl>())
            {
                for (auto inheritanceDecl : decl->getMembersOfType<InheritanceDecl>())
                {
                    checkConformance(aggTypeDeclRef, inheritanceDecl);
                }
            }
        }
    }

    void SemanticsVisitor::checkAggTypeConformance(AggTypeDecl* decl)
    {
        // After we've checked members, we need to go through
        // any inheritance clauses on the type itself, and
        // confirm that the type actually provides whatever
        // those clauses require.

        if (auto interfaceDecl = as<InterfaceDecl>(decl))
        {
            // Don't check that an interface conforms to the
            // things it inherits from.
        }
        else if (auto assocTypeDecl = as<AssocTypeDecl>(decl))
        {
            // Don't check that an associated type decl conforms to the
            // things it inherits from.
        }
        else
        {
            // For non-interface types we need to check conformance.
            //
            // TODO: Need to figure out what this should do for
            // `abstract` types if we ever add them. Should they
            // be required to implement all interface requirements,
            // just with `abstract` methods that replicate things?
            // (That's what C# does).
            for (auto inheritanceDecl : decl->getMembersOfType<InheritanceDecl>())
            {
                checkConformance(makeDeclRef(decl), inheritanceDecl);
            }
        }
    }

    void SemanticsDeclVisitor::visitAggTypeDecl(AggTypeDecl* decl)
    {
        if (decl->IsChecked(getCheckedState()))
            return;

        // TODO: we should check inheritance declarations
        // first, since they need to be validated before
        // we can make use of the type (e.g., you need
        // to know that `A` inherits from `B` in order
        // to check an expression like `aValue.bMethod()`
        // where `aValue` is of type `A` but `bMethod`
        // is defined in type `B`.
        //
        // TODO: We should also add a pass that takes
        // all the stated inheritance relationships,
        // expands them to include implicit inheritance,
        // and then linearizes them. This would allow
        // later passes that need to know everything
        // a type inherits from to proceed linearly
        // through the list, rather than having to
        // recurse (and potentially see the same interface
        // more than once).

        decl->SetCheckState(DeclCheckState::CheckedHeader);

        // Now check all of the member declarations.
        for (auto member : decl->Members)
        {
            checkDecl(member);
        }
        decl->SetCheckState(getCheckedState());
    }

    bool SemanticsVisitor::isIntegerBaseType(BaseType baseType)
    {
        switch(baseType)
        {
        default:
            return false;

        case BaseType::Int8:
        case BaseType::Int16:
        case BaseType::Int:
        case BaseType::Int64:
        case BaseType::UInt8:
        case BaseType::UInt16:
        case BaseType::UInt:
        case BaseType::UInt64:
            return true;
        }
    }

    void SemanticsVisitor::validateEnumTagType(Type* type, SourceLoc const& loc)
    {
        if(auto basicType = as<BasicExpressionType>(type))
        {
            // Allow the built-in integer types.
            if(isIntegerBaseType(basicType->baseType))
                return;

            // By default, don't allow other types to be used
            // as an `enum` tag type.
        }

        getSink()->diagnose(loc, Diagnostics::invalidEnumTagType, type);
    }

    void SemanticsDeclVisitor::visitEnumDecl(EnumDecl* decl)
    {
        if (decl->IsChecked(getCheckedState()))
            return;

        // We need to be careful to avoid recursion in the
        // type-checking logic. We will do the minimal work
        // to make the type usable in the first phase, and
        // then check the actual cases in the second phase.
        //
        if(getCheckingPhase() == CheckingPhase::Header)
        {
            // Look at inheritance clauses, and
            // see if one of them is making the enum
            // "inherit" from a concrete type.
            // This will become the "tag" type
            // of the enum.
            RefPtr<Type>        tagType;
            InheritanceDecl*    tagTypeInheritanceDecl = nullptr;
            for(auto inheritanceDecl : decl->getMembersOfType<InheritanceDecl>())
            {
                checkDecl(inheritanceDecl);

                // Look at the type being inherited from.
                auto superType = inheritanceDecl->base.type;

                if(auto errorType = as<ErrorType>(superType))
                {
                    // Ignore any erroneous inheritance clauses.
                    continue;
                }
                else if(auto declRefType = as<DeclRefType>(superType))
                {
                    if(auto interfaceDeclRef = declRefType->declRef.as<InterfaceDecl>())
                    {
                        // Don't consider interface bases as candidates for
                        // the tag type.
                        continue;
                    }
                }

                if(tagType)
                {
                    // We already found a tag type.
                    getSink()->diagnose(inheritanceDecl, Diagnostics::enumTypeAlreadyHasTagType);
                    getSink()->diagnose(tagTypeInheritanceDecl, Diagnostics::seePreviousTagType);
                    break;
                }
                else
                {
                    tagType = superType;
                    tagTypeInheritanceDecl = inheritanceDecl;
                }
            }

            // If a tag type has not been set, then we
            // default it to the built-in `int` type.
            //
            // TODO: In the far-flung future we may want to distinguish
            // `enum` types that have a "raw representation" like this from
            // ones that are purely abstract and don't expose their
            // type of their tag.
            if(!tagType)
            {
                tagType = getSession()->getIntType();
            }
            else
            {
                // TODO: Need to establish that the tag
                // type is suitable. (e.g., if we are going
                // to allow raw values for case tags to be
                // derived automatically, then the tag
                // type needs to be some kind of integer type...)
                //
                // For now we will just be harsh and require it
                // to be one of a few builtin types.
                validateEnumTagType(tagType, tagTypeInheritanceDecl->loc);
            }
            decl->tagType = tagType;


            // An `enum` type should automatically conform to the `__EnumType` interface.
            // The compiler needs to insert this conformance behind the scenes, and this
            // seems like the best place to do it.
            {
                // First, look up the type of the `__EnumType` interface.
                RefPtr<Type> enumTypeType = getSession()->getEnumTypeType();

                RefPtr<InheritanceDecl> enumConformanceDecl = new InheritanceDecl();
                enumConformanceDecl->ParentDecl = decl;
                enumConformanceDecl->loc = decl->loc;
                enumConformanceDecl->base.type = getSession()->getEnumTypeType();
                decl->Members.add(enumConformanceDecl);

                // The `__EnumType` interface has one required member, the `__Tag` type.
                // We need to satisfy this requirement automatically, rather than require
                // the user to actually declare a member with this name (otherwise we wouldn't
                // let them define a tag value with the name `__Tag`).
                //
                RefPtr<WitnessTable> witnessTable = new WitnessTable();
                enumConformanceDecl->witnessTable = witnessTable;

                Name* tagAssociatedTypeName = getSession()->getNameObj("__Tag");
                Decl* tagAssociatedTypeDecl = nullptr;
                if(auto enumTypeTypeDeclRefType = enumTypeType.dynamicCast<DeclRefType>())
                {
                    if(auto enumTypeTypeInterfaceDecl = as<InterfaceDecl>(enumTypeTypeDeclRefType->declRef.getDecl()))
                    {
                        for(auto memberDecl : enumTypeTypeInterfaceDecl->Members)
                        {
                            if(memberDecl->getName() == tagAssociatedTypeName)
                            {
                                tagAssociatedTypeDecl = memberDecl;
                                break;
                            }
                        }
                    }
                }
                if(!tagAssociatedTypeDecl)
                {
                    SLANG_DIAGNOSE_UNEXPECTED(getSink(), decl, "failed to find built-in declaration '__Tag'");
                }

                // Okay, add the conformance witness for `__Tag` being satisfied by `tagType`
                witnessTable->requirementDictionary.Add(tagAssociatedTypeDecl, RequirementWitness(tagType));

                // TODO: we actually also need to synthesize a witness for the conformance of `tagType`
                // to the `__BuiltinIntegerType` interface, because that is a constraint on the
                // associated type `__Tag`.

                // TODO: eventually we should consider synthesizing other requirements for
                // the min/max tag values, or the total number of tags, so that people don't
                // have to declare these as additional cases.

                enumConformanceDecl->SetCheckState(DeclCheckState::Checked);
            }
        }
        else if( getCheckingPhase() == CheckingPhase::Body )
        {
            auto enumType = DeclRefType::Create(
                getSession(),
                makeDeclRef(decl));

            auto tagType = decl->tagType;

            // Check the enum cases in order.
            for(auto caseDecl : decl->getMembersOfType<EnumCaseDecl>())
            {
                // Each case defines a value of the enum's type.
                //
                // TODO: If we ever support enum cases with payloads,
                // then they would probably have a type that is a
                // `FunctionType` from the payload types to the
                // enum type.
                //
                caseDecl->type.type = enumType;

                checkDecl(caseDecl);
            }

            // For any enum case that didn't provide an explicit
            // tag value, derived an appropriate tag value.
            IntegerLiteralValue defaultTag = 0;
            for(auto caseDecl : decl->getMembersOfType<EnumCaseDecl>())
            {
                if(auto explicitTagValExpr = caseDecl->tagExpr)
                {
                    // This tag has an initializer, so it should establish
                    // the tag value for a successor case that doesn't
                    // provide an explicit tag.

                    RefPtr<IntVal> explicitTagVal = TryConstantFoldExpr(explicitTagValExpr);
                    if(explicitTagVal)
                    {
                        if(auto constIntVal = as<ConstantIntVal>(explicitTagVal))
                        {
                            defaultTag = constIntVal->value;
                        }
                        else
                        {
                            // TODO: need to handle other possibilities here
                            getSink()->diagnose(explicitTagValExpr, Diagnostics::unexpectedEnumTagExpr);
                        }
                    }
                    else
                    {
                        // If this happens, then the explicit tag value expression
                        // doesn't seem to be a constant after all. In this case
                        // we expect the checking logic to have applied already.
                    }
                }
                else
                {
                    // This tag has no initializer, so it should use
                    // the default tag value we are tracking.
                    RefPtr<IntegerLiteralExpr> tagValExpr = new IntegerLiteralExpr();
                    tagValExpr->loc = caseDecl->loc;
                    tagValExpr->type = QualType(tagType);
                    tagValExpr->value = defaultTag;

                    caseDecl->tagExpr = tagValExpr;
                }

                // Default tag for the next case will be one more than
                // for the most recent case.
                //
                // TODO: We might consider adding a `[flags]` attribute
                // that modifies this behavior to be `defaultTagForCase <<= 1`.
                //
                defaultTag++;
            }

            // Now check any other member declarations.
            for(auto memberDecl : decl->Members)
            {
                // Already checked inheritance declarations above.
                if(auto inheritanceDecl = as<InheritanceDecl>(memberDecl))
                    continue;

                // Already checked enum case declarations above.
                if(auto caseDecl = as<EnumCaseDecl>(memberDecl))
                    continue;

                // TODO: Right now we don't support other kinds of
                // member declarations on an `enum`, but that is
                // something we may want to allow in the long run.
                //
                checkDecl(memberDecl);
            }
        }
        decl->SetCheckState(getCheckedState());
    }

    void SemanticsDeclVisitor::visitEnumCaseDecl(EnumCaseDecl* decl)
    {
        if (decl->IsChecked(getCheckedState()))
            return;

        if(getCheckingPhase() == CheckingPhase::Body)
        {
            // An enum case had better appear inside an enum!
            //
            // TODO: Do we need/want to support generic cases some day?
            auto parentEnumDecl = as<EnumDecl>(decl->ParentDecl);
            SLANG_ASSERT(parentEnumDecl);

            // The tag type should have already been set by
            // the surrounding `enum` declaration.
            auto tagType = parentEnumDecl->tagType;
            SLANG_ASSERT(tagType);

            // Need to check the init expression, if present, since
            // that represents the explicit tag for this case.
            if(auto initExpr = decl->tagExpr)
            {
                initExpr = CheckExpr(initExpr);
                initExpr = coerce(tagType, initExpr);

                // We want to enforce that this is an integer constant
                // expression, but we don't actually care to retain
                // the value.
                CheckIntegerConstantExpression(initExpr);

                decl->tagExpr = initExpr;
            }
        }

        decl->SetCheckState(getCheckedState());
    }

    void SemanticsDeclVisitor::visitDeclGroup(DeclGroup* declGroup)
    {
        for (auto decl : declGroup->decls)
        {
            dispatchDecl(decl);
        }
    }

    void SemanticsDeclVisitor::visitTypeDefDecl(TypeDefDecl* decl)
    {
        if (decl->IsChecked(getCheckedState())) return;
        if (getCheckingPhase() == CheckingPhase::Header)
        {
            decl->type = CheckProperType(decl->type);
        }
        decl->SetCheckState(getCheckedState());
    }

    void SemanticsDeclVisitor::visitGlobalGenericParamDecl(GlobalGenericParamDecl* decl)
    {
        if (decl->IsChecked(getCheckedState())) return;
        if (getCheckingPhase() == CheckingPhase::Header)
        {
            decl->SetCheckState(DeclCheckState::CheckedHeader);
            // global generic param only allowed in global scope
            auto program = as<ModuleDecl>(decl->ParentDecl);
            if (!program)
                getSink()->diagnose(decl, Slang::Diagnostics::globalGenParamInGlobalScopeOnly);
            // Now check all of the member declarations.
            for (auto member : decl->Members)
            {
                checkDecl(member);
            }
        }
        decl->SetCheckState(getCheckedState());
    }

    void SemanticsDeclVisitor::visitAssocTypeDecl(AssocTypeDecl* decl)
    {
        if (decl->IsChecked(getCheckedState())) return;
        if (getCheckingPhase() == CheckingPhase::Header)
        {
            decl->SetCheckState(DeclCheckState::CheckedHeader);

            // assoctype only allowed in an interface
            auto interfaceDecl = as<InterfaceDecl>(decl->ParentDecl);
            if (!interfaceDecl)
                getSink()->diagnose(decl, Slang::Diagnostics::assocTypeInInterfaceOnly);

            // Now check all of the member declarations.
            for (auto member : decl->Members)
            {
                checkDecl(member);
            }
        }
        decl->SetCheckState(getCheckedState());
    }

    void SemanticsDeclVisitor::visitFuncDecl(FuncDecl* functionNode)
    {
        if (functionNode->IsChecked(getCheckedState()))
            return;

        if (getCheckingPhase() == CheckingPhase::Header)
        {
            VisitFunctionDeclaration(functionNode);
        }
        // TODO: This should really only set "checked header"
        functionNode->SetCheckState(getCheckedState());

        if (getCheckingPhase() == CheckingPhase::Body)
        {
            // TODO: should put the checking of the body onto a "work list"
            // to avoid recursion here.
            if (functionNode->Body)
            {
                auto& function = getShared()->function;
                auto oldFunc = function;
                function = functionNode;
                checkStmt(functionNode->Body);
                function = oldFunc;
            }
        }
    }

    void SemanticsVisitor::getGenericParams(
        GenericDecl*                        decl,
        List<Decl*>&                        outParams,
        List<GenericTypeConstraintDecl*>    outConstraints)
    {
        for (auto dd : decl->Members)
        {
            if (dd == decl->inner)
                continue;

            if (auto typeParamDecl = as<GenericTypeParamDecl>(dd))
                outParams.add(typeParamDecl);
            else if (auto valueParamDecl = as<GenericValueParamDecl>(dd))
                outParams.add(valueParamDecl);
            else if (auto constraintDecl = as<GenericTypeConstraintDecl>(dd))
                outConstraints.add(constraintDecl);
        }
    }

    bool SemanticsVisitor::doGenericSignaturesMatch(
        GenericDecl*    fst,
        GenericDecl*    snd)
    {
        // First we'll extract the parameters and constraints
        // in each generic signature. We will consider parameters
        // and constraints separately so that we are independent
        // of the order in which constraints are given (that is,
        // a constraint like `<T : IFoo>` would be considered
        // the same as `<T>` with a later `where T : IFoo`.

        List<Decl*> fstParams;
        List<GenericTypeConstraintDecl*> fstConstraints;
        getGenericParams(fst, fstParams, fstConstraints);

        List<Decl*> sndParams;
        List<GenericTypeConstraintDecl*> sndConstraints;
        getGenericParams(snd, sndParams, sndConstraints);

        // For there to be any hope of a match, the
        // two need to have the same number of parameters.
        Index paramCount = fstParams.getCount();
        if (paramCount != sndParams.getCount())
            return false;

        // Now we'll walk through the parameters.
        for (Index pp = 0; pp < paramCount; ++pp)
        {
            Decl* fstParam = fstParams[pp];
            Decl* sndParam = sndParams[pp];

            if (auto fstTypeParam = as<GenericTypeParamDecl>(fstParam))
            {
                if (auto sndTypeParam = as<GenericTypeParamDecl>(sndParam))
                {
                    // TODO: is there any validation that needs to be performed here?
                }
                else
                {
                    // Type and non-type parameters can't match.
                    return false;
                }
            }
            else if (auto fstValueParam = as<GenericValueParamDecl>(fstParam))
            {
                if (auto sndValueParam = as<GenericValueParamDecl>(sndParam))
                {
                    // Need to check that the parameters have the same type.
                    //
                    // Note: We are assuming here that the type of a value
                    // parameter cannot be dependent on any of the type
                    // parameters in the same signature. This is a reasonable
                    // assumption for now, but could get thorny down the road.
                    if (!fstValueParam->getType()->Equals(sndValueParam->getType()))
                    {
                        // Type mismatch.
                        return false;
                    }

                    // TODO: This is not the right place to check on default
                    // values for the parameter, because they won't affect
                    // the signature, but we should make sure to do validation
                    // later on (e.g., that only one declaration can/should
                    // be allowed to provide a default).
                }
                else
                {
                    // Value and non-value parameters can't match.
                    return false;
                }
            }
        }

        // If we got this far, then it means the parameter signatures *seem*
        // to match up all right, but now we need to check that the constraints
        // placed on those parameters are also consistent.
        //
        // For now I'm going to assume/require that all declarations must
        // declare the signature in a way that matches exactly.
        Index constraintCount = fstConstraints.getCount();
        if(constraintCount != sndConstraints.getCount())
            return false;

        for (Index cc = 0; cc < constraintCount; ++cc)
        {
            //auto fstConstraint = fstConstraints[cc];
            //auto sndConstraint = sndConstraints[cc];

            // TODO: the challenge here is that the
            // constraints are going to be expressed
            // in terms of the parameters, which means
            // we need to be doing substitution here.
        }

        // HACK: okay, we'll just assume things match for now.
        return true;
    }

    bool SemanticsVisitor::doFunctionSignaturesMatch(
        DeclRef<FuncDecl> fst,
        DeclRef<FuncDecl> snd)
    {

        // TODO(tfoley): This copies the parameter array, which is bad for performance.
        auto fstParams = GetParameters(fst).ToArray();
        auto sndParams = GetParameters(snd).ToArray();

        // If the functions have different numbers of parameters, then
        // their signatures trivially don't match.
        auto fstParamCount = fstParams.getCount();
        auto sndParamCount = sndParams.getCount();
        if (fstParamCount != sndParamCount)
            return false;

        for (Index ii = 0; ii < fstParamCount; ++ii)
        {
            auto fstParam = fstParams[ii];
            auto sndParam = sndParams[ii];

            // If a given parameter type doesn't match, then signatures don't match
            if (!GetType(fstParam)->Equals(GetType(sndParam)))
                return false;

            // If one parameter is `out` and the other isn't, then they don't match
            //
            // Note(tfoley): we don't consider `out` and `inout` as distinct here,
            // because there is no way for overload resolution to pick between them.
            if (fstParam.getDecl()->HasModifier<OutModifier>() != sndParam.getDecl()->HasModifier<OutModifier>())
                return false;

            // If one parameter is `ref` and the other isn't, then they don't match.
            //
            if(fstParam.getDecl()->HasModifier<RefModifier>() != sndParam.getDecl()->HasModifier<RefModifier>())
                return false;
        }

        // Note(tfoley): return type doesn't enter into it, because we can't take
        // calling context into account during overload resolution.

        return true;
    }

    RefPtr<GenericSubstitution> SemanticsVisitor::createDummySubstitutions(
        GenericDecl* genericDecl)
    {
        RefPtr<GenericSubstitution> subst = new GenericSubstitution();
        subst->genericDecl = genericDecl;
        for (auto dd : genericDecl->Members)
        {
            if (dd == genericDecl->inner)
                continue;

            if (auto typeParam = as<GenericTypeParamDecl>(dd))
            {
                auto type = DeclRefType::Create(getSession(),
                    makeDeclRef(typeParam));
                subst->args.add(type);
            }
            else if (auto valueParam = as<GenericValueParamDecl>(dd))
            {
                auto val = new GenericParamIntVal(
                    makeDeclRef(valueParam));
                subst->args.add(val);
            }
            // TODO: need to handle constraints here?
        }
        return subst;
    }

    void SemanticsVisitor::ValidateFunctionRedeclaration(FuncDecl* funcDecl)
    {
        auto parentDecl = funcDecl->ParentDecl;
        SLANG_ASSERT(parentDecl);
        if (!parentDecl) return;

        Decl* childDecl = funcDecl;

        // If this is a generic function (that is, its parent
        // declaration is a generic), then we need to look
        // for sibling declarations of the parent.
        auto genericDecl = as<GenericDecl>(parentDecl);
        if (genericDecl)
        {
            parentDecl = genericDecl->ParentDecl;
            childDecl = genericDecl;
        }

        // Look at previously-declared functions with the same name,
        // in the same container
        //
        // Note: there is an assumption here that declarations that
        // occur earlier in the program  text will be *later* in
        // the linked list of declarations with the same name.
        // We are also assuming/requiring that the check here is
        // symmetric, in that it is okay to test (A,B) or (B,A),
        // and there is no need to test both.
        //
        buildMemberDictionary(parentDecl);
        for (auto pp = childDecl->nextInContainerWithSameName; pp; pp = pp->nextInContainerWithSameName)
        {
            auto prevDecl = pp;

            // Look through generics to the declaration underneath
            auto prevGenericDecl = as<GenericDecl>(prevDecl);
            if (prevGenericDecl)
                prevDecl = prevGenericDecl->inner.Ptr();

            // We only care about previously-declared functions
            // Note(tfoley): although we should really error out if the
            // name is already in use for something else, like a variable...
            auto prevFuncDecl = as<FuncDecl>(prevDecl);
            if (!prevFuncDecl)
                continue;

            // If one declaration is a prefix/postfix operator, and the
            // other is not a matching operator, then don't consider these
            // to be re-declarations.
            //
            // Note(tfoley): Any attempt to call such an operator using
            // ordinary function-call syntax (if we decided to allow it)
            // would be ambiguous in such a case, of course.
            //
            if (funcDecl->HasModifier<PrefixModifier>() != prevDecl->HasModifier<PrefixModifier>())
                continue;
            if (funcDecl->HasModifier<PostfixModifier>() != prevDecl->HasModifier<PostfixModifier>())
                continue;

            // If one is generic and the other isn't, then there is no match.
            if ((genericDecl != nullptr) != (prevGenericDecl != nullptr))
                continue;

            // We are going to be comparing the signatures of the
            // two functions, but if they are *generic* functions
            // then we will need to compare them with consistent
            // specializations in place.
            //
            // We'll go ahead and create some (unspecialized) declaration
            // references here, just to be prepared.
            DeclRef<FuncDecl> funcDeclRef(funcDecl, nullptr);
            DeclRef<FuncDecl> prevFuncDeclRef(prevFuncDecl, nullptr);

            // If we are working with generic functions, then we need to
            // consider if their generic signatures match.
            if (genericDecl)
            {
                SLANG_ASSERT(prevGenericDecl); // already checked above
                if (!doGenericSignaturesMatch(genericDecl, prevGenericDecl))
                    continue;

                // Now we need specialize the declaration references
                // consistently, so that we can compare.
                //
                // First we create a "dummy" set of substitutions that
                // just reference the parameters of the first generic.
                auto subst = createDummySubstitutions(genericDecl);
                //
                // Then we use those parameters to specialize the *other*
                // generic.
                //
                subst->genericDecl = prevGenericDecl;
                prevFuncDeclRef.substitutions.substitutions = subst;
                //
                // One way to think about it is that if we have these
                // declarations (ignore the name differences...):
                //
                //     // prevFuncDecl:
                //     void foo1<T>(T x);
                //
                //     // funcDecl:
                //     void foo2<U>(U x);
                //
                // Then we will compare `foo2` against `foo1<U>`.
            }

            // If the parameter signatures don't match, then don't worry
            if (!doFunctionSignaturesMatch(funcDeclRef, prevFuncDeclRef))
                continue;

            // If we get this far, then we've got two declarations in the same
            // scope, with the same name and signature, so they appear
            // to be redeclarations.
            //
            // We will track that redeclaration occured, so that we can
            // take it into account for overload resolution.
            //
            // A huge complication that we'll need to deal with is that
            // multiple declarations might introduce default values for
            // (different) parameters, and we might need to merge across
            // all of them (which could get complicated if defaults for
            // parameters can reference earlier parameters).

            // If the previous declaration wasn't already recorded
            // as being part of a redeclaration family, then make
            // it the primary declaration of a new family.
            if (!prevFuncDecl->primaryDecl)
            {
                prevFuncDecl->primaryDecl = prevFuncDecl;
            }

            // The new declaration will belong to the family of
            // the previous one, and so it will share the same
            // primary declaration.
            funcDecl->primaryDecl = prevFuncDecl->primaryDecl;
            funcDecl->nextDecl = nullptr;

            // Next we want to chain the new declaration onto
            // the linked list of redeclarations.
            auto link = &prevFuncDecl->nextDecl;
            while (*link)
                link = &(*link)->nextDecl;
            *link = funcDecl;

            // Now that we've added things to a group of redeclarations,
            // we can do some additional validation.

            // First, we will ensure that the return types match
            // between the declarations, so that they are truly
            // interchangeable.
            //
            // Note(tfoley): If we ever decide to add a beefier type
            // system to Slang, we might allow overloads like this,
            // so long as the desired result type can be disambiguated
            // based on context at the call type. In that case we would
            // consider result types earlier, as part of the signature
            // matching step.
            //
            auto resultType = GetResultType(funcDeclRef);
            auto prevResultType = GetResultType(prevFuncDeclRef);
            if (!resultType->Equals(prevResultType))
            {
                // Bad redeclaration
                getSink()->diagnose(funcDecl, Diagnostics::functionRedeclarationWithDifferentReturnType, funcDecl->getName(), resultType, prevResultType);
                getSink()->diagnose(prevFuncDecl, Diagnostics::seePreviousDeclarationOf, funcDecl->getName());

                // Don't bother emitting other errors at this point
                break;
            }

            // Note(tfoley): several of the following checks should
            // really be looping over all the previous declarations
            // in the same group, and not just the one previous
            // declaration we found just now.

            // TODO: Enforce that the new declaration had better
            // not specify a default value for any parameter that
            // already had a default value in a prior declaration.

            // We are going to want to enforce that we cannot have
            // two declarations of a function both specify bodies.
            // Before we make that check, however, we need to deal
            // with the case where the two function declarations
            // might represent different target-specific versions
            // of a function.
            //
            // TODO: if the two declarations are specialized for
            // different targets, then skip the body checks below.

            // If both of the declarations have a body, then there
            // is trouble, because we wouldn't know which one to
            // use during code generation.
            if (funcDecl->Body && prevFuncDecl->Body)
            {
                // Redefinition
                getSink()->diagnose(funcDecl, Diagnostics::functionRedefinition, funcDecl->getName());
                getSink()->diagnose(prevFuncDecl, Diagnostics::seePreviousDefinitionOf, funcDecl->getName());

                // Don't bother emitting other errors
                break;
            }

            // At this point we've processed the redeclaration and
            // put it into a group, so there is no reason to keep
            // looping and looking at prior declarations.
            return;
        }
    }

    void SemanticsDeclVisitor::visitScopeDecl(ScopeDecl*)
    {
        // Nothing to do
    }

    void SemanticsDeclVisitor::visitParamDecl(ParamDecl* paramDecl)
    {
        // TODO: This logic should be shared with the other cases of
        // variable declarations. The main reason I am not doing it
        // yet is that we use a `ParamDecl` with a null type as a
        // special case in attribute declarations, and that could
        // trip up the ordinary variable checks.

        auto typeExpr = paramDecl->type;
        if(typeExpr.exp)
        {
            typeExpr = CheckUsableType(typeExpr);
            paramDecl->type = typeExpr;
        }

        paramDecl->SetCheckState(DeclCheckState::CheckedHeader);

        // The "initializer" expression for a parameter represents
        // a default argument value to use if an explicit one is
        // not supplied.
        if(auto initExpr = paramDecl->initExpr)
        {
            // We must check the expression and coerce it to the
            // actual type of the parameter.
            //
            initExpr = CheckExpr(initExpr);
            initExpr = coerce(typeExpr.type, initExpr);
            paramDecl->initExpr = initExpr;

            // TODO: a default argument expression needs to
            // conform to other constraints to be valid.
            // For example, it should not be allowed to refer
            // to other parameters of the same function (or maybe
            // only the parameters to its left...).

            // A default argument value should not be allowed on an
            // `out` or `inout` parameter.
            //
            // TODO: we could relax this by requiring the expression
            // to yield an lvalue, but that seems like a feature
            // with limited practical utility (and an easy source
            // of confusing behavior).
            //
            // Note: the `InOutModifier` class inherits from `OutModifier`,
            // so we only need to check for the base case.
            //
            if(paramDecl->FindModifier<OutModifier>())
            {
                getSink()->diagnose(initExpr, Diagnostics::outputParameterCannotHaveDefaultValue);
            }
        }

        paramDecl->SetCheckState(DeclCheckState::Checked);
    }

    void SemanticsVisitor::VisitFunctionDeclaration(FuncDecl *functionNode)
    {
        if (functionNode->IsChecked(DeclCheckState::CheckedHeader)) return;
        functionNode->SetCheckState(DeclCheckState::CheckingHeader);

        auto& function = getShared()->function;
        auto oldFunc = function;
        function = functionNode;

        auto resultType = functionNode->ReturnType;
        if(resultType.exp)
        {
            resultType = CheckProperType(functionNode->ReturnType);
        }
        else
        {
            resultType = TypeExp(getSession()->getVoidType());
        }
        functionNode->ReturnType = resultType;


        HashSet<Name*> paraNames;
        for (auto & para : functionNode->GetParameters())
        {
            EnsureDecl(para, DeclCheckState::CheckedHeader);

            if (paraNames.Contains(para->getName()))
            {
                getSink()->diagnose(para, Diagnostics::parameterAlreadyDefined, para->getName());
            }
            else
                paraNames.Add(para->getName());
        }
        function = oldFunc;
        functionNode->SetCheckState(DeclCheckState::CheckedHeader);

        // One last bit of validation: check if we are redeclaring an existing function
        ValidateFunctionRedeclaration(functionNode);
    }

    IntegerLiteralValue SemanticsVisitor::GetMinBound(RefPtr<IntVal> val)
    {
        if (auto constantVal = as<ConstantIntVal>(val))
            return constantVal->value;

        // TODO(tfoley): Need to track intervals so that this isn't just a lie...
        return 1;
    }

    void SemanticsVisitor::maybeInferArraySizeForVariable(VarDeclBase* varDecl)
    {
        // Not an array?
        auto arrayType = as<ArrayExpressionType>(varDecl->type);
        if (!arrayType) return;

        // Explicit element count given?
        auto elementCount = arrayType->ArrayLength;
        if (elementCount) return;

        // No initializer?
        auto initExpr = varDecl->initExpr;
        if(!initExpr) return;

        // Is the type of the initializer an array type?
        if(auto arrayInitType = as<ArrayExpressionType>(initExpr->type))
        {
            elementCount = arrayInitType->ArrayLength;
        }
        else
        {
            // Nothing to do: we couldn't infer a size
            return;
        }

        // Create a new array type based on the size we found,
        // and install it into our type.
        varDecl->type.type = getArrayType(
            arrayType->baseType,
            elementCount);
    }

    void SemanticsVisitor::validateArraySizeForVariable(VarDeclBase* varDecl)
    {
        auto arrayType = as<ArrayExpressionType>(varDecl->type);
        if (!arrayType) return;

        auto elementCount = arrayType->ArrayLength;
        if (!elementCount)
        {
            // Note(tfoley): For now we allow arrays of unspecified size
            // everywhere, because some source languages (e.g., GLSL)
            // allow them in specific cases.
#if 0
            getSink()->diagnose(varDecl, Diagnostics::invalidArraySize);
#endif
            return;
        }

        // TODO(tfoley): How to handle the case where bound isn't known?
        if (GetMinBound(elementCount) <= 0)
        {
            getSink()->diagnose(varDecl, Diagnostics::invalidArraySize);
            return;
        }
    }

    void SemanticsDeclVisitor::visitVarDecl(VarDecl* varDecl)
    {
        CheckVarDeclCommon(varDecl);
    }

    void SemanticsDeclVisitor::registerExtension(ExtensionDecl* decl)
    {
        if (decl->IsChecked(DeclCheckState::CheckedHeader))
            return;

        decl->SetCheckState(DeclCheckState::CheckingHeader);
        decl->targetType = CheckProperType(decl->targetType);
        decl->SetCheckState(DeclCheckState::CheckedHeader);

        // TODO: need to check that the target type names a declaration...

        if (auto targetDeclRefType = as<DeclRefType>(decl->targetType))
        {
            // Attach our extension to that type as a candidate...
            if (auto aggTypeDeclRef = targetDeclRefType->declRef.as<AggTypeDecl>())
            {
                auto aggTypeDecl = aggTypeDeclRef.getDecl();
                decl->nextCandidateExtension = aggTypeDecl->candidateExtensions;
                aggTypeDecl->candidateExtensions = decl;
                return;
            }
        }
        getSink()->diagnose(decl->targetType.exp, Diagnostics::unimplemented, "expected a nominal type here");
    }

    void SemanticsDeclVisitor::visitExtensionDecl(ExtensionDecl* decl)
    {
        if (decl->IsChecked(getCheckedState())) return;

        if (!as<DeclRefType>(decl->targetType))
        {
            getSink()->diagnose(decl->targetType.exp, Diagnostics::unimplemented, "expected a nominal type here");
        }
        // now check the members of the extension
        for (auto m : decl->Members)
        {
            checkDecl(m);
        }
        decl->SetCheckState(getCheckedState());
    }

    RefPtr<Type> SemanticsVisitor::findResultTypeForConstructorDecl(ConstructorDecl* decl)
    {
        // We want to look at the parent of the declaration,
        // but if the declaration is generic, the parent will be
        // the `GenericDecl` and we need to skip past that to
        // the grandparent.
        //
        auto parent = decl->ParentDecl;
        auto genericParent = as<GenericDecl>(parent);
        if (genericParent)
        {
            parent = genericParent->ParentDecl;
        }

        // Now look at the type of the parent (or grandparent).
        if (auto aggTypeDecl = as<AggTypeDecl>(parent))
        {
            // We are nested in an aggregate type declaration,
            // so the result type of the initializer will just
            // be the surrounding type.
            return DeclRefType::Create(
                getSession(),
                makeDeclRef(aggTypeDecl));
        }
        else if (auto extDecl = as<ExtensionDecl>(parent))
        {
            // We are nested inside an extension, so the result
            // type needs to be the type being extended.
            return extDecl->targetType.type;
        }
        else
        {
            getSink()->diagnose(decl, Diagnostics::initializerNotInsideType);
            return nullptr;
        }
    }

    void SemanticsDeclVisitor::visitConstructorDecl(ConstructorDecl* decl)
    {
        if (decl->IsChecked(getCheckedState())) return;
        if (getCheckingPhase() == CheckingPhase::Header)
        {
            decl->SetCheckState(DeclCheckState::CheckingHeader);

            for (auto& paramDecl : decl->GetParameters())
            {
                paramDecl->type = CheckUsableType(paramDecl->type);
            }

            // We need to compute the result tyep for this declaration,
            // since it wasn't filled in for us.
            decl->ReturnType.type = findResultTypeForConstructorDecl(decl);
        }
        else
        {
            // TODO(tfoley): check body
        }
        decl->SetCheckState(getCheckedState());
    }

    void SemanticsDeclVisitor::visitSubscriptDecl(SubscriptDecl* decl)
    {
        if (decl->IsChecked(getCheckedState())) return;
        for (auto& paramDecl : decl->GetParameters())
        {
            paramDecl->type = CheckUsableType(paramDecl->type);
        }

        decl->ReturnType = CheckUsableType(decl->ReturnType);

        // If we have a subscript declaration with no accessor declarations,
        // then we should create a single `GetterDecl` to represent
        // the implicit meaning of their declaration, so:
        //
        //      subscript(uint index) -> T;
        //
        // becomes:
        //
        //      subscript(uint index) -> T { get; }
        //

        bool anyAccessors = false;
        for(auto accessorDecl : decl->getMembersOfType<AccessorDecl>())
        {
            anyAccessors = true;
        }

        if(!anyAccessors)
        {
            RefPtr<GetterDecl> getterDecl = new GetterDecl();
            getterDecl->loc = decl->loc;

            getterDecl->ParentDecl = decl;
            decl->Members.add(getterDecl);
        }

        for(auto mm : decl->Members)
        {
            checkDecl(mm);
        }

        decl->SetCheckState(getCheckedState());
    }

    void SemanticsDeclVisitor::visitAccessorDecl(AccessorDecl* decl)
    {
        if (getCheckingPhase() == CheckingPhase::Header)
        {
            // An accessor must appear nested inside a subscript declaration (today),
            // or a property declaration (when we add them). It will derive
            // its return type from the outer declaration, so we handle both
            // of these checks at the same place.
            auto parent = decl->ParentDecl;
            if (auto parentSubscript = as<SubscriptDecl>(parent))
            {
                decl->ReturnType = parentSubscript->ReturnType;
            }
            // TODO: when we add "property" declarations, check for them here
            else
            {
                getSink()->diagnose(decl, Diagnostics::accessorMustBeInsideSubscriptOrProperty);
            }

        }
        else
        {
            // TODO: check the body!
        }
        decl->SetCheckState(getCheckedState());
    }

    GenericDecl* SemanticsVisitor::GetOuterGeneric(Decl* decl)
    {
        auto parentDecl = decl->ParentDecl;
        if (!parentDecl) return nullptr;
        auto parentGeneric = as<GenericDecl>(parentDecl);
        return parentGeneric;
    }

    DeclRef<ExtensionDecl> SemanticsVisitor::ApplyExtensionToType(
        ExtensionDecl*  extDecl,
        RefPtr<Type>    type)
    {
        DeclRef<ExtensionDecl> extDeclRef = makeDeclRef(extDecl);

        // If the extension is a generic extension, then we
        // need to infer type arguments that will give
        // us a target type that matches `type`.
        //
        if (auto extGenericDecl = GetOuterGeneric(extDecl))
        {
            ConstraintSystem constraints;
            constraints.loc = extDecl->loc;
            constraints.genericDecl = extGenericDecl;

            if (!TryUnifyTypes(constraints, extDecl->targetType.Ptr(), type))
                return DeclRef<ExtensionDecl>();

            auto constraintSubst = TrySolveConstraintSystem(&constraints, DeclRef<Decl>(extGenericDecl, nullptr).as<GenericDecl>());
            if (!constraintSubst)
            {
                return DeclRef<ExtensionDecl>();
            }

            // Construct a reference to the extension with our constraint variables
            // set as they were found by solving the constraint system.
            extDeclRef = DeclRef<Decl>(extDecl, constraintSubst).as<ExtensionDecl>();
        }

        // Now extract the target type from our (possibly specialized) extension decl-ref.
        RefPtr<Type> targetType = GetTargetType(extDeclRef);

        // As a bit of a kludge here, if the target type of the extension is
        // an interface, and the `type` we are trying to match up has a this-type
        // substitution for that interface, then we want to attach a matching
        // substitution to the extension decl-ref.
        if(auto targetDeclRefType = as<DeclRefType>(targetType))
        {
            if(auto targetInterfaceDeclRef = targetDeclRefType->declRef.as<InterfaceDecl>())
            {
                // Okay, the target type is an interface.
                //
                // Is the type we want to apply to also an interface?
                if(auto appDeclRefType = as<DeclRefType>(type))
                {
                    if(auto appInterfaceDeclRef = appDeclRefType->declRef.as<InterfaceDecl>())
                    {
                        if(appInterfaceDeclRef.getDecl() == targetInterfaceDeclRef.getDecl())
                        {
                            // Looks like we have a match in the types,
                            // now let's see if we have a this-type substitution.
                            if(auto appThisTypeSubst = appInterfaceDeclRef.substitutions.substitutions.as<ThisTypeSubstitution>())
                            {
                                if(appThisTypeSubst->interfaceDecl == appInterfaceDeclRef.getDecl())
                                {
                                    // The type we want to apply to has a this-type substitution,
                                    // and (by construction) the target type currently does not.
                                    //
                                    SLANG_ASSERT(!targetInterfaceDeclRef.substitutions.substitutions.as<ThisTypeSubstitution>());

                                    // We will create a new substitution to apply to the target type.
                                    RefPtr<ThisTypeSubstitution> newTargetSubst = new ThisTypeSubstitution();
                                    newTargetSubst->interfaceDecl = appThisTypeSubst->interfaceDecl;
                                    newTargetSubst->witness = appThisTypeSubst->witness;
                                    newTargetSubst->outer = targetInterfaceDeclRef.substitutions.substitutions;

                                    targetType = DeclRefType::Create(getSession(),
                                        DeclRef<InterfaceDecl>(targetInterfaceDeclRef.getDecl(), newTargetSubst));

                                    // Note: we are constructing a this-type substitution that
                                    // we will apply to the extension declaration as well.
                                    // This is not strictly allowed by our current representation
                                    // choices, but we need it in order to make sure that
                                    // references to the target type of the extension
                                    // declaration have a chance to resolve the way we want them to.

                                    RefPtr<ThisTypeSubstitution> newExtSubst = new ThisTypeSubstitution();
                                    newExtSubst->interfaceDecl = appThisTypeSubst->interfaceDecl;
                                    newExtSubst->witness = appThisTypeSubst->witness;
                                    newExtSubst->outer = extDeclRef.substitutions.substitutions;

                                    extDeclRef = DeclRef<ExtensionDecl>(
                                        extDeclRef.getDecl(),
                                        newExtSubst);

                                    // TODO: Ideally we should also apply the chosen specialization to
                                    // the decl-ref for the extension, so that subsequent lookup through
                                    // the members of this extension will retain that substitution and
                                    // be able to apply it.
                                    //
                                    // E.g., if an extension method returns a value of an associated
                                    // type, then we'd want that to become specialized to a concrete
                                    // type when using the extension method on a value of concrete type.
                                    //
                                    // The challenge here that makes me reluctant to just staple on
                                    // such a substitution is that it wouldn't follow our implicit
                                    // rules about where `ThisTypeSubstitution`s can appear.
                                }
                            }
                        }
                    }
                }
            }
        }

        // In order for this extension to apply to the given type, we
        // need to have a match on the target types.
        if (!type->Equals(targetType))
            return DeclRef<ExtensionDecl>();


        return extDeclRef;
    }

    QualType SemanticsVisitor::GetTypeForDeclRef(DeclRef<Decl> declRef)
    {
        RefPtr<Type> typeResult;
        return getTypeForDeclRef(
            getSession(),
            this,
            getSink(),
            declRef,
            &typeResult);
    }

    void SemanticsVisitor::importModuleIntoScope(Scope* scope, ModuleDecl* moduleDecl)
    {
        // If we've imported this one already, then
        // skip the step where we modify the current scope.
        auto& importedModules = getShared()->importedModules;
        if (importedModules.Contains(moduleDecl))
        {
            return;
        }
        importedModules.Add(moduleDecl);


        // Create a new sub-scope to wire the module
        // into our lookup chain.
        auto subScope = new Scope();
        subScope->containerDecl = moduleDecl;

        subScope->nextSibling = scope->nextSibling;
        scope->nextSibling = subScope;

        // Also import any modules from nested `import` declarations
        // with the `__exported` modifier
        for (auto importDecl : moduleDecl->getMembersOfType<ImportDecl>())
        {
            if (!importDecl->HasModifier<ExportedModifier>())
                continue;

            importModuleIntoScope(scope, importDecl->importedModuleDecl.Ptr());
        }
    }

    void SemanticsDeclVisitor::visitEmptyDecl(EmptyDecl* /*decl*/)
    {
        // nothing to do
    }

    void SemanticsDeclVisitor::visitImportDecl(ImportDecl* decl)
    {
        if(decl->IsChecked(DeclCheckState::CheckedHeader))
            return;

        // We need to look for a module with the specified name
        // (whether it has already been loaded, or needs to
        // be loaded), and then put its declarations into
        // the current scope.

        auto name = decl->moduleNameAndLoc.name;
        auto scope = decl->scope;

        // Try to load a module matching the name
        auto importedModule = findOrImportModule(
            getLinkage(),
            name,
            decl->moduleNameAndLoc.loc,
            getSink());

        // If we didn't find a matching module, then bail out
        if (!importedModule)
            return;

        // Record the module that was imported, so that we can use
        // it later during code generation.
        auto importedModuleDecl = importedModule->getModuleDecl();
        decl->importedModuleDecl = importedModuleDecl;

        // Add the declarations from the imported module into the scope
        // that the `import` declaration is set to extend.
        //
        importModuleIntoScope(scope.Ptr(), importedModuleDecl);

        // Record the `import`ed module (and everything it depends on)
        // as a dependency of the module we are compiling.
        if(auto module = getModule(decl))
        {
            module->addModuleDependency(importedModule);
        }

        decl->SetCheckState(getCheckedState());
    }

}
