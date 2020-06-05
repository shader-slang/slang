// slang-check-constraint.cpp
#include "slang-check-impl.h"

// This file provides the core services for creating
// and solving constraint systems during semantic checking.
//
// We currently use constraint systems primarily to solve
// for the implied values to use for generic parameters when a
// generic declaration is being applied without explicit
// generic arguments.
//
// Conceptually, our constraint-solving strategy starts by
// trying to "unify" the actual argument types to a call
// with the parameter types of the callee (which may mention
// generic parameters). E.g., if we have a situation like:
//
//      void doIt<T>(T a, vector<T,3> b);
//
//      int x, y;
//      ...
//      doIt(x, y);
//
// then an we would try to unify the type of the argument
// `x` (which is `int`) with the type of the parameter `a`
// (which is `T`). Attempting to unify a concrete type
// and a generic type parameter would (in the simplest case)
// give rise to a constraint that, e.g., `T` must be `int`.
//
// In our example, unifying `y` and `b` creates a more complex
// scenario, because we cannot ever unify `int` with `vector<T,3>`;
// there is no possible value of `T` for which those two types
// are equivalent.
//
// So instead of the simpler approach to unification (which
// works well for languages without implicit type conversion),
// our approach to unification recognizes that scalar types
// can be promoted to vectors, and thus tries to unify the
// type of `y` with the element type of `b`.
//
// When it comes time to actually solve the constraints, we
// might have seemingly conflicting constraints:
//
//      void another<U>(U a, U b);
//
//      float x; int y;
//      another(x, y);
//
// In this case we'd have constraints that `U` must be `int`,
// *and* that `U` must be `float`, which is clearly impossible
// to satisfy. Instead, our constraints are treated as a kind
// of "lower bound" on the type variable, and we combine
// those lower bounds using the "join" operation (in the
// sense of "meet" and "join" on lattices), which ideally
// gives us a type for `U` that all the argument types can
// convert to.

namespace Slang
{
    Type* SemanticsVisitor::TryJoinVectorAndScalarType(
        VectorExpressionType* vectorType,
        BasicExpressionType*  scalarType)
    {
        // Join( vector<T,N>, S ) -> vetor<Join(T,S), N>
        //
        // That is, the join of a vector and a scalar type is
        // a vector type with a joined element type.
        auto joinElementType = TryJoinTypes(
            vectorType->elementType,
            scalarType);
        if(!joinElementType)
            return nullptr;

        return createVectorType(
            joinElementType,
            vectorType->elementCount);
    }

    Type* SemanticsVisitor::TryJoinTypeWithInterface(
        Type*            type,
        DeclRef<InterfaceDecl>      interfaceDeclRef)
    {
        // The most basic test here should be: does the type declare conformance to the trait.
        if(isDeclaredSubtype(type, interfaceDeclRef))
            return type;

        // Just because `type` doesn't conform to the given `interfaceDeclRef`, that
        // doesn't necessarily indicate a failure. It is possible that we have a call
        // like `sqrt(2)` so that `type` is `int` and `interfaceDeclRef` is
        // `__BuiltinFloatingPointType`. The "obvious" answer is that we should infer
        // the type `float`, but it seems like the compiler would have to synthesize
        // that answer from thin air.
        //
        // A robsut/correct solution here might be to enumerate set of types types `S`
        // such that for each type `X` in `S`:
        //
        // * `type` is implicitly convertible to `X`
        // * `X` conforms to the interface named by `interfaceDeclRef`
        //
        // If the set `S` is non-empty then we would try to pick the "best" type from `S`.
        // The "best" type would be a type `Y` such that `Y` is implicitly convertible to
        // every other type in `S`.
        //
        // We are going to implement a much simpler strategy for now, where we only apply
        // the search process if `type` is a builtin scalar type, and then we only search
        // through types `X` that are also builtin scalar types.
        //
        Type* bestType = nullptr;
        if(auto basicType = dynamicCast<BasicExpressionType>(type))
        {
            for(Int baseTypeFlavorIndex = 0; baseTypeFlavorIndex < Int(BaseType::CountOf); baseTypeFlavorIndex++)
            {
                // Don't consider `type`, since we already know it doesn't work.
                if(baseTypeFlavorIndex == Int(basicType->baseType))
                    continue;

                // Look up the type in our session.
                auto candidateType = type->getASTBuilder()->getBuiltinType(BaseType(baseTypeFlavorIndex));
                if(!candidateType)
                    continue;

                // We only want to consider types that implement the target interface.
                if(!isDeclaredSubtype(candidateType, interfaceDeclRef))
                    continue;

                // We only want to consider types where we can implicitly convert from `type`
                if(!canConvertImplicitly(candidateType, type))
                    continue;

                // At this point, we have a candidate type that is usable.
                //
                // If this is our first viable candidate, then it is our best one:
                //
                if(!bestType)
                {
                    bestType = candidateType;
                }
                else
                {
                    // Otherwise, we want to pick the "better" type between `candidateType`
                    // and `bestType`.
                    //
                    // We are going to be a bit loose here, and not worry about the
                    // case where conversion is allowed in both directions.
                    //
                    // TODO: make this completely robust.
                    //
                    if(canConvertImplicitly(bestType, candidateType))
                    {
                        // Our candidate can convert to the current "best" type, so
                        // it is logically a more specific type that satisfies our
                        // constraints, therefore we should keep it.
                        //
                        bestType = candidateType;
                    }
                }
            }
            if(bestType)
                return bestType;
        }

        // For all other cases, we will just bail out for now.
        //
        // TODO: In the future we should build some kind of side data structure
        // to accelerate either one or both of these queries:
        //
        // * Given a type `T`, what types `U` can it convert to implicitly?
        //
        // * Given an interface `I`, what types `U` conform to it?
        //
        // The intersection of the sets returned by these two queries is
        // the set of candidates we would like to consider here.

        return nullptr;
    }

    Type* SemanticsVisitor::TryJoinTypes(
        Type*  left,
        Type*  right)
    {
        // Easy case: they are the same type!
        if (left->equals(right))
            return left;

        // We can join two basic types by picking the "better" of the two
        if (auto leftBasic = as<BasicExpressionType>(left))
        {
            if (auto rightBasic = as<BasicExpressionType>(right))
            {
                auto leftFlavor = leftBasic->baseType;
                auto rightFlavor = rightBasic->baseType;

                // TODO(tfoley): Need a special-case rule here that if
                // either operand is of type `half`, then we promote
                // to at least `float`

                // Return the one that had higher rank...
                if (leftFlavor > rightFlavor)
                    return left;
                else
                {
                    SLANG_ASSERT(rightFlavor > leftFlavor); // equality was handles at the top of this function
                    return right;
                }
            }

            // We can also join a vector and a scalar
            if(auto rightVector = as<VectorExpressionType>(right))
            {
                return TryJoinVectorAndScalarType(rightVector, leftBasic);
            }
        }

        // We can join two vector types by joining their element types
        // (and also their sizes...)
        if( auto leftVector = as<VectorExpressionType>(left))
        {
            if(auto rightVector = as<VectorExpressionType>(right))
            {
                // Check if the vector sizes match
                if(!leftVector->elementCount->equalsVal(rightVector->elementCount))
                    return nullptr;

                // Try to join the element types
                auto joinElementType = TryJoinTypes(
                    leftVector->elementType,
                    rightVector->elementType);
                if(!joinElementType)
                    return nullptr;

                return createVectorType(
                    joinElementType,
                    leftVector->elementCount);
            }

            // We can also join a vector and a scalar
            if(auto rightBasic = as<BasicExpressionType>(right))
            {
                return TryJoinVectorAndScalarType(leftVector, rightBasic);
            }
        }

        // HACK: trying to work trait types in here...
        if(auto leftDeclRefType = as<DeclRefType>(left))
        {
            if( auto leftInterfaceRef = leftDeclRefType->declRef.as<InterfaceDecl>() )
            {
                //
                return TryJoinTypeWithInterface(right, leftInterfaceRef);
            }
        }
        if(auto rightDeclRefType = as<DeclRefType>(right))
        {
            if( auto rightInterfaceRef = rightDeclRefType->declRef.as<InterfaceDecl>() )
            {
                //
                return TryJoinTypeWithInterface(left, rightInterfaceRef);
            }
        }

        // TODO: all the cases for vectors apply to matrices too!

        // Default case is that we just fail.
        return nullptr;
    }

    SubstitutionSet SemanticsVisitor::TrySolveConstraintSystem(
        ConstraintSystem*		system,
        DeclRef<GenericDecl>          genericDeclRef)
    {
        // For now the "solver" is going to be ridiculously simplistic.

        // The generic itself will have some constraints, and for now we add these
        // to the system of constrains we will use for solving for the type variables.
        //
        // TODO: we need to decide whether constraints are used like this to influence
        // how we solve for type/value variables, or whether constraints in the parameter
        // list just work as a validation step *after* we've solved for the types.
        //
        // That is, should we allow `<T : Int>` to be written, and cause us to "infer"
        // that `T` should be the type `Int`? That seems a little silly.
        //
        // Eventually, though, we may want to support type identity constraints, especially
        // on associated types, like `<C where C : IContainer && C.IndexType == Int>`
        // These seem more reasonable to have influence constraint solving, since it could
        // conceivably let us specialize a `X<T> : IContainer` to `X<Int>` if we find
        // that `X<T>.IndexType == T`.
        for( auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(genericDeclRef) )
        {
            if(!TryUnifyTypes(*system, getSub(m_astBuilder, constraintDeclRef), getSup(m_astBuilder, constraintDeclRef)))
                return SubstitutionSet();
        }
        SubstitutionSet resultSubst = genericDeclRef.substitutions;
        // We will loop over the generic parameters, and for
        // each we will try to find a way to satisfy all
        // the constraints for that parameter
        List<Val*> args;
        for (auto m : getMembers(genericDeclRef))
        {
            if (auto typeParam = m.as<GenericTypeParamDecl>())
            {
                Type* type = nullptr;
                for (auto& c : system->constraints)
                {
                    if (c.decl != typeParam.getDecl())
                        continue;

                    auto cType = as<Type>(c.val);
                    SLANG_RELEASE_ASSERT(cType);

                    if (!type)
                    {
                        type = cType;
                    }
                    else
                    {
                        auto joinType = TryJoinTypes(type, cType);
                        if (!joinType)
                        {
                            // failure!
                            return SubstitutionSet();
                        }
                        type = joinType;
                    }

                    c.satisfied = true;
                }

                if (!type)
                {
                    // failure!
                    return SubstitutionSet();
                }
                args.add(type);
            }
            else if (auto valParam = m.as<GenericValueParamDecl>())
            {
                // TODO(tfoley): maybe support more than integers some day?
                // TODO(tfoley): figure out how this needs to interact with
                // compile-time integers that aren't just constants...
                IntVal* val = nullptr;
                for (auto& c : system->constraints)
                {
                    if (c.decl != valParam.getDecl())
                        continue;

                    auto cVal = as<IntVal>(c.val);
                    SLANG_RELEASE_ASSERT(cVal);

                    if (!val)
                    {
                        val = cVal;
                    }
                    else
                    {
                        if(!val->equalsVal(cVal))
                        {
                            // failure!
                            return SubstitutionSet();
                        }
                    }

                    c.satisfied = true;
                }

                if (!val)
                {
                    // failure!
                    return SubstitutionSet();
                }
                args.add(val);
            }
            else
            {
                // ignore anything that isn't a generic parameter
            }
        }

        // After we've solved for the explicit arguments, we need to
        // make a second pass and consider the implicit arguments,
        // based on what we've already determined to be the values
        // for the explicit arguments.

        // Before we begin, we are going to go ahead and create the
        // "solved" substitution that we will return if everything works.
        // This is because we are going to use this substitution,
        // partially filled in with the results we know so far,
        // in order to specialize any constraints on the generic.
        //
        // E.g., if the generic parameters were `<T : ISidekick>`, and
        // we've already decided that `T` is `Robin`, then we want to
        // search for a conformance `Robin : ISidekick`, which involved
        // apply the substitutions we already know...

        GenericSubstitution* solvedSubst = m_astBuilder->create<GenericSubstitution>();
        solvedSubst->genericDecl = genericDeclRef.getDecl();
        solvedSubst->outer = genericDeclRef.substitutions.substitutions;
        solvedSubst->args = args;
        resultSubst.substitutions = solvedSubst;

        for( auto constraintDecl : genericDeclRef.getDecl()->getMembersOfType<GenericTypeConstraintDecl>() )
        {
            DeclRef<GenericTypeConstraintDecl> constraintDeclRef(
                constraintDecl,
                solvedSubst);

            // Extract the (substituted) sub- and super-type from the constraint.
            auto sub = getSub(m_astBuilder, constraintDeclRef);
            auto sup = getSup(m_astBuilder, constraintDeclRef);

            // Search for a witness that shows the constraint is satisfied.
            auto subTypeWitness = tryGetSubtypeWitness(sub, sup);
            if(subTypeWitness)
            {
                // We found a witness, so it will become an (implicit) argument.
                solvedSubst->args.add(subTypeWitness);
            }
            else
            {
                // No witness was found, so the inference will now fail.
                //
                // TODO: Ideally we should print an error message in
                // this case, to let the user know why things failed.
                return SubstitutionSet();
            }

            // TODO: We may need to mark some constrains in our constraint
            // system as being solved now, as a result of the witness we found.
        }

        // Make sure we haven't constructed any spurious constraints
        // that we aren't able to satisfy:
        for (auto c : system->constraints)
        {
            if (!c.satisfied)
            {
                return SubstitutionSet();
            }
        }

        return resultSubst;
    }

    bool SemanticsVisitor::TryUnifyVals(
        ConstraintSystem&	constraints,
        Val*			fst,
        Val*			snd)
    {
        // if both values are types, then unify types
        if (auto fstType = as<Type>(fst))
        {
            if (auto sndType = as<Type>(snd))
            {
                return TryUnifyTypes(constraints, fstType, sndType);
            }
        }

        // if both values are constant integers, then compare them
        if (auto fstIntVal = as<ConstantIntVal>(fst))
        {
            if (auto sndIntVal = as<ConstantIntVal>(snd))
            {
                return fstIntVal->value == sndIntVal->value;
            }
        }

        // Check if both are integer values in general
        if (auto fstInt = as<IntVal>(fst))
        {
            if (auto sndInt = as<IntVal>(snd))
            {
                auto fstParam = as<GenericParamIntVal>(fstInt);
                auto sndParam = as<GenericParamIntVal>(sndInt);

                bool okay = false;
                if (fstParam)
                {
                    if(TryUnifyIntParam(constraints, fstParam->declRef, sndInt))
                        okay = true;
                }
                if (sndParam)
                {
                    if(TryUnifyIntParam(constraints, sndParam->declRef, fstInt))
                        okay = true;
                }
                return okay;
            }
        }

        if (auto fstWit = as<DeclaredSubtypeWitness>(fst))
        {
            if (auto sndWit = as<DeclaredSubtypeWitness>(snd))
            {
                auto constraintDecl1 = fstWit->declRef.as<TypeConstraintDecl>();
                auto constraintDecl2 = sndWit->declRef.as<TypeConstraintDecl>();
                SLANG_ASSERT(constraintDecl1);
                SLANG_ASSERT(constraintDecl2);
                return TryUnifyTypes(constraints,
                    constraintDecl1.getDecl()->getSup().type,
                    constraintDecl2.getDecl()->getSup().type);
            }
        }

        SLANG_UNIMPLEMENTED_X("value unification case");

        // default: fail
        //return false;
    }

    bool SemanticsVisitor::tryUnifySubstitutions(
        ConstraintSystem&       constraints,
        Substitutions*   fst,
        Substitutions*   snd)
    {
        // They must both be NULL or non-NULL
        if (!fst || !snd)
            return !fst && !snd;

        if(auto fstGeneric = as<GenericSubstitution>(fst))
        {
            if(auto sndGeneric = as<GenericSubstitution>(snd))
            {
                return tryUnifyGenericSubstitutions(
                    constraints,
                    fstGeneric,
                    sndGeneric);
            }
        }

        // TODO: need to handle other cases here

        return false;
    }

    bool SemanticsVisitor::tryUnifyGenericSubstitutions(
        ConstraintSystem&           constraints,
        GenericSubstitution* fst,
        GenericSubstitution* snd)
    {
        SLANG_ASSERT(fst);
        SLANG_ASSERT(snd);

        auto fstGen = fst;
        auto sndGen = snd;
        // They must be specializing the same generic
        if (fstGen->genericDecl != sndGen->genericDecl)
            return false;

        // Their arguments must unify
        SLANG_RELEASE_ASSERT(fstGen->args.getCount() == sndGen->args.getCount());
        Index argCount = fstGen->args.getCount();
        bool okay = true;
        for (Index aa = 0; aa < argCount; ++aa)
        {
            if (!TryUnifyVals(constraints, fstGen->args[aa], sndGen->args[aa]))
            {
                okay = false;
            }
        }

        // Their "base" specializations must unify
        if (!tryUnifySubstitutions(constraints, fstGen->outer, sndGen->outer))
        {
            okay = false;
        }

        return okay;
    }

    bool SemanticsVisitor::TryUnifyTypeParam(
        ConstraintSystem&				constraints,
        GenericTypeParamDecl*	typeParamDecl,
        Type*			type)
    {
        // We want to constrain the given type parameter
        // to equal the given type.
        Constraint constraint;
        constraint.decl = typeParamDecl;
        constraint.val = type;

        constraints.constraints.add(constraint);

        return true;
    }

    bool SemanticsVisitor::TryUnifyIntParam(
        ConstraintSystem&               constraints,
        GenericValueParamDecl*	paramDecl,
        IntVal*                  val)
    {
        // We only want to accumulate constraints on
        // the parameters of the declarations being
        // specialized (don't accidentially constrain
        // parameters of a generic function based on
        // calls in its body).
        if(paramDecl->parentDecl != constraints.genericDecl)
            return false;

        // We want to constrain the given parameter to equal the given value.
        Constraint constraint;
        constraint.decl = paramDecl;
        constraint.val = val;

        constraints.constraints.add(constraint);

        return true;
    }

    bool SemanticsVisitor::TryUnifyIntParam(
        ConstraintSystem&       constraints,
        DeclRef<VarDeclBase> const&   varRef,
        IntVal*          val)
    {
        if(auto genericValueParamRef = varRef.as<GenericValueParamDecl>())
        {
            return TryUnifyIntParam(constraints, genericValueParamRef.getDecl(), val);
        }
        else
        {
            return false;
        }
    }

    bool SemanticsVisitor::TryUnifyTypesByStructuralMatch(
        ConstraintSystem&       constraints,
        Type*  fst,
        Type*  snd)
    {
        if (auto fstDeclRefType = as<DeclRefType>(fst))
        {
            auto fstDeclRef = fstDeclRefType->declRef;

            if (auto typeParamDecl = as<GenericTypeParamDecl>(fstDeclRef.getDecl()))
                return TryUnifyTypeParam(constraints, typeParamDecl, snd);

            if (auto sndDeclRefType = as<DeclRefType>(snd))
            {
                auto sndDeclRef = sndDeclRefType->declRef;

                if (auto typeParamDecl = as<GenericTypeParamDecl>(sndDeclRef.getDecl()))
                    return TryUnifyTypeParam(constraints, typeParamDecl, fst);

                // can't be unified if they refer to different declarations.
                if (fstDeclRef.getDecl() != sndDeclRef.getDecl()) return false;

                // next we need to unify the substitutions applied
                // to each declaration reference.
                if (!tryUnifySubstitutions(
                    constraints,
                    fstDeclRef.substitutions.substitutions,
                    sndDeclRef.substitutions.substitutions))
                {
                    return false;
                }

                return true;
            }
        }

        return false;
    }

    bool SemanticsVisitor::TryUnifyTypes(
        ConstraintSystem&       constraints,
        Type*  fst,
        Type*  snd)
    {
        if (fst->equals(snd)) return true;

        // An error type can unify with anything, just so we avoid cascading errors.

        if (auto fstErrorType = as<ErrorType>(fst))
            return true;

        if (auto sndErrorType = as<ErrorType>(snd))
            return true;

        // A generic parameter type can unify with anything.
        // TODO: there actually needs to be some kind of "occurs check" sort
        // of thing here...

        if (auto fstDeclRefType = as<DeclRefType>(fst))
        {
            auto fstDeclRef = fstDeclRefType->declRef;

            if (auto typeParamDecl = as<GenericTypeParamDecl>(fstDeclRef.getDecl()))
            {
                if(typeParamDecl->parentDecl == constraints.genericDecl )
                    return TryUnifyTypeParam(constraints, typeParamDecl, snd);
            }
        }

        if (auto sndDeclRefType = as<DeclRefType>(snd))
        {
            auto sndDeclRef = sndDeclRefType->declRef;

            if (auto typeParamDecl = as<GenericTypeParamDecl>(sndDeclRef.getDecl()))
            {
                if(typeParamDecl->parentDecl == constraints.genericDecl )
                    return TryUnifyTypeParam(constraints, typeParamDecl, fst);
            }
        }

        // If we can unify the types structurally, then we are golden
        if(TryUnifyTypesByStructuralMatch(constraints, fst, snd))
            return true;

        // Now we need to consider cases where coercion might
        // need to be applied. For now we can try to do this
        // in a completely ad hoc fashion, but eventually we'd
        // want to do it more formally.

        if(auto fstVectorType = as<VectorExpressionType>(fst))
        {
            if(auto sndScalarType = as<BasicExpressionType>(snd))
            {
                return TryUnifyTypes(
                    constraints,
                    fstVectorType->elementType,
                    sndScalarType);
            }
        }

        if(auto fstScalarType = as<BasicExpressionType>(fst))
        {
            if(auto sndVectorType = as<VectorExpressionType>(snd))
            {
                return TryUnifyTypes(
                    constraints,
                    fstScalarType,
                    sndVectorType->elementType);
            }
        }

        // TODO: the same thing for vectors...

        return false;
    }


}
