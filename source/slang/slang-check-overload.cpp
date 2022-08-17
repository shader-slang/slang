// slang-check-overload.cpp
#include "slang-check-impl.h"

#include "slang-lookup.h"
#include "slang-ast-print.h"

// This file implements semantic checking logic related
// to resolving overloading call operations, by checking
// the applicability and relative priority of various candidates.

namespace Slang
{
    SemanticsVisitor::ParamCounts SemanticsVisitor::CountParameters(FilteredMemberRefList<ParamDecl> params)
    {
        ParamCounts counts = { 0, 0 };
        for (auto param : params)
        {
            counts.allowed++;

            // No initializer means no default value
            //
            // TODO(tfoley): The logic here is currently broken in two ways:
            //
            // 1. We are assuming that once one parameter has a default, then all do.
            //    This can/should be validated earlier, so that we can assume it here.
            //
            // 2. We are not handling the possibility of multiple declarations for
            //    a single function, where we'd need to merge default parameters across
            //    all the declarations.
            if (!param.getDecl()->initExpr)
            {
                counts.required++;
            }
        }
        return counts;
    }

    SemanticsVisitor::ParamCounts SemanticsVisitor::CountParameters(DeclRef<GenericDecl> genericRef)
    {
        ParamCounts counts = { 0, 0 };
        for (auto m : genericRef.getDecl()->members)
        {
            if (auto typeParam = as<GenericTypeParamDecl>(m))
            {
                counts.allowed++;
                if (!typeParam->initType.Ptr())
                {
                    counts.required++;
                }
            }
            else if (auto valParam = as<GenericValueParamDecl>(m))
            {
                counts.allowed++;
                if (!valParam->initExpr)
                {
                    counts.required++;
                }
            }
        }
        return counts;
    }

    bool SemanticsVisitor::TryCheckOverloadCandidateClassNewMatchUp(OverloadResolveContext& context, OverloadCandidate const& candidate)
    {
        // Check that a constructor call to a class type must be in a `new` expr, and a `new` expr
        // is only used to construct a class.
        bool isClassType = false;
        bool isNewExpr = false;
        if (auto ctorDeclRef = candidate.item.declRef.as<ConstructorDecl>())
        {
            if (auto resultType = as<DeclRefType>(candidate.resultType))
            {
                if (resultType->declRef.as<ClassDecl>())
                {
                    isClassType = true;
                }
            }
        }
        if (as<NewExpr>(context.originalExpr))
        {
            isNewExpr = true;
        }

        if (isNewExpr && !isClassType)
        {
            getSink()->diagnose(context.originalExpr, Diagnostics::newCanOnlyBeUsedToInitializeAClass);
            return false;
        }
        if (!isNewExpr && isClassType && context.originalExpr)
        {
            getSink()->diagnose(context.originalExpr, Diagnostics::classCanOnlyBeInitializedWithNew);
            return false;
        }
        return true;
    }

    bool SemanticsVisitor::TryCheckOverloadCandidateArity(
        OverloadResolveContext&		context,
        OverloadCandidate const&	candidate)
    {
        UInt argCount = context.getArgCount();
        ParamCounts paramCounts = { 0, 0 };
        switch (candidate.flavor)
        {
        case OverloadCandidate::Flavor::Func:
            paramCounts = CountParameters(getParameters(candidate.item.declRef.as<CallableDecl>()));
            break;

        case OverloadCandidate::Flavor::Generic:
            paramCounts = CountParameters(candidate.item.declRef.as<GenericDecl>());
            break;

        case OverloadCandidate::Flavor::Expr:
            {
                auto paramCount = candidate.funcType->getParamCount();
                paramCounts.allowed = paramCount;
                paramCounts.required = paramCount;
            }
            break;

        default:
            SLANG_UNEXPECTED("unknown flavor of overload candidate");
            break;
        }

        if (argCount >= paramCounts.required && argCount <= paramCounts.allowed)
            return true;

        // Emit an error message if we are checking this call for real
        if (context.mode != OverloadResolveContext::Mode::JustTrying)
        {
            if (argCount < paramCounts.required)
            {
                getSink()->diagnose(context.loc, Diagnostics::notEnoughArguments, argCount, paramCounts.required);
            }
            else
            {
                SLANG_ASSERT(argCount > paramCounts.allowed);
                getSink()->diagnose(context.loc, Diagnostics::tooManyArguments, argCount, paramCounts.allowed);
            }
        }

        return false;
    }

    bool SemanticsVisitor::TryCheckOverloadCandidateFixity(
        OverloadResolveContext&		context,
        OverloadCandidate const&	candidate)
    {
        auto expr = context.originalExpr;

        auto decl = candidate.item.declRef.decl;

        if(auto prefixExpr = as<PrefixExpr>(expr))
        {
            if(decl->hasModifier<PrefixModifier>())
                return true;

            if (context.mode != OverloadResolveContext::Mode::JustTrying)
            {
                getSink()->diagnose(context.loc, Diagnostics::expectedPrefixOperator);
                getSink()->diagnose(decl, Diagnostics::seeDefinitionOf, decl->getName());
            }

            return false;
        }
        else if(auto postfixExpr = as<PostfixExpr>(expr))
        {
            if(decl->hasModifier<PostfixModifier>())
                return true;

            if (context.mode != OverloadResolveContext::Mode::JustTrying)
            {
                getSink()->diagnose(context.loc, Diagnostics::expectedPostfixOperator);
                getSink()->diagnose(decl, Diagnostics::seeDefinitionOf, decl->getName());
            }

            return false;
        }
        else
        {
            return true;
        }
    }

    bool SemanticsVisitor::TryCheckGenericOverloadCandidateTypes(
        OverloadResolveContext&	context,
        OverloadCandidate&		candidate)
    {
        auto genericDeclRef = candidate.item.declRef.as<GenericDecl>();

        // The basic idea here is that we need to check that the
        // arguments to a generic application (e.g., `F<A1, A2, ...>`)
        // have the right "type," which in this context means
        // checking that:
        //
        // * The argument for any generic type parameter is a (proper) type.
        //
        // * The argument for any generic value parameter is a
        //   specialization-time constant value of the appropriate type.
        //
        // Some additional checks are *not* handled at this point:
        //
        // * We don't check that a type argument actually conforms to
        //   the constraints on the parameter.
        //
        // Along the way we will build up a `GenericSubstitution`
        // to represent the arguments that have been coerced to
        // appropriate forms.
        //
        auto genSubst = m_astBuilder->create<GenericSubstitution>();
        candidate.subst = genSubst;
        auto& checkedArgs = genSubst->args;

        // Rather than bail out as soon as we hit a problem,
        // we are going to process *all* of the parameters of the
        // generic and place suitable arguments into the `checkedArgs`
        // array. This is important so that we don't cause crashes
        // in cases where the arguments fail this step of checking,
        // but we decide to proceed with subsequent steps (e.g.,
        // because the candidate we are trying here is the *only*
        // candidate).
        //
        bool success = true;

        Index aa = 0;
        for (auto memberRef : getMembers(genericDeclRef))
        {
            if (auto typeParamRef = memberRef.as<GenericTypeParamDecl>())
            {
                // We have a type parameter, and we expect to find
                // a type argument.
                //
                TypeExp typeArg;
                if( aa >= context.argCount )
                {
                    // If we have run out of arguments, then we definitely
                    // fail checking (in principle this should have been
                    // checked already by an earlier step).
                    //
                    success = false;
                }
                else
                {
                    // If we have at least one argument left, we grab
                    // it and try to coerce it to a proper type. The
                    // manner in which we handle the coercion depends
                    // on whether we are "just trying" the candidate
                    // (so a failure would rule out the candidate, but
                    // shouldn't be reported to the user), or are doing
                    // the checking "for real" in which case any errors
                    // we run into need to be reported.
                    //
                    auto arg = context.getArg(aa++);
                    if (context.mode == OverloadResolveContext::Mode::JustTrying)
                    {
                        typeArg = tryCoerceToProperType(TypeExp(arg));
                    }
                    else
                    {
                        arg = ExpectATypeRepr(arg);
                        typeArg = CoerceToProperType(TypeExp(arg));
                    }
                }

                // If we failed to get a valid type (either because
                // there was no matching argument, or because the
                // "just trying" coercion failed), then we create
                // an error type to stand in for the argument
                //
                if( !typeArg.type )
                {
                    typeArg.type = m_astBuilder->getErrorType();
                    success = false;
                }

                checkedArgs.add(typeArg.type);
            }
            else if (auto valParamRef = memberRef.as<GenericValueParamDecl>())
            {
                // The case for a generic value parameter is similar to that
                // for a generic type parameter.
                //
                Expr* arg = nullptr;
                if( aa >= context.argCount )
                {
                    // If there are no arguments left to consume, then
                    // we have a definite failure.
                    //
                    success = false;
                }
                else
                {
                    // If we have an argument then we need to coerce it
                    // to the type of the parameter (and fail if the
                    // coercion is not possible)
                    //
                    arg = context.getArg(aa++);
                    if (context.mode == OverloadResolveContext::Mode::JustTrying)
                    {
                        ConversionCost cost = kConversionCost_None;
                        if (!canCoerce(getType(m_astBuilder, valParamRef), arg->type, arg, &cost))
                        {
                            success = false;
                        }
                        candidate.conversionCostSum += cost;
                    }
                    else
                    {
                        arg = coerce(getType(m_astBuilder, valParamRef), arg);
                    }
                }

                // If we have an argument to work with, then we will
                // try to extract its speicalization-time constant value.
                //
                Val* val = nullptr;
                if( arg )
                {
                    val = ExtractGenericArgInteger(arg, getType(m_astBuilder, valParamRef), context.mode == OverloadResolveContext::Mode::JustTrying ? nullptr : getSink());
                }

                // If any of the above checking steps fail and we don't
                // have a value to work with here, we will instead
                // use an "error" value to stand in for the argument.
                //
                if( !val )
                {
                    val = m_astBuilder->create<ErrorIntVal>();
                }
                checkedArgs.add(val);
            }
            else
            {
                continue;
            }
        }

        // Once we are done processing the parameters of the generic,
        // we will have build up a usable `checkedArgs` array and
        // can return to the caller a report of whether we
        // were successful or not.
        //
        return success;
    }

    bool SemanticsVisitor::TryCheckOverloadCandidateTypes(
        OverloadResolveContext&	context,
        OverloadCandidate&		candidate)
    {
        Index argCount = context.getArgCount();

        List<Type*> paramTypes;
//        List<DeclRef<ParamDecl>> params;
        switch (candidate.flavor)
        {
        case OverloadCandidate::Flavor::Func:
            for (auto param : getParameters(candidate.item.declRef.as<CallableDecl>()))
            {
                auto paramType = getType(m_astBuilder, param);
                paramTypes.add(paramType);
            }
            break;

        case OverloadCandidate::Flavor::Expr:
            {
                auto funcType = candidate.funcType;
                Count paramCount = funcType->getParamCount();
                for (Index i = 0; i < paramCount; ++i)
                {
                    auto paramType = funcType->getParamType(i);

                    if(auto paramDirectionType = as<ParamDirectionType>(paramType))
                    {
                        paramType = paramDirectionType->getValueType();
                    }

                    paramTypes.add(paramType);
                }
            }
            break;

        case OverloadCandidate::Flavor::Generic:
            return TryCheckGenericOverloadCandidateTypes(context, candidate);

        default:
            SLANG_UNEXPECTED("unknown flavor of overload candidate");
            break;
        }

        // Note(tfoley): We might have fewer arguments than parameters in the
        // case where one or more parameters had defaults.
        SLANG_RELEASE_ASSERT(argCount <= paramTypes.getCount());

        for (Index ii = 0; ii < argCount; ++ii)
        {
            auto& arg = context.getArg(ii);
            auto argType = context.getArgType(ii);
            auto paramType = paramTypes[ii];

            if (context.mode == OverloadResolveContext::Mode::JustTrying)
            {
                ConversionCost cost = kConversionCost_None;
                if( context.disallowNestedConversions )
                {
                    // We need an exact match in this case.
                    if(!paramType->equals(argType))
                        return false;
                }
                else if (!canCoerce(paramType, argType, arg, &cost))
                {
                    return false;
                }
                candidate.conversionCostSum += cost;
            }
            else
            {
                arg = coerce(paramType, arg);
            }
        }
        return true;
    }

    bool isEffectivelyMutating(CallableDecl* decl)
    {
        if(decl->hasModifier<MutatingAttribute>())
            return true;

        if(decl->hasModifier<NonmutatingAttribute>())
            return false;

        if(as<SetterDecl>(decl))
            return true;

        return false;
    }

    bool SemanticsVisitor::TryCheckOverloadCandidateDirections(
        OverloadResolveContext&     context,
        OverloadCandidate const&    candidate)
    {
        if(candidate.flavor != OverloadCandidate::Flavor::Func)
            return true;

        auto funcDeclRef = candidate.item.declRef.as<CallableDecl>();
        SLANG_ASSERT(funcDeclRef);

        // Note: This operation was originally introduced as
        // a place to add checking around l-value-ness of arguments
        // and parameters, but currently that checking is being
        // done in other places.
        //
        // For now we will only use this step to check the
        // mutability of the `this` parameter where necessary.
        //
        if(!isEffectivelyStatic(funcDeclRef.getDecl()))
        {
            if(isEffectivelyMutating(funcDeclRef.getDecl()))
            {
                if(context.baseExpr && !context.baseExpr->type.isLeftValue)
                {
                    if(context.mode == OverloadResolveContext::Mode::ForReal)
                    {
                        getSink()->diagnose(context.loc, Diagnostics::mutatingMethodOnImmutableValue, funcDeclRef.getName());
                        maybeDiagnoseThisNotLValue(context.baseExpr);
                    }
                    return false;
                }
            }
        }

        return true;
    }

    bool SemanticsVisitor::TryCheckOverloadCandidateConstraints(
        OverloadResolveContext&		context,
        OverloadCandidate const&	candidate)
    {
        // We only need this step for generics, so always succeed on
        // everything else.
        if(candidate.flavor != OverloadCandidate::Flavor::Generic)
            return true;

        auto genericDeclRef = candidate.item.declRef.as<GenericDecl>();
        SLANG_ASSERT(genericDeclRef); // otherwise we wouldn't be a generic candidate...

        // We should have the existing arguments to the generic
        // handy, so that we can construct a substitution list.

        auto subst = as<GenericSubstitution>(candidate.subst);
        SLANG_ASSERT(subst);

        subst->genericDecl = genericDeclRef.getDecl();
        subst->outer = genericDeclRef.substitutions.substitutions;

        for( auto constraintDecl : genericDeclRef.getDecl()->getMembersOfType<GenericTypeConstraintDecl>() )
        {
            auto subset = genericDeclRef.substitutions;
            subset.substitutions = subst;
            DeclRef<GenericTypeConstraintDecl> constraintDeclRef(
                constraintDecl, subset);

            auto sub = getSub(m_astBuilder, constraintDeclRef);
            auto sup = getSup(m_astBuilder, constraintDeclRef);

            auto subTypeWitness = tryGetSubtypeWitness(sub, sup);
            if(subTypeWitness)
            {
                subst->args.add(subTypeWitness);
            }
            else
            {
                if(context.mode != OverloadResolveContext::Mode::JustTrying)
                {
                    getSink()->diagnose(context.loc, Diagnostics::typeArgumentDoesNotConformToInterface, sub, sup);
                }
                return false;
            }
        }

        // Done checking all the constraints, hooray.
        return true;
    }

    void SemanticsVisitor::TryCheckOverloadCandidate(
        OverloadResolveContext&		context,
        OverloadCandidate&			candidate)
    {
        if (!TryCheckOverloadCandidateArity(context, candidate))
            return;

        candidate.status = OverloadCandidate::Status::ArityChecked;
        if (!TryCheckOverloadCandidateFixity(context, candidate))
            return;

        candidate.status = OverloadCandidate::Status::FixityChecked;
        if (!TryCheckOverloadCandidateTypes(context, candidate))
            return;

        candidate.status = OverloadCandidate::Status::TypeChecked;
        if (!TryCheckOverloadCandidateDirections(context, candidate))
            return;

        candidate.status = OverloadCandidate::Status::DirectionChecked;
        if (!TryCheckOverloadCandidateConstraints(context, candidate))
            return;

        candidate.status = OverloadCandidate::Status::Applicable;
    }

    Expr* SemanticsVisitor::createGenericDeclRef(
        Expr*                baseExpr,
        Expr*                originalExpr,
        GenericSubstitution* subst)
    {
        auto baseDeclRefExpr = as<DeclRefExpr>(baseExpr);
        if (!baseDeclRefExpr)
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), baseExpr, "expected a reference to a generic declaration");
            return CreateErrorExpr(originalExpr);
        }
        auto baseGenericRef = baseDeclRefExpr->declRef.as<GenericDecl>();
        if (!baseGenericRef)
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), baseExpr, "expected a reference to a generic declaration");
            return CreateErrorExpr(originalExpr);
        }

        subst->genericDecl = baseGenericRef.getDecl();
        subst->outer = baseGenericRef.substitutions.substitutions;

        DeclRef<Decl> innerDeclRef(getInner(baseGenericRef), subst);

        Expr* base = nullptr;
        if (auto mbrExpr = as<MemberExpr>(baseExpr))
            base = mbrExpr->baseExpression;

        return ConstructDeclRefExpr(
            innerDeclRef,
            base,
            originalExpr->loc,
            originalExpr);
    }

    Expr* SemanticsVisitor::CompleteOverloadCandidate(
        OverloadResolveContext&		context,
        OverloadCandidate&			candidate)
    {
        // special case for generic argument inference failure
        if (candidate.status == OverloadCandidate::Status::GenericArgumentInferenceFailed)
        {
            String callString = getCallSignatureString(context);
            getSink()->diagnose(
                context.loc,
                Diagnostics::genericArgumentInferenceFailed,
                callString);

            String declString = ASTPrinter::getDeclSignatureString(candidate.item, m_astBuilder);
            getSink()->diagnose(candidate.item.declRef, Diagnostics::genericSignatureTried, declString);
            goto error;
        }

        context.mode = OverloadResolveContext::Mode::ForReal;

        if (!TryCheckOverloadCandidateClassNewMatchUp(context, candidate))
            goto error;

        if (!TryCheckOverloadCandidateArity(context, candidate))
            goto error;

        if (!TryCheckOverloadCandidateFixity(context, candidate))
            goto error;

        if (!TryCheckOverloadCandidateTypes(context, candidate))
            goto error;

        if (!TryCheckOverloadCandidateDirections(context, candidate))
            goto error;

        if (!TryCheckOverloadCandidateConstraints(context, candidate))
            goto error;

        {
            auto originalAppExpr = as<AppExprBase>(context.originalExpr);


            Expr* baseExpr;
            switch(candidate.flavor)
            {
            case OverloadCandidate::Flavor::Func:
            case OverloadCandidate::Flavor::Generic:
                baseExpr = ConstructLookupResultExpr(
                    candidate.item,
                    context.baseExpr,
                    context.funcLoc,
                    originalAppExpr ? originalAppExpr->functionExpr : nullptr);
                break;
            case OverloadCandidate::Flavor::Expr:
            default:
                baseExpr = nullptr;
                break;
            }

            switch(candidate.flavor)
            {
            case OverloadCandidate::Flavor::Func:
                {
                    AppExprBase* callExpr = as<InvokeExpr>(context.originalExpr);
                    if(!callExpr)
                    {
                        callExpr = m_astBuilder->create<InvokeExpr>();
                        callExpr->loc = context.loc;
                        for(Index aa = 0; aa < context.argCount; ++aa)
                            callExpr->arguments.add(context.getArg(aa));
                    }

                    callExpr->originalFunctionExpr = callExpr->functionExpr;
                    callExpr->functionExpr = baseExpr;
                    callExpr->type = QualType(candidate.resultType);

                    // A call may yield an l-value, and we should take a look at the candidate to be sure
                    if(auto subscriptDeclRef = candidate.item.declRef.as<SubscriptDecl>())
                    {
                        const auto& decl = subscriptDeclRef.getDecl();
                        if (decl->getMembersOfType<SetterDecl>().isNonEmpty() || decl->getMembersOfType<RefAccessorDecl>().isNonEmpty())
                        {
                            callExpr->type.isLeftValue = true;
                        }
                    }

                    // TODO: there may be other cases that confer l-value-ness

                    return callExpr;
                }

                break;

            case OverloadCandidate::Flavor::Expr:
                {
                    AppExprBase* callExpr = as<InvokeExpr>(context.originalExpr);
                    if (!callExpr)
                    {
                        callExpr = m_astBuilder->create<InvokeExpr>();
                        callExpr->loc = context.loc;
                        for (Index aa = 0; aa < context.argCount; ++aa)
                            callExpr->arguments.add(context.getArg(aa));
                    }

                    callExpr->originalFunctionExpr = callExpr->functionExpr;
                    callExpr->type = QualType(candidate.resultType);

                    return callExpr;

                }
                break;

            case OverloadCandidate::Flavor::Generic:
                return createGenericDeclRef(
                    baseExpr,
                    context.originalExpr,
                    as<GenericSubstitution>(candidate.subst));
                break;

            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), context.loc, "unknown overload candidate flavor");
                break;
            }
        }


    error:

        if(context.originalExpr)
        {
            return CreateErrorExpr(context.originalExpr);
        }
        else
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), context.loc, "no original expression for overload result");
            return nullptr;
        }
    }

        /// Does the given `declRef` represent an interface requirement?
    bool isInterfaceRequirement(DeclRef<Decl> const& declRef)
    {
        if(!declRef)
            return false;

        auto parent = declRef.getParent();
        if(parent.as<GenericDecl>())
            parent = parent.getParent();

        if(parent.as<InterfaceDecl>())
            return true;

        return false;
    }

        /// If `declRef` representations a specialization of a generic, returns the number of specialized generic arguments.
        /// Otherwise, returns zero.
        ///
    Int SemanticsVisitor::getSpecializedParamCount(DeclRef<Decl> const& declRef)
    {
        if(!declRef)
            return 0;

        // A specialization of a generic must point at the
        // "inner" declaration of a generic. That means that
        // the parent of the decl ref must be a generic.
        //
        auto parentGeneric = declRef.getParent().as<GenericDecl>();
        if(!parentGeneric)
            return 0;
        //
        // Furthermore, the declaration we are considering
        // must be the single "inner" declaration of the
        // parent generic (and not somthing like a generic
        // parameter).
        //
        if( parentGeneric.getDecl()->inner != declRef.getDecl())
            return 0;

        return CountParameters(parentGeneric).required;
    }

    int SemanticsVisitor::CompareLookupResultItems(
        LookupResultItem const& left,
        LookupResultItem const& right)
    {
        // It is possible for lookup to return both an interface requirement
        // and the concrete function that satisfies that requirement.
        // We always want to favor a concrete method over an interface
        // requirement it might override.
        //
        // TODO: This should turn into a more detailed check such that
        // a candidate for declaration A is always better than a candidate
        // for declaration B if A is an override of B. We can't
        // easily make that check right now because we aren't tracking
        // this kind of "is an override of ..." information on declarations
        // directly (it is only visible through the requirement witness
        // information for inheritance declarations).
        //
        bool leftIsInterfaceRequirement = isInterfaceRequirement(left.declRef);
        bool rightIsInterfaceRequirement = isInterfaceRequirement(right.declRef);
        if(leftIsInterfaceRequirement != rightIsInterfaceRequirement)
            return int(leftIsInterfaceRequirement) - int(rightIsInterfaceRequirement);

        // TODO: We should always have rules such that in a tie a declaration
        // A::m is better than B::m when all other factors are equal and
        // A inherits from B.

        // TODO: There are other cases like this we need to add in terms
        // of ranking/prioritizing overloads, around things like
        // "transparent" members, or when lookup proceeds from an "inner"
        // to an "outer" scope. In many cases the right way to proceed
        // could involve attaching a distance/cost/rank to things directly
        // as part of lookup, and in other cases it might be best handled
        // as a semantic check based on the actual declarations found.

        return 0;
    }

    int SemanticsVisitor::compareOverloadCandidateSpecificity(
        LookupResultItem const& left,
        LookupResultItem const& right)
    {
        // HACK: if both items refer to the same declaration,
        // then arbitrarily pick one.
        if(left.declRef.equals(right.declRef))
            return -1;

        // There is a very general rule that we would like to enforce
        // in principle:
        //
        // Given candidates A and B, if A being applicable to some
        // arguments implies that B is also applicable, but not vice versa,
        // then A is a more specific/specialized candidate than B.
        //
        // A number of conclusions follow from this general rule.
        // For example, a non-generic declaration will always be
        // more specific than a generic declaration that was specialized
        // to matching types:
        //
        //      int doThing(int a);
        //      T doThing<T>(T a);
        //
        // It is clear that if the non-generic `doThing` is applicable
        // to an argument `x`, then `doThing<int>` is also applicable to
        // `x`. However, knowing that the generic `doThing` was applicable
        // to some `y` doesn't tell us that the non-generic `doThing` can
        // be called on `y`, because `y` could have some type that can't
        // convert to `int`.
        //
        // Similarly, a generic declaration with a subset of the parameters
        // of another generic is always more specialized:
        //
        //      int doThing<T>(vector<T,3> value);
        //      int doThing<T, let N : int>(vector<T,N> value);
        //
        // Here we know that both overloads can apply to `float3`, but only
        // one can apply to `float4`, so the first overload is more
        // specialized/specific.
        //
        // As a final example, a generic which places more constraints
        // on its generic parameters is more specific, all other things
        // being equal:
        //
        //      int doThing<T : IFoo>( T value );
        //      int doThing<T>(T value);
        //
        // In this case we know that the first overload is applicable
        // to a strict subset of the types that the second overload can
        // apply to.
        //
        // The above rules represent the idealized principles we want
        // to implement, but actually implementing that full check here
        // could make overload resolution far more expensive.
        //
        // For now we are going to do something far simpler and hackier,
        // which is to say that a candidate with more generic parameters
        // is always preferred over one with fewer.
        //
        // TODO: We could extend this definition to account for constraints
        // on generic parameters in the count, which would handle the
        // need to prefer a more-constrained generic when possible.
        //
        // TODO: In the long run we should clearly replace this with
        // the more general "does A being applicable imply B being applicable"
        // test.
        //
        // TODO: The principle stated here doesn't take the actual
        // arguments or their types into account, and it might be that
        // in some cases disambiguation of which declaration should be
        // preferred will depend on knowing the actual arguments.
        //
        auto leftSpecCount = getSpecializedParamCount(left.declRef);
        auto rightSpecCount = getSpecializedParamCount(right.declRef);
        if(leftSpecCount != rightSpecCount)
            return int(leftSpecCount - rightSpecCount);

        return 0;
    }

    int SemanticsVisitor::CompareOverloadCandidates(
        OverloadCandidate*	left,
        OverloadCandidate*	right)
    {
        // If one candidate got further along in validation, pick it
        if (left->status != right->status)
            return int(right->status) - int(left->status);

        // If both candidates are applicable, then we need to compare
        // the costs of their type conversion sequences
        if(left->status == OverloadCandidate::Status::Applicable)
        {
            // If one candidate incurred less cost related to
            // implicit conversion of arguments to matching
            // parameter types, then we should prefer that
            // candidate.
            //
            // TODO: This eventually should be refined into
            // a test that checks conversion cost per-argument,
            // and only considers a candidate "better" if it
            // has lower cost for at least one argument, and
            // does not have higher cost for any.
            //
            if (left->conversionCostSum != right->conversionCostSum)
                return left->conversionCostSum - right->conversionCostSum;

            // If both candidates appear to be equally good when it
            // comes to the per-argument conversions required,
            // then we have two other categories of criteria we
            // can look at to disambiguate things:
            //
            // 1. We can look at how the lookup process found `left` and `right`
            //    do decide which is a better match based purely on how "far away"
            //    they are for lookup purposes. A canonincal example here would
            //    be if one declaration shadows or overrides the other.
            //
            // 2. We can look at parameter lists of `left` and `right`, their types, etc.
            //    do decide which is a better match based purely on structure.
            //    Canonical examples in this case would be preferring a non-generic
            //    candidate over a generic one, preferring a non-variadic candidate
            //    over a variadic one, and preferring a candidate with fewer
            //    default parameters over one with more.
            //
            // Deciding how to order/interleave these two categories of criteria
            // is an important design decision.
            //
            // For example, consider:
            //
            //      float f(float x);
            //
            //      struct S
            //      {
            //          int f<T>(T x);
            //
            //          float g(float y) { return f(y); }
            //      }
            //
            // In terms of structural/type matching, the global `f` is a more specialized
            // candidate at the call site, while in terms of lookup/lexical crieteria
            // the `S.f` declaration is better.
            //
            // For now we are considering lookup/overriding concerns first (so
            // we would bias in favor of selecting `S.f` in the above example), and then
            // structural/type concerns, but a more nuanced approach may be
            // required in the future to better match programmer intuition.
            //
            auto itemDiff = CompareLookupResultItems(left->item, right->item);
            if(itemDiff)
                return itemDiff;
            auto specificityDiff = compareOverloadCandidateSpecificity(left->item, right->item);
            if(specificityDiff)
                return specificityDiff;
        }

        return 0;
    }

    void SemanticsVisitor::AddOverloadCandidateInner(
        OverloadResolveContext& context,
        OverloadCandidate&		candidate)
    {
        // Filter our existing candidates, to remove any that are worse than our new one

        bool keepThisCandidate = true; // should this candidate be kept?

        if (context.bestCandidates.getCount() != 0)
        {
            // We have multiple candidates right now, so filter them.
            bool anyFiltered = false;
            // Note that we are querying the list length on every iteration,
            // because we might remove things.
            for (Index cc = 0; cc < context.bestCandidates.getCount(); ++cc)
            {
                int cmp = CompareOverloadCandidates(&candidate, &context.bestCandidates[cc]);
                if (cmp < 0)
                {
                    // our new candidate is better!

                    // remove it from the list (by swapping in a later one)
                    context.bestCandidates.fastRemoveAt(cc);
                    // and then reduce our index so that we re-visit the same index
                    --cc;

                    anyFiltered = true;
                }
                else if(cmp > 0)
                {
                    // our candidate is worse!
                    keepThisCandidate = false;
                }
            }
            // It should not be possible that we removed some existing candidate *and*
            // chose not to keep this candidate (otherwise the better-ness relation
            // isn't transitive). Therefore we confirm that we either chose to keep
            // this candidate (in which case filtering is okay), or we didn't filter
            // anything.
            SLANG_ASSERT(keepThisCandidate || !anyFiltered);
        }
        else if(context.bestCandidate)
        {
            // There's only one candidate so far
            int cmp = CompareOverloadCandidates(&candidate, context.bestCandidate);
            if(cmp < 0)
            {
                // our new candidate is better!
                context.bestCandidate = nullptr;
            }
            else if (cmp > 0)
            {
                // our candidate is worse!
                keepThisCandidate = false;
            }
        }

        // If our candidate isn't good enough, then drop it
        if (!keepThisCandidate)
            return;

        // Otherwise we want to keep the candidate
        if (context.bestCandidates.getCount() > 0)
        {
            // There were already multiple candidates, and we are adding one more
            context.bestCandidates.add(candidate);
        }
        else if (context.bestCandidate)
        {
            // There was a unique best candidate, but now we are ambiguous
            context.bestCandidates.add(*context.bestCandidate);
            context.bestCandidates.add(candidate);
            context.bestCandidate = nullptr;
        }
        else
        {
            // This is the only candidate worth keeping track of right now
            context.bestCandidateStorage = candidate;
            context.bestCandidate = &context.bestCandidateStorage;
        }
    }

    void SemanticsVisitor::AddOverloadCandidate(
        OverloadResolveContext& context,
        OverloadCandidate&		candidate)
    {
        // Try the candidate out, to see if it is applicable at all.
        TryCheckOverloadCandidate(context, candidate);

        // Now (potentially) add it to the set of candidate overloads to consider.
        AddOverloadCandidateInner(context, candidate);
    }

    void SemanticsVisitor::AddFuncOverloadCandidate(
        LookupResultItem			item,
        DeclRef<CallableDecl>             funcDeclRef,
        OverloadResolveContext&		context)
    {
        auto funcDecl = funcDeclRef.getDecl();
        ensureDecl(funcDecl, DeclCheckState::CanUseFuncSignature);

        // If this function is a redeclaration,
        // then we don't want to include it multiple times,
        // and mistakenly think we have an ambiguous call.
        //
        // Instead, we will carefully consider only the
        // "primary" declaration of any callable.
        if (auto primaryDecl = funcDecl->primaryDecl)
        {
            if (funcDecl != primaryDecl)
            {
                // This is a redeclaration, so we don't
                // want to consider it. The primary
                // declaration should also get considered
                // for the call site and it will match
                // anything this declaration would have
                // matched.
                return;
            }
        }

        OverloadCandidate candidate;
        candidate.flavor = OverloadCandidate::Flavor::Func;
        candidate.item = item;
        candidate.resultType = getResultType(m_astBuilder, funcDeclRef);

        AddOverloadCandidate(context, candidate);
    }

    void SemanticsVisitor::AddFuncOverloadCandidate(
        FuncType*        funcType,
        OverloadResolveContext& context)
    {
        OverloadCandidate candidate;
        candidate.flavor = OverloadCandidate::Flavor::Expr;
        candidate.funcType = funcType;
        candidate.resultType = funcType->getResultType();

        AddOverloadCandidate(context, candidate);
    }

    void SemanticsVisitor::AddCtorOverloadCandidate(
        LookupResultItem            typeItem,
        Type*                type,
        DeclRef<ConstructorDecl>    ctorDeclRef,
        OverloadResolveContext&     context,
        Type*                resultType)
    {
        SLANG_UNUSED(type)

        ensureDecl(ctorDeclRef, DeclCheckState::CanUseFuncSignature);

        // `typeItem` refers to the type being constructed (the thing
        // that was applied as a function) so we need to construct
        // a `LookupResultItem` that refers to the constructor instead

        LookupResultItem ctorItem;
        ctorItem.declRef = ctorDeclRef;
        ctorItem.breadcrumbs = new LookupResultItem::Breadcrumb(
            LookupResultItem::Breadcrumb::Kind::Member,
            typeItem.declRef,
            nullptr,
            typeItem.breadcrumbs);

        OverloadCandidate candidate;
        candidate.flavor = OverloadCandidate::Flavor::Func;
        candidate.item = ctorItem;
        candidate.resultType = resultType;

        AddOverloadCandidate(context, candidate);
    }

    DeclRef<Decl> SemanticsVisitor::SpecializeGenericForOverload(
        DeclRef<GenericDecl>    genericDeclRef,
        OverloadResolveContext& context)
    {
        ensureDecl(genericDeclRef, DeclCheckState::CanSpecializeGeneric);

        ConstraintSystem constraints;
        constraints.loc = context.loc;
        constraints.genericDecl = genericDeclRef.getDecl();

        // Construct a reference to the inner declaration that has any generic
        // parameter substitutions in place already, but *not* any substutions
        // for the generic declaration we are currently trying to infer.
        auto innerDecl = getInner(genericDeclRef);
        DeclRef<Decl> unspecializedInnerRef = DeclRef<Decl>(innerDecl, genericDeclRef.substitutions);

        // Check what type of declaration we are dealing with, and then try
        // to match it up with the arguments accordingly...
        if (auto funcDeclRef = unspecializedInnerRef.as<CallableDecl>())
        {
            auto params = getParameters(funcDeclRef).toArray();

            Index argCount = context.getArgCount();
            Index paramCount = params.getCount();

            // If there are too many arguments, we cannot possibly have a match.
            //
            // Note that if there are *too few* arguments, we might still have
            // a match, because the other arguments might have default values
            // that can be used.
            //
            if (argCount > paramCount)
            {
                return DeclRef<Decl>(nullptr, nullptr);
            }

            for (Index aa = 0; aa < argCount; ++aa)
            {
                // The question here is whether failure to "unify" an argument
                // and parameter should lead to immediate failure.
                //
                // The case that is interesting is if we want to unify, say:
                // `vector<float,N>` and `vector<int,3>`
                //
                // It is clear that we should solve with `N = 3`, and then
                // a later step may find that the resulting types aren't
                // actually a match.
                //
                // A more refined approach to "unification" could of course
                // see that `int` can convert to `float` and use that fact.
                // (and indeed we already use something like this to unify
                // `float` and `vector<T,3>`)
                //
                // So the question is then whether a mismatch during the
                // unification step should be taken as an immediate failure...

                TryUnifyTypes(constraints, context.getArgTypeForInference(aa, this), getType(m_astBuilder, params[aa]));
            }
        }
        else
        {
            // TODO(tfoley): any other cases needed here?
            return DeclRef<Decl>(nullptr, nullptr);
        }

        auto constraintSubst = TrySolveConstraintSystem(&constraints, genericDeclRef);
        if (!constraintSubst)
        {
            // constraint solving failed
            return DeclRef<Decl>(nullptr, nullptr);
        }

        // We can now construct a reference to the inner declaration using
        // the solution to our constraints.
        return DeclRef<Decl>(innerDecl, constraintSubst);
    }

    void SemanticsVisitor::AddTypeOverloadCandidates(
        Type*            type,
        OverloadResolveContext&	context)
    {
        // The code being checked is trying to apply `type` like a function.
        // Semantically, the operations `T(args...)` is equivalent to
        // `T.__init(args...)` if we had a surface syntax that supported
        // looking up `__init` declarations by that name.
        //
        // Internally, all `__init` declarations are stored with the name
        // `$init`, to avoid potential conflicts if a user decided to name
        // a field/method `__init`.
        //
        // We will look up all the initializers on `type` by looking up
        // its members named `$init`, and then proceed to perform overload
        // resolution with what we find.
        //
        // TODO: One wrinkle here is single-argument constructor syntax.
        // An operation like `(T) oneArg` or `T(oneArg)` is currently
        // treated as a call expression, but we might want such cases
        // to go through the type coercion logic first/instead, because
        // by doing so we could weed out cases where a type is "constructed"
        // from a value of the same type. There is no need in Slang for
        // "copy constructors" but the stdlib currently has to define
        // some just to make code that does, e.g., `float(1.0f)` work.

        LookupResult initializers = lookUpMember(
            m_astBuilder,
            this,
            getName("$init"),
            type,
            LookupMask::Default,
            LookupOptions::NoDeref);

        AddOverloadCandidates(initializers, context);
    }

    void SemanticsVisitor::AddDeclRefOverloadCandidates(
        LookupResultItem		item,
        OverloadResolveContext&	context)
    {
        auto declRef = item.declRef;

        if (auto funcDeclRef = item.declRef.as<CallableDecl>())
        {
            AddFuncOverloadCandidate(item, funcDeclRef, context);
        }
        else if (auto aggTypeDeclRef = item.declRef.as<AggTypeDecl>())
        {
            auto type = DeclRefType::create(m_astBuilder, aggTypeDeclRef);
            AddTypeOverloadCandidates(type, context);
        }
        else if (auto genericDeclRef = item.declRef.as<GenericDecl>())
        {
            // Try to infer generic arguments, based on the context
            DeclRef<Decl> innerRef = SpecializeGenericForOverload(genericDeclRef, context);

            if (innerRef)
            {
                // If inference works, then we've now got a
                // specialized declaration reference we can apply.

                LookupResultItem innerItem;
                innerItem.breadcrumbs = item.breadcrumbs;
                innerItem.declRef = innerRef;

                AddDeclRefOverloadCandidates(innerItem, context);
            }
            else
            {
                // If inference failed, then we need to create
                // a candidate that can be used to reflect that fact
                // (so we can report a good error)
                OverloadCandidate candidate;
                candidate.item = item;
                candidate.flavor = OverloadCandidate::Flavor::UnspecializedGeneric;
                candidate.status = OverloadCandidate::Status::GenericArgumentInferenceFailed;

                AddOverloadCandidateInner(context, candidate);
            }
        }
        else if( auto typeDefDeclRef = item.declRef.as<TypeDefDecl>() )
        {
            auto type = getNamedType(m_astBuilder, typeDefDeclRef);
            AddTypeOverloadCandidates(type, context);
        }
        else if( auto genericTypeParamDeclRef = item.declRef.as<GenericTypeParamDecl>() )
        {
            auto type = DeclRefType::create(m_astBuilder, genericTypeParamDeclRef);
            AddTypeOverloadCandidates(type, context);
        }
        else
        {
            // TODO(tfoley): any other cases needed here?
        }
    }

    void SemanticsVisitor::AddOverloadCandidates(
        LookupResult const&     result,
        OverloadResolveContext&	context)
    {
        if(result.isOverloaded())
        {
            for(auto item : result.items)
            {
                AddDeclRefOverloadCandidates(item, context);
            }
        }
        else
        {
            AddDeclRefOverloadCandidates(result.item, context);
        }
    }

    void SemanticsVisitor::AddOverloadCandidates(
        Expr*            funcExpr,
        OverloadResolveContext& context)
    {
        // A call of the form `(<something>)(<args>)` should be
        // resolved as if the user wrote `<something>(<args>)`,
        // so that we avoid introducing intermediate expressions
        // of function type in cases where they are not needed.
        //
        while(auto parenExpr = as<ParenExpr>(funcExpr))
        {
            funcExpr = parenExpr->base;
        }

        auto funcExprType = funcExpr->type;

        if (auto declRefExpr = as<DeclRefExpr>(funcExpr))
        {
            // The expression directly referenced a declaration,
            // so we can use that declaration directly to look
            // for anything applicable.
            AddDeclRefOverloadCandidates(LookupResultItem(declRefExpr->declRef), context);
        }
        else if (auto funcType = as<FuncType>(funcExprType))
        {
            // TODO(tfoley): deprecate this path...
            AddFuncOverloadCandidate(funcType, context);
        }
        else if (auto overloadedExpr = as<OverloadedExpr>(funcExpr))
        {
            AddOverloadCandidates(overloadedExpr->lookupResult2, context);
        }
        else if (auto overloadedExpr2 = as<OverloadedExpr2>(funcExpr))
        {
            for (auto item : overloadedExpr2->candidiateExprs)
            {
                AddOverloadCandidates(item, context);
            }
        }
        else if (auto typeType = as<TypeType>(funcExprType))
        {
            // If none of the above cases matched, but we are
            // looking at a type, then I suppose we have
            // a constructor call on our hands.
            //
            // TODO(tfoley): are there any meaningful types left
            // that aren't declaration references?
            auto type = typeType->type;
            AddTypeOverloadCandidates(type, context);
            return;
        }
    }

    String SemanticsVisitor::getCallSignatureString(
        OverloadResolveContext&     context)
    {
        StringBuilder argsListBuilder;
        argsListBuilder << "(";

        UInt argCount = context.getArgCount();
        for( UInt aa = 0; aa < argCount; ++aa )
        {
            if(aa != 0) argsListBuilder << ", ";
            context.getArgType(aa)->toText(argsListBuilder);
        }
        argsListBuilder << ")";
        return argsListBuilder.ProduceString();
    }

    Expr* SemanticsVisitor::ResolveInvoke(InvokeExpr * expr)
    {
        OverloadResolveContext context;
        // check if this is a stdlib operator call, if so we want to use cached results
        // to speed up compilation
        bool shouldAddToCache = false;
        OperatorOverloadCacheKey key;
        TypeCheckingCache* typeCheckingCache = getLinkage()->getTypeCheckingCache();
        if (auto opExpr = as<OperatorExpr>(expr))
        {
            if (key.fromOperatorExpr(opExpr))
            {
                OverloadCandidate candidate;
                if (typeCheckingCache->resolvedOperatorOverloadCache.TryGetValue(key, candidate))
                {
                    context.bestCandidateStorage = candidate;
                    context.bestCandidate = &context.bestCandidateStorage;
                }
                else
                {
                    shouldAddToCache = true;
                }
            }
        }

        // Look at the base expression for the call, and figure out how to invoke it.
        auto funcExpr = expr->functionExpr;
        auto funcExprType = funcExpr->type;

        // If we are trying to apply an erroneous expression, then just bail out now.
        if(IsErrorExpr(funcExpr))
        {
            return CreateErrorExpr(expr);
        }
        // If any of the arguments is an error, then we should bail out, to avoid
        // cascading errors where we successfully pick an overload, but not the one
        // the user meant.
        for (auto arg : expr->arguments)
        {
            if (IsErrorExpr(arg))
                return CreateErrorExpr(expr);
        }

        for (auto& arg : expr->arguments)
        {
            arg = maybeOpenRef(arg);
        }

        auto funcType = as<FuncType>(funcExprType);
        for (Index i = 0; i < expr->arguments.getCount(); i++)
        {
            auto& arg = expr->arguments[i];
            if (funcType && i < (Index)funcType->getParamCount())
            {
                if (funcType->getParamDirection(i) == kParameterDirection_Out)
                    continue;
            }
            arg = maybeOpenExistential(arg);
        }

        context.originalExpr = expr;
        context.funcLoc = funcExpr->loc;

        context.argCount = expr->arguments.getCount();
        context.args = expr->arguments.getBuffer();
        context.loc = expr->loc;

        if (auto funcMemberExpr = as<MemberExpr>(funcExpr))
        {
            context.baseExpr = funcMemberExpr->baseExpression;
        }
        else if (auto funcOverloadExpr = as<OverloadedExpr>(funcExpr))
        {
            context.baseExpr = funcOverloadExpr->base;
        }
        else if (auto funcOverloadExpr2 = as<OverloadedExpr2>(funcExpr))
        {
            context.baseExpr = funcOverloadExpr2->base;
        }

        // TODO: We should have a special case here where an `InvokeExpr`
        // with a single argument where the base/func expression names
        // a type should always be treated as an explicit type coercion
        // (and hence bottleneck through `coerce()`) instead of just
        // as a constructor call.
        //
        // Such a special-case would help us handle cases of identity
        // casts (casting an expression to the type it already has),
        // without needing dummy initializer/constructor declarations.
        //
        // Handling that special casing here (rather than in, say,
        // `visitTypeCastExpr`) would allow us to continue to ensure
        // that `(T) expr` and `T(expr)` continue to be semantically
        // equivalent in (almost) all cases.

        if (!context.bestCandidate)
        {
            AddOverloadCandidates(funcExpr, context);
        }

        if (context.bestCandidates.getCount() > 0)
        {
            // Things were ambiguous.

            // It might be that things were only ambiguous because
            // one of the argument expressions had an error, and
            // so a bunch of candidates could match at that position.
            //
            // If any argument was an error, we skip out on printing
            // another message, to avoid cascading errors.
            for (auto arg : expr->arguments)
            {
                if (IsErrorExpr(arg))
                {
                    return CreateErrorExpr(expr);
                }
            }

            Name* funcName = nullptr;
            {
                Expr* baseExpr = funcExpr;

                if(auto baseGenericApp = as<GenericAppExpr>(baseExpr))
                    baseExpr = baseGenericApp->functionExpr;

                if (auto baseVar = as<VarExpr>(baseExpr))
                    funcName = baseVar->name;
                else if(auto baseMemberRef = as<MemberExpr>(baseExpr))
                    funcName = baseMemberRef->name;
                else if(auto baseOverloaded = as<OverloadedExpr>(baseExpr))
                    funcName = baseOverloaded->name;
            }

            String argsList = getCallSignatureString(context);

            if (context.bestCandidates[0].status != OverloadCandidate::Status::Applicable)
            {
                // There were multiple equally-good candidates, but none actually usable.
                // We will construct a diagnostic message to help out.

                if (funcName)
                {
                    getSink()->diagnose(expr, Diagnostics::noApplicableOverloadForNameWithArgs, funcName, argsList);
                }
                else
                {
                    getSink()->diagnose(expr, Diagnostics::noApplicableWithArgs, argsList);
                }
            }
            else
            {
                // There were multiple applicable candidates, so we need to report them.

                if (funcName)
                {
                    getSink()->diagnose(expr, Diagnostics::ambiguousOverloadForNameWithArgs, funcName, argsList);
                }
                else
                {
                    getSink()->diagnose(expr, Diagnostics::ambiguousOverloadWithArgs, argsList);
                }
            }

            {
                Index candidateCount = context.bestCandidates.getCount();
                Index maxCandidatesToPrint = 10; // don't show too many candidates at once...
                Index candidateIndex = 0;
                for (auto candidate : context.bestCandidates)
                {
                    String declString = ASTPrinter::getDeclSignatureString(candidate.item, m_astBuilder);

//                        declString = declString + "[" + String(candidate.conversionCostSum) + "]";

#if 0
                    // Debugging: ensure that we don't consider multiple declarations of the same operation
                    if (auto decl = as<CallableDecl>(candidate.item.declRef.decl))
                    {
                        char buffer[1024];
                        sprintf_s(buffer, sizeof(buffer), "[this:%p, primary:%p, next:%p]",
                            decl,
                            decl->primaryDecl,
                            decl->nextDecl);
                        declString.append(buffer);
                    }
#endif

                    getSink()->diagnose(candidate.item.declRef, Diagnostics::overloadCandidate, declString);

                    candidateIndex++;
                    if (candidateIndex == maxCandidatesToPrint)
                        break;
                }
                if (candidateIndex != candidateCount)
                {
                    getSink()->diagnose(expr, Diagnostics::moreOverloadCandidates, candidateCount - candidateIndex);
                }
            }

            return CreateErrorExpr(expr);
        }
        else if (context.bestCandidate)
        {
            // There was one best candidate, even if it might not have been
            // applicable in the end.
            // We will report errors for this one candidate, then, to give
            // the user the most help we can.
            if (shouldAddToCache)
                typeCheckingCache->resolvedOperatorOverloadCache[key] = *context.bestCandidate;
            return CompleteOverloadCandidate(context, *context.bestCandidate);
        }
        else
        {
            // Nothing at all was found that we could even consider invoking
            getSink()->diagnose(expr->functionExpr, Diagnostics::expectedFunction, funcExprType);
            expr->type = QualType(m_astBuilder->getErrorType());
            return expr;
        }
    }

    void SemanticsVisitor::AddGenericOverloadCandidate(
        LookupResultItem		baseItem,
        OverloadResolveContext&	context)
    {
        if (auto genericDeclRef = baseItem.declRef.as<GenericDecl>())
        {
            ensureDecl(genericDeclRef, DeclCheckState::CanSpecializeGeneric);

            OverloadCandidate candidate;
            candidate.flavor = OverloadCandidate::Flavor::Generic;
            candidate.item = baseItem;
            candidate.resultType = nullptr;

            AddOverloadCandidate(context, candidate);
        }
    }

    void SemanticsVisitor::AddGenericOverloadCandidates(
        Expr*	baseExpr,
        OverloadResolveContext&			context)
    {
        if(auto baseDeclRefExpr = as<DeclRefExpr>(baseExpr))
        {
            auto declRef = baseDeclRefExpr->declRef;
            AddGenericOverloadCandidate(LookupResultItem(declRef), context);
        }
        else if (auto overloadedExpr = as<OverloadedExpr>(baseExpr))
        {
            // We are referring to a bunch of declarations, each of which might be generic
            LookupResult result;
            for (auto item : overloadedExpr->lookupResult2.items)
            {
                AddGenericOverloadCandidate(item, context);
            }
        }
        else
        {
            // any other cases?
        }
    }

    Expr* SemanticsExprVisitor::visitGenericAppExpr(GenericAppExpr* genericAppExpr)
    {
        // Start by checking the base expression and arguments.
        auto& baseExpr = genericAppExpr->functionExpr;
        baseExpr = CheckTerm(baseExpr);
        auto& args = genericAppExpr->arguments;
        for (auto& arg : args)
        {
            arg = CheckTerm(arg);
        }

        return checkGenericAppWithCheckedArgs(genericAppExpr);
    }

        /// Check a generic application where the operands have already been checked.
    Expr* SemanticsVisitor::checkGenericAppWithCheckedArgs(GenericAppExpr* genericAppExpr)
    {
        // We are applying a generic to arguments, but there might be multiple generic
        // declarations with the same name, so this becomes a specialized case of
        // overload resolution.

        auto& baseExpr = genericAppExpr->functionExpr;
        auto& args = genericAppExpr->arguments;

        // If there was an error in the base expression,  or in any of
        // the arguments, then just bail.
        if (IsErrorExpr(baseExpr))
        {
            return CreateErrorExpr(genericAppExpr);
        }
        for (auto argExpr : args)
        {
            if (IsErrorExpr(argExpr))
            {
                return CreateErrorExpr(genericAppExpr);
            }
        }

        // Otherwise, let's start looking at how to find an overload...

        OverloadResolveContext context;
        context.originalExpr = genericAppExpr;
        context.funcLoc = baseExpr->loc;
        context.argCount = args.getCount();
        context.args = args.getBuffer();
        context.loc = genericAppExpr->loc;

        context.baseExpr = GetBaseExpr(baseExpr);

        AddGenericOverloadCandidates(baseExpr, context);

        if (context.bestCandidates.getCount() > 0)
        {
            // Things were ambiguous.
            if (context.bestCandidates[0].status != OverloadCandidate::Status::Applicable)
            {
                // There were multiple equally-good candidates, but none actually usable.
                // We will construct a diagnostic message to help out.

                // TODO(tfoley): print a reasonable message here...

                getSink()->diagnose(genericAppExpr, Diagnostics::unimplemented, "no applicable generic");

                return CreateErrorExpr(genericAppExpr);
            }
            else
            {
                // There were multiple viable candidates, but that isn't an error: we just need
                // to complete all of them and create an overloaded expression as a result.

                auto overloadedExpr = m_astBuilder->create<OverloadedExpr2>();
                overloadedExpr->base = context.baseExpr;
                for (auto candidate : context.bestCandidates)
                {
                    auto candidateExpr = CompleteOverloadCandidate(context, candidate);
                    overloadedExpr->candidiateExprs.add(candidateExpr);
                }
                return overloadedExpr;
            }
        }
        else if (context.bestCandidate)
        {
            // There was one best candidate, even if it might not have been
            // applicable in the end.
            // We will report errors for this one candidate, then, to give
            // the user the most help we can.
            return CompleteOverloadCandidate(context, *context.bestCandidate);
        }
        else
        {
            // Nothing at all was found that we could even consider invoking
            getSink()->diagnose(genericAppExpr, Diagnostics::expectedAGeneric, baseExpr->type);
            return CreateErrorExpr(genericAppExpr);
        }
    }

}
