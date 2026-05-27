// slang-check-constraint.cpp
#include "slang-check-impl.h"

// This file provides the core services for creating and solving constraint
// systems during semantic checking. Those services are used most visibly by
// generic application solving: the generic constraint solver builds the full
// argument list for a generic application and forms a valid `DeclRef` to the
// generic declaration with solved type, value, and witness arguments.
//
// Generic argument model
// ----------------------
//
// A generic application's serialized argument list is made of ordinary arguments
// followed by witness arguments. Ordinary arguments correspond to source
// type/value parameters that user code can name in a generic application, such
// as `T`, `U`, `let N`, or `each Ts`. Witness arguments correspond to source
// generic constraints, such as `T : IFoo` or `U : IData`; they are
// compiler-formed proof values that the final decl-ref carries beside the
// ordinary arguments.
//
// Before solving, `getDefaultSubstitutionArgs()` provides default substitution
// arguments for the whole list. These are the declaration's own substitution
// values, such as `T`, `N`, or the declared witness for `T : IFoo`. They keep
// dependent values like `T.Value` structurally valid while the solver is finding
// answers. The solver's live argument list is the current `m_args` array: it
// starts from those default substitution arguments and is updated in place as
// ordinary arguments and witness arguments become known.
//
// The solver has two phases. The collection phase gathers solver constraints
// from all sources that can affect the live argument list: caller-provided
// ordinary arguments, ordinary constraints discovered by unification, default
// generic arguments, and source generic constraints that require witness
// arguments. The resolution phase runs one iterative work-list loop over that
// single collected set. Keeping ordinary and witness work in the same loop is
// the important property: an ordinary argument can unblock a witness, a witness
// can unblock a dependent default, and solving a witness can discover more
// ordinary constraints through unification.
//
// Work-list states and invariants
// -------------------------------
//
// A solver constraint is `Done` when it is satisfied under the current live
// argument list, `MadeProgress` when it just changed that list, `Blocked` when
// it is waiting for another ordinary or witness argument, and `Failed` when the
// generic application is not viable. Progress wakes dependent work
// conservatively; exact wake-up precision is an optimization, not part of the
// semantic contract. A blocked constraint remains unsatisfied, so if the queue
// becomes empty while blocked work remains, the solver rejects the candidate
// instead of looping. Repeated retries do not count as progress by themselves:
// a handler that computes the same answer reports `Done`, and newly discovered
// unification facts are drained into the same work table instead of reusing a
// separate "types first, witnesses later" pass.
//
// To understand how the pieces fit together, consider this code:
//
//      interface IData {}
//      struct IntData : IData {}
//
//      interface IFoo
//      {
//          associatedtype Value : IData;
//      }
//
//      struct ConcreteFoo : IFoo
//      {
//          typealias Value = IntData;
//      }
//
//      U load<T : IFoo, U : IData = T.Value>(T value);
//
//      void check(ConcreteFoo foo)
//      {
//          load(foo);
//      }
//
// To check `load(foo)`, the compiler must form a decl-ref for `load` as if the
// program had written the full generic argument list. The value-level call first
// compares the actual argument type `ConcreteFoo` with the parameter type `T`.
// That comparison is unification: it asks what must be true for two semantic
// values to agree. Here unification discovers the ordinary solver constraint
// `T = ConcreteFoo`. In more structured cases, such as comparing `vector<T, 3>`
// with a scalar argument, unification may look through promotions and nested
// types before it discovers the ordinary constraints.
//
// Collection for this call produces four pieces of work: `T = ConcreteFoo`, the
// declaration-time default `U = T.Value`, the witness constraint for `T : IFoo`,
// and the witness constraint for `U : IData`. The default for `U` cannot be
// substituted yet because the associated-type lookup needs both the ordinary
// argument `T` and the witness that proves `T : IFoo`.
//
// The work list can solve `T = ConcreteFoo` immediately, then ask the subtype
// system for the witness proving `ConcreteFoo : IFoo`. With that proof stored in
// the same live argument list, substituting the default `U = T.Value` can reduce
// the lookup to `ConcreteFoo.Value`, which is `IntData`. The solver stores
// `IntData` as the ordinary argument for `U`, and the witness constraint for
// `U : IData` can then prove `IntData : IData`.
//
// Unification can also occur while solving witness constraints. If a source
// constraint requires `T : IBox<U>` and the solved subject is known to conform to
// `IBox<IntData>`, solving that witness compares the required interface shape
// with the actual conformance shape. That comparison discovers `U = IntData` as
// a new ordinary solver constraint. The solver drains that discovered constraint
// into the same work list so the final decl-ref is built from one coherent live
// argument list: ordinary arguments first, followed by the witness arguments
// that prove the source generic constraints.

namespace Slang
{

bool SemanticsVisitor::isRelevantGeneric(GenericInferenceContext& system, Decl* generic)
{
    for (auto genericDecl = system.genericDecl; genericDecl;
         genericDecl = as<GenericDecl>(genericDecl->parentDecl))
    {
        if (generic == genericDecl)
            return true;
    }

    return false;
}

Type* SemanticsVisitor::TryJoinVectorAndScalarType(
    GenericInferenceContext* constraints,
    VectorExpressionType* vectorType,
    BasicExpressionType* scalarType)
{
    // Join( vector<T,N>, S ) -> vector<Join(T,S), N>
    //
    // That is, the join of a vector and a scalar type is
    // a vector type with a joined element type.
    auto joinElementType = TryJoinTypes(constraints, vectorType->getElementType(), scalarType);
    if (!joinElementType)
        return nullptr;

    return createVectorType(joinElementType, vectorType->getElementCount());
}

Type* SemanticsVisitor::_tryJoinTypeWithInterface(
    GenericInferenceContext* constraints,
    Type* type,
    Type* interfaceType)
{
    // The most basic test here should be: does the type declare conformance to the trait.

    if (constraints->subTypeForAdditionalWitnesses == type)
    {
        // If additional subtype witnesses are provided for `type` in `constraints`,
        // try to use them to see if the interface is satisfied.
        if (constraints->additionalSubtypeWitnesses->containsKey(interfaceType))
            return type;
    }
    else
    {
        if (isSubtype(
                type,
                interfaceType,
                constraints->additionalSubtypeWitnesses ? IsSubTypeOptions::NoCaching
                                                        : IsSubTypeOptions::None))
            return type;
    }

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
    ConversionCost bestCost = kConversionCost_Explicit;
    if (auto basicType = dynamicCast<BasicExpressionType>(type))
    {
        for (Int baseTypeFlavorIndex = 0; baseTypeFlavorIndex < Int(BaseType::CountOfPrimitives);
             baseTypeFlavorIndex++)
        {
            // Don't consider `type`, since we already know it doesn't work.
            if (baseTypeFlavorIndex == Int(basicType->getBaseType()))
                continue;

            // Look up the type in our session.
            auto candidateType =
                getCurrentASTBuilder()->getBuiltinType(BaseType(baseTypeFlavorIndex));
            if (!candidateType)
                continue;

            // We only want to consider types that implement the target interface.
            if (!isSubtype(candidateType, interfaceType, IsSubTypeOptions::None))
                continue;

            // We only want to consider types where we can implicitly convert from `type`
            auto conversionCost = getConversionCost(candidateType, type);
            if (!canConvertImplicitly(conversionCost))
                continue;

            // At this point, we have a candidate type that is usable.
            //
            // If this is our first viable candidate, then it is our best one:
            //
            if (!bestType)
            {
                bestType = candidateType;
                bestCost = conversionCost;
            }
            else
            {
                // Otherwise, we want to pick the "better" type between `candidateType`
                // and `bestType`.
                //
                // The candidate type that has lower conversion cost from `type` is better.
                //
                if (conversionCost < bestCost)
                {
                    // Our candidate can convert to the current "best" type, so
                    // it is logically a more specific type that satisfies our
                    // constraints, therefore we should keep it.
                    //
                    bestType = candidateType;
                    bestCost = conversionCost;
                }
            }
        }
        if (bestType)
        {
            // Track the conversion cost for type promotion in the inference context.
            // This cost represents promoting a type (e.g., int -> float) to satisfy
            // an interface constraint (e.g., __BuiltinFloatingPointType).
            // This ensures that overload resolution prefers exact type matches over
            // candidates that require type promotion.
            constraints->typePromotionCost += bestCost;
            return bestType;
        }
    }

    // If `interfaceType` represents some generic interface type, such as
    // `IFoo<T>`, and `type` conforms to some `IFoo<X>`, then we should attempt
    // to unify them to discover constraints for `T`.
    if (auto interfaceDeclRef = isDeclRefTypeOf<InterfaceDecl>(interfaceType))
    {
        if (as<GenericAppDeclRef>(interfaceDeclRef.declRefBase))
        {
            auto inheritanceInfo = getShared()->getInheritanceInfo(type);
            for (auto facet : inheritanceInfo.facets)
            {
                if (facet->origin.declRef.getDecl() == interfaceDeclRef.getDecl())
                {
                    auto unificationResult = TryUnifyTypes(
                        *constraints,
                        UnificationOptions(),
                        QualType(facet->getType()),
                        interfaceType);

                    if (unificationResult)
                        return type;
                }
            }
            if (constraints->subTypeForAdditionalWitnesses)
            {
                for (auto witnessKV : *constraints->additionalSubtypeWitnesses)
                {
                    auto unificationResult = TryUnifyTypes(
                        *constraints,
                        UnificationOptions(),
                        QualType(witnessKV.first),
                        interfaceType);
                    if (unificationResult)
                        return type;
                }
            }
        }
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
    GenericInferenceContext* constraints,
    QualType left,
    QualType right)
{
    // Easy case: they are the same type!
    if (left->equals(right))
        return left;

    // We can join two basic types by picking the "better" of the two
    if (auto leftBasic = as<BasicExpressionType>(left))
    {
        if (auto rightBasic = as<BasicExpressionType>(right))
        {
            auto costConvertRightToLeft = getConversionCost(leftBasic, right);
            auto costConvertLeftToRight = getConversionCost(rightBasic, left);

            // Return the one that had lower conversion cost.
            if (costConvertRightToLeft > costConvertLeftToRight)
                return right;
            else
            {
                return left;
            }
        }

        // We can also join a vector and a scalar
        if (auto rightVector = as<VectorExpressionType>(right))
        {
            return TryJoinVectorAndScalarType(constraints, rightVector, leftBasic);
        }
    }

    // We can join two vector types by joining their element types
    // (and also their sizes...)
    if (auto leftVector = as<VectorExpressionType>(left))
    {
        if (auto rightVector = as<VectorExpressionType>(right))
        {
            // Check if the vector sizes match
            if (!leftVector->getElementCount()->equals(rightVector->getElementCount()))
                return nullptr;

            // Try to join the element types
            auto joinElementType = TryJoinTypes(
                constraints,
                QualType(leftVector->getElementType(), left.isLeftValue),
                QualType(rightVector->getElementType(), right.isLeftValue));
            if (!joinElementType)
                return nullptr;

            return createVectorType(joinElementType, leftVector->getElementCount());
        }

        // We can also join a vector and a scalar
        if (auto rightBasic = as<BasicExpressionType>(right))
        {
            return TryJoinVectorAndScalarType(constraints, leftVector, rightBasic);
        }
    }

    // HACK: trying to work trait types in here...
    if (auto leftDeclRefType = as<DeclRefType>(left))
    {
        if (auto leftInterfaceRef = leftDeclRefType->getDeclRef().as<InterfaceDecl>())
        {
            //
            return _tryJoinTypeWithInterface(constraints, right, left);
        }
    }
    if (auto rightDeclRefType = as<DeclRefType>(right))
    {
        if (auto rightInterfaceRef = rightDeclRefType->getDeclRef().as<InterfaceDecl>())
        {
            //
            return _tryJoinTypeWithInterface(constraints, left, right);
        }
    }

    // We can recursively join two TypePacks.
    if (auto leftTypePack = as<ConcreteTypePack>(left))
    {
        if (auto rightTypePack = as<ConcreteTypePack>(right))
        {
            if (leftTypePack->getTypeCount() != rightTypePack->getTypeCount())
                return nullptr;
            ShortList<Type*> joinedTypes;
            for (Index i = 0; i < leftTypePack->getTypeCount(); ++i)
            {
                auto joinedType = TryJoinTypes(
                    constraints,
                    QualType(leftTypePack->getElementType(i), left.isLeftValue),
                    QualType(rightTypePack->getElementType(i), right.isLeftValue));
                if (!joinedType)
                    return nullptr;
                joinedTypes.add(joinedType);
            }
            return m_astBuilder->getTypePack(joinedTypes.getArrayView().arrayView);
        }
    }

    // TODO: all the cases for vectors apply to matrices too!

    // Default case is that we just fail.
    return nullptr;
}

// Find the witness value for an already-substituted type-coercion generic constraint.
static TypeCoercionWitness* findTypeCoercionWitnessForSubstitutedConstraint(
    ASTBuilder* astBuilder,
    SemanticsVisitor* visitor,
    DeclRef<TypeCoercionConstraintDecl> constraintDeclRef,
    Decl* genericInnerDeclForDiagnostics,
    SemanticsVisitor::OverloadResolveContext* maybeContext,
    bool shouldEmitError)
{
    // Diagnostic emission needs the overload context location. The solver often
    // probes a candidate without emitting errors, so the context is optional
    // unless this helper is being asked to produce user-facing diagnostics.
    SLANG_ASSERT(!shouldEmitError || shouldEmitError && maybeContext);

    auto constraintDecl = constraintDeclRef.getDecl();
    auto fromType = getFromType(astBuilder, constraintDeclRef);
    auto toType = getToType(astBuilder, constraintDeclRef);

    // `_coerce` is the existing source of truth for conversions. Calling it
    // here avoids duplicating conversion rules in the generic solver, and also
    // lets user-defined conversion functions produce the witness value that the
    // final generic application needs to carry.
    TypeCoercionWitness* typeCoercionWitness = nullptr;
    DeclRef<Decl> declRefUsedToConvert{};
    ConversionCost conversionCost = kConversionCost_Impossible;
    visitor->_coerce(
        CoercionSite::General,
        toType,
        nullptr,
        fromType,
        nullptr,
        visitor->getSink(),
        &conversionCost,
        &typeCoercionWitness);
    if (constraintDecl->findModifier<ImplicitConversionModifier>())
    {
        // An `implicit` coercion constraint must be satisfied by an implicit
        // conversion. A general conversion might be viable for an explicit cast,
        // but it cannot be used to prove this generic constraint.
        if (conversionCost > kConversionCost_GeneralConversion)
        {
            if (shouldEmitError)
            {
                visitor->getSink()->diagnose(
                    Diagnostics::ImplicitTypeCoerceConstraintWithNonImplicitConversion{
                        .fromType = fromType,
                        .toType = toType,
                        .location = maybeContext->loc});

                if (auto declRefTypeCoercionWitness =
                        as<DeclRefTypeCoercionWitness>(typeCoercionWitness))
                {
                    visitor->getSink()->diagnose(Diagnostics::SeeDefinitionOfConversionFunction{
                        .decl = declRefTypeCoercionWitness->getDeclRef().getDecl()});
                }
                visitor->getSink()->diagnose(
                    Diagnostics::SeeDefinitionOfConstraint{.decl = constraintDecl});
            }
            return nullptr;
        }
    }

    // If conversion checking found no path at all, the witness argument cannot
    // be formed and this generic application is not viable.
    if (conversionCost == kConversionCost_Impossible)
    {
        if (shouldEmitError)
        {
            visitor->getSink()->diagnose(Diagnostics::TypeCoerceConstraintMissingConversion{
                .fromType = fromType,
                .toType = toType,
                .location = maybeContext->loc});
            if (genericInnerDeclForDiagnostics)
                visitor->getSink()->diagnose(
                    Diagnostics::SeeDefinitionOf{.decl = genericInnerDeclForDiagnostics});
        }
        return nullptr;
    }

    // If `_coerce` accepted the conversion but didn't provide a concrete
    // witness value, treat it as a builtin conversion witness so later
    // lowering still has a concrete generic argument to pass through.
    if (!typeCoercionWitness)
        typeCoercionWitness = astBuilder->getBuiltinTypeCoercionWitness(fromType, toType);

    return typeCoercionWitness;
}

// Find the witness value for a type-coercion generic constraint.
TypeCoercionWitness* findTypeCoercionWitnessForConstraint(
    ASTBuilder* astBuilder,
    SemanticsVisitor* visitor,
    TypeCoercionConstraintDecl* constraintDecl,
    DeclRef<GenericDecl> genericDeclRef,
    SemanticsVisitor::OverloadResolveContext* maybeContext,
    ArrayView<Val*> args,
    bool shouldEmitError)
{
    // The source constraint is stored on the generic declaration, but the
    // witness must prove the constraint under the current generic arguments.
    // For `Foo<T>(...) where T : __ConvertibleTo<U>`, this substitutes the
    // current `T` and `U` before checking convertibility.
    DeclRef<TypeCoercionConstraintDecl> constraintDeclRef =
        astBuilder->getGenericAppDeclRef(genericDeclRef, args, constraintDecl)
            .as<TypeCoercionConstraintDecl>();
    return findTypeCoercionWitnessForSubstitutedConstraint(
        astBuilder,
        visitor,
        constraintDeclRef,
        genericDeclRef.getDecl()->inner,
        maybeContext,
        shouldEmitError);
}

// Find the witness value for an already-substituted differentiability-info generic constraint.
static Witness* findDiffTypeInfoWitnessForSubstitutedConstraint(
    ASTBuilder* astBuilder,
    SemanticsVisitor* visitor,
    DeclRef<HasDiffTypeInfoConstraintDecl> constraintDeclRef,
    SemanticsVisitor::OverloadResolveContext* maybeContext,
    bool shouldEmitError)
{
    // Diagnostic emission needs the overload context location, while the solver
    // often probes this helper silently during overload candidate checking.
    SLANG_ASSERT(!shouldEmitError || maybeContext);

    auto constraintDecl = constraintDeclRef.getDecl();
    auto constrainedType = getBaseType(astBuilder, constraintDeclRef);
    if (!constrainedType)
    {
        // A missing substituted type means the constraint cannot even identify
        // its subject. In diagnostic mode we still point back to the source
        // constraint so the user can find the failing where-clause.
        if (shouldEmitError)
        {
            visitor->getSink()->diagnose(Diagnostics::TypeDoesNotHaveDiffTypeInfo{
                .type = astBuilder->getErrorType(),
                .location = maybeContext->loc});
            visitor->getSink()->diagnose(
                Diagnostics::SeeDefinitionOfConstraint{.decl = constraintDecl});
        }
        return nullptr;
    }

    // The semantic visitor owns the canonical diff-type-info lookup. The solver
    // only supplies the substituted subject type and returns the witness if the
    // visitor can prove the constraint.
    if (auto witness = visitor->getDiffTypeInfoWitness(constrainedType))
        return witness;

    if (shouldEmitError)
    {
        // When the lookup fails in diagnostic mode, emit the concrete subject
        // type. This produces messages about the actual `float`, `MyType`, or
        // substituted associated type rather than the declaration-time `T`.
        visitor->getSink()->diagnose(Diagnostics::TypeDoesNotHaveDiffTypeInfo{
            .type = constrainedType,
            .location = maybeContext->loc});
        visitor->getSink()->diagnose(
            Diagnostics::SeeDefinitionOfConstraint{.decl = constraintDecl});
    }

    return nullptr;
}

// Find the witness value for a differentiability-info generic constraint.
Witness* findDiffTypeInfoWitnessForConstraint(
    ASTBuilder* astBuilder,
    SemanticsVisitor* visitor,
    HasDiffTypeInfoConstraintDecl* constraintDecl,
    DeclRef<GenericDecl> genericDeclRef,
    SemanticsVisitor::OverloadResolveContext* maybeContext,
    ArrayView<Val*> args,
    bool shouldEmitError)
{
    // The constraint declaration is substituted through the candidate's current
    // generic arguments before asking for diff-type info. For
    // `Foo<T>() where __hasDiffTypeInfo(T)`, this is what turns the declaration
    // type `T` into the concrete or inferred type currently stored for `T`.
    auto constraintDeclRef = astBuilder->getGenericAppDeclRef(genericDeclRef, args, constraintDecl)
                                 .as<HasDiffTypeInfoConstraintDecl>();
    return findDiffTypeInfoWitnessForSubstitutedConstraint(
        astBuilder,
        visitor,
        constraintDeclRef,
        maybeContext,
        shouldEmitError);
}

// Find the witness value for a non-empty-pack generic constraint.
Witness* findNonEmptyPackWitnessForConstraint(
    ASTBuilder* astBuilder,
    SemanticsVisitor* visitor,
    Val* constrainedArg,
    SemanticsVisitor::OverloadResolveContext* maybeContext,
    bool shouldEmitError)
{
    // Diagnostics need the call/application context. The solver uses this
    // helper speculatively, so diagnostic context is only required when the
    // caller explicitly asks for user-facing errors.
    SLANG_ASSERT(!shouldEmitError || maybeContext);

    // A non-empty-pack witness is valid only when the pack shape is already
    // known to contain at least one element. Unknown packs cannot produce a
    // proof, and known empty packs reject required constraints.
    auto packCardinality = constrainedArg ? visitor->getPackCardinality(constrainedArg)
                                          : VariadicPackCardinality::Unknown;
    if (packCardinality != VariadicPackCardinality::NonEmpty)
    {
        if (shouldEmitError)
        {
            if (packCardinality == VariadicPackCardinality::Empty)
            {
                visitor->getSink()->diagnose(Diagnostics::EmptyPackDoesNotSatisfyNonEmptyConstraint{
                    .location = maybeContext->loc});
            }
            else
            {
                auto diagExpr = maybeContext->originalExpr ? maybeContext->originalExpr
                                                           : maybeContext->baseExpr;
                visitor->getSink()->diagnose(Diagnostics::PackQueryRequiresNonEmptyPack{
                    .queryName = "nonempty(...)",
                    .expr = diagExpr});
            }
        }
        return nullptr;
    }

    // The pack value itself is stored in the witness so later stages can rely on
    // it as proof that the source generic constraint was satisfied for this
    // exact pack argument.
    return astBuilder->getNonEmptyPackWitness(constrainedArg);
}

// An `ArgState` classifies the current argument stored in `m_args` for one
// ordinary argument or witness argument position. The state decides whether that
// current argument is ready for substitution, and whether later solver
// constraints may replace it.
enum class ArgState
{
    // The current argument is still the value provided by
    // `getDefaultSubstitutionArgs()`. It keeps the generic application
    // structurally valid, but it is not a solved ordinary or witness argument.
    DefaultSubstitutionArg,

    // The current ordinary argument was provided by the generic application
    // being checked. Default generic arguments must not replace it.
    CallerProvidedOrdinaryArg,

    // The current ordinary argument is the same parameter that owns the
    // argument position, such as `T` for the `T` parameter. It is usable in a
    // dependent substitution, but local inference or a default generic argument
    // may still provide a more specific value.
    DependentOrdinaryArg,

    // Ordinary solver constraints produced this ordinary argument.
    SolvedOrdinaryArg,

    // A default generic argument produced this ordinary argument. Later
    // ordinary inference can still replace it.
    DefaultGenericArg,

    // The ordinary argument is an empty pack produced for an omitted pack
    // parameter.
    EmptyPackArg,

    // The current witness argument proves the corresponding source generic
    // constraint.
    SolvedWitnessArg,
};

// `ArgInfo` stores the solver-local metadata for one argument position. The
// argument value itself lives in `m_args`; this type only tracks how that value
// may be used or replaced, plus payloads used by ordinary-argument solving and
// overload ranking.
struct ArgInfo
{
    using ConstraintPriority = SemanticsVisitor::ConstraintPriority;

    ArgState getState() const { return m_state; }

    void setState(ArgState state)
    {
        m_state = state;
        if (state != ArgState::DefaultGenericArg)
            m_substitutedDefaultArg = nullptr;
    }

    Val* getSubstitutedDefaultArg() const
    {
        SLANG_ASSERT(m_state == ArgState::DefaultGenericArg || !m_substitutedDefaultArg);
        return m_substitutedDefaultArg;
    }

    void setDefaultGenericArg(Val* substitutedDefaultArg)
    {
        SLANG_ASSERT(substitutedDefaultArg);
        m_state = ArgState::DefaultGenericArg;
        m_substitutedDefaultArg = substitutedDefaultArg;
    }

    ConstraintPriority getPriority() const { return m_priority; }
    void setPriority(ConstraintPriority priority) { m_priority = priority; }

    bool isConstrainedForOverloadRanking() const { return m_constrainedForOverloadRanking; }
    void markConstrainedForOverloadRanking() { m_constrainedForOverloadRanking = true; }

    ShortList<QualType, 8>& getTypeConstraints()
    {
        SLANG_ASSERT(
            m_state == ArgState::DefaultSubstitutionArg ||
            m_state == ArgState::DependentOrdinaryArg || m_state == ArgState::SolvedOrdinaryArg ||
            m_state == ArgState::DefaultGenericArg || m_state == ArgState::EmptyPackArg);
        return m_typeConstraints;
    }

private:
    ArgState m_state = ArgState::DefaultSubstitutionArg;
    Val* m_substitutedDefaultArg = nullptr;
    ConstraintPriority m_priority = ConstraintPriority::Default;
    ShortList<QualType, 8> m_typeConstraints;
    bool m_constrainedForOverloadRanking = false;
};

// Return the default generic argument for one ordinary parameter.
Val* getGenericParamDefaultArgVal(SemanticsVisitor* visitor, Decl* paramDecl)
{
    if (auto typeParam = as<GenericTypeParamDecl>(paramDecl))
    {
        // The default-value expression for a generic type parameter has already
        // been checked as part of checking the parameter declaration, so the
        // checked `initType` is the argument value to use. For
        // `Foo<T, U = T.Element>`, this returns the semantic value for
        // `T.Element`.
        return typeParam->initType.type;
    }
    else if (auto valParam = as<GenericValueParamDecl>(paramDecl))
    {
        if (!valParam->initExpr)
            return nullptr;

        // The default-value expression for a generic value parameter must be
        // folded into an `IntVal` before it can be stored in the argument list.
        // For `Foo<let N : int = 4>`, this returns the folded integer value
        // `4`. Constant folding can encounter circular defaults like
        // `N = M, M = N`, so this path uses the same circularity guard as other
        // semantic checks.
        SemanticsVisitor::ConstantFoldingCircularityInfo newCircularityInfo(
            makeDeclRef(paramDecl),
            nullptr);
        return visitor->ExtractGenericArgVal(valParam->initExpr, &newCircularityInfo);
    }
    return nullptr;
}

// Recover a generic value-parameter reference from an integer value.
DeclRefIntVal* getDeclRefIntValIgnoringCasts(IntVal* intVal)
{
    // Semantic checking may wrap a value-parameter reference in casts while
    // normalizing integer types. For example, a reference to `N` can appear as
    // `int(N)` after unification chooses an expected integer type, but it still
    // names the same ordinary generic argument.
    while (const auto castIntVal = as<TypeCastIntVal>(intVal))
    {
        intVal = as<IntVal>(castIntVal->getBase());
        if (!intVal)
            return nullptr;
    }

    // Only decl-ref integer values identify ordinary value parameters. Concrete
    // integer values such as `4` are already answers, not dependencies on a
    // parameter.
    return as<DeclRefIntVal>(intVal);
}

// Return the ordinary-argument merge mode requested by unification.
SemanticsVisitor::SolverConstraint::OrdinaryArgMergeMode getOrdinaryArgMergeMode(
    SemanticsVisitor::UnificationOptions const& unificationOptions)
{
    // Ordinary call inference keeps Slang's existing type-join behavior, while
    // source equality requirements produce exact answers. Value-parameter
    // unification overrides this helper and always uses exact answers because a
    // value parameter must settle on one value.
    if (unificationOptions.equalityConstraint)
        return SemanticsVisitor::SolverConstraint::OrdinaryArgMergeMode::Exact;
    return SemanticsVisitor::SolverConstraint::OrdinaryArgMergeMode::TypeJoin;
}

// Return the argument index for an ordinary generic parameter.
Index getGenericParamIndex(Decl* genericParamDecl)
{
    // Type parameters, type-pack parameters, value parameters, and value-pack
    // parameters all share the ordinary-argument index space assigned by their
    // declarations.
    if (auto typeParamDecl = as<GenericTypeParamDeclBase>(genericParamDecl))
        return typeParamDecl->parameterIndex;
    if (auto valuePackParamDecl = as<GenericValuePackParamDecl>(genericParamDecl))
        return valuePackParamDecl->parameterIndex;
    if (auto valueParamDecl = as<GenericValueParamDecl>(genericParamDecl))
        return valueParamDecl->parameterIndex;

    // Witness arguments deliberately have no index here; they are serialized
    // after ordinary arguments and are found by scanning source generic
    // constraint declarations.
    return -1;
}

// Owns one generic-application solve. The class collects ordinary constraints,
// default generic arguments, and witness constraints into solver constraints,
// runs the work-list loop, and stores the current argument arrays used to build
// the final substituted decl-ref.
class GenericArgumentSolver
{
public:
    using ConstraintPriority = SemanticsVisitor::ConstraintPriority;
    using GenericInferenceContext = SemanticsVisitor::GenericInferenceContext;
    using SolverConstraint = SemanticsVisitor::SolverConstraint;
    using UnificationOptions = SemanticsVisitor::UnificationOptions;

    // `ConstraintSolvingState` is the protocol between one constraint solver
    // routine and the work-list loop. A handler can report that the constraint
    // is already satisfied, that it changed an argument in `m_args`, that it is
    // waiting for another ordinary or witness argument, or that the generic
    // application cannot be solved.
    enum class ConstraintSolvingState
    {
        // The constraint is satisfied and did not change `m_args`.
        Done,

        // The constraint wrote a new or changed ordinary/witness argument, or
        // discovered follow-up ordinary constraints. Other solver constraints
        // may need to run again under the updated work table and argument list.
        MadeProgress,

        // The constraint cannot be attempted yet because it refers to an
        // argument that still contains a default substitution arg.
        Blocked,

        // The constraint proved that this generic application cannot be solved.
        Failed,
    };

    // Create a solver for one generic application.
    GenericArgumentSolver(
        SemanticsVisitor* visitor,
        GenericInferenceContext&& inferenceContext,
        DeclRef<GenericDecl> genericDeclRef,
        ArrayView<Val*> providedOrdinaryArgs,
        ConversionCost& outBaseCost)
        : m_visitor(visitor)
        , m_astBuilder(visitor->getASTBuilder())
        , m_context(_Move(inferenceContext))
        , m_genericDeclRef(genericDeclRef)
        , m_providedOrdinaryArgs(providedOrdinaryArgs)
        , m_outBaseCost(outBaseCost)
    {
    }

    // Solve the generic application and return the substituted inner decl-ref.
    DeclRef<Decl> solve()
    {
        // Start with a clean overload cost for this candidate. The caller owns
        // the storage, but the solver owns the full computation: ordinary
        // inference can add type-promotion cost, and final witness arguments can
        // add conformance cost.
        m_outBaseCost = kConversionCost_None;

        // The first phase collects every fact that can constrain the argument
        // list. For `Foo<T : IFoo, U = T.A>(x)`, that includes call-site
        // unification, the default `U = T.A`, and the witness constraint
        // `T : IFoo`.
        collectSolverConstraints();
        if (!initializeArgs())
            return DeclRef<Decl>();
        if (!applyProvidedOrdinaryArgs())
            return DeclRef<Decl>();

        // The second phase lets those collected solver constraints solve together. A
        // default argument can wait for a witness, and a witness can wait for an
        // ordinary argument; keeping them in one loop is what lets dependent
        // defaults with associated-type lookups converge.
        if (!runWorkList())
            return DeclRef<Decl>();

        // Once the work list is quiet, validate both halves of the argument
        // list. Ordinary solver constraints must have been consumed, and every
        // required source generic constraint must have a witness argument in
        // `m_args`.
        if (!areOrdinaryConstraintsSatisfied())
            return DeclRef<Decl>();
        if (!areFinalArgsValid())
            return DeclRef<Decl>();

        // The final overload cost is computed after solving so retried
        // constraints do not double-count. A candidate with `T : IFoo` remains
        // more specific than an unconstrained `T`, and a subtype witness
        // contributes the conversion cost chosen by `isSubtype()`.
        addUnconstrainedGenericParamCost();
        m_outBaseCost += m_context.typePromotionCost + computeFinalWitnessCost();

        // The final decl-ref uses the current argument arrays: ordinary
        // arguments first, then witness arguments. Returning the inner decl-ref
        // lets overload resolution proceed as if the generic application had
        // been written with the full solved argument list.
        return buildSubstDeclRef(m_genericDeclRef.getDecl()->inner);
    }

private:
    // -------------------------------------------------------------------------
    // Constraint collection
    //
    // These routines build the solver's complete work table before resolution
    // begins. They do not try to prove anything. Their job is to put call-site
    // unification facts, defaults, and witness-producing source constraints
    // into one representation so the work-list loop can solve them together.
    // Caller-provided ordinary arguments are already known, so they are
    // installed directly into the live argument state after initialization.

    // Collect solver constraints before the work-list phase starts.
    void collectSolverConstraints()
    {
        // Generic substitutions are nested, so the solver starts by collecting the
        // chain from the outermost generic to the generic being applied.
        collectGenericDeclChain();

        // The incoming inference context already contains ordinary constraints
        // collected by argument/parameter unification. Each one enters the same
        // solver-constraint table as defaults and witnesses.
        addSolverConstraintsForDiscoveredConstraints();

        // Default generic arguments and source generic constraints are collected
        // next. They enter the same work list as ordinary constraints.
        collectGenericMemberConstraints();

        // Argument-array initialization is deliberately separate: collection
        // decides what work exists, while initialization creates the live
        // declaration-order storage that the work-list mutates.
    }

    // Collect the nested generic declarations being solved.
    void collectGenericDeclChain()
    {
        // Generic applications can be nested through declarations such as
        // `Outer<T>.Inner<U>`. We first walk from `Inner` to `Outer`, then put
        // the collected declarations in outer-to-inner order so later
        // substitution can build `Outer` before using that substituted decl-ref
        // as the parent for `Inner`.
        for (auto genericDecl = m_genericDeclRef.getDecl(); genericDecl;
             genericDecl = as<GenericDecl>(genericDecl->parentDecl))
        {
            m_genericDecls.add(genericDecl);
        }

        for (Index ii = 0, jj = m_genericDecls.getCount() - 1; ii < jj; ii++, jj--)
        {
            auto tmp = m_genericDecls[ii];
            m_genericDecls[ii] = m_genericDecls[jj];
            m_genericDecls[jj] = tmp;
        }
    }

    // Add solver constraints discovered by unification.
    void addSolverConstraintsForDiscoveredConstraints()
    {
        // `GenericInferenceContext::discoveredConstraints` is a short-lived
        // inbox shared with helpers such as `TryUnifyTypes()` and
        // `TryJoinTypes()`. Initial call-site matching can fill it before the
        // solver starts, and solving one solver constraint can fill it again
        // with follow-up constraints like `U = T.A`.
        for (auto constraint : m_context.discoveredConstraints)
        {
            // Once copied here, the solver-constraint table becomes the durable
            // source of work for the iterative loop. This keeps ordinary
            // constraints, default generic arguments, and witness constraints
            // in the same scheduling mechanism. Unification has already chosen
            // how ordinary-argument constraints should merge, so preserve that
            // information here.
            addSolverConstraint(constraint);
        }

        // Clearing the inbox matters because later unification calls should only
        // expose newly discovered constraints. Otherwise the solver would keep
        // re-adding the same initial constraints after every solver constraint.
        m_context.discoveredConstraints.clear();
    }

    // Collect member-defined solver constraints from every generic in the chain.
    void collectGenericMemberConstraints()
    {
        // Outer generic declarations contribute their own defaults and source
        // generic constraints before the inner declaration is specialized. For
        // `Outer<T>.Inner<U = T.A>`, both `Outer` and `Inner` must share the
        // same final substitution chain.
        for (auto genericDecl : m_genericDecls)
            collectGenericMemberConstraints(genericDecl);
    }

    // Collect member-defined solver constraints from one generic declaration.
    void collectGenericMemberConstraints(GenericDecl* genericDecl)
    {
        // Direct members are in declaration order. Ordinary parameters with
        // defaults become default-generic-argument constraints, while source
        // generic constraints become witness constraints. In
        // `Foo<T : IFoo, U = T.A>`, this adds both the witness constraint for
        // `T : IFoo` and the default generic argument for `U = T.A`.
        for (auto member : genericDecl->getDirectMemberDecls())
        {
            if (ordinaryParamDeclHasDefaultGenericArg(member))
                addDefaultGenericArgConstraint(genericDecl, member);
            else if (isGenericConstraintDecl(member))
                addWitnessConstraint(genericDecl, member);
        }
    }

    // Initialize the live generic-application argument arrays.
    bool initializeArgs()
    {
        for (auto genericDecl : m_genericDecls)
        {
            auto& args = m_args[genericDecl];
            args.clear();
            // `getDefaultSubstitutionArgs()` already builds the exact argument
            // layout consumed by `getGenericAppDeclRef`: ordinary arguments
            // first, then witness arguments. These default substitution args are
            // not solver answers; they are values such as `T`, `N`, or a
            // declared witness that keep dependent values like `T.A` well formed
            // until a solver constraint replaces them.
            auto defaultArgs = getDefaultSubstitutionArgs(m_astBuilder, m_visitor, genericDecl);
            for (auto arg : defaultArgs)
            {
                if (!arg)
                    return false;
                args.add(arg);
            }

            // Omitted packs use empty pack values rather than their default
            // substitution args. A later non-empty-pack witness constraint
            // decides whether that empty value is legal for this candidate.
            for (auto member : genericDecl->getDirectMemberDecls())
            {
                if (as<GenericTypePackParamDecl>(member))
                {
                    if (!setCurrentArg(member, m_astBuilder->getTypePack(ArrayView<Type*>())))
                        return false;
                    setArgState(member, ArgState::EmptyPackArg);
                }
                else if (as<GenericValuePackParamDecl>(member))
                {
                    if (!setCurrentArg(member, m_astBuilder->getIntValPack(ArrayView<IntVal*>())))
                        return false;
                    setArgState(member, ArgState::EmptyPackArg);
                }
            }
        }
        return true;
    }

    // Install ordinary arguments supplied by a generic application before the
    // work-list starts. These are fixed input facts, not solver conclusions.
    bool applyProvidedOrdinaryArgs()
    {
        if (m_providedOrdinaryArgs.getCount() == 0)
            return true;

        // `providedOrdinaryArgs` follows the historical convention used by this
        // solver: it applies to the outermost generic declaration in the chain.
        // For `Outer<T>.Inner<U>`, that means the provided array is matched
        // against `Outer`'s ordinary parameters.
        if (m_genericDecls.getCount() == 0)
            return true;
        auto genericDecl = m_genericDecls[0];

        for (auto member : genericDecl->getDirectMemberDecls())
        {
            Index argIndex = getGenericParamIndex(member);
            if (argIndex < 0 || argIndex >= m_providedOrdinaryArgs.getCount())
                continue;

            if (!setProvidedArg(member, m_providedOrdinaryArgs[argIndex]))
                return false;
        }
        return true;
    }

    // Install one caller-provided ordinary argument into the live argument list.
    bool setProvidedArg(Decl* paramDecl, Val* arg)
    {
        // Some internal callers provide the parameter's own default
        // substitution arg, such as `T` for the `T` parameter, only to preserve
        // a dependent declaration shape. That value is ready for substitution,
        // but it is not fixed user input, so later inference may still replace
        // it.
        auto argState = isDefaultSubstitutionArgForParam(paramDecl, arg)
                            ? ArgState::DependentOrdinaryArg
                            : ArgState::CallerProvidedOrdinaryArg;

        if (!setCurrentArg(paramDecl, arg))
            return false;
        setArgState(paramDecl, argState);
        return true;
    }

    // Return true if an ordinary generic parameter declares a default argument
    // that can be serialized as a generic application argument.
    bool ordinaryParamDeclHasDefaultGenericArg(Decl* member)
    {
        // Pack parameters are handled by omission rules, not default generic
        // arguments. Treating an omitted pack as a default would blur the
        // distinction needed by non-empty-pack witness constraints.
        if (as<GenericTypePackParamDecl>(member) || as<GenericValuePackParamDecl>(member))
            return false;

        // Type defaults are stored as checked types and value defaults as
        // initializer expressions. These tests intentionally mirror
        // `getGenericParamDefaultArgVal()` so collection only creates work for
        // defaults that can produce an argument.
        if (auto typeParamDecl = as<GenericTypeParamDecl>(member))
            return typeParamDecl->initType.type != nullptr;
        if (auto valParamDecl = as<GenericValueParamDecl>(member))
            return valParamDecl->initExpr != nullptr;
        return false;
    }

    // Return true if a generic member declares a witness-producing constraint.
    bool isGenericConstraintDecl(Decl* member)
    {
        // These source declarations are represented as witness arguments in the
        // final generic application. `T : IFoo`, type coercions, non-empty-pack
        // checks, and differentiability constraints all need compiler-formed
        // witness values.
        return as<GenericTypeConstraintDecl>(member) || as<TypeCoercionConstraintDecl>(member) ||
               as<NonEmptyPackConstraintDecl>(member) || as<HasDiffTypeInfoConstraintDecl>(member);
    }

    // Add a solver constraint for one default generic argument.
    void addDefaultGenericArgConstraint(GenericDecl* genericDecl, Decl* paramDecl)
    {
        // The solver constraint keeps the declaration-time default, such as `T.A`,
        // rather than the current substituted value. It may need to retry after
        // another argument or witness changes the meaning of that default.
        Val* defaultArg = getGenericParamDefaultArgVal(m_visitor, paramDecl);
        if (!defaultArg)
            return;

        // The parameter declaration is the ordinary argument that will be
        // updated when this constraint solves.
        addSolverConstraint(SolverConstraint::makeDefaultArg(genericDecl, paramDecl, defaultArg));
    }

    // Add a solver constraint for one source generic constraint.
    void addWitnessConstraint(GenericDecl* genericDecl, Decl* constraintDecl)
    {
        // The constraint declaration is also the declaration that owns the
        // witness argument position. For `T : IFoo`, solving this constraint
        // sets the subtype witness at that position in `m_args`.
        addSolverConstraint(SolverConstraint::makeWitness(genericDecl, constraintDecl));
    }

    // -------------------------------------------------------------------------
    // Work-list scheduling
    //
    // These routines own the queue discipline. Work-list entries are indices
    // into `m_solverConstraints`, so each logical constraint has one stable
    // state even if conservative wakeups enqueue it multiple times.

    // Add a solver constraint to the stable table and queue it.
    void addSolverConstraint(SolverConstraint const& constraint)
    {
        // Work-list entries are indices rather than copies. That lets the
        // solver mark a logical constraint as satisfied, blocked, or queued in
        // one stable table entry even when conservative wakeups enqueue it
        // multiple times.
        Index constraintIndex = m_solverConstraints.getCount();
        m_solverConstraints.add(constraint);
        enqueueSolverConstraint(constraintIndex);
    }

    // Queue a solver constraint if it still needs a turn.
    void enqueueSolverConstraint(Index constraintIndex)
    {
        // A satisfied constraint does not need another turn until progress
        // explicitly marks it unsatisfied, and an already queued constraint
        // already has a future turn. This keeps repeated wakeups cheap when
        // several solved arguments become available before a blocked constraint
        // runs again.
        auto& constraint = m_solverConstraints[constraintIndex];
        if (constraint.satisfied || constraint.queued)
            return;
        constraint.queued = true;
        m_workList.add(constraintIndex);
    }

    // Wake solver constraints after one argument has changed.
    void wakeSolverConstraintsAfterProgress(Index solvedConstraintIndex)
    {
        // The changed declaration is the dependency key used by default values,
        // witness constraints, and conformance-shape inference. If solving `T`
        // changed the argument list, defaults like `U = T.A` and witnesses like
        // `U : IBar<T>` must be allowed to observe that new value.
        Decl* solvedDecl = m_solverConstraints[solvedConstraintIndex].decl;
        for (Index constraintIndex = 0; constraintIndex < m_solverConstraints.getCount();
             constraintIndex++)
        {
            if (constraintIndex == solvedConstraintIndex)
                continue;

            auto& constraint = m_solverConstraints[constraintIndex];
            // Blocked constraints are unsatisfied, so any progress gives them
            // another turn. Satisfied constraints only need another turn when
            // their stored value mentions the changed argument.
            if (constraint.satisfied)
            {
                if (!solverConstraintDependsOnArg(constraint, solvedDecl))
                    continue;

                // Ordinary constraints are consumed facts. If unification
                // learns a new fact later, it appends a new ordinary
                // constraint; re-running this same one would merge the same
                // constraint value twice. Defaults and witnesses are different:
                // they substitute through the current argument list, so a
                // dependency change can make the old substituted value stale.
                if (constraint.kind == SolverConstraint::Kind::OrdinaryArgConstraint)
                    continue;

                constraint.satisfied = false;
            }
            enqueueSolverConstraint(constraintIndex);
        }
    }

    // Run the work-list solver loop.
    bool runWorkList()
    {
        // The work list is intentionally not split by argument kind. A default
        // generic argument that needs a witness argument simply blocks, and the
        // witness constraint is free to run next. Conversely, a witness
        // constraint that needs a defaulted ordinary argument blocks until that
        // ordinary argument has been produced.
        while (m_workListReadIndex < m_workList.getCount())
        {
            Index constraintIndex = m_workList[m_workListReadIndex++];
            m_solverConstraints[constraintIndex].queued = false;

            if (m_solverConstraints[constraintIndex].satisfied)
                continue;

            ConstraintSolvingState state = trySolveSolverConstraint(constraintIndex);

            // Failure rejects this overload candidate immediately. Blocked work
            // stays unfinished; a later solved argument can wake it and enqueue
            // another attempt.
            if (state == ConstraintSolvingState::Failed)
                return false;
            if (state == ConstraintSolvingState::Blocked)
                continue;

            // `Done` and `MadeProgress` are both terminal for the current
            // argument list. `MadeProgress` additionally wakes dependent work
            // because defaults and witnesses may see a new substitution.
            m_solverConstraints[constraintIndex].satisfied = true;
            if (state == ConstraintSolvingState::MadeProgress)
                wakeSolverConstraintsAfterProgress(constraintIndex);
        }

        // If the queue becomes empty while a constraint remains unsolved, then all
        // remaining work is waiting on arguments that never acquired answers.
        for (Index constraintIndex = 0; constraintIndex < m_solverConstraints.getCount();
             constraintIndex++)
        {
            if (!m_solverConstraints[constraintIndex].satisfied)
                return false;
        }
        return true;
    }

    // -------------------------------------------------------------------------
    // Constraint solving
    //
    // These routines implement the state transitions for one queued solver
    // constraint. A handler either installs an ordinary or witness argument,
    // reports that it is waiting for dependencies, or rejects the generic
    // application.

    // Try to solve one queued solver constraint.
    ConstraintSolvingState trySolveSolverConstraint(Index constraintIndex)
    {
        auto& constraint = m_solverConstraints[constraintIndex];

        // Each constraint kind has a focused solver routine. The routines share
        // a common state protocol so the outer loop can treat ordinary
        // constraints, default generic arguments, and witness constraints the
        // same way after this dispatch point.
        ConstraintSolvingState state = ConstraintSolvingState::Failed;
        switch (constraint.kind)
        {
        case SolverConstraint::Kind::OrdinaryArgConstraint:
            state = solveOrdinaryConstraint(constraint);
            break;
        case SolverConstraint::Kind::DefaultArgConstraint:
            state = solveDefaultGenericArg(constraint);
            break;
        case SolverConstraint::Kind::WitnessConstraint:
            state = solveWitnessConstraint(constraint);
            break;
        }

        // Solving one constraint can discover more constraints. For example,
        // solving a source generic constraint such as `Storage : IStorage<T>`
        // can call into subtype checking, which may compare `IStorage<T>`
        // against an existing conformance shape like `IStorage<U>`. That
        // comparison can invoke value unification and discover a new solver
        // constraint such as `U = T`. Shared unification helpers store those
        // discoveries in `m_context.discoveredConstraints`, so update the
        // state by adding those discovered constraints as solver constraints
        // before the loop advances.
        return updateConstraintSolvingStateForDiscoveredConstraints(state);
    }

    // Update one solver-constraint state after side-effecting unification.
    ConstraintSolvingState updateConstraintSolvingStateForDiscoveredConstraints(
        ConstraintSolvingState state)
    {
        // A failed constraint has already rejected this candidate.
        if (state == ConstraintSolvingState::Failed)
            return state;

        // An empty inbox means the handler's state already tells the whole
        // story for this iteration.
        if (m_context.discoveredConstraints.getCount() == 0)
            return state;

        // Newly discovered ordinary constraints are added even when the current
        // constraint remains blocked. A witness constraint for `T : IFoo<U>`,
        // for example, can discover `U = X` from the solved shape of `T` and
        // still need to wait for that new ordinary constraint before the
        // witness proof itself can be formed.
        addSolverConstraintsForDiscoveredConstraints();
        if (state == ConstraintSolvingState::Blocked)
            return ConstraintSolvingState::Blocked;
        return ConstraintSolvingState::MadeProgress;
    }

    // Return true if a constraint solves an ordinary type/value argument.
    bool isOrdinarySolverConstraint(SolverConstraint const& constraint)
    {
        // Ordinary constraints directly affect the type/value argument prefix.
        // Defaults and witnesses may produce ordinary arguments too, but they
        // do it through their own source-specific solver routines. Provided
        // ordinary arguments are installed before the work list starts.
        return constraint.kind == SolverConstraint::Kind::OrdinaryArgConstraint;
    }

    // Return true if an ordinary constraint requires an exact argument answer.
    bool isExactOrdinaryArgConstraint(SolverConstraint const& constraint)
    {
        return constraint.ordinaryArgMergeMode == SolverConstraint::OrdinaryArgMergeMode::Exact;
    }

    enum class WitnessConstraintInferenceResult
    {
        NotApplicable,
        NoNewOrdinaryConstraint,
        AddedOrdinaryConstraint,
        FailedToAddOrdinaryConstraint,
    };

    enum class JoinedSubtypeInferenceResult
    {
        NoConstraintNeeded,
        AddedConstraint,
        FailedToAddConstraint,
    };

    // Solve a unification-derived constraint for an ordinary argument.
    ConstraintSolvingState solveOrdinaryConstraint(SolverConstraint& c)
    {
        // A satisfied constraint has already contributed its information. This
        // can happen when a solver constraint is conservatively woken after an
        // unrelated argument changes.
        if (c.satisfied)
            return ConstraintSolvingState::Done;

        // Dependent ordinary constraints wait until the values they mention can
        // be substituted. A constraint like `U = T.A` cannot update `U` until
        // `T` and the witness opening `T.A` are ready.
        if (hasUnreadyDependenciesForVal(c.val, c.decl))
            return ConstraintSolvingState::Blocked;

        // Once dependencies are ready, substitute the constraint value through
        // the current generic application. This turns declaration-time values
        // like `T.A` into the current associated type selected by the solved
        // witness.
        if (!substitutePotentiallyDependentConstraint(c))
            return ConstraintSolvingState::Failed;

        // The target declaration decides how the constraint value is stored: type
        // parameters merge according to the constraint kind, value parameters
        // choose exact integer values, and packs assemble pack arguments.
        return solveParameterConstraint(c);
    }

    // Solve an ordinary constraint with the parameter-specific merge routine.
    ConstraintSolvingState solveParameterConstraint(SolverConstraint& c)
    {
        // The same `SolverConstraint` shape is used for all ordinary arguments. The
        // target declaration tells us whether the value should be interpreted as
        // a type, an integer value, or a value pack.
        if (auto typeParam = as<GenericTypeParamDeclBase>(c.decl))
            return solveTypeParamConstraint(c, typeParam);
        if (auto valPackParam = as<GenericValuePackParamDecl>(c.decl))
            return solveValuePackParamConstraint(c, valPackParam);
        if (auto valParam = as<GenericValueParamDecl>(c.decl))
            return solveValueParamConstraint(c, valParam);

        // Any other target means unification discovered a constraint this
        // generic-argument solver does not know how to satisfy.
        return ConstraintSolvingState::Failed;
    }

    // Solve one default generic argument.
    ConstraintSolvingState solveDefaultGenericArg(SolverConstraint const& constraint)
    {
        // Defaults are fallback answers. If a caller provided the ordinary
        // argument or inference already solved it, the default must not replace
        // that concrete answer.
        if (hasNonDefaultOrdinaryArg(constraint.decl))
            return ConstraintSolvingState::Done;

        // Defaults can depend on other ordinary and witness arguments. In
        // `Foo<T : IFoo, U = T.A>`, the `U` default must wait until `T` and the
        // witness for `T : IFoo` are ready.
        if (hasUnreadyDependenciesForVal(constraint.val))
            return ConstraintSolvingState::Blocked;

        // Substitute the raw declaration-time default through the current full
        // generic application. This is the moment `T.A` becomes the associated
        // type selected by the solved witness argument.
        auto fullSubst = SubstitutionSet(buildSubstDeclRef(m_genericDeclRef.getDecl()->inner));
        int diff = 0;
        auto substArg = constraint.val->substituteImpl(m_astBuilder, fullSubst, &diff);
        if (!substArg)
            return ConstraintSolvingState::Failed;

        // Canonicalize the substituted answer before comparing or storing it so
        // repeated retries of the same default do not look like progress.
        substArg = substArg->resolve();
        if (auto substArgType = as<Type>(substArg))
            substArg = substArgType->getCanonicalType();

        // A default constraint can be woken multiple times. If it recomputes the
        // same argument, report `Done` so dependents are not needlessly woken.
        auto& argInfo = m_argInfo[constraint.decl];
        if (auto previousDefaultArg = argInfo.getSubstitutedDefaultArg())
        {
            if (previousDefaultArg->equals(substArg))
                return ConstraintSolvingState::Done;
        }

        // Set both the argument and the state. The value goes into `m_args`
        // because substitutions read only that array; the state remains
        // replaceable so later ordinary inference can still override a default.
        if (!setCurrentArg(constraint.decl, substArg))
            return ConstraintSolvingState::Failed;
        argInfo.setDefaultGenericArg(substArg);
        return ConstraintSolvingState::MadeProgress;
    }

    // Solve one source generic constraint into a witness argument.
    ConstraintSolvingState solveWitnessConstraint(SolverConstraint const& constraint)
    {
        // A subtype/equality witness can expose ordinary-argument inference
        // before the witness itself is ready. For `T : IFoo<U>`, once `T` is
        // known, comparing its actual conformance shape against `IFoo<U>` can
        // discover `U = X`; the witness still waits until that new ordinary
        // constraint has solved `U`.
        auto inferenceResult = tryInferOrdinaryArgsFromWitnessConstraint(constraint);
        switch (inferenceResult)
        {
        case WitnessConstraintInferenceResult::AddedOrdinaryConstraint:
            return ConstraintSolvingState::Blocked;
        case WitnessConstraintInferenceResult::FailedToAddOrdinaryConstraint:
            // Shape inference is only a source of additional ordinary-argument
            // hints. If adding such a hint fails, keep going to the normal
            // witness proof below; that proof remains the authoritative
            // accept/reject step for this source generic constraint.
            break;
        case WitnessConstraintInferenceResult::NotApplicable:
        case WitnessConstraintInferenceResult::NoNewOrdinaryConstraint:
            break;
        }

        // A witness can only be proved after the ordinary and witness arguments
        // it mentions are ready. For example, `T : IFoo<U>` waits for both `T`
        // and `U`, and a coercion constraint waits for its substituted source
        // and destination types.
        if (hasUnreadyDependenciesForWitnessConstraint(constraint.decl))
            return ConstraintSolvingState::Blocked;

        // Remember the previous witness so a conservative retry can report
        // `Done` when it proves the same value again.
        Val* oldWitnessArg = getSolvedWitnessArg(constraint.decl);

        // The constraint kind decides how the witness is produced: subtype
        // constraints use `isSubtype()`, coercion constraints use `_coerce`,
        // and pack/diff constraints use their specialized proof helpers. The
        // helper only returns the witness value; this function owns installing
        // that value into the live argument list so every witness constraint has
        // the same write path.
        Val* solvedWitness = trySolveWitnessForConstraint(constraint.genericDecl, constraint.decl);
        if (!solvedWitness)
            return ConstraintSolvingState::Failed;

        // Store the proof in `m_args` immediately. Later ordinary defaults and
        // witness constraints substitute through this same array, so a solved
        // witness must become visible before the next work-list iteration.
        if (!setSolvedWitnessArg(constraint.decl, solvedWitness))
            return ConstraintSolvingState::Failed;

        // Retried witness work only counts as progress when the actual witness
        // changed. This prevents a stable `T : IFoo` proof from waking the whole
        // loop again after unrelated progress.
        if (oldWitnessArg && oldWitnessArg->equals(solvedWitness))
            return ConstraintSolvingState::Done;

        return ConstraintSolvingState::MadeProgress;
    }

    // Try to discover ordinary-argument constraints from one witness constraint.
    WitnessConstraintInferenceResult tryInferOrdinaryArgsFromWitnessConstraint(
        SolverConstraint const& constraint)
    {
        // Only subtype/equality constraints have a conformance shape that can
        // expose more ordinary arguments. Coercion, pack, and differentiability
        // witnesses prove their own source constraint but do not compare a
        // solved subject type against an interface with generic arguments.
        auto typeConstraintDecl = as<GenericTypeConstraintDecl>(constraint.decl);
        if (!typeConstraintDecl)
            return WitnessConstraintInferenceResult::NotApplicable;

        // The subject side must be ready before the shape comparison is useful.
        // In `T : IFoo<U>`, comparing before `T` is known would only restate the
        // source constraint; after `T` is solved to a concrete type, the subtype
        // system can find its actual `IFoo<X>` facet and unification can
        // discover `U = X`.
        if (hasUnreadyDependenciesForVal(typeConstraintDecl->sub.type))
            return WitnessConstraintInferenceResult::NoNewOrdinaryConstraint;

        // Substitute the source constraint through the current argument list.
        // The target side may still contain default substitution args, such as
        // `U` in `IFoo<U>`, because those are exactly the ordinary arguments we
        // are trying to discover.
        auto constraintDeclRef =
            buildSubstDeclRef(typeConstraintDecl).as<GenericTypeConstraintDecl>();
        auto sub = getSub(m_astBuilder, constraintDeclRef);
        auto sup = getSup(m_astBuilder, constraintDeclRef);

        // `TryJoinTypes()` is the existing path that compares a concrete type
        // against an interface shape and uses facet unification to append
        // ordinary constraints into `m_context.discoveredConstraints`. The
        // witness solver does not interpret failure here as proof failure; the
        // normal witness step below remains responsible for accepting optional
        // constraints or rejecting required ones.
        auto joinedSub = m_visitor->TryJoinTypes(&m_context, QualType(sub), QualType(sup));

        // Joining can also produce a better subject type. For `sqrt(1)`, call
        // inference first solves `T` as `int`, then the source constraint
        // `T : __BuiltinFloatingPointType` joins `int` with the floating-point
        // interface and selects `float`. That `float` is not a witness yet; it
        // is a better ordinary argument for `T`, so add it to the same
        // work-list loop and let the witness retry after `T` changes.
        auto joinedSubjectResult =
            addSubjectConstraintForJoinedSubtypeIfNeeded(typeConstraintDecl, sub, joinedSub);
        switch (joinedSubjectResult)
        {
        case JoinedSubtypeInferenceResult::NoConstraintNeeded:
            return WitnessConstraintInferenceResult::NoNewOrdinaryConstraint;
        case JoinedSubtypeInferenceResult::AddedConstraint:
            return WitnessConstraintInferenceResult::AddedOrdinaryConstraint;
        case JoinedSubtypeInferenceResult::FailedToAddConstraint:
            return WitnessConstraintInferenceResult::FailedToAddOrdinaryConstraint;
        }
        return WitnessConstraintInferenceResult::NoNewOrdinaryConstraint;
    }

    // Add an ordinary constraint when subtype joining improves the subject type.
    JoinedSubtypeInferenceResult addSubjectConstraintForJoinedSubtypeIfNeeded(
        GenericTypeConstraintDecl* constraintDecl,
        Type* currentSub,
        Type* joinedSub)
    {
        // If joining failed, or if it confirmed the subject type already in the
        // argument list, there is no new ordinary argument to solve before the
        // witness is attempted.
        if (!joinedSub || currentSub->equals(joinedSub))
            return JoinedSubtypeInferenceResult::NoConstraintNeeded;

        // Re-unify the declaration-time subject with the joined type rather
        // than writing to one parameter directly. This keeps the general
        // unification behavior for subjects such as `vector<T, N>`: a joined
        // subject `vector<float, N>` should discover `T = float`, while a
        // direct subject `T` discovers the simpler `T = float` constraint.
        UnificationOptions unificationOptions;
        unificationOptions.optionalConstraint =
            constraintDecl->hasModifier<OptionalConstraintModifier>();
        unificationOptions.equalityConstraint = constraintDecl->isEqualityConstraint;
        if (m_visitor->TryUnifyTypes(
                m_context,
                unificationOptions,
                QualType(constraintDecl->sub.type),
                QualType(joinedSub)))
        {
            return JoinedSubtypeInferenceResult::AddedConstraint;
        }
        return JoinedSubtypeInferenceResult::FailedToAddConstraint;
    }

    // Substitute a dependent ordinary constraint through current arguments.
    bool substitutePotentiallyDependentConstraint(SolverConstraint& c)
    {
        // Non-dependent constraints already contain their final value. For
        // example, `T = int` can be merged directly without rebuilding a
        // substitution.
        if (!c.potentiallyDependent)
            return true;

        // Potentially dependent ordinary constraints are substituted through
        // the current partial substitution. The work-list dependency check has
        // already ensured that any ordinary or witness argument mentioned by
        // `c.val` is ready, so this substitution can turn projections like
        // `T.A` into the value that should constrain the target argument.
        auto fullSubst = SubstitutionSet(buildSubstDeclRef(m_genericDeclRef.getDecl()->inner));
        auto newVal = c.val->substitute(m_astBuilder, fullSubst);
        if (newVal)
            c.val = newVal;
        return true;
    }

    // Solve one ordinary constraint for a type argument.
    ConstraintSolvingState solveTypeParamConstraint(
        SolverConstraint& c,
        GenericTypeParamDeclBase* typeParam)
    {
        SLANG_ASSERT(typeParam->parameterIndex != -1);

        // Caller-provided ordinary arguments are fixed. If the user wrote
        // `foo<float>(x)`, inference from `x` should not rewrite `T`.
        if (getArgState(typeParam) == ArgState::CallerProvidedOrdinaryArg)
        {
            c.satisfied = true;
            return ConstraintSolvingState::Done;
        }

        // Non-pack type parameters keep a single accumulated type. Type packs
        // keep one accumulated entry per pack element, so a constraint on
        // `T[2]` grows the list to at least three entries.
        bool isPack = as<GenericTypePackParamDecl>(typeParam) != nullptr;
        auto& argInfo = m_argInfo[typeParam];
        auto oldState = argInfo.getState();
        auto oldArg = getCurrentArg(typeParam);
        auto& types = argInfo.getTypeConstraints();
        if (!isPack)
            types.setCount(1);

        QualType* ptype = nullptr;
        if (isPack)
        {
            types.setCount(Math::Max(types.getCount(), c.indexInPack + 1));
            ptype = &types[c.indexInPack];
        }
        else
        {
            ptype = &types[0];
        }

        // The constraint value must be a type at this point. It may have come
        // directly from call-site inference, or from substituting a dependent
        // value like `T.A`.
        QualType& type = *ptype;
        auto cType = QualType(as<Type>(c.val), c.isUsedAsLValue);
        SLANG_RELEASE_ASSERT(cType);

        // The first type constraint initializes the accumulated type. Ordinary type
        // inference can start from this value and later join with additional
        // type constraints; exact constraints require later exact constraints
        // to agree with it.
        if (!type)
        {
            type = cType;
            argInfo.setPriority(c.priority);
        }
        // Later constraints are merged according to their merge mode. Equality
        // constraints must agree with the current answer. Ordinary type
        // constraints use the existing join behavior for call inference.
        else if (!mergeTypeConstraint(type, argInfo.getPriority(), c, cType))
        {
            return ConstraintSolvingState::Failed;
        }

        // Preserve the strongest priority seen so later merges know whether a
        // default, optional hint, or required constraint owns the current type.
        if (c.priority < argInfo.getPriority())
            argInfo.setPriority(c.priority);

        // Publish the current argument to `m_args` immediately. Dependent
        // defaults and witnesses read from `m_args`, so type-pack progress must
        // be visible even before the whole work list is done.
        Val* arg = isPack ? getTypePackParamArg(typeParam) : type.type;
        if (arg)
        {
            if (!setCurrentArg(typeParam, arg))
                return ConstraintSolvingState::Failed;
            argInfo.setState(ArgState::SolvedOrdinaryArg);
        }

        // Report progress only when this constraint produced a new published
        // argument. A consumed hint with no argument should not wake dependents.
        c.satisfied = true;
        if (!arg)
            return ConstraintSolvingState::Done;
        if (oldState != ArgState::SolvedOrdinaryArg)
            return ConstraintSolvingState::MadeProgress;
        if (!oldArg || !oldArg->equals(arg))
            return ConstraintSolvingState::MadeProgress;
        return ConstraintSolvingState::Done;
    }

    // Combine a new inferred type with the current type argument.
    bool mergeTypeConstraint(
        QualType& ioType,
        ConstraintPriority currentPriority,
        SolverConstraint const& c,
        QualType cType)
    {
        // Equality constraints are direct answers such as `T = int` or
        // `U = T.A`. Same-priority equality constraints must agree exactly;
        // otherwise the candidate cannot form one concrete ordinary argument.
        if (isExactOrdinaryArgConstraint(c))
        {
            if (c.priority < currentPriority)
            {
                ioType = cType;
                return true;
            }
            if (c.priority > currentPriority)
                return true;
            if (!ioType->equals(cType))
                return false;
            ioType = QualType(ioType.type, ioType.isLeftValue || cType.isLeftValue);
            return true;
        }

        // Non-exact type constraints use the join path. This preserves existing
        // common-type behavior for ordinary call inference, such as picking a
        // type that several arguments can convert to.
        auto joinType = m_visitor->TryJoinTypes(&m_context, ioType, cType);
        if (!joinType)
        {
            // If no join exists, priority decides whether a newer constraint
            // can replace the current answer. Required constraints beat
            // optional hints, optional hints beat defaults, and same-priority
            // non-exact conflicts reject the candidate.
            if (c.priority < currentPriority)
                joinType = cType;
            else if (c.priority > currentPriority)
                joinType = ioType;
            else
                return false;
        }

        // Preserve l-value information from either side because a joined type
        // used later for parameter checking must still remember whether any
        // source constraint required l-value treatment.
        ioType = QualType(joinType, ioType.isLeftValue || cType.isLeftValue);
        return true;
    }

    // Solve one ordinary constraint for a generic value-pack argument.
    ConstraintSolvingState solveValuePackParamConstraint(
        SolverConstraint& c,
        GenericValuePackParamDecl* valPackParam)
    {
        SLANG_ASSERT(valPackParam->parameterIndex != -1);

        // Caller-provided packs are fixed in the same way as other ordinary
        // arguments. Inference may confirm them, but it must not replace them.
        if (getArgState(valPackParam) == ArgState::CallerProvidedOrdinaryArg)
        {
            c.satisfied = true;
            return ConstraintSolvingState::Done;
        }

        // Keep the old published value so the work-list result can distinguish
        // real progress from a retry that rediscovered the same pack.
        auto& argInfo = m_argInfo[valPackParam];
        auto oldState = argInfo.getState();
        auto oldArg = getCurrentArg(valPackParam);
        Val* val = argInfo.getState() == ArgState::SolvedOrdinaryArg ? getCurrentArg(valPackParam)
                                                                     : nullptr;

        // A value pack can be solved either by a concrete pack, such as
        // `[1, 2, 3]`, or by forwarding another pack parameter. Other shapes are
        // ignored here and will fail final validation if no value is found.
        if (auto cValPack = as<ConcreteIntValPack>(c.val))
        {
            if (!val)
                val = cValPack;
        }
        else if (auto declRefIntVal = as<DeclRefIntVal>(c.val))
        {
            if (!val)
                val = declRefIntVal;
        }

        // Publish the pack as soon as it is known so a non-empty-pack witness or
        // a dependent default can inspect it in the same work-list loop.
        if (val)
        {
            if (!setCurrentArg(valPackParam, val))
                return ConstraintSolvingState::Failed;
            argInfo.setState(ArgState::SolvedOrdinaryArg);
        }

        // Only a newly published or changed pack wakes dependents. If the
        // constraint did not recognize a pack value, it is consumed without
        // progress so another solver constraint can still solve the parameter later.
        c.satisfied = true;
        if (!val)
            return ConstraintSolvingState::Done;
        if (oldState != ArgState::SolvedOrdinaryArg)
            return ConstraintSolvingState::MadeProgress;
        if (!oldArg || !oldArg->equals(val))
            return ConstraintSolvingState::MadeProgress;
        return ConstraintSolvingState::Done;
    }

    // Solve one ordinary constraint for a generic value argument.
    ConstraintSolvingState solveValueParamConstraint(
        SolverConstraint& c,
        GenericValueParamDecl* valParam)
    {
        SLANG_ASSERT(valParam->parameterIndex != -1);

        // Caller-provided value arguments are fixed. For `Foo<4>(x)`, inference
        // from `x` may verify the call but cannot change `N` to another value.
        if (getArgState(valParam) == ArgState::CallerProvidedOrdinaryArg)
        {
            c.satisfied = true;
            return ConstraintSolvingState::Done;
        }

        // Keep the old value and priority so the work-list result can tell
        // whether this constraint actually changed the current argument.
        auto& argInfo = m_argInfo[valParam];
        auto oldState = argInfo.getState();
        auto oldArg = getCurrentArg(valParam);
        auto oldPriority = argInfo.getPriority();
        Val* val =
            argInfo.getState() == ArgState::SolvedOrdinaryArg ? getCurrentArg(valParam) : nullptr;
        ConstraintPriority valPriority = argInfo.getPriority();

        auto cVal = as<IntVal>(c.val);
        SLANG_RELEASE_ASSERT(cVal);

        // A higher-priority value constraint replaces a lower-priority value.
        // If two
        // same-priority constraints disagree, as in conflicting requirements
        // `N = 4` and `N = 8`, this candidate is unsolvable.
        if (!val || (c.priority < valPriority))
        {
            val = cVal;
            valPriority = c.priority;
        }
        else if (valPriority == c.priority && !val->equals(cVal))
        {
            return ConstraintSolvingState::Failed;
        }

        // Publish the value immediately because defaults and witness
        // constraints can depend on it, for example `Array<T, N>` or
        // `where N : INonZero`.
        if (val && !setCurrentArg(valParam, val))
            return ConstraintSolvingState::Failed;
        argInfo.setPriority(valPriority);
        argInfo.setState(ArgState::SolvedOrdinaryArg);

        // Waking dependents is only necessary if the published value or the
        // priority that selected it changed.
        c.satisfied = true;
        if (oldState != ArgState::SolvedOrdinaryArg || oldPriority != valPriority)
            return ConstraintSolvingState::MadeProgress;
        if (!oldArg || !oldArg->equals(val))
            return ConstraintSolvingState::MadeProgress;
        return ConstraintSolvingState::Done;
    }

    // -------------------------------------------------------------------------
    // Dependency analysis
    //
    // These routines answer two questions for scheduling: whether a value can be
    // substituted through the current live argument list, and whether a solved
    // argument should wake another solver constraint.

    // Return true if a witness constraint still depends on unready arguments.
    bool hasUnreadyDependenciesForWitnessConstraint(Decl* constraintDecl)
    {
        // Subtype/equality witnesses need both sides substituted. A constraint
        // like `T : IFoo<U>` waits for `T`, `U`, and any witness needed by
        // associated-type lookups inside those types.
        if (auto genericTypeConstraintDecl = as<GenericTypeConstraintDecl>(constraintDecl))
        {
            return hasUnreadyDependenciesForVal(genericTypeConstraintDecl->sub.type) ||
                   hasUnreadyDependenciesForVal(genericTypeConstraintDecl->sup.type);
        }

        // Coercion witnesses similarly wait for their source and destination
        // types before asking `_coerce` for a proof.
        if (auto typeCoercionConstraintDecl = as<TypeCoercionConstraintDecl>(constraintDecl))
        {
            return hasUnreadyDependenciesForVal(typeCoercionConstraintDecl->fromType.type) ||
                   hasUnreadyDependenciesForVal(typeCoercionConstraintDecl->toType.type);
        }

        // Non-empty-pack witnesses name the pack through syntax rather than a
        // `Val`. The pack argument must have a current value before we can ask
        // whether it is known to be non-empty.
        if (auto nonEmptyConstraintDecl = as<NonEmptyPackConstraintDecl>(constraintDecl))
        {
            auto packDeclRef = getPackDeclRefForNonEmptyConstraint(nonEmptyConstraintDecl);
            if (auto typePackDeclRef = packDeclRef.as<GenericTypePackParamDecl>())
                return !isArgReady(typePackDeclRef.getDecl());
            if (auto valuePackDeclRef = packDeclRef.as<GenericValuePackParamDecl>())
                return !isArgReady(valuePackDeclRef.getDecl());
            return true;
        }

        // Diff-type-info witnesses wait for their substituted subject type.
        if (auto hasDiffTypeInfoConstraintDecl = as<HasDiffTypeInfoConstraintDecl>(constraintDecl))
        {
            return hasUnreadyDependenciesForVal(hasDiffTypeInfoConstraintDecl->type.type);
        }
        return false;
    }

    // Return true if a value mentions unready ordinary or witness arguments.
    bool hasUnreadyDependenciesForVal(Val* val, Decl* subjectParamDecl = nullptr)
    {
        // Dependency collection is structural and cached, so this check can run
        // before every dependent substitution. The optional subject parameter
        // lets `T : IFoo<T>` inspect its own conformance shape without treating
        // the constraint on `T` as an unready dependency.
        auto dependencies = getDependentDeclsForVal(val);
        for (auto dependencyDecl : dependencies)
        {
            if (!isDependencyReady(dependencyDecl, subjectParamDecl))
                return true;
        }
        return false;
    }

    // Return true if a value syntactically mentions a changed argument.
    bool valDependsOnArg(Val* val, Decl* changedArgDecl)
    {
        // A missing changed declaration means the solver constraint reported progress
        // without publishing an ordinary or witness argument that another value
        // can depend on.
        if (!changedArgDecl)
            return false;

        // Use the same cached dependency list as readiness checks. Here the
        // question is whether a previously satisfied constraint should be
        // retried after this specific argument changed.
        auto dependencies = getDependentDeclsForVal(val);
        for (auto dependencyDecl : dependencies)
        {
            if (dependencyDecl == changedArgDecl)
                return true;
        }
        return false;
    }

    // Return true if a solver constraint depends on a changed argument.
    bool solverConstraintDependsOnArg(SolverConstraint const& constraint, Decl* changedArgDecl)
    {
        switch (constraint.kind)
        {
        case SolverConstraint::Kind::OrdinaryArgConstraint:
            // Ordinary constraints depend on the value they will merge.
            // A constraint like `U = T.A` should rerun when `T` or the witness
            // used to open `T.A` changes.
            return valDependsOnArg(constraint.val, changedArgDecl);

        case SolverConstraint::Kind::DefaultArgConstraint:
            // Default generic arguments are declaration-time values substituted
            // through current arguments. `U = T.A` depends on `T` and the
            // witness for `T : IFoo`.
            return valDependsOnArg(constraint.val, changedArgDecl);

        case SolverConstraint::Kind::WitnessConstraint:
            // Witness constraints have several declaration shapes, so delegate
            // to the shape-aware helper.
            return witnessConstraintDependsOnArg(constraint.decl, changedArgDecl);
        }
        return false;
    }

    // Return true if a witness constraint depends on a changed argument.
    bool witnessConstraintDependsOnArg(Decl* constraintDecl, Decl* changedArgDecl)
    {
        // A missing changed declaration cannot invalidate any already solved
        // witness.
        if (!changedArgDecl)
            return false;

        // Subtype/equality constraints mention dependencies through their
        // source and target types. `T : IFoo<U>` depends on both `T` and `U`.
        if (auto genericTypeConstraintDecl = as<GenericTypeConstraintDecl>(constraintDecl))
        {
            return valDependsOnArg(genericTypeConstraintDecl->sub.type, changedArgDecl) ||
                   valDependsOnArg(genericTypeConstraintDecl->sup.type, changedArgDecl);
        }

        // Coercion constraints are analogous: either side can contain ordinary
        // or witness arguments that affect the conversion proof.
        if (auto typeCoercionConstraintDecl = as<TypeCoercionConstraintDecl>(constraintDecl))
        {
            return valDependsOnArg(typeCoercionConstraintDecl->fromType.type, changedArgDecl) ||
                   valDependsOnArg(typeCoercionConstraintDecl->toType.type, changedArgDecl);
        }

        // Non-empty-pack constraints name the pack declaration through syntax
        // instead of a `Val`, so compare the referenced pack declaration
        // directly.
        if (auto nonEmptyConstraintDecl = as<NonEmptyPackConstraintDecl>(constraintDecl))
        {
            auto packDeclRef = getPackDeclRefForNonEmptyConstraint(nonEmptyConstraintDecl);
            if (auto typePackDeclRef = packDeclRef.as<GenericTypePackParamDecl>())
                return typePackDeclRef.getDecl() == changedArgDecl;
            if (auto valuePackDeclRef = packDeclRef.as<GenericValuePackParamDecl>())
                return valuePackDeclRef.getDecl() == changedArgDecl;
            return false;
        }

        // Diff-type-info constraints depend on the subject type they inspect.
        if (auto hasDiffTypeInfoConstraintDecl = as<HasDiffTypeInfoConstraintDecl>(constraintDecl))
            return valDependsOnArg(hasDiffTypeInfoConstraintDecl->type.type, changedArgDecl);
        return false;
    }

    // Return the ordinary or witness argument declarations mentioned by a value.
    ArrayView<Decl*> getDependentDeclsForVal(Val* val)
    {
        auto shared = m_visitor->getShared();
        if (!val)
            return ArrayView<Decl*>();

        // The cache stores only syntax-level dependencies. A value like `T.A`
        // has the same dependency list before and after `T` is solved, so the
        // entry can be reused across specializations. Empty dependency lists
        // are cached too; an empty `List` does not allocate its element buffer.
        if (auto cachedDependencies =
                shared->m_genericSolverValToDependentDeclsCache.tryGetValue(val))
            return cachedDependencies->getArrayView();

        // Walk once on a cache miss. The recursive walk collects declarations,
        // not readiness, because readiness is solver-local and depends on which
        // generic application is currently being solved.
        ShortList<Decl*> dependencies;
        HashSet<Val*> visitedVals;
        collectDependentDeclsInVal(val, visitedVals, dependencies);

        // Cache dependencies in a heap-backed `List` and return a by-value
        // `ArrayView`. The list may be empty, which records the common
        // no-dependency case without a second dictionary. Callers should not
        // keep the view across operations that may mutate the shared dependency
        // cache.
        List<Decl*> cachedDependencies;
        cachedDependencies.addRange(dependencies.getArrayView().arrayView);
        shared->m_genericSolverValToDependentDeclsCache.add(val, _Move(cachedDependencies));
        return shared->m_genericSolverValToDependentDeclsCache.tryGetValue(val)->getArrayView();
    }

    // Collect argument declarations mentioned by a value tree.
    void collectDependentDeclsInVal(
        Val* val,
        HashSet<Val*>& ioVisitedVals,
        ShortList<Decl*>& ioDependencies)
    {
        // Values can share substructure or contain cycles through canonical
        // nodes. The visited set makes the walk robust and keeps one complex
        // dependent value from expanding exponentially.
        if (!val || ioVisitedVals.contains(val))
            return;
        ioVisitedVals.add(val);

        // The dependency protocol is local to decl-ref nodes. A `DeclRefType`,
        // `DeclRefIntVal`, or `DeclaredSubtypeWitness` carries its decl-ref as a
        // value operand, so the recursive walk reaches this branch for ordinary
        // arguments and witness arguments alike.
        if (auto declRef = as<DeclRefBase>(val))
        {
            auto decl = declRef->getDecl();
            if (as<GenericTypeParamDeclBase>(decl))
                addDependentDecl(ioDependencies, decl);
            else if (isGenericValueParam(decl))
                addDependentDecl(ioDependencies, decl);
            else if (isGenericConstraintDecl(decl))
                addDependentDecl(ioDependencies, decl);
        }

        // Associated-type lookups and other dependent values can hide the
        // relevant decl-refs inside operands. Recursing over all value operands
        // lets `T.A` collect both the ordinary argument `T` and any declared
        // subtype witness used to open the associated type.
        for (auto operand : val->m_operands)
        {
            if (operand.kind == ValNodeOperandKind::ValNode)
                collectDependentDeclsInVal(operand.getVal(), ioVisitedVals, ioDependencies);
        }
    }

    // Add one declaration to a dependency list.
    void addDependentDecl(ShortList<Decl*>& ioDependencies, Decl* dependencyDecl)
    {
        // A value can mention the same argument through several operands, but
        // one entry is enough for readiness and wakeup checks. Keeping the list
        // compact matters because these dependency lists are cached and reused.
        if (dependencyDecl && !ioDependencies.contains(dependencyDecl))
            ioDependencies.add(dependencyDecl);
    }

    // Return true if a cached dependency can be used now.
    bool isDependencyReady(Decl* dependencyDecl, Decl* subjectParamDecl)
    {
        // Missing dependencies and direct self references do not block. The
        // self case lets a conformance hint inspect a shape like `T : IFoo<T>`
        // while it is trying to infer other arguments from that same constraint.
        if (!dependencyDecl || dependencyDecl == subjectParamDecl)
            return true;

        // A source generic constraint targeting the subject parameter is also
        // considered ready. For `T : IFoo<U>`, the constraint itself is the fact
        // we are using to compare conformance shapes and infer `U`.
        if (isConstraintForGenericParam(dependencyDecl, subjectParamDecl))
            return true;

        // Every other dependency uses the common ordinary/witness readiness
        // state for the generic application being solved.
        return isArgReady(dependencyDecl);
    }

    // Return true if a source generic constraint targets a parameter.
    bool isConstraintForGenericParam(Decl* constraintDecl, Decl* paramDecl)
    {
        // Without a subject parameter there is no self-conformance special case
        // to recognize.
        if (!paramDecl)
            return false;

        // Only subtype/equality constraints have a subject type. Other witness
        // constraints, such as non-empty-pack checks, cannot be "for" a generic
        // type parameter in this sense.
        auto typeConstraintDecl = as<GenericTypeConstraintDecl>(constraintDecl);
        if (!typeConstraintDecl)
            return false;

        // The constraint is for the parameter when its source side is exactly a
        // decl-ref to that parameter, as in `T : IFoo`.
        auto subDeclRef = isDeclRefTypeOf<Decl>(typeConstraintDecl->sub.type);
        return subDeclRef && subDeclRef.getDecl() == paramDecl;
    }

    // -------------------------------------------------------------------------
    // Argument access and state
    //
    // These routines centralize the serialized argument layout. Ordinary
    // arguments use their parameter index, witness arguments follow in source
    // generic-constraint order, and `ArgState` records whether the current value
    // in `m_args` is a real answer or still a default substitution argument.

    // Return the pack decl-ref named by a non-empty-pack constraint.
    DeclRef<Decl> getPackDeclRefForNonEmptyConstraint(NonEmptyPackConstraintDecl* constraintDecl)
    {
        // The pack is stored as syntax because the constraint is about a source
        // generic parameter rather than a `Val` tree. Keep the decl-ref intact
        // while classifying it so any substitution context on the expression is
        // still available to the caller.
        if (auto declRefExpr = as<DeclRefExpr>(constraintDecl->packExpr))
            return getDeclRef(m_astBuilder, declRefExpr);
        return DeclRef<Decl>();
    }

    // Return true if an ordinary or witness argument is ready for substitution.
    bool isArgReady(Decl* argDecl)
    {
        // Values can mention declarations that are not generic arguments at all.
        // Those declarations are fixed facts from the surrounding program and
        // never block this solver.
        if (!argDecl)
            return true;

        // Only arguments owned by the generic declarations currently being
        // solved can be unready. Arguments from an outer already-substituted
        // scope are fixed by their own decl-ref and should not block this
        // application.
        if (getGenericParamIndex(argDecl) >= 0 || isGenericConstraintDecl(argDecl))
        {
            if (!isGenericDeclBeingSolved(argDecl->parentDecl))
                return true;
            return isReadyArgState(getArgState(argDecl));
        }

        return true;
    }

    // Return true if an argument is the default substitution arg for a parameter.
    bool isDefaultSubstitutionArgForParam(Decl* paramDecl, Val* arg)
    {
        // A type parameter's default substitution arg is a direct type reference
        // to itself, such as `DeclRefType(T)` for `T`.
        if (auto declRefType = as<DeclRefType>(arg))
            return declRefType->getDeclRef().getDecl() == paramDecl;

        // A value parameter's default substitution arg is an integer decl-ref to
        // itself. Casts can be inserted while checking integer expressions, so
        // peel them before comparing the referenced declaration.
        if (auto intVal = as<IntVal>(arg))
        {
            if (auto declRefIntVal = getDeclRefIntValIgnoringCasts(intVal))
                return declRefIntVal->getDeclRef().getDecl() == paramDecl;
        }
        return false;
    }

    // Return the current state for an argument.
    ArgState getArgState(Decl* argDecl)
    {
        // Metadata is created lazily when an argument changes. If no metadata
        // exists yet, the current value in `m_args` is still the initial default
        // substitution arg from `getDefaultSubstitutionArgs()`.
        auto solvedInfo = m_argInfo.tryGetValue(argDecl);
        return solvedInfo ? solvedInfo->getState() : ArgState::DefaultSubstitutionArg;
    }

    // Set only the state metadata for an argument.
    void setArgState(Decl* argDecl, ArgState state)
    {
        // Argument values live in `m_args`; this metadata explains how to
        // interpret that value. Keeping the two separate lets `m_args` stay the
        // single source used by substitution while `ArgInfo` tracks readiness,
        // replaceability, and overload-ranking metadata.
        m_argInfo[argDecl].setState(state);
    }

    // Return true if an argument state can be used for substitution.
    bool isReadyArgState(ArgState state)
    {
        // Every state except `DefaultSubstitutionArg` represents a value that is
        // usable in a partial substitution. Defaults remain replaceable, but
        // they are still useful for dependent work such as `V = U.A`.
        switch (state)
        {
        case ArgState::CallerProvidedOrdinaryArg:
        case ArgState::DependentOrdinaryArg:
        case ArgState::SolvedOrdinaryArg:
        case ArgState::DefaultGenericArg:
        case ArgState::EmptyPackArg:
        case ArgState::SolvedWitnessArg:
            return true;
        case ArgState::DefaultSubstitutionArg:
            return false;
        }
        return false;
    }

    // Return true if a generic declaration is currently being solved.
    bool isGenericDeclBeingSolved(Decl* maybeGenericDecl)
    {
        // `m_args` is keyed only for the generic declarations in the current
        // application chain. A cached dependency from another specialization or
        // outer scope should therefore be treated as already fixed.
        auto genericDecl = as<GenericDecl>(maybeGenericDecl);
        return genericDecl && m_args.containsKey(genericDecl);
    }

    // Return true if an ordinary argument has a non-default answer.
    bool hasNonDefaultOrdinaryArg(Decl* paramDecl)
    {
        // Caller-provided and inferred ordinary arguments outrank defaults.
        // Dependent self references, empty packs, and previous default generic
        // arguments remain replaceable by a more concrete ordinary solution.
        auto state = getArgState(paramDecl);
        return state == ArgState::CallerProvidedOrdinaryArg || state == ArgState::SolvedOrdinaryArg;
    }

    // Return the solved witness argument for one source generic constraint.
    Val* getSolvedWitnessArg(Decl* constraintDecl)
    {
        // The current value in `m_args` may still be the default substitution
        // arg. Only the `SolvedWitnessArg` state means it is an actual proof
        // produced by the solver.
        if (getArgState(constraintDecl) != ArgState::SolvedWitnessArg)
            return nullptr;
        return getCurrentArg(constraintDecl);
    }

    // Return the current argument for one argument declaration.
    Val* getCurrentArg(Decl* argDecl)
    {
        // Every ordinary or witness argument belongs to a generic declaration.
        // Without that owner, there is no argument list to query.
        auto genericDecl = as<GenericDecl>(argDecl->parentDecl);
        if (!genericDecl)
            return nullptr;

        // The argument list may be absent for declarations outside the current
        // generic application chain.
        auto args = m_args.tryGetValue(genericDecl);
        if (!args)
            return nullptr;
        return getCurrentArg(argDecl, *args);
    }

    // Return one argument from an already-selected argument list.
    Val* getCurrentArg(Decl* argDecl, ShortList<Val*> const& args)
    {
        // Serialization order is shared with `getGenericAppDeclRef`: ordinary
        // arguments first, then witness arguments. If the declaration is not
        // serializable for this generic, the caller cannot read an argument.
        Index argIndex = getSerializedArgIndex(argDecl);
        if (argIndex < 0 || argIndex >= args.getCount())
            return nullptr;
        return args[argIndex];
    }

    // Set one argument in its owning generic application's argument list.
    bool setCurrentArg(Decl* argDecl, Val* arg)
    {
        // The declaration's parent identifies the generic argument list to
        // update. A null argument would make the final generic application
        // malformed, so treat it as failure instead of storing it.
        auto genericDecl = as<GenericDecl>(argDecl->parentDecl);
        if (!genericDecl || !arg)
            return false;

        // Ordinary arguments use declaration `parameterIndex`; witness arguments
        // are serialized after the ordinary arguments according to source
        // generic constraint order.
        Index argIndex = getSerializedArgIndex(argDecl);
        if (argIndex < 0)
            return false;

        // Initialization creates the complete serialized argument list before
        // solving starts. If the index is out of range here, the solver has lost
        // the generic application's argument layout, and growing the list would
        // hide that bug while leaving earlier entries with meaningless values.
        auto args = m_args.tryGetValue(genericDecl);
        if (!args)
            return false;
        SLANG_ASSERT(argIndex < args->getCount());
        if (argIndex >= args->getCount())
            return false;
        (*args)[argIndex] = arg;
        return true;
    }

    // Set a solved witness argument.
    bool setSolvedWitnessArg(Decl* argDecl, Val* arg)
    {
        // Witness readiness is tracked in metadata, while the witness value
        // itself must live in `m_args` so dependent substitutions can use it to
        // open associated-type lookups.
        setArgState(argDecl, ArgState::SolvedWitnessArg);
        return setCurrentArg(argDecl, arg);
    }

    // Return the serialized index for an ordinary or witness argument declaration.
    Index getSerializedArgIndex(Decl* argDecl)
    {
        // Ordinary arguments already carry their index on the parameter
        // declaration.
        Index paramIndex = getGenericParamIndex(argDecl);
        if (paramIndex >= 0)
            return paramIndex;

        // Witness arguments are only meaningful for source generic constraints
        // owned by a generic declaration.
        auto genericDecl = as<GenericDecl>(argDecl->parentDecl);
        if (!genericDecl || !isGenericConstraintDecl(argDecl))
            return -1;

        // Count ordinary arguments first because generic applications serialize
        // all type/value parameters before any witness arguments.
        Index argIndex = 0;
        for (auto member : genericDecl->getDirectMemberDecls())
        {
            if (as<GenericTypeParamDeclBase>(member) || as<GenericValueParamDecl>(member) ||
                as<GenericValuePackParamDecl>(member))
            {
                argIndex++;
            }
        }

        // Then scan source generic constraints in declaration order. The target
        // constraint's position after the ordinary prefix is its witness
        // argument index.
        for (auto member : genericDecl->getDirectMemberDecls())
        {
            if (!isGenericConstraintDecl(member))
                continue;
            if (member == argDecl)
                return argIndex;
            argIndex++;
        }

        return -1;
    }

    // Return true if the final argument arrays contain all required arguments.
    bool areFinalArgsValid()
    {
        for (auto genericDecl : m_genericDecls)
        {
            // Every generic declaration in the specialization chain must have an
            // argument array. Missing an array means collection or initialization
            // skipped part of a nested application such as `Outer<T>.Inner<U>`.
            if (!m_args.containsKey(genericDecl))
                return false;

            // Ordinary arguments are valid when they are ready. Pack parameters
            // are allowed to be empty, but they still need an actual pack value
            // so serialization does not leave a null entry in the final decl-ref.
            for (auto member : genericDecl->getDirectMemberDecls())
            {
                if (as<GenericTypePackParamDecl>(member) || as<GenericValuePackParamDecl>(member))
                {
                    if (!getCurrentArg(member))
                        return false;
                    continue;
                }

                if (as<GenericTypeParamDeclBase>(member) || as<GenericValueParamDecl>(member))
                {
                    if (isArgReady(member))
                        continue;
                    return false;
                }
            }

            // Witness arguments are compiler-formed proofs for source generic
            // constraints. A remaining default substitution arg here would mean
            // the final generic application still carries the declaration's
            // initial witness value rather than a proof built for this
            // specialization.
            for (auto member : genericDecl->getDirectMemberDecls())
            {
                if (!isGenericConstraintDecl(member))
                    continue;
                if (!isArgReady(member))
                    return false;
            }
        }
        return true;
    }

    // Serialize the current argument for a type-pack parameter.
    Val* getTypePackParamArg(GenericTypeParamDeclBase* typeParam)
    {
        // If no inference has populated the pack, the generic application uses
        // the empty-pack convention. A separate non-empty-pack witness
        // constraint can still reject the candidate if an empty pack is illegal.
        auto argInfo = m_argInfo.tryGetValue(typeParam);
        if (!argInfo)
            return m_astBuilder->getTypePack(ArrayView<Type*>());

        // Type-pack constraints can arrive by element. Until every element up
        // to the highest constrained index is known, the pack cannot be
        // serialized without inventing missing types.
        auto& types = argInfo->getTypeConstraints();
        for (auto t : types)
        {
            if (!t)
                return nullptr;
        }

        // If inference forwarded an existing type pack, preserve that pack
        // value rather than wrapping it in a one-element concrete pack.
        if (types.getCount() == 1 && isTypePack(types[0]))
            return types[0].type;

        // Otherwise rebuild a concrete type pack from the per-element types.
        ShortList<Type*> typeList;
        for (auto t : types)
            typeList.add(t.type);
        return m_astBuilder->getTypePack(typeList.getArrayView().arrayView);
    }

    // -------------------------------------------------------------------------
    // Decl-ref construction
    //
    // These routines rebuild specialized decl-refs from the live argument list.
    // Defaults and witness checks use them to substitute source declarations
    // through exactly the same arguments that the final generic application will
    // serialize.

    // Build a substituted decl-ref using the current argument arrays.
    DeclRef<Decl> buildSubstDeclRef(Decl* memberDecl)
    {
        // The target member decides where the nested substitution chain stops.
        // Passing `inner` builds the final solved generic body, while passing a
        // source generic constraint builds the specialized declaration used to
        // compute that constraint's witness argument.
        auto memberGenericDecl = as<GenericDecl>(memberDecl->parentDecl);
        DeclRef<Decl> substDeclRef;

        // Rebuild nested applications from outermost to innermost. This mirrors
        // how `Outer<T>.Inner<U>` is named: the decl-ref for `Inner` must be
        // based on the already-specialized decl-ref for `Outer`.
        for (auto genericDecl : m_genericDecls)
        {
            DeclRef<GenericDecl> genericDeclRef;
            if (substDeclRef)
            {
                genericDeclRef = substDeclRef.as<GenericDecl>();
                if (!genericDeclRef)
                    genericDeclRef = m_astBuilder->getMemberDeclRef(substDeclRef, genericDecl);
            }
            else
            {
                genericDeclRef = getRootGenericDeclRef(genericDecl);
            }
            if (!genericDeclRef)
                return DeclRef<Decl>();

            // `m_args` is the live argument list. Because solver constraints update it
            // immediately, this substitution always reflects the current state
            // of ordinary and witness solving.
            substDeclRef = m_astBuilder->getGenericAppDeclRef(
                genericDeclRef,
                m_args[genericDecl].getArrayView().arrayView,
                genericDecl == memberGenericDecl ? memberDecl : genericDecl->inner);

            // Stop once the requested member has been reached. Outer generics
            // before it are needed for context; inner generics after it would be
            // unrelated to this particular decl-ref.
            if (genericDecl == memberGenericDecl)
                break;
        }
        return substDeclRef;
    }

    // Return the root decl-ref for a substituted generic application.
    DeclRef<GenericDecl> getRootGenericDeclRef(GenericDecl* genericDecl)
    {
        // Direct references can be rebuilt directly for each generic in the
        // chain. Non-direct references, such as lookup/member references, carry
        // access-path information that must be preserved when specializing.
        if (genericDecl == m_genericDeclRef.getDecl())
            return m_genericDeclRef;
        for (auto declRef = DeclRef<Decl>(m_genericDeclRef); declRef; declRef = declRef.getParent())
        {
            if (auto genericAppDeclRef = as<GenericAppDeclRef>(declRef.declRefBase))
            {
                if (genericAppDeclRef->getGenericDecl() == genericDecl)
                    return DeclRef<GenericDecl>(genericAppDeclRef->getGenericDeclRef());
            }
            if (declRef.getDecl() == genericDecl)
                return declRef.as<GenericDecl>();
        }
        return makeDeclRef(genericDecl);
    }

    // -------------------------------------------------------------------------
    // Witness construction
    //
    // These routines delegate the actual proof search to the existing semantic
    // services. The solver decides when a proof is ready to be requested and
    // where the resulting witness argument is stored.

    // Try to solve the witness for one source generic constraint.
    Val* trySolveWitnessForConstraint(GenericDecl* genericDecl, Decl* constraintDecl)
    {
        // The source declaration kind determines which existing semantic rule is
        // authoritative. The solver only coordinates when the proof is needed
        // and lets the caller store the solved witness as the generic argument.
        if (auto genericTypeConstraintDecl = as<GenericTypeConstraintDecl>(constraintDecl))
            return trySolveSubtypeWitnessForConstraint(genericTypeConstraintDecl);
        if (auto typeCoercionConstraintDecl = as<TypeCoercionConstraintDecl>(constraintDecl))
            return trySolveTypeCoercionWitnessForConstraint(
                genericDecl,
                typeCoercionConstraintDecl);
        if (auto nonEmptyConstraintDecl = as<NonEmptyPackConstraintDecl>(constraintDecl))
            return trySolveNonEmptyPackWitnessForConstraint(genericDecl, nonEmptyConstraintDecl);
        if (auto hasDiffTypeInfoConstraintDecl = as<HasDiffTypeInfoConstraintDecl>(constraintDecl))
            return trySolveDiffTypeInfoWitnessForConstraint(
                genericDecl,
                hasDiffTypeInfoConstraintDecl);

        // Unknown constraint declarations do not have a witness proof to solve,
        // so they cannot satisfy a witness constraint.
        return nullptr;
    }

    // Try to solve the witness for a subtype or equality constraint.
    Val* trySolveSubtypeWitnessForConstraint(GenericTypeConstraintDecl* constraintDecl)
    {
        // First substitute the constraint declaration through the current
        // argument list. This turns a declaration like `T : IFoo<U>` into the
        // concrete or partially solved `sub` and `sup` that the subtype checker
        // should reason about.
        auto constraintDeclRef = buildSubstDeclRef(constraintDecl).as<GenericTypeConstraintDecl>();
        auto sub = getSub(m_astBuilder, constraintDeclRef);
        auto sup = getSup(m_astBuilder, constraintDeclRef);

        // The raw declaration also matters for overload ranking: `T : IFoo`
        // makes this candidate more specific even if `T` has already been
        // substituted to `float` by the time the witness is computed.
        if (!markArgConstrainedBySubtypeConstraint(constraintDecl))
            return nullptr;

        // Requirements declared inside an interface are checked while the
        // interface's abstract self type is still in scope. For example,
        // `interface INode { associatedtype Next : INode; }` can ask the solver
        // about a conformance whose substituted source and target both denote
        // `INode` before any concrete implementation exists. That equality
        // means the requirement is still abstract; it is not a witness-table
        // proof that a concrete type conforms to the interface, so the solver
        // must leave the witness unsolved here instead of capturing one from the
        // interface declaration's own generic context.
        if (sub->equals(sup) && isDeclRefTypeOf<InterfaceDecl>(sup))
            return nullptr;

        // Subtype witness construction belongs to `isSubtype()`. That path owns
        // facet linearization, generic-parameter conformances, associated-type
        // constraints, concrete conformances, and cached answers; the solver
        // only supplies the fully substituted `sub` and `sup` for this witness
        // argument.
        SubtypeWitness* subTypeWitness = nullptr;
        // Additional subtype witnesses are local facts collected while a type's
        // extension closure is being built. They let extension application use
        // bases already discovered for the subject type without re-entering the
        // inheritance query for that same subject.
        //
        // Interface declarations are the canonical source for their own
        // inheritance graph, so an interface subject should be answered by
        // `isSubtype()` instead. Otherwise an extension like
        // `extension<T : IBar> T : IFoo` can use the provisional `IFoo : IBar`
        // fact to apply the extension while `IFoo` itself is still being
        // linearized, manufacturing `IFoo` as one of its own bases.
        if (m_context.additionalSubtypeWitnesses &&
            sub == m_context.subTypeForAdditionalWitnesses && !isDeclRefTypeOf<InterfaceDecl>(sub))
        {
            m_context.additionalSubtypeWitnesses->tryGetValue(sup, subTypeWitness);
        }
        else
        {
            subTypeWitness = m_visitor->isSubtype(
                sub,
                sup,
                m_context.additionalSubtypeWitnesses ? IsSubTypeOptions::NoCaching
                                                     : IsSubTypeOptions::None);
        }

        if (constraintDecl->isEqualityConstraint && !isTypeEqualityWitness(subTypeWitness))
            subTypeWitness = nullptr;

        // Optionality can come from either the witness found by subtype checking
        // or the source constraint itself. A real unchecked-optional witness is
        // accepted only when the source constraint is optional; otherwise a
        // required constraint must fail instead of silently carrying a weak
        // proof.
        bool witnessIsOptional = m_visitor->isWitnessUncheckedOptional(subTypeWitness);
        bool constraintIsOptional = constraintDecl->hasModifier<OptionalConstraintModifier>();

        // A concrete witness, or an optional witness for an optional
        // constraint, solves this source constraint.
        if (subTypeWitness && (!witnessIsOptional || constraintIsOptional))
            return subTypeWitness;

        // Optional source constraints can use `NoneWitness` when no proof
        // exists. Overload ranking later charges this as a failed optional
        // constraint instead of rejecting the candidate.
        if (!subTypeWitness && constraintIsOptional)
            return m_astBuilder->getOrCreate<NoneWitness>();

        // A required subtype constraint with no acceptable proof rejects the
        // generic application.
        return nullptr;
    }

    // Mark the argument corresponding to `constraintDecl` as constrained by
    // a subtype constraint in `m_argInfo`.
    bool markArgConstrainedBySubtypeConstraint(GenericTypeConstraintDecl* constraintDecl)
    {
        // Both sides of a subtype/equality constraint can restrict ordinary
        // arguments. `T.A : IBar<U>` constrains `T` through the associated-type
        // subject and `U` through the interface target, so mark dependencies
        // from both declaration-time values.
        return markArgConstrainedByTypeVal(constraintDecl->sub.type) &&
               markArgConstrainedByTypeVal(constraintDecl->sup.type);
    }

    // Mark every ordinary argument mentioned by `typeVal` as constrained in `m_argInfo`.
    bool markArgConstrainedByTypeVal(Val* typeVal)
    {
        // The specificity rule is about source generic structure, not the final
        // substituted type. A direct subject like `T : IFoo`, a nested subject
        // like `Array<T> : IFoo`, and an associated-type subject like
        // `T.A : IBar` should all mark the ordinary argument for `T`. The same
        // cached dependency walk used by readiness checks already sees through
        // these shapes, so reuse it instead of recognizing only direct
        // parameter references.
        for (auto dependencyDecl : getDependentDeclsForVal(typeVal))
        {
            // Witness dependencies can also appear in values such as `T.A`, but
            // this overload-ranking bit is for ordinary generic arguments. The
            // final cost pass only charges ordinary type parameters, so value
            // parameters are harmless here and keep the metadata faithful to the
            // argument graph.
            if (as<GenericTypeParamDeclBase>(dependencyDecl) || isGenericValueParam(dependencyDecl))
                markArgConstrainedForOverloadRanking(dependencyDecl);
        }
        return true;
    }

    // Mark the arguments corresponding to `constraintDecl` as constrained by a
    // type-coercion constraint in `m_argInfo`.
    bool markArgConstrainedByTypeCoercionConstraint(TypeCoercionConstraintDecl* constraintDecl)
    {
        // A coercion constraint can constrain an argument on either side of the
        // conversion, such as `T -> float` or `float -> T`. Mark both
        // declaration-time sides before solving the witness.
        return markArgConstrainedByTypeVal(constraintDecl->fromType.type) &&
               markArgConstrainedByTypeVal(constraintDecl->toType.type);
    }

    // Mark the argument corresponding to `constraintDecl` as constrained by a
    // differentiability constraint in `m_argInfo`.
    bool markArgConstrainedByDiffTypeInfoConstraint(HasDiffTypeInfoConstraintDecl* constraintDecl)
    {
        // Like subtype constraints, differentiability constraints are ranked by
        // the declaration-time type. `where T has diff type info` marks the
        // argument for `T` even if `T` has already been solved to a concrete
        // type.
        return markArgConstrainedByTypeVal(constraintDecl->type.type);
    }

    // Mark the argument corresponding to `ordinaryParamDecl` as constrained for
    // overload ranking in `m_argInfo`.
    void markArgConstrainedForOverloadRanking(Decl* ordinaryParamDecl)
    {
        // Some constraints may not be rooted in a generic parameter, so a null
        // declaration simply means there is no argument to mark.
        if (!ordinaryParamDecl)
            return;

        // This is metadata about an ordinary argument rather than the argument
        // value itself, so it lives beside the state in `m_argInfo`.
        m_argInfo[ordinaryParamDecl].markConstrainedForOverloadRanking();
    }

    // Return true if the argument corresponding to `ordinaryParamDecl` is marked
    // as constrained for overload ranking in `m_argInfo`.
    bool isArgConstrainedForOverloadRanking(Decl* ordinaryParamDecl)
    {
        // Missing metadata means no source generic constraint marked the
        // argument, so the old unconstrained-parameter penalty still applies.
        auto argInfo = m_argInfo.tryGetValue(ordinaryParamDecl);
        return argInfo && argInfo->isConstrainedForOverloadRanking();
    }

    // Try to solve the witness for a type-coercion constraint.
    Val* trySolveTypeCoercionWitnessForConstraint(
        GenericDecl* genericDecl,
        TypeCoercionConstraintDecl* constraintDecl)
    {
        SLANG_UNUSED(genericDecl);

        // Mark source-level constrained arguments before substitution changes
        // them. A constraint such as `T -> float` should still make the
        // argument for `T` count as constrained for overload ranking.
        if (!markArgConstrainedByTypeCoercionConstraint(constraintDecl))
            return nullptr;

        // Reuse the shared coercion helper so the solver does not duplicate
        // conversion logic. It receives the current full argument list, so the
        // witness is proved for the same specialization that will be returned.
        auto constraintDeclRef = buildSubstDeclRef(constraintDecl).as<TypeCoercionConstraintDecl>();
        auto witness = findTypeCoercionWitnessForSubstitutedConstraint(
            m_astBuilder,
            m_visitor,
            constraintDeclRef,
            nullptr,
            nullptr,
            false);
        return witness;
    }

    // Try to solve the witness for a non-empty-pack constraint.
    Val* trySolveNonEmptyPackWitnessForConstraint(
        GenericDecl* genericDecl,
        NonEmptyPackConstraintDecl* constraintDecl)
    {
        SLANG_UNUSED(genericDecl);

        // The constraint stores its subject pack as syntax, so first recover
        // the decl-ref referenced by that expression. Keeping a `DeclRef` here
        // preserves any useful substitution context while deciding whether this
        // is a type pack or value pack.
        auto constrainedPackDeclRef = getPackDeclRefForNonEmptyConstraint(constraintDecl);

        // Read the current pack argument from `m_args`. Type and value packs
        // share the ordinary-argument prefix but use their own declaration
        // classes. Once the decl-ref has been classified, the raw declaration is
        // the stable key/index owner for the current solver argument list.
        Val* constrainedArg = nullptr;
        if (auto typePackDeclRef = constrainedPackDeclRef.as<GenericTypePackParamDecl>())
        {
            constrainedArg = getCurrentArg(typePackDeclRef.getDecl());
        }
        else if (auto valuePackDeclRef = constrainedPackDeclRef.as<GenericValuePackParamDecl>())
        {
            constrainedArg = getCurrentArg(valuePackDeclRef.getDecl());
        }

        // Reuse the same proof helper as overload validation. The solver passes
        // no diagnostic context because a failed witness simply rejects this
        // candidate during speculative solving.
        return findNonEmptyPackWitnessForConstraint(
            m_astBuilder,
            m_visitor,
            constrainedArg,
            nullptr,
            false);
    }

    // Try to solve the witness for a differentiability constraint.
    Val* trySolveDiffTypeInfoWitnessForConstraint(
        GenericDecl* genericDecl,
        HasDiffTypeInfoConstraintDecl* constraintDecl)
    {
        SLANG_UNUSED(genericDecl);

        // Mark the declaration-time subject argument for overload ranking
        // before the witness helper substitutes it to a concrete type.
        if (!markArgConstrainedByDiffTypeInfoConstraint(constraintDecl))
            return nullptr;

        // Use the shared diff-type-info lookup so the solver does not duplicate
        // differentiability rules. The current argument list provides the
        // specialization under which the witness must be valid.
        auto constraintDeclRef =
            buildSubstDeclRef(constraintDecl).as<HasDiffTypeInfoConstraintDecl>();
        auto witness = findDiffTypeInfoWitnessForSubstitutedConstraint(
            m_astBuilder,
            m_visitor,
            constraintDeclRef,
            nullptr,
            false);
        return witness;
    }

    // -------------------------------------------------------------------------
    // Final validation and cost
    //
    // These routines run after the work list is quiet. They reject any remaining
    // unsatisfied work and compute overload cost once from the final argument
    // list, avoiding double-counting from retried constraints.

    // Return true if all ordinary solver constraints were consumed.
    bool areOrdinaryConstraintsSatisfied()
    {
        // Witness work is checked by final argument validation. This pass only
        // guards against ordinary constraints that never reached a terminal
        // state, such as an unsolved `U = T.A` whose dependencies never became
        // ready.
        for (auto const& constraint : m_solverConstraints)
        {
            if (!isOrdinarySolverConstraint(constraint))
                continue;
            if (!constraint.satisfied)
                return false;
        }
        return true;
    }

    // Add overload cost for unconstrained type parameters.
    void addUnconstrainedGenericParamCost()
    {
        // Preserve the previous ranking rule: a candidate with `T : IFoo` is
        // more specific than one with unconstrained `T`. The marking step uses
        // source generic constraints, so this final pass only needs to charge
        // type parameters that were never marked.
        for (auto genericDecl : m_genericDecls)
        {
            for (auto typeParamDecl : genericDecl->getMembersOfType<GenericTypeParamDecl>())
            {
                if (!isArgConstrainedForOverloadRanking(typeParamDecl))
                    m_outBaseCost += kConversionCost_UnconstraintGenericParam;
            }
        }
    }

    // Compute overload cost from final witness arguments.
    ConversionCost computeFinalWitnessCost()
    {
        // Witness constraints may be retried after unrelated progress because
        // the solver wakes conservatively. Cost is therefore accumulated once,
        // after the loop is quiet, from the final witness values stored in
        // `m_args`.
        ConversionCost cost = kConversionCost_None;
        for (auto const& constraint : m_solverConstraints)
        {
            // Only subtype witnesses participate in overload ranking here.
            // Coercion and diff-info constraints have their own costs accounted
            // through the shared checking paths that produced their witnesses.
            if (constraint.kind != SolverConstraint::Kind::WitnessConstraint)
                continue;
            if (!as<GenericTypeConstraintDecl>(constraint.decl))
                continue;

            // Optional constraints that failed contribute the optional-failure
            // cost. Concrete subtype witnesses contribute the cost determined by
            // the subtype checker, such as the number of interface upcasts.
            auto witnessArg = getSolvedWitnessArg(constraint.decl);
            if (as<NoneWitness>(witnessArg))
                cost += kConversionCost_FailedOptionalConstraint;
            else if (auto subTypeWitness = as<SubtypeWitness>(witnessArg))
                cost += subTypeWitness->getOverloadResolutionCost();
        }
        return cost;
    }

private:
    // -------------------------------------------------------------------------
    // Stored solver state
    //
    // The members below mirror the problem model above: shared semantic context,
    // the generic declaration chain being specialized, one live argument list per
    // generic, side metadata for those arguments, and the durable work table plus
    // queue used by the iterative solver.

    // Semantic visitor used for all checker operations that need broader
    // context: unification, subtype checks, default lookup, diagnostics, and
    // access to shared semantic caches.
    SemanticsVisitor* m_visitor = nullptr;

    // AST builder paired with `m_visitor`. The solver creates substituted
    // decl-refs, witness values, packs, and canonical values through this
    // builder so all generated values belong to the active AST arena.
    ASTBuilder* m_astBuilder = nullptr;

    // Shared unification context owned by the solver while it runs. Its
    // `discoveredConstraints` list is only a transient inbox for helpers that
    // discover ordinary constraints before or during the work-list loop.
    GenericInferenceContext m_context;

    // Decl-ref for the generic being specialized. The solver preserves this
    // lookup path when it forms substituted decl-refs for the generic body or
    // for source generic constraint declarations.
    DeclRef<GenericDecl> m_genericDeclRef;

    // Caller-provided ordinary arguments, such as `int` in `foo<int>(x)`.
    // These are installed into `m_args` after initialization and before the
    // solver work-list starts.
    ArrayView<Val*> m_providedOrdinaryArgs;

    // Output overload cost owned by the caller. The solver writes the final
    // accumulated cost after all ordinary arguments and witnesses are solved.
    ConversionCost& m_outBaseCost;

    // Nested generic declarations from outermost to innermost. A substituted
    // decl-ref for `Outer<T>.Inner<U>` consumes arguments for `Outer` before it
    // can name `Inner`, so the solver stores the declarations in that order.
    ShortList<GenericDecl*, 4> m_genericDecls;

    // Current generic-application arguments keyed by generic declaration. This
    // is the declaration-order view consumed by `getGenericAppDeclRef`: ordinary
    // arguments first, then witness arguments. The arrays begin with default
    // substitution args and solver constraints replace arguments as they solve.
    Dictionary<Decl*, ShortList<Val*>> m_args;

    // Argument metadata keyed by the declaration that owns the corresponding
    // ordinary or witness argument. `m_args` stores the current argument values
    // used by substitution.
    Dictionary<Decl*, ArgInfo> m_argInfo;

    // Stable storage for all collected solver constraints: ordinary constraints,
    // default generic arguments, and witness constraints. The queue stores
    // indices into this table.
    ShortList<SolverConstraint, 16> m_solverConstraints;

    // Append-only queue of solver-constraint indices. New and reawakened
    // constraints are added at the end, and `m_workListReadIndex` advances
    // through this list.
    ShortList<Index, 16> m_workList;

    // Cursor into `m_workList` for the next queued constraint to try.
    Index m_workListReadIndex = 0;
};

// Solve generic arguments for overload resolution.
DeclRef<Decl> SemanticsVisitor::trySolveGenericArguments(
    GenericInferenceContext&& inferenceContext,
    DeclRef<GenericDecl> genericDeclRef,
    ArrayView<Val*> providedOrdinaryArgs,
    ConversionCost& outBaseCost)
{
    // The solver reads generic members while collecting ordinary constraints,
    // default generic arguments, and witness constraints. The generic
    // declaration must therefore be ready for lookup before the solver starts
    // walking direct members like `T`, `U = T.A`, and `T : IFoo`.
    ensureDecl(genericDeclRef.getDecl(), DeclCheckState::ReadyForLookup);

    // Move the accumulated inference context into the solver. Solving mutates
    // ordinary constraints, discovers follow-up constraints through unification,
    // and updates overload cost, so ownership is intentionally transferred at
    // the call boundary.
    GenericArgumentSolver
        solver(this, _Move(inferenceContext), genericDeclRef, providedOrdinaryArgs, outBaseCost);

    // The caller only needs the final substituted inner decl-ref and the output
    // cost. All work-list state, current arguments, and witness arguments remain
    // encapsulated in the solver object.
    return solver.solve();
}

bool SemanticsVisitor::TryUnifyVals(
    GenericInferenceContext& constraints,
    UnificationOptions unificationOptions,
    Val* fst,
    bool fstLVal,
    Val* snd,
    bool sndLVal)
{
    // if both values are types, then unify types
    if (auto fstType = as<Type>(fst))
    {
        if (auto sndType = as<Type>(snd))
        {
            return TryUnifyTypes(
                constraints,
                unificationOptions,
                QualType(fstType, fstLVal),
                QualType(sndType, sndLVal));
        }
    }

    // if both values are constant integers, then compare them
    if (auto fstIntVal = as<ConstantIntVal>(fst))
    {
        if (auto sndIntVal = as<ConstantIntVal>(snd))
        {
            return fstIntVal->getValue() == sndIntVal->getValue();
        }
    }

    // Check if both are integer values in general
    const auto fstInt = as<IntVal>(fst);
    const auto sndInt = as<IntVal>(snd);
    if (fstInt && sndInt)
    {
        auto fstParam = getDeclRefIntValIgnoringCasts(fstInt);
        auto sndParam = getDeclRefIntValIgnoringCasts(sndInt);

        bool okay = false;
        if (fstParam)
            okay |=
                TryUnifyIntParam(constraints, unificationOptions, fstParam->getDeclRef(), sndInt);
        if (sndParam)
            okay |=
                TryUnifyIntParam(constraints, unificationOptions, sndParam->getDeclRef(), fstInt);
        return okay;
    }

    if (auto fstWit = as<DeclaredSubtypeWitness>(fst))
    {
        if (auto sndWit = as<DeclaredSubtypeWitness>(snd))
        {
            auto constraintDecl1 = fstWit->getDeclRef().as<TypeConstraintDecl>();
            auto constraintDecl2 = sndWit->getDeclRef().as<TypeConstraintDecl>();
            SLANG_ASSERT(constraintDecl1);
            SLANG_ASSERT(constraintDecl2);
            return TryUnifyTypes(
                constraints,
                unificationOptions,
                getSup(m_astBuilder, constraintDecl1),
                getSup(m_astBuilder, constraintDecl2));
        }
    }

    if (auto fstWit = as<TypeCoercionWitness>(fst); fstWit)
    {
        if (auto sndWit = as<TypeCoercionWitness>(snd); sndWit)
        {
            // Coercion-witness comparison is an inference hint only. The
            // generic solver later substitutes the final source/destination
            // types and asks the coercion checker to build the real witness, so
            // this path must not reject a candidate just because the provisional
            // witness values do not expose useful ordinary-argument facts.
            return true;
        }
    }

    if (as<TypeEqualityWitness>(fst) && as<DeclaredSubtypeWitness>(snd))
    {
        if (as<DeclaredSubtypeWitness>(snd)->isEquality())
        {
            // Equality-witness comparison can discover ordinary-argument hints
            // from the two sides of the equality proof. Failure is ignored here
            // because this is not the proof step: associated-type projections
            // may be too dependent to unify as hints, while the final
            // substituted equality witness is still validated by the generic
            // solver.
            TryUnifyTypes(
                constraints,
                unificationOptions,
                as<SubtypeWitness>(snd)->getSub(),
                as<SubtypeWitness>(fst)->getSub());
            TryUnifyTypes(
                constraints,
                unificationOptions,
                as<SubtypeWitness>(snd)->getSup(),
                as<SubtypeWitness>(fst)->getSup());
            return true;
        }
    }

    if (as<DeclaredSubtypeWitness>(fst) && as<TypeEqualityWitness>(snd))
    {
        if (as<DeclaredSubtypeWitness>(fst)->isEquality())
        {
            // Equality-witness comparison can discover ordinary-argument hints
            // from the two sides of the equality proof. Failure is ignored here
            // because this is not the proof step: associated-type projections
            // may be too dependent to unify as hints, while the final
            // substituted equality witness is still validated by the generic
            // solver.
            TryUnifyTypes(
                constraints,
                unificationOptions,
                as<SubtypeWitness>(snd)->getSub(),
                as<SubtypeWitness>(fst)->getSub());
            TryUnifyTypes(
                constraints,
                unificationOptions,
                as<SubtypeWitness>(snd)->getSup(),
                as<SubtypeWitness>(fst)->getSup());
            return true;
        }
    }

    // Two subtype witnesses can be unified if they exist (non-null) and
    // prove that some pair of types are subtypes of types that can be unified.
    //
    const auto fstSubtypeWitness = as<SubtypeWitness>(fst);
    const auto sndSubtypeWitness = as<SubtypeWitness>(snd);
    const auto fstNoneWitness = as<NoneWitness>(fst);
    const auto sndNoneWitness = as<NoneWitness>(snd);
    if (fstSubtypeWitness && sndSubtypeWitness)
        return TryUnifyTypes(
            constraints,
            unificationOptions,
            fstSubtypeWitness->getSup(),
            sndSubtypeWitness->getSup());
    else if (fstNoneWitness && sndNoneWitness)
        return true;
    else if ((fstNoneWitness && sndSubtypeWitness) || (fstSubtypeWitness && sndNoneWitness))
        return false;

    // default: fail
    return false;
}

bool SemanticsVisitor::tryUnifyDeclRef(
    GenericInferenceContext& constraints,
    UnificationOptions unificationOptions,
    DeclRefBase* fst,
    bool fstIsLVal,
    DeclRefBase* snd,
    bool sndIsLVal)
{
    if (fst == snd)
        return true;
    if (fst == nullptr || snd == nullptr)
        return false;
    auto fstGen = SubstitutionSet(fst).findGenericAppDeclRef();
    auto sndGen = SubstitutionSet(snd).findGenericAppDeclRef();
    if (fstGen == sndGen)
        return true;
    if (fstGen == nullptr || sndGen == nullptr)
        return false;
    return tryUnifyGenericAppDeclRef(
        constraints,
        unificationOptions,
        fstGen,
        fstIsLVal,
        sndGen,
        sndIsLVal);
}

bool SemanticsVisitor::tryUnifyGenericAppDeclRef(
    GenericInferenceContext& constraints,
    UnificationOptions unificationOptions,
    GenericAppDeclRef* fst,
    bool fstIsLVal,
    GenericAppDeclRef* snd,
    bool sndIsLVal)
{
    SLANG_ASSERT(fst);
    SLANG_ASSERT(snd);

    auto fstGen = fst;
    auto sndGen = snd;
    // They must be specializing the same generic
    if (fstGen->getGenericDecl() != sndGen->getGenericDecl())
        return false;

    // Their arguments must unify
    SLANG_RELEASE_ASSERT(fstGen->getArgs().getCount() == sndGen->getArgs().getCount());
    Index argCount = fstGen->getArgs().getCount();
    bool okay = true;
    for (Index aa = 0; aa < argCount; ++aa)
    {
        if (!TryUnifyVals(
                constraints,
                unificationOptions,
                fstGen->getArgs()[aa],
                fstIsLVal,
                sndGen->getArgs()[aa],
                sndIsLVal))
        {
            okay = false;
        }
    }

    // Their "base" specializations must unify
    auto fstBase = fst->getBase();
    auto sndBase = snd->getBase();

    if (!tryUnifyDeclRef(constraints, unificationOptions, fstBase, fstIsLVal, sndBase, sndIsLVal))
    {
        okay = false;
    }

    return okay;
}

bool SemanticsVisitor::TryUnifyTypeParam(
    GenericInferenceContext& constraints,
    UnificationOptions unificationOptions,
    GenericTypeParamDeclBase* typeParamDecl,
    QualType type)
{
    // Record one ordinary type-argument fact discovered by unification. Equality
    // unification records an exact answer; ordinary call inference records a
    // type-join fact that may merge with other argument/parameter facts later.
    auto priority = unificationOptions.optionalConstraint ? ConstraintPriority::Optional
                                                          : ConstraintPriority::Required;
    constraints.discoveredConstraints.add(SolverConstraint::makeOrdinaryArg(
        typeParamDecl,
        type,
        priority,
        getOrdinaryArgMergeMode(unificationOptions),
        unificationOptions.indexInTypePack,
        type.isLeftValue));

    return true;
}

bool SemanticsVisitor::TryUnifyIntParam(
    GenericInferenceContext& constraints,
    UnificationOptions unificationOptions,
    GenericValueParamDecl* paramDecl,
    IntVal* val)
{
    // Only value parameters from the generic application currently being solved
    // should receive solver constraints. `isRelevantGeneric()` accepts the whole
    // nested generic chain, so an inner generic can still infer an outer
    // parameter when both declarations participate in the same application.
    if (!isRelevantGeneric(constraints, paramDecl->parentDecl))
        return false;

    // Value arguments are exact answers. Even if value unification is reached
    // while a witness is doing shape inference, the discovered fact is still
    // `N = 4`, not something that should use type-join merging.
    auto priority = unificationOptions.optionalConstraint ? ConstraintPriority::Optional
                                                          : ConstraintPriority::Required;
    // If `val` is of different type than `paramDecl`, we want to insert a type cast.
    if (val->getType() != paramDecl->getType())
    {
        auto cast = m_astBuilder->getTypeCastIntVal(paramDecl->getType(), val);
        val = cast;
    }

    constraints.discoveredConstraints.add(SolverConstraint::makeOrdinaryArg(
        paramDecl,
        val,
        priority,
        SolverConstraint::OrdinaryArgMergeMode::Exact));

    return true;
}

bool SemanticsVisitor::TryUnifyIntParam(
    GenericInferenceContext& constraints,
    UnificationOptions unificationOptions,
    DeclRef<VarDeclBase> const& varRef,
    IntVal* val)
{
    if (auto genericValueParamRef = varRef.as<GenericValueParamDecl>())
    {
        return TryUnifyIntParam(
            constraints,
            unificationOptions,
            genericValueParamRef.getDecl(),
            val);
    }
    else if (auto genericValuePackParamRef = varRef.as<GenericValuePackParamDecl>())
    {
        if (!isRelevantGeneric(constraints, genericValuePackParamRef.getDecl()->parentDecl))
            return false;
        auto priority = unificationOptions.optionalConstraint ? ConstraintPriority::Optional
                                                              : ConstraintPriority::Required;
        constraints.discoveredConstraints.add(SolverConstraint::makeOrdinaryArg(
            genericValuePackParamRef.getDecl(),
            val,
            priority,
            SolverConstraint::OrdinaryArgMergeMode::Exact));
        return true;
    }
    else
    {
        return false;
    }
}

bool SemanticsVisitor::TryUnifyFunctorByStructuralMatch(
    GenericInferenceContext& constraints,
    UnificationOptions unificationOptions,
    StructDecl* fstStructDecl,
    FuncType* sndFuncType)
{
    // Use the functor's invocation signature as an inference hint. This helper
    // does not validate that the functor is callable in the final program; later
    // constraint and overload checks still have to prove that.
    FuncDecl* functorInvokeMethod =
        as<FuncDecl>(fstStructDecl->findLastDirectMemberDeclOfName(getName("()")));
    if (!functorInvokeMethod)
        return false;

    return TryUnifyFuncTypesByStructuralMatch(
        constraints,
        unificationOptions,
        getFuncType(this->getASTBuilder(), functorInvokeMethod),
        sndFuncType);
}

bool SemanticsVisitor::TryUnifyFuncTypesByStructuralMatch(
    GenericInferenceContext& constraints,
    UnificationOptions unificationOptions,
    FuncType* fstFunType,
    FuncType* sndFunType)
{
    const Index numParams = fstFunType->getParamCount();
    if (numParams != sndFunType->getParamCount())
        return false;
    for (Index i = 0; i < numParams; ++i)
    {
        if (!TryUnifyTypes(
                constraints,
                unificationOptions,
                fstFunType->getParamTypeWithModeWrapper(i),
                sndFunType->getParamTypeWithModeWrapper(i)))
            return false;
    }
    return TryUnifyTypes(
        constraints,
        unificationOptions,
        fstFunType->getResultType(),
        sndFunType->getResultType());
}

bool SemanticsVisitor::TryUnifyTypesByStructuralMatch(
    GenericInferenceContext& constraints,
    UnificationOptions unificationOptions,
    QualType fst,
    QualType snd)
{
    if (auto sndDeclRefType = as<DeclRefType>(snd))
    {
        auto sndDeclRef = sndDeclRefType->getDeclRef();

        if (auto sndStructDecl = as<StructDecl>(sndDeclRef))
        {
            if (auto fstFunType = as<FuncType>(fst))
                return TryUnifyFunctorByStructuralMatch(
                    constraints,
                    unificationOptions,
                    sndStructDecl.getDecl(),
                    fstFunType);
        }
    }

    if (auto fstDeclRefType = as<DeclRefType>(fst))
    {
        auto fstDeclRef = fstDeclRefType->getDeclRef();

        if (auto fstStructDecl = as<StructDecl>(fstDeclRef))
        {
            if (auto sndFunType = as<FuncType>(snd))
                return TryUnifyFunctorByStructuralMatch(
                    constraints,
                    unificationOptions,
                    fstStructDecl.getDecl(),
                    sndFunType);
        }

        if (auto typeParamDeclRef = fstDeclRef.as<GenericTypeParamDecl>())
        {
            auto typeParamDecl = typeParamDeclRef.getDecl();
            // if (typeParamDecl->parentDecl == constraints.genericDecl)
            if (isRelevantGeneric(constraints, typeParamDecl->parentDecl))
                return TryUnifyTypeParam(constraints, unificationOptions, typeParamDecl, snd);
        }

        if (auto sndDeclRefType = as<DeclRefType>(snd))
        {
            auto sndDeclRef = sndDeclRefType->getDeclRef();

            if (auto typeParamDeclRef = sndDeclRef.as<GenericTypeParamDecl>())
            {
                auto typeParamDecl = typeParamDeclRef.getDecl();
                if (isRelevantGeneric(constraints, typeParamDecl->parentDecl))
                    return TryUnifyTypeParam(constraints, unificationOptions, typeParamDecl, fst);
            }

            // If they refer to different declarations, we need to check if one type's super type
            // matches the other type, if so we can unify them.
            if (fstDeclRef.getDecl() != sndDeclRef.getDecl())
            {
                {
                    auto fstTypeInheritanceInfo = getShared()->getInheritanceInfo(fstDeclRefType);
                    for (auto supType : fstTypeInheritanceInfo.facets)
                    {
                        if (supType->origin.declRef.getDecl() == sndDeclRef.getDecl())
                        {
                            fstDeclRef = supType->origin.declRef;
                            goto endMatch;
                        }
                    }
                }
                // try the other direction
                {
                    auto sndTypeInheritanceInfo = getShared()->getInheritanceInfo(sndDeclRefType);
                    for (auto supType : sndTypeInheritanceInfo.facets)
                    {
                        if (supType->origin.declRef.getDecl() == fstDeclRef.getDecl())
                        {
                            sndDeclRef = supType->origin.declRef;
                            goto endMatch;
                        }
                    }
                }
            endMatch:;
                // If they still refer to different decls, then we can't unify them.
                if (fstDeclRef.getDecl() != sndDeclRef.getDecl())
                    return false;
            }

            // next we need to unify the substitutions applied
            // to each declaration reference.
            if (!tryUnifyDeclRef(
                    constraints,
                    unificationOptions,
                    fstDeclRef,
                    fst.isLeftValue,
                    sndDeclRef,
                    snd.isLeftValue))
            {
                return false;
            }

            return true;
        }
    }
    else if (auto fstFunType = as<FuncType>(fst))
    {
        if (auto sndFunType = as<FuncType>(snd))
        {
            return TryUnifyFuncTypesByStructuralMatch(
                constraints,
                unificationOptions,
                fstFunType,
                sndFunType);
        }
    }
    else if (auto expandType = as<ExpandType>(fst))
    {
        if (auto sndExpandType = as<ExpandType>(snd))
        {
            return TryUnifyTypes(
                constraints,
                unificationOptions,
                expandType->getPatternType(),
                sndExpandType->getPatternType());
        }
    }
    else if (auto eachType = as<EachType>(fst))
    {
        if (auto sndEachType = as<EachType>(snd))
        {
            return TryUnifyTypes(
                constraints,
                unificationOptions,
                eachType->getElementType(),
                sndEachType->getElementType());
        }
    }
    else if (auto typePack = as<ConcreteTypePack>(fst))
    {
        if (auto sndTypePack = as<ConcreteTypePack>(snd))
        {
            if (typePack->getTypeCount() != sndTypePack->getTypeCount())
                return false;
            for (Index i = 0; i < typePack->getTypeCount(); ++i)
            {
                if (!TryUnifyTypes(
                        constraints,
                        unificationOptions,
                        QualType(typePack->getElementType(i), fst.isLeftValue),
                        QualType(sndTypePack->getElementType(i), snd.isLeftValue)))
                    return false;
            }
            return true;
        }
    }
    return false;
}

bool SemanticsVisitor::TryUnifyConjunctionType(
    GenericInferenceContext& constraints,
    UnificationOptions unificationOptions,
    QualType fst,
    QualType snd)
{
    // When the conjunction is on the left, every conjunct must satisfy the
    // right-hand type: `(A & B)` against `T` records facts from both `A` and `B`.
    // When the conjunction is on the right, either conjunct can provide a viable
    // match for `T`; the later conversion/checking path validates the selected
    // candidate as usual.
    if (auto fstAndType = as<AndType>(fst))
    {
        return TryUnifyTypes(
                   constraints,
                   unificationOptions,
                   QualType(fstAndType->getLeft(), fst.isLeftValue),
                   snd) &&
               TryUnifyTypes(
                   constraints,
                   unificationOptions,
                   QualType(fstAndType->getRight(), fst.isLeftValue),
                   snd);
    }
    else if (auto sndAndType = as<AndType>(snd))
    {
        return TryUnifyTypes(
                   constraints,
                   unificationOptions,
                   fst,
                   QualType(sndAndType->getLeft(), snd.isLeftValue)) ||
               TryUnifyTypes(
                   constraints,
                   unificationOptions,
                   fst,
                   QualType(sndAndType->getRight(), snd.isLeftValue));
    }
    else
        return false;
}

bool SemanticsVisitor::tryAddOptionalIntParamConstraintIfUnconstrained(
    GenericInferenceContext& constraints,
    UnificationOptions unificationOptions,
    IntVal* param,
    IntVal* arg,
    bool paramIsLVal)
{
    SLANG_UNUSED(unificationOptions);

    // Scalar/vector unification uses this as a non-binding hint: when a vector
    // element count is an unconstrained generic integer parameter, comparing the
    // vector to a scalar suggests `N = 1`. Existing constraints win, and failure
    // to add the hint must not reject the later scalar-to-vector conversion.
    if (auto typeCastParam = as<TypeCastIntVal>(param))
    {
        param = as<IntVal>(typeCastParam->getBase());
    }
    auto intParam = as<DeclRefIntVal>(param);
    if (!intParam)
        return false;
    auto intParamDecl = intParam->getDeclRef().getDecl();

    // Unification can compare a generic being inferred against a type or value
    // that still mentions some surrounding generic declaration. Only the generic
    // currently being solved should receive new solver constraints here; an
    // unrelated integer parameter, such as the `N` from an enclosing
    // `extension vector<T, N>`, is already fixed by the decl-ref that selected
    // the extension and must not be added as a fresh argument for this solver.
    if (!isGenericValueParam(intParamDecl) ||
        !isRelevantGeneric(constraints, intParamDecl->parentDecl))
    {
        return false;
    }
    for (auto c : constraints.discoveredConstraints)
        if (c.decl == intParamDecl)
            return false;
    constraints.discoveredConstraints.add(SolverConstraint::makeOrdinaryArg(
        intParamDecl,
        arg,
        ConstraintPriority::Optional,
        SolverConstraint::OrdinaryArgMergeMode::Exact,
        0,
        paramIsLVal));
    return true;
}

// A contiguous range in one flattened type-pack sequence.
struct FlattenedTypeRange
{
    Index index;
    Index count;

    FlattenedTypeRange()
        : index(0), count(0)
    {
    }
    FlattenedTypeRange(Index idx, Index cnt)
        : index(idx), count(cnt)
    {
    }
};

// A pair of ranges that should be unified recursively after both sides of a
// pack-shaped type have been flattened.
struct FlattenedTypeRangePair
{
    FlattenedTypeRange first;
    FlattenedTypeRange second;

    FlattenedTypeRangePair() {}
    FlattenedTypeRangePair(FlattenedTypeRange f, FlattenedTypeRange s)
        : first(f), second(s)
    {
    }
};

// Convert one flattened tuple/pack mapping span back into the `QualType` used
// by recursive unification. If the opposite side is pack-shaped, a span like
// `[A, B]` must stay bundled as a type pack; otherwise the span represents one
// ordinary type.
QualType makeQualTypeForFlattenedTypeRange(
    ASTBuilder* astBuilder,
    const FlattenedTypeRange& range,
    ShortList<Type*>& typeList,
    bool isLeftValue,
    Type* otherType)
{
    // When the opposite side is a pack parameter or expansion, the mapped span
    // stays bundled so recursive unification sees a pack-shaped value.
    if (isDeclRefTypeOf<GenericTypePackParamDecl>(otherType) || as<ExpandType>(otherType))
    {
        auto typesView = makeArrayView(&typeList[range.index], range.count);
        auto typePack = astBuilder->getTypePack(typesView);
        return QualType(typePack, isLeftValue);
    }

    // Non-pack mappings are valid only for a single element; the caller already
    // asserted the mapping cardinality before asking for a `QualType`.
    SLANG_ASSERT(range.count == 1);
    return QualType(typeList[range.index], isLeftValue);
}

// Flatten a type-pack-shaped value into the sequence used by recursive type
// unification. Concrete packs contribute each element. Abstract packs, including
// pack parameters and expansions, contribute one expandable element whose final
// span is chosen by `computeTypePackUnificationMapping()`.
static void flattenTypePackForUnification(
    Type* type,
    ShortList<Type*>& outTypes,
    int& outExpandableCount)
{
    if (auto concretePack = as<ConcreteTypePack>(type))
    {
        for (Index i = 0; i < concretePack->getTypeCount(); ++i)
        {
            outTypes.add(concretePack->getElementType(i));
            if (isAbstractTypePack(concretePack->getElementType(i)))
                outExpandableCount++;
        }
    }
    else if (isAbstractTypePack(type))
    {
        outTypes.add(type);
        outExpandableCount++;
    }
}

// Compute how two flattened pack-shaped values should be paired for recursive
// unification.
//
// Each `FlattenedTypeRangePair` in `outMapping` says that a contiguous span from
// `outFlattenedFirst` should be unified with a contiguous span from
// `outFlattenedSecond`. Most mappings are one element to one element. A span can
// contain several elements when the opposite side has an expandable pack element
// that must absorb those elements. The function returns false when the concrete
// element counts cannot be distributed evenly across the expandable elements.
static bool computeTypePackUnificationMapping(
    Type* firstType,
    Type* secondType,
    ShortList<Type*>& outFlattenedFirst,
    ShortList<Type*>& outFlattenedSecond,
    ShortList<FlattenedTypeRangePair>& outMapping)
{
    // The flattened arrays are owned by the caller because later recursive
    // unification needs to turn each mapping span back into a `QualType`.
    ShortList<Type*>& firstTypes = outFlattenedFirst;
    ShortList<Type*>& secondTypes = outFlattenedSecond;

    // Count expandable elements while flattening so the mapping step can decide
    // which side has to absorb extra concrete elements.
    int firstExpandableCount = 0;
    int secondExpandableCount = 0;

    flattenTypePackForUnification(firstType, firstTypes, firstExpandableCount);
    flattenTypePackForUnification(secondType, secondTypes, secondExpandableCount);

    // Decide which side, if any, must expand. The side with fewer
    // non-expandable elements absorbs concrete elements from the other side.
    // When both sides have the same number of non-expandable elements, the side
    // with fewer expandable elements expands so mappings stay as specific as
    // possible.
    //
    // Consider the following cases:
    //
    //   left = [ expand, expand ]
    //   right = [ int, float, expand ]
    //
    // The left side has fewer non-expandable elements, so its first expansion
    // absorbs `int` and `float`.
    //
    //   left = [ int, float, expand, expand ]
    //   right = [ int, float, expand ]
    //
    // The non-expandable counts match, so the side with fewer expandable
    // elements expands. Here the right-side expansion absorbs the first
    // left-side expansion.
    //
    //   left = ConcreteTypePack(ExpandType, ExpandType)
    //   right = ConcreteTypePack(int, bool, float, double).
    //
    // Both left expansions share the concrete right-side elements evenly:
    // `[int, bool]` for the first expansion and `[float, double]` for the
    // second. If the elements cannot be divided evenly, there is no valid
    // structural pack mapping.
    //
    int firstCount = (int)firstTypes.getCount();
    int secondCount = (int)secondTypes.getCount();
    int countDifference =
        (firstCount - firstExpandableCount) - (secondCount - secondExpandableCount);

    bool shouldExpandFirst =
        (firstExpandableCount > 0) &&
        ((countDifference < 0) ||
         (countDifference == 0 && firstExpandableCount < secondExpandableCount));

    bool shouldExpandSecond =
        (secondExpandableCount > 0) &&
        ((countDifference > 0) ||
         (countDifference == 0 && firstExpandableCount > secondExpandableCount));

    // Once the expanding side is known, every expandable element on that side
    // must absorb the same number of elements from the opposite side. Uneven
    // division would make the mapping order-dependent, so report failure.
    int typesPerExpand = 0;
    if (shouldExpandSecond)
    {
        int countToMatch = countDifference + firstExpandableCount;
        SLANG_ASSERT(secondExpandableCount != 0);
        if (countToMatch % secondExpandableCount != 0)
            return false;
        typesPerExpand = countToMatch / secondExpandableCount;
    }
    else if (shouldExpandFirst)
    {
        int countToMatch = -countDifference + secondExpandableCount;
        SLANG_ASSERT(firstExpandableCount != 0);
        if (countToMatch % firstExpandableCount != 0)
            return false;
        typesPerExpand = countToMatch / firstExpandableCount;
    }
    // Walk both flattened arrays and emit the span pairs. A non-expandable
    // element maps one-to-one. An expandable element maps one-to-many only when
    // this pass selected its side to absorb extra elements.
    Index firstIndex = 0;
    Index secondIndex = 0;

    while (firstIndex < firstCount && secondIndex < secondCount)
    {
        FlattenedTypeRangePair mapping;

        if (shouldExpandFirst)
        {
            if (isAbstractTypePack(firstTypes[firstIndex]))
            {
                mapping.first = FlattenedTypeRange(firstIndex, 1);
                mapping.second = FlattenedTypeRange(secondIndex, typesPerExpand);
                secondIndex += typesPerExpand;
            }
            else
            {
                mapping.first = FlattenedTypeRange(firstIndex, 1);
                mapping.second = FlattenedTypeRange(secondIndex, 1);
                secondIndex++;
            }
            firstIndex++;
        }
        else if (shouldExpandSecond)
        {
            if (isAbstractTypePack(secondTypes[secondIndex]))
            {
                mapping.first = FlattenedTypeRange(firstIndex, typesPerExpand);
                mapping.second = FlattenedTypeRange(secondIndex, 1);
                firstIndex += typesPerExpand;
            }
            else
            {
                mapping.first = FlattenedTypeRange(firstIndex, 1);
                mapping.second = FlattenedTypeRange(secondIndex, 1);
                firstIndex++;
            }
            secondIndex++;
        }
        else
        {
            mapping.first = FlattenedTypeRange(firstIndex, 1);
            mapping.second = FlattenedTypeRange(secondIndex, 1);
            firstIndex++;
            secondIndex++;
        }

        outMapping.add(mapping);
    }

    SLANG_ASSERT(!shouldExpandSecond || firstIndex == firstCount);
    SLANG_ASSERT(!shouldExpandFirst || secondIndex == secondCount);
    return true;
}

bool SemanticsVisitor::TryUnifyTypes(
    GenericInferenceContext& constraints,
    UnificationOptions unificationOptions,
    QualType fst,
    QualType snd)
{
    if (!fst)
        return false;

    if (fst->equals(snd))
        return true;

    // An error type can unify with anything, just so we avoid cascading errors.

    if (const auto fstErrorType = as<ErrorType>(fst); fstErrorType)
        return true;

    if (const auto sndErrorType = as<ErrorType>(snd); sndErrorType)
        return true;

    // If one or the other of the types is a conjunction `X & Y`,
    // then we want to recurse on both `X` and `Y`.
    //
    // Note that we check this case *before* we check if one of
    // the types is a generic parameter below, so that we should
    // never end up trying to match up a type parameter with
    // a conjunction directly, and will instead find all of the
    // "leaf" types we need to constrain it to.
    //
    if (as<AndType>(fst) || as<AndType>(snd))
    {
        return TryUnifyConjunctionType(constraints, unificationOptions, fst, snd);
    }

    // Pack-shaped values may need a structural mapping before recursive
    // unification. For example, a single expansion can correspond to several
    // concrete elements on the other side, so compute spans first and then
    // convert each span back into the `QualType` shape expected by recursion.
    ShortList<FlattenedTypeRangePair> typeMapping;
    ShortList<Type*> flattenedFirst;
    ShortList<Type*> flattenedSecond;
    if (computeTypePackUnificationMapping(fst, snd, flattenedFirst, flattenedSecond, typeMapping) &&
        typeMapping.getCount() > 1)
    {
        // Each mapping span is a smaller unification problem that preserves the
        // pack structure selected above.
        for (const auto& mapping : typeMapping)
        {
            // The mapper only emits the three shapes recursive unification
            // understands: one-to-one, one-to-many, or many-to-one.
            SLANG_ASSERT(mapping.first.count > 0 && mapping.second.count > 0);
            SLANG_ASSERT(mapping.first.count == 1 || mapping.second.count == 1);

            // Rebuild each span into the `QualType` shape that preserves whether
            // the opposite side expects a scalar type or a pack-shaped value.
            QualType firstArg = makeQualTypeForFlattenedTypeRange(
                m_astBuilder,
                mapping.first,
                flattenedFirst,
                fst.isLeftValue,
                flattenedSecond[mapping.second.index]);
            QualType secondArg = makeQualTypeForFlattenedTypeRange(
                m_astBuilder,
                mapping.second,
                flattenedSecond,
                snd.isLeftValue,
                flattenedFirst[mapping.first.index]);

            // Recursive unification may discover ordinary solver constraints
            // from the mapped pair, just as it would for a non-pack type.
            if (!TryUnifyTypes(constraints, unificationOptions, firstArg, secondArg))
                return false;
        }

        return true;
    }

    if (auto fstTypePack = as<ConcreteTypePack>(fst))
    {
        if (auto sndTypePack = as<ConcreteTypePack>(snd))
        {
            if (fstTypePack->getTypeCount() != sndTypePack->getTypeCount())
                return false;
            for (Index i = 0; i < fstTypePack->getTypeCount(); ++i)
            {
                if (!TryUnifyTypes(
                        constraints,
                        unificationOptions,
                        QualType(fstTypePack->getElementType(i), fst.isLeftValue),
                        QualType(sndTypePack->getElementType(i), snd.isLeftValue)))
                    return false;
            }
            return true;
        }
        else if (auto sndExpandType = as<ExpandType>(snd))
        {
            for (Index i = 0; i < fstTypePack->getTypeCount(); ++i)
            {
                UnificationOptions subUnificationOptions = unificationOptions;
                subUnificationOptions.indexInTypePack = i;
                if (!TryUnifyTypes(
                        constraints,
                        subUnificationOptions,
                        QualType(fstTypePack->getElementType(i), fst.isLeftValue),
                        QualType(sndExpandType->getPatternType(), snd.isLeftValue)))
                    return false;
            }
            return true;
        }
    }

    if (auto sndTypePack = as<ConcreteTypePack>(snd))
    {
        if (auto fstExpandType = as<ExpandType>(fst))
        {
            for (Index i = 0; i < sndTypePack->getTypeCount(); ++i)
            {
                UnificationOptions subUnificationOptions = unificationOptions;
                subUnificationOptions.indexInTypePack = i;
                if (!TryUnifyTypes(
                        constraints,
                        subUnificationOptions,
                        QualType(fstExpandType->getPatternType(), fst.isLeftValue),
                        QualType(sndTypePack->getElementType(i), snd.isLeftValue)))
                    return false;
            }
            return true;
        }
    }

    // A generic parameter type can unify with anything.
    // TODO: there actually needs to be some kind of "occurs check" sort
    // of thing here...

    if (auto fstDeclRefType = as<DeclRefType>(fst))
    {
        auto fstDeclRef = fstDeclRefType->getDeclRef();

        if (auto typeParamDeclRef = fstDeclRef.as<GenericTypeParamDecl>())
        {
            auto typeParamDecl = typeParamDeclRef.getDecl();
            if (isRelevantGeneric(constraints, typeParamDecl->parentDecl))
                return TryUnifyTypeParam(constraints, unificationOptions, typeParamDecl, snd);
        }
        else if (auto typePackParamDeclRef = fstDeclRef.as<GenericTypePackParamDecl>())
        {
            auto typePackParamDecl = typePackParamDeclRef.getDecl();
            if (isRelevantGeneric(constraints, typePackParamDecl->parentDecl) && isTypePack(snd))
                return TryUnifyTypeParam(constraints, unificationOptions, typePackParamDecl, snd);
        }
    }

    if (auto sndDeclRefType = as<DeclRefType>(snd))
    {
        auto sndDeclRef = sndDeclRefType->getDeclRef();

        if (auto typeParamDeclRef = sndDeclRef.as<GenericTypeParamDeclBase>())
        {
            auto typeParamDecl = typeParamDeclRef.getDecl();
            if (isRelevantGeneric(constraints, typeParamDecl->parentDecl))
                return TryUnifyTypeParam(constraints, unificationOptions, typeParamDecl, fst);
        }
        else if (auto typePackParamDeclRef = sndDeclRef.as<GenericTypePackParamDecl>())
        {
            auto typePackParamDecl = typePackParamDeclRef.getDecl();
            if (isRelevantGeneric(constraints, typePackParamDecl->parentDecl) && isTypePack(fst))
                return TryUnifyTypeParam(constraints, unificationOptions, typePackParamDecl, fst);
        }
    }

    // Structural matching handles identical generic applications, functor
    // signatures, expansions, and concrete type packs. It may append ordinary
    // solver constraints for generic parameters it finds in those structures.
    if (TryUnifyTypesByStructuralMatch(constraints, unificationOptions, fst, snd))
        return true;

    // The remaining cases are inference hints for conversions that the normal
    // post-inference type checker will validate later. A scalar can convert to a
    // vector, so `vector<T, N>` compared with `float` can still usefully infer
    // `T = float` and, when `N` is unconstrained, the optional hint `N = 1`.

    if (auto fstVectorType = as<VectorExpressionType>(fst))
    {
        if (auto sndScalarType = as<BasicExpressionType>(snd))
        {
            // Try unify the vector count param. In case the vector count is defined by a generic
            // value parameter, we want to be able to infer that parameter should be 1. However, we
            // don't want a failed unification to fail the entire generic argument inference,
            // because a scalar can still be casted into a vector of any length.

            tryAddOptionalIntParamConstraintIfUnconstrained(
                constraints,
                unificationOptions,
                fstVectorType->getElementCount(),
                m_astBuilder->getIntVal(m_astBuilder->getIntType(), 1),
                fst.isLeftValue);
            return TryUnifyTypes(
                constraints,
                unificationOptions,
                QualType(fstVectorType->getElementType(), fst.isLeftValue),
                QualType(sndScalarType, snd.isLeftValue));
        }
    }

    if (auto fstScalarType = as<BasicExpressionType>(fst))
    {
        if (auto sndVectorType = as<VectorExpressionType>(snd))
        {
            tryAddOptionalIntParamConstraintIfUnconstrained(
                constraints,
                unificationOptions,
                sndVectorType->getElementCount(),
                m_astBuilder->getIntVal(m_astBuilder->getIntType(), 1),
                snd.isLeftValue);
            return TryUnifyTypes(
                constraints,
                unificationOptions,
                QualType(fstScalarType, fst.isLeftValue),
                QualType(sndVectorType->getElementType(), snd.isLeftValue));
        }
    }

    if (auto fstUniformParamGroupType = as<UniformParameterGroupType>(fst))
        return TryUnifyTypes(
            constraints,
            unificationOptions,
            QualType(fstUniformParamGroupType->getElementType(), fst.isLeftValue),
            snd);
    if (auto sndUniformParamGroupType = as<UniformParameterGroupType>(snd))
        return TryUnifyTypes(
            constraints,
            unificationOptions,
            fst,
            QualType(sndUniformParamGroupType->getElementType(), snd.isLeftValue));

    // Each T can coerce with any DeclRefType.
    if (auto eachSnd = as<EachType>(snd))
    {
        if (auto innerSnd = eachSnd->getElementDeclRefType())
        {
            auto innerSndDeclRef = innerSnd->getDeclRef();
            if (auto sndTypePackParamDeclRef = innerSndDeclRef.as<GenericTypePackParamDecl>())
            {
                auto sndTypePackParamDecl = sndTypePackParamDeclRef.getDecl();
                if (isRelevantGeneric(constraints, sndTypePackParamDecl->parentDecl))
                {
                    return TryUnifyTypeParam(
                        constraints,
                        unificationOptions,
                        sndTypePackParamDecl,
                        fst);
                }
            }
        }
    }
    if (auto eachFst = as<EachType>(fst))
    {
        if (auto innerFst = eachFst->getElementDeclRefType())
        {
            auto innerFstDeclRef = innerFst->getDeclRef();
            if (auto fstTypePackParamDeclRef = innerFstDeclRef.as<GenericTypePackParamDecl>())
            {
                auto fstTypePackParamDecl = fstTypePackParamDeclRef.getDecl();
                if (isRelevantGeneric(constraints, fstTypePackParamDecl->parentDecl))
                {
                    return TryUnifyTypeParam(
                        constraints,
                        unificationOptions,
                        fstTypePackParamDecl,
                        snd);
                }
            }
        }
    }

    if (as<ModifiedType>(fst) || as<ModifiedType>(snd))
    {
        // We can ignore modifiers for the purpose of unification, but only if the underlying
        // type unifies.
        //
        // Modifiers are usually checked separately for compatibility based on the context.
        //
        auto fstModifiedType = as<ModifiedType>(fst);
        auto sndModifiedType = as<ModifiedType>(snd);
        return TryUnifyTypes(
            constraints,
            unificationOptions,
            QualType(fstModifiedType ? fstModifiedType->getBase() : fst.type, fst.isLeftValue),
            QualType(sndModifiedType ? sndModifiedType->getBase() : snd.type, snd.isLeftValue));
    }
    return false;
}


} // namespace Slang
