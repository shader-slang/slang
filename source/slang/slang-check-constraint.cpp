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

bool SemanticsVisitor::isRelevantGeneric(ConstraintSystem& system, Decl* generic)
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
    ConstraintSystem* constraints,
    VectorExpressionType* vectorType,
    BasicExpressionType* scalarType)
{
    // Join( vector<T,N>, S ) -> vetor<Join(T,S), N>
    //
    // That is, the join of a vector and a scalar type is
    // a vector type with a joined element type.
    auto joinElementType = TryJoinTypes(constraints, vectorType->getElementType(), scalarType);
    if (!joinElementType)
        return nullptr;

    return createVectorType(joinElementType, vectorType->getElementCount());
}

Type* SemanticsVisitor::_tryJoinTypeWithInterface(
    ConstraintSystem* constraints,
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
            // Track the conversion cost for type promotion in the constraint system.
            // This cost represents promoting a type (e.g., int -> float) to satisfy
            // an interface constraint (e.g., __BuiltinFloatingPointType).
            // This ensures that overload resolution prefers exact type matches over
            // candidates that require type promotion.
            constraints->typePromotionCost += bestCost;
            return bestType;
        }
    }

    // If `interfaceType` represents some generic interface type, such as `IFoo<T>`, and `type`
    // conforms to some `IFoo<X>`, then we should attempt to unify the them to discover constraints
    // for `T`.
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
                        ValUnificationContext(),
                        QualType(facet->getType()),
                        interfaceType);

                    if (unificationResult)
                        return type;
                }
            }
            if (auto typeDeclRefType = as<DeclRefType>(type))
            {
                // A generic parameter may not have inheritance facets of its
                // own, but its where-clause conformances are still evidence.
                // When `InArray : IArrayAccessor<V>` is compared against
                // `IArrayAccessor<U>`, unifying the two interface shapes gives
                // the generic solver the missing ordinary constraint `U = V`.
                if (auto typeParamDecl =
                        as<GenericTypeParamDeclBase>(typeDeclRefType->getDeclRef().getDecl()))
                {
                    if (auto genericDecl = as<GenericDecl>(typeParamDecl->parentDecl))
                    {
                        if (isRelevantGeneric(*constraints, genericDecl))
                        {
                            for (auto member : genericDecl->getDirectMemberDecls())
                            {
                                auto conformanceDecl = as<GenericTypeConstraintDecl>(member);
                                if (!conformanceDecl)
                                    continue;

                                auto conformanceDeclRef = createDefaultSubstitutionsIfNeeded(
                                                              m_astBuilder,
                                                              this,
                                                              conformanceDecl->getDefaultDeclRef())
                                                              .as<GenericTypeConstraintDecl>();
                                if (!conformanceDeclRef)
                                    continue;

                                auto conformanceSub = getSub(m_astBuilder, conformanceDeclRef);
                                auto conformanceSup = getSup(m_astBuilder, conformanceDeclRef);
                                if (!conformanceSub || !conformanceSup ||
                                    !conformanceSub->equals(type))
                                    continue;

                                auto unificationResult = TryUnifyTypes(
                                    *constraints,
                                    ValUnificationContext(),
                                    QualType(conformanceSup),
                                    interfaceType);
                                if (unificationResult)
                                    return type;
                            }
                        }
                    }
                }
            }
            if (constraints->subTypeForAdditionalWitnesses)
            {
                for (auto witnessKV : *constraints->additionalSubtypeWitnesses)
                {
                    auto unificationResult = TryUnifyTypes(
                        *constraints,
                        ValUnificationContext(),
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

Type* SemanticsVisitor::TryJoinTypes(ConstraintSystem* constraints, QualType left, QualType right)
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

bool findTypeCoercionWitnessForConstraint(
    ASTBuilder* astBuilder,
    SemanticsVisitor* visitor,
    TypeCoercionConstraintDecl* constraintDecl,
    DeclRef<GenericDecl> genericDeclRef,
    SemanticsVisitor::OverloadResolveContext* maybeContext,
    ArrayView<Val*> args,
    TypeCoercionWitness*& outWitness,
    bool shouldEmitError)
{
    // To emit an error, `maybeContext` must be provided.
    SLANG_ASSERT(!shouldEmitError || shouldEmitError && maybeContext);
    outWitness = nullptr;

    DeclRef<TypeCoercionConstraintDecl> constraintDeclRef =
        astBuilder->getGenericAppDeclRef(genericDeclRef, args, constraintDecl)
            .as<TypeCoercionConstraintDecl>();
    auto fromType = getFromType(astBuilder, constraintDeclRef);
    auto toType = getToType(astBuilder, constraintDeclRef);
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
        // The viable conversion is not an implicit conversion, but implicit was requested, fail
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
            return false;
        }
    }

    // The type arguments are not convertible, return failure.
    if (conversionCost == kConversionCost_Impossible)
    {
        if (shouldEmitError)
        {
            visitor->getSink()->diagnose(Diagnostics::TypeCoerceConstraintMissingConversion{
                .fromType = fromType,
                .toType = toType,
                .location = maybeContext->loc});
            visitor->getSink()->diagnose(
                Diagnostics::SeeDefinitionOf{.decl = genericDeclRef.getDecl()->inner});
        }
        return false;
    }

    // If `_coerce` accepted the conversion but didn't provide an explicit
    // witness object, treat it as a builtin conversion witness so later
    // lowering still has a concrete generic argument to pass through.
    if (!typeCoercionWitness)
        typeCoercionWitness = astBuilder->getBuiltinTypeCoercionWitness(fromType, toType);

    outWitness = typeCoercionWitness;
    return true;
}

bool findDiffTypeInfoWitnessForConstraint(
    ASTBuilder* astBuilder,
    SemanticsVisitor* visitor,
    HasDiffTypeInfoConstraintDecl* constraintDecl,
    DeclRef<GenericDecl> genericDeclRef,
    SemanticsVisitor::OverloadResolveContext* maybeContext,
    ArrayView<Val*> args,
    Witness*& outWitness,
    bool shouldEmitError)
{
    SLANG_ASSERT(!shouldEmitError || maybeContext);
    outWitness = nullptr;

    auto constraintDeclRef = astBuilder->getGenericAppDeclRef(genericDeclRef, args, constraintDecl)
                                 .as<HasDiffTypeInfoConstraintDecl>();
    auto constrainedType = getBaseType(astBuilder, constraintDeclRef);
    if (!constrainedType)
    {
        if (shouldEmitError)
        {
            visitor->getSink()->diagnose(Diagnostics::TypeDoesNotHaveDiffTypeInfo{
                .type = astBuilder->getErrorType(),
                .location = maybeContext->loc});
            visitor->getSink()->diagnose(
                Diagnostics::SeeDefinitionOfConstraint{.decl = constraintDecl});
        }
        return false;
    }
    if (auto witness = visitor->getDiffTypeInfoWitness(constrainedType))
    {
        outWitness = witness;
        return true;
    }

    if (shouldEmitError)
    {
        visitor->getSink()->diagnose(Diagnostics::TypeDoesNotHaveDiffTypeInfo{
            .type = constrainedType,
            .location = maybeContext->loc});
        visitor->getSink()->diagnose(
            Diagnostics::SeeDefinitionOfConstraint{.decl = constraintDecl});
    }

    return false;
}

// The live argument value for a slot lives in `m_args`; this enum describes how
// trustworthy that value is. A placeholder can be used to keep substitution
// well formed, a default can unblock later dependent work, and a concrete
// solution prevents a default from replacing inference.
enum class SolvedArgState
{
    // No solver answer has been recorded. `m_args` may still contain the
    // declaration-shaped placeholder from `getDefaultSubstitutionArgs()`.
    Placeholder,

    // The caller supplied an explicit non-placeholder argument. Defaults should
    // not replace it.
    Known,

    // The caller supplied the identity argument for the same parameter, such as
    // `<T>` inside a dependent generic context. This is ready for substitution
    // but still replaceable by inference or by a real default.
    KnownDependent,

    // Ordinary constraints inferred the parameter value. This is a concrete
    // solver answer and takes precedence over defaults.
    Solved,

    // A default argument work item produced the current value. Later concrete
    // inference can still replace it.
    Default,

    // A pack parameter was omitted, so the current value is the empty pack used
    // by generic application solving. Non-empty-pack witness work decides
    // whether that empty value is legal.
    OmittedPack,

    // A hidden witness requirement has been proved and its witness argument has
    // replaced the declaration-shaped placeholder.
    Witness,
};

// Additional information about one generic-application slot. The actual current
// argument value lives in `m_args`; this record keeps the extra state needed to
// interpret or update that argument, such as the solution state, type-join state
// for type parameters, priority for value constraints, overload-ranking facts,
// and the last substituted default value.
struct SolvedArgInfo
{
    SolvedArgState state = SolvedArgState::Placeholder;
    Val* defaultVal = nullptr;
    SemanticsVisitor::ConstraintPriority priority = SemanticsVisitor::ConstraintPriority::Default;
    ShortList<QualType, 8> types;
    bool isConstrainedGenericParam = false;
};

// Return the user-written default for one generic type or value parameter.
// For `Foo<T, U = T.Element, let N : int = 4>`, this helper returns the
// semantic `T.Element` type for `U` and the folded integer value `4` for `N`.
// Parameters without defaults, pack parameters, and witness constraints return
// `nullptr` because they are not defaulted parameter slots.
Val* tryGetGenericParamDefaultVal(SemanticsVisitor* visitor, Decl* paramDecl)
{
    // Type defaults have already been checked as type syntax, so the stored type
    // is the generic argument value that the solver will replay.
    if (auto typeParam = as<GenericTypeParamDecl>(paramDecl))
    {
        return typeParam->initType.type;
    }
    else if (auto valParam = as<GenericValueParamDecl>(paramDecl))
    {
        // Value defaults live as expressions until generic solving needs an
        // `IntVal`; a missing initializer simply means there is no default.
        if (!valParam->initExpr)
            return nullptr;

        // Default expressions can refer back into generic state, so use the same
        // constant-folding circularity guard as the rest of semantic checking.
        SemanticsVisitor::ConstantFoldingCircularityInfo newCircularityInfo(
            makeDeclRef(paramDecl),
            nullptr);
        return visitor->ExtractGenericArgVal(valParam->initExpr, &newCircularityInfo);
    }
    return nullptr;
}

// Recover a generic value-parameter reference from an integer value, allowing a
// single cast wrapper inserted by checking. For example, a reference to `N` may
// appear as `int(N)` after unification normalizes the expected integer type.
DeclRefIntVal* getDeclRefIntValUnderCast(IntVal* intVal)
{
    // Integer generic arguments can be wrapped in a type cast.  Unification
    // peels that cast so both sides can expose the same parameter reference.
    if (const auto castIntVal = as<TypeCastIntVal>(intVal))
        intVal = as<IntVal>(castIntVal->getBase());

    // Concrete integer values were handled before this helper is called, so the
    // only useful answer here is a value-parameter reference.
    return as<DeclRefIntVal>(intVal);
}

// Solve one generic application by collecting every available source of
// information, then letting a single work list discover type arguments, value
// arguments, default arguments, and witness arguments together. The incoming
// `ConstraintSystem` remains the scratchpad used by unification helpers, but
// the solver drains that scratchpad into `SolverWorkItem`s immediately so the
// work-item table is the only persistent list of solver obligations. A case
// like `Foo<T : IFoo, U = T.A>` is handled by allowing the `T : IFoo` witness
// item and the `U` default item to wake each other as their prerequisites
// become available.
class GenericConstraintSystemSolver
{
public:
    using Constraint = SemanticsVisitor::Constraint;
    using ConstraintPriority = SemanticsVisitor::ConstraintPriority;
    using ConstraintSystem = SemanticsVisitor::ConstraintSystem;
    using ValUnificationContext = SemanticsVisitor::ValUnificationContext;

    // A work item is one pending question in the unified solver loop. The
    // solver stores these items in a stable table and queues their indices; when
    // a slot changes, the other items are marked unfinished and queued again.
    // Each item is small enough to describe the source of work; the current
    // serialized answers live in `m_args`, and `m_solvedArgsInfo` keeps the extra
    // metadata needed to compare and update those answers.
    struct SolverWorkItem
    {
        enum class Kind
        {
            // An inference fact produced by unification. A call like
            // `bar<T>(42)` can produce `T = int`; comparing conformance shapes
            // can also produce facts such as `U = T.A`.
            OrdinaryConstraint,

            // A generic parameter default that still needs substitution through
            // the current partial answers. In `Foo<T : IFoo, U = T.A>`, the
            // `U` default is represented by this kind of item.
            DefaultArg,

            // A type-level requirement that must produce a hidden witness
            // argument. Examples include `T : IFoo`, type-coercion
            // constraints, non-empty-pack constraints, and differentiation
            // type-info constraints.
            Witness,
        };

        // Selects which solver routine owns this item. The work-list loop only
        // understands the common `WorkResult` protocol; this kind tells it
        // whether the item solves an ordinary slot, a defaulted slot, or a
        // witness slot.
        Kind kind = Kind::OrdinaryConstraint;

        // The generic declaration whose argument list this item contributes to.
        // Defaults and witnesses need this to form the right substitution
        // prefix; ordinary constraints do not need it because their target slot
        // is stored in `constraint.decl`.
        GenericDecl* genericDecl = nullptr;

        // The declaration slot directly owned by this item. For an ordinary
        // constraint this is the target type/value parameter; for a default this
        // is the defaulted parameter; for a witness this is the type-constraint
        // declaration that will receive the solved witness argument.
        Decl* decl = nullptr;

        // The ordinary unification constraint owned by this item. Only
        // `OrdinaryConstraint` items use this field. Once a constraint reaches
        // this field it no longer lives in `ConstraintSystem::constraints`; the
        // work-item table is the single durable list of both initial and
        // discovered solver work.
        Constraint constraint;

        // The raw default value for a default-argument item. It is deliberately
        // stored before substitution so the work item can be retried after
        // newly solved parameters or witnesses make dependent expressions such
        // as `T.A` meaningful.
        Val* val = nullptr;

        // True once the item has reached a terminal result for the current
        // partial solution. A later solved slot clears this bit so dependent
        // defaults, witnesses, and shape constraints can observe the new
        // substitution.
        bool done = false;

        // True while this item's index is already present in `m_workList`.
        // Keeping the bit on the item avoids a parallel bookkeeping array and
        // keeps repeated conservative wakeups from adding duplicate queue
        // entries.
        bool queued = false;
    };

    // The common result protocol for the work-list loop. Each solver routine
    // either proves that its work is already settled, publishes a new answer,
    // waits for another slot, or rejects the overload candidate. A work item is
    // considered "solved" by the loop when it returns `Done` or `Solved`.
    // `Solved` additionally wakes the rest of the work list because the live
    // generic arguments may now substitute dependent values differently.
    enum class WorkResult
    {
        // The item is complete and did not change any answer. The loop marks it
        // done and does not wake the work list. This covers duplicate constraints,
        // defaults skipped because a concrete solution already exists, and
        // witnesses that re-derived the same value.
        Done,

        // The item wrote a new or changed answer to a type/value/default/witness
        // slot. The loop marks it done and gives every other item a chance to
        // observe the new partial substitution.
        Solved,

        // The item cannot run yet because some parameter or witness it depends
        // on is still unresolved. The loop leaves it unfinished; it will only be
        // retried when a later `Solved` result wakes the work list.
        Blocked,

        // The item discovered that this generic application cannot be solved.
        // Examples include incompatible same-priority ordinary constraints or a
        // required witness that cannot be found. The whole solver fails
        // immediately for this overload candidate.
        Failed,
    };

    // Capture the semantic context, take ownership of the constraint system, and
    // remember the explicit visible argument prefix already known at the call
    // site. The solver writes the final overload cost through `outBaseCost` once
    // all work items settle.
    GenericConstraintSystemSolver(
        SemanticsVisitor* visitor,
        ConstraintSystem&& system,
        DeclRef<GenericDecl> genericDeclRef,
        ArrayView<Val*> knownGenericArgs,
        ConversionCost& outBaseCost)
        : m_visitor(visitor)
        , m_astBuilder(visitor->getASTBuilder())
        , m_system(_Move(system))
        , m_genericDeclRef(genericDeclRef)
        , m_knownGenericArgs(knownGenericArgs)
        , m_outBaseCost(outBaseCost)
    {
    }

    // Run the full two-phase algorithm and return the substituted decl-ref for
    // the generic body. On success, `Foo<int>` becomes a decl-ref whose generic
    // application contains ordinary arguments followed by the solved hidden
    // witnesses; on failure, an empty decl-ref tells overload resolution to
    // reject this candidate.
    DeclRef<Decl> solve()
    {
        // The solver is deliberately split into two phases.  First it gathers
        // every ordinary argument constraint, default argument, and implicit
        // witness requirement into one list of work items.
        m_outBaseCost = kConversionCost_None;
        if (!collectConstraints())
            return DeclRef<Decl>();

        // The second phase is a single work-list loop.  Each work item either
        // fills a parameter slot, fills a witness slot, or blocks until another
        // slot is available.  Any solved slot wakes the blocked items so types,
        // values, defaults, and witnesses are discovered together.
        if (!runWorkList())
            return DeclRef<Decl>();

        // Once the loop is quiet, every required ordinary constraint and every
        // hidden witness slot must have an answer. `m_args` has been updated
        // incrementally throughout solving, so the final step only validates
        // that no required slot is still represented by its placeholder.
        if (!verifyOrdinaryConstraintsSatisfied())
            return DeclRef<Decl>();
        if (!validateFinalArgs())
            return DeclRef<Decl>();

        // The final cost mirrors the previous solver: unconstrained generic
        // parameters are less specific, and witness/type-promotion work
        // contributes to overload ranking.
        addUnconstrainedGenericParamCost();
        m_outBaseCost += m_system.typePromotionCost + getFinalWitnessCost();

        return getSubstDeclRef(m_genericDeclRef.getDecl()->inner);
    }

private:
    // Build the initial work-list universe. Ordinary argument constraints,
    // defaulted parameter slots, and implicit witness slots are all collected
    // before solving starts so no answer kind gets a privileged earlier phase.
    bool collectConstraints()
    {
        // Generic substitutions are nested, so the solver starts by recording the
        // chain from the outermost generic to the generic being applied.
        collectGenericDeclChain();

        // Conformance requirements still provide useful shape information for
        // legacy cases such as `This`-based interface members. They enter the
        // work list as ordinary hints, while their real validity is checked by
        // the witness items collected below.
        if (!collectGenericParamConformanceConstraints())
            return false;

        // The constraint system already contains inference facts collected by
        // argument/parameter unification.  Each one becomes a work item that can
        // fill a type or value parameter slot.
        drainPendingConstraintsIntoWorkItems();

        // Defaults and where-clauses are collected after ordinary constraints,
        // but they are not solved in a later phase.  They are simply more work
        // items in the same loop.
        collectGenericMemberWorkItems();

        // The argument arrays start from the canonical default-substitution
        // shape already used elsewhere in the checker. Work items then replace
        // the specific slots they solve, so `m_args` stays live throughout the
        // loop.
        return initializeCurrentArgs();
    }

    // Record the nested generic declarations from outermost to innermost. For
    // `Outer<T>.Inner<U>`, substitutions must be rebuilt for `Outer` before
    // `Inner`, so the stored order is `[Outer, Inner]`.
    void collectGenericDeclChain()
    {
        for (auto genericDecl = m_genericDeclRef.getDecl(); genericDecl;
             genericDecl = as<GenericDecl>(genericDecl->parentDecl))
        {
            m_genericDecls.add(genericDecl);
        }
        m_genericDecls.reverse();
    }

    // Turn generic parameter conformance requirements into ordinary
    // unification constraints. In `<T : IFoo<U>>`, the conformance requirement
    // does not solve `T` by itself, but after `T` is known it can expose the
    // interface argument `U`.
    bool collectGenericParamConformanceConstraints()
    {
        for (auto genericDeclRef = m_genericDeclRef; genericDeclRef;
             genericDeclRef = genericDeclRef.getParent().as<GenericDecl>())
        {
            for (auto constraintDeclRef :
                 getMembersOfType<GenericTypeConstraintDecl>(m_astBuilder, genericDeclRef))
            {
                ValUnificationContext ctx;
                ctx.optionalConstraint =
                    constraintDeclRef.getDecl()->hasModifier<OptionalConstraintModifier>();
                ctx.equalityConstraint = constraintDeclRef.getDecl()->isEqualityConstraint;
                ctx.genericParamConformanceConstraint = true;

                if (!m_visitor->TryUnifyTypes(
                        m_system,
                        ctx,
                        getSub(m_astBuilder, constraintDeclRef),
                        getSup(m_astBuilder, constraintDeclRef)))
                    return false;
            }
        }
        return true;
    }

    // Move ordinary constraints produced by shared unification helpers into the
    // solver's work-item list. `ConstraintSystem::constraints` is treated as a
    // short-lived inbox: initial call-site unification fills it before the
    // solver starts, and later `TryJoinTypes`/`TryUnifyTypes` calls may fill it
    // again while a work item is running. Draining and clearing it here keeps
    // all real solver progress in one table.
    void drainPendingConstraintsIntoWorkItems()
    {
        for (auto constraint : m_system.constraints)
        {
            SolverWorkItem workItem;
            workItem.kind = SolverWorkItem::Kind::OrdinaryConstraint;
            workItem.decl = constraint.decl;
            workItem.constraint = constraint;
            addWorkItem(workItem);
        }
        m_system.constraints.clear();
    }

    // Visit every generic declaration in the chain and collect the member-based
    // work: defaulted visible parameters and hidden witness requirements.
    void collectGenericMemberWorkItems()
    {
        for (auto genericDecl : m_genericDecls)
            collectGenericMemberWorkItems(genericDecl);
    }

    // Collect the work represented by one generic declaration's direct members.
    // For `Foo<T : IFoo, U = T.A>`, this adds the `U` default item and the
    // `T : IFoo` witness item.
    void collectGenericMemberWorkItems(GenericDecl* genericDecl)
    {
        for (auto member : genericDecl->getDirectMemberDecls())
        {
            if (isDefaultableParam(member))
                addDefaultArgWorkItem(genericDecl, member);
            else if (isTypeConstraintDecl(member))
                addWitnessWorkItem(genericDecl, member);
        }
    }

    // Create the live generic-argument arrays before the work list starts.
    // `getDefaultSubstitutionArgs()` already knows the declaration-order shape
    // of a generic application, including the hidden witness slots after the
    // visible parameter prefix. The solver uses that shape as its initial
    // partial substitution and then overwrites slots as real answers are found.
    bool initializeCurrentArgs()
    {
        for (auto genericDecl : m_genericDecls)
        {
            auto defaultArgs = getDefaultSubstitutionArgs(m_astBuilder, m_visitor, genericDecl);
            auto& args = m_args[genericDecl];
            args.clear();

            // Default substitutions are placeholders, not solver answers. They
            // are nevertheless the right starting point because they make every
            // parameter and hidden requirement addressable by the same index it
            // will occupy in the final generic application.
            for (auto arg : defaultArgs)
            {
                if (!arg)
                    return false;
                args.add(arg);
            }

            if (!initializeVisibleArgSlots(genericDecl))
                return false;
        }
        return true;
    }

    // Initialize the visible parameter prefix of one generic argument list.
    // Caller-provided arguments overwrite their placeholders and become known
    // slots; omitted packs use the empty-pack convention expected by generic
    // application solving. Hidden witness slots are deliberately left as their
    // default-substitution placeholders until witness work items prove them.
    bool initializeVisibleArgSlots(GenericDecl* genericDecl)
    {
        for (auto member : genericDecl->getDirectMemberDecls())
        {
            Val* knownArg = nullptr;
            if (tryGetKnownGenericArg(member, knownArg))
            {
                if (!setCurrentArg(member, knownArg))
                    return false;
                setArgState(
                    member,
                    isSelfReferencePlaceholder(member, knownArg) ? SolvedArgState::KnownDependent
                                                                 : SolvedArgState::Known);
                continue;
            }

            if (as<GenericTypePackParamDecl>(member))
            {
                if (!setCurrentArg(member, m_astBuilder->getTypePack(ArrayView<Type*>())))
                    return false;
                setArgState(member, SolvedArgState::OmittedPack);
            }
            else if (as<GenericValuePackParamDecl>(member))
            {
                if (!setCurrentArg(member, m_astBuilder->getIntValPack(ArrayView<IntVal*>())))
                    return false;
                setArgState(member, SolvedArgState::OmittedPack);
            }
        }
        return true;
    }

    // True for ordinary type and value parameters that may carry a default.
    // Pack parameters are excluded because the solver treats an omitted pack as
    // an empty pack rather than a default initializer.
    bool isDefaultableParam(Decl* member)
    {
        if (as<GenericTypePackParamDecl>(member) || as<GenericValuePackParamDecl>(member))
            return false;
        return as<GenericTypeParamDecl>(member) || as<GenericValueParamDecl>(member);
    }

    // True for generic members that declare type-level requirements. These
    // declarations occupy hidden witness argument positions once solved:
    // `T : IFoo`, type coercions, non-empty-pack checks, and differentiation
    // type-info checks all produce witness arguments for the final application.
    bool isTypeConstraintDecl(Decl* member)
    {
        return as<GenericTypeConstraintDecl>(member) || as<TypeCoercionConstraintDecl>(member) ||
               as<NonEmptyPackConstraintDecl>(member) || as<HasDiffTypeInfoConstraintDecl>(member);
    }

    // Add one default-argument work item when a parameter has an initializer.
    // The item stores the raw default value, such as `T.A`, and later substitutes
    // it through the current partial answers before recording it.
    void addDefaultArgWorkItem(GenericDecl* genericDecl, Decl* paramDecl)
    {
        Val* defaultVal = tryGetGenericParamDefaultVal(m_visitor, paramDecl);
        if (!defaultVal)
            return;

        SolverWorkItem workItem;
        workItem.kind = SolverWorkItem::Kind::DefaultArg;
        workItem.genericDecl = genericDecl;
        workItem.decl = paramDecl;
        workItem.val = defaultVal;
        addWorkItem(workItem);
    }

    // Add one hidden-witness work item for a generic requirement. For example,
    // a `T : IFoo` member gets its own slot that must eventually contain the
    // subtype witness proving the chosen `T` conforms to `IFoo`.
    void addWitnessWorkItem(GenericDecl* genericDecl, Decl* constraintDecl)
    {
        SolverWorkItem workItem;
        workItem.kind = SolverWorkItem::Kind::Witness;
        workItem.genericDecl = genericDecl;
        workItem.decl = constraintDecl;
        addWorkItem(workItem);
    }

    // Append a work item to the solver's stable item table and queue it once.
    // The stable index lets dependency tracking wake the same logical item
    // without copying its payload.
    void addWorkItem(SolverWorkItem const& workItem)
    {
        Index itemIndex = m_solverWorkItems.getCount();
        m_solverWorkItems.add(workItem);
        enqueueWorkItem(itemIndex);
    }

    // Queue a pending item if it is neither already queued nor already done.
    // This keeps repeated wakeups cheap when several solved slots all happen
    // before a blocked item gets another turn.
    void enqueueWorkItem(Index itemIndex)
    {
        auto& workItem = m_solverWorkItems[itemIndex];
        if (workItem.done || workItem.queued)
            return;
        workItem.queued = true;
        m_workList.add(itemIndex);
    }

    // Reconsider every other work item after one item publishes progress. The
    // work list is usually small, and this rule keeps the algorithm easy to
    // reason about: no solved default, inferred type, or hidden witness can be
    // missed because its dependency was represented indirectly in a substituted
    // decl-ref. Items that are still settled will simply return `Done` when they
    // run again.
    void wakeWorkItemsAfterProgress(Index solvedItemIndex)
    {
        for (Index itemIndex = 0; itemIndex < m_solverWorkItems.getCount(); itemIndex++)
        {
            if (itemIndex == solvedItemIndex)
                continue;

            m_solverWorkItems[itemIndex].done = false;
            enqueueWorkItem(itemIndex);
        }
    }

    // Drive the unified solving loop. Each item either records an answer,
    // reports that it is already settled, blocks on another slot, or fails the
    // candidate; for example, a default `U = T.A` blocks until both `T` and the
    // witness used by `T.A` are ready.
    bool runWorkList()
    {
        // The work list is intentionally not split by answer kind.  A default
        // that needs a witness simply blocks, and the witness constraint is free
        // to run next.  Conversely, a witness that needs a defaulted type blocks
        // until that type slot receives an answer.
        while (m_workListReadIndex < m_workList.getCount())
        {
            Index itemIndex = m_workList[m_workListReadIndex++];
            m_solverWorkItems[itemIndex].queued = false;

            if (m_solverWorkItems[itemIndex].done)
                continue;

            WorkResult result = trySolveWorkItem(itemIndex);
            if (result == WorkResult::Failed)
                return false;
            if (result == WorkResult::Blocked)
                continue;

            m_solverWorkItems[itemIndex].done = true;
            if (result == WorkResult::Solved)
                wakeWorkItemsAfterProgress(itemIndex);
        }

        // If the queue drained while an item remained unsolved, then all
        // remaining work is waiting on slots that never acquired answers.
        for (Index itemIndex = 0; itemIndex < m_solverWorkItems.getCount(); itemIndex++)
        {
            if (!m_solverWorkItems[itemIndex].done)
                return false;
        }
        return true;
    }

    // Dispatch one queued item to the handler for its kind, then collect any
    // new ordinary constraints discovered as a side effect. This keeps the
    // contract simple for the individual handlers: they solve their own item,
    // while the dispatcher owns the shared inbox used by `TryJoinTypes()` and
    // `TryUnifyTypes()`.
    WorkResult trySolveWorkItem(Index itemIndex)
    {
        auto& workItem = m_solverWorkItems[itemIndex];
        WorkResult result = WorkResult::Failed;
        switch (workItem.kind)
        {
        case SolverWorkItem::Kind::OrdinaryConstraint:
            result = solveOrdinaryConstraint(workItem);
            break;
        case SolverWorkItem::Kind::DefaultArg:
            result = solveDefaultArg(workItem);
            break;
        case SolverWorkItem::Kind::Witness:
            result = solveWitness(workItem);
            break;
        }
        return drainDiscoveredConstraints(result);
    }

    // Drain ordinary constraints that were discovered while solving one work
    // item. Blocked and failed items keep their original result; otherwise, new
    // work means the item made progress from the work-list's point of view and
    // should wake the rest of the loop.
    WorkResult drainDiscoveredConstraints(WorkResult result)
    {
        if (result == WorkResult::Blocked || result == WorkResult::Failed)
            return result;
        if (m_system.constraints.getCount() == 0)
            return result;

        drainPendingConstraintsIntoWorkItems();
        return WorkResult::Solved;
    }

    // Apply a unification-derived constraint to a type or value parameter slot.
    // For a generic function like `bar<T, int N>(T value)`, checking a call
    // such as `bar(42)` might contribute `T = int` or `N = 42`; dependent
    // constraints are replayed through the current partial substitution before
    // they update a slot.
    WorkResult solveOrdinaryConstraint(SolverWorkItem& workItem)
    {
        Constraint& c = workItem.constraint;
        if (c.satisfied)
            return WorkResult::Done;
        if (c.isGenericParamConformance && !c.isEquality)
            return solveGenericParamConformanceConstraint(c);
        if (hasBlockersForVal(c.val, c.decl))
            return WorkResult::Blocked;

        if (!substitutePotentiallyDependentConstraint(c))
            return WorkResult::Failed;

        return solveParameterConstraint(c);
    }

    // Use a generic parameter conformance requirement as an inference hint once
    // the subject slot has a ready substitution value. In
    // `<InputArray : IArrayAccessor<Element>>`, the requirement can infer
    // `Element` only after `InputArray` is known, inferred, defaulted, or fixed
    // as a dependent caller-provided placeholder.
    WorkResult solveGenericParamConformanceConstraint(Constraint& c)
    {
        // A conformance requirement is not an answer for the parameter it
        // mentions. It is an edge that becomes informative after the subject
        // slot has an answer: `InputArray : IArrayAccessor<U>` can then compare
        // the known `InputArray` against that interface shape and may discover
        // `U`.
        if (!isSlotReady(c.decl))
            return WorkResult::Blocked;

        // The interface side of the requirement is allowed to mention
        // parameters that this same requirement is trying to infer. The live
        // `m_args` placeholders keep those references visible to
        // `TryJoinTypes`, which can add the real ordinary constraints when it
        // recognizes an existing generic witness.
        if (!substitutePotentiallyDependentConstraint(c))
            return WorkResult::Failed;

        WorkResult result = solveParameterConstraint(c);

        // Shape-comparison failures are left to the witness item, which can
        // produce the actual proof or fail the specialization. The ordinary item
        // only exists to contribute any inference that falls out of comparing
        // generic interface arguments.
        Count discoveredConstraintCount = m_system.constraints.getCount();
        if (result == WorkResult::Failed && discoveredConstraintCount == 0)
        {
            c.satisfied = true;
            return WorkResult::Done;
        }
        if (discoveredConstraintCount != 0)
            return WorkResult::Solved;
        return result;
    }

    // Dispatch one ordinary constraint to the solver for the slot it targets.
    // Type parameters, value-pack parameters, and value parameters each merge
    // evidence differently, but they all return the same work-list result so
    // callers can distinguish real progress from a no-op confirmation.
    WorkResult solveParameterConstraint(Constraint& c)
    {
        if (auto typeParam = as<GenericTypeParamDeclBase>(c.decl))
            return solveTypeParamConstraint(c, typeParam);
        if (auto valPackParam = as<GenericValuePackParamDecl>(c.decl))
            return solveValuePackParamConstraint(c, valPackParam);
        if (auto valParam = as<GenericValueParamDecl>(c.decl))
            return solveValueParamConstraint(c, valParam);
        return WorkResult::Failed;
    }

    // Substitute and record a default argument once its dependencies are ready.
    // For `Foo<T : IFoo, U = T.A>`, this turns the raw `T.A` default into the
    // associated type reached through the solved `T : IFoo` witness.
    WorkResult solveDefaultArg(SolverWorkItem const& workItem)
    {
        if (hasNonDefaultSolution(workItem.decl))
            return WorkResult::Done;
        if (hasBlockersForVal(workItem.val))
            return WorkResult::Blocked;
        auto fullSubst = SubstitutionSet(getSubstDeclRef(m_genericDeclRef.getDecl()->inner));
        int diff = 0;
        auto substArg = workItem.val->substituteImpl(m_astBuilder, fullSubst, &diff);
        if (!substArg)
            return WorkResult::Failed;

        substArg = substArg->resolve();
        if (auto substArgType = as<Type>(substArg))
            substArg = substArgType->getCanonicalType();

        auto& solvedArg = m_solvedArgsInfo[workItem.decl];
        if (solvedArg.defaultVal && solvedArg.defaultVal->equals(substArg))
            return WorkResult::Done;

        solvedArg.defaultVal = substArg;
        if (!setCurrentArg(workItem.decl, substArg))
            return WorkResult::Failed;
        solvedArg.state = SolvedArgState::Default;
        return WorkResult::Solved;
    }

    // Solve one hidden witness slot. A requirement such as `T : IFoo` records
    // the subtype witness for the current `T`, while pack and coercion
    // constraints record their specialized witness values.
    WorkResult solveWitness(SolverWorkItem const& workItem)
    {
        if (hasBlockersForWitness(workItem.decl))
            return WorkResult::Blocked;
        Val* oldWitnessArg = getSolvedWitnessArg(workItem.decl);
        if (!findWitnessForConstraint(workItem.genericDecl, workItem.decl))
            return WorkResult::Failed;

        Val* newWitnessArg = getSolvedWitnessArg(workItem.decl);
        if (!newWitnessArg)
            return WorkResult::Failed;
        if (oldWitnessArg && oldWitnessArg->equals(newWitnessArg))
            return WorkResult::Done;

        return WorkResult::Solved;
    }

    // Replay a dependent ordinary constraint through the current partial
    // generic application. A constraint written as `V = U` or `V = T.A` should
    // see the latest answer for `U` or the latest witness used to open `T.A`.
    bool substitutePotentiallyDependentConstraint(Constraint& c)
    {
        if (!c.potentiallyDependent)
            return true;

        // Potentially dependent ordinary constraints are replayed through the
        // current partial substitution.  The work-list dependency check has already
        // ensured that any parameter or witness mentioned by `c.val` has an
        // answer, so this substitution can turn projections like `T.A` into
        // the concrete value that should constrain the target slot.
        auto genSubst = m_astBuilder->getGenericAppDeclRef(
            m_genericDeclRef,
            m_args[c.decl->parentDecl].getArrayView().arrayView);
        auto newVal = c.val->substitute(m_astBuilder, SubstitutionSet(genSubst));
        if (newVal)
            c.val = newVal;
        return true;
    }

    // Merge one ordinary constraint into a type parameter slot. Non-pack
    // parameters store a single joined type, while type packs store one entry
    // per pack element.
    WorkResult solveTypeParamConstraint(Constraint& c, GenericTypeParamDeclBase* typeParam)
    {
        SLANG_ASSERT(typeParam->parameterIndex != -1);

        if (getArgState(typeParam) == SolvedArgState::Known)
        {
            c.satisfied = true;
            return WorkResult::Done;
        }

        bool isPack = as<GenericTypePackParamDecl>(typeParam) != nullptr;
        auto& solvedArg = m_solvedArgsInfo[typeParam];
        auto oldState = solvedArg.state;
        auto oldArg = getCurrentArg(typeParam);
        auto& types = solvedArg.types;
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

        QualType& type = *ptype;
        auto cType = QualType(as<Type>(c.val), c.isUsedAsLValue);
        SLANG_RELEASE_ASSERT(cType);

        if (!type)
        {
            if (c.isGenericParamConformance && !c.isEquality)
            {
                c.satisfied = true;
                return WorkResult::Done;
            }
            type = cType;
            solvedArg.priority = c.priority;
        }
        else if (!mergeTypeConstraint(type, solvedArg.priority, c, cType))
        {
            if (!c.isGenericParamConformance)
                return WorkResult::Failed;
        }

        if (c.priority < solvedArg.priority)
            solvedArg.priority = c.priority;

        Val* arg = isPack ? getTypePackParamArg(typeParam) : type.type;
        if (arg)
        {
            if (!setCurrentArg(typeParam, arg))
                return WorkResult::Failed;
            solvedArg.state = SolvedArgState::Solved;
        }

        c.satisfied = true;
        if (!arg)
            return WorkResult::Done;
        if (oldState != SolvedArgState::Solved)
            return WorkResult::Solved;
        if (!oldArg || !oldArg->equals(arg))
            return WorkResult::Solved;
        return WorkResult::Done;
    }

    // Combine a newly inferred type with the existing type for a slot. The join
    // handles cases like `int` and `float` choosing a common type, while
    // priority decides which side wins when no join is available.
    bool mergeTypeConstraint(
        QualType& ioType,
        ConstraintPriority currentPriority,
        Constraint const& c,
        QualType cType)
    {
        auto joinType = m_visitor->TryJoinTypes(&m_system, ioType, cType);
        if (!joinType)
        {
            if (c.priority < currentPriority)
                joinType = cType;
            else if (c.priority > currentPriority)
                joinType = ioType;
            else if (c.isEquality)
                joinType = ioType;
            else
                return false;
        }

        ioType = QualType(joinType, ioType.isLeftValue || cType.isLeftValue);
        return true;
    }

    // Record the value for a generic value pack. A concrete pack like
    // `[1, 2, 3]` is kept as a pack value, while a forwarded pack parameter is
    // kept as its decl-ref value.
    WorkResult solveValuePackParamConstraint(Constraint& c, GenericValuePackParamDecl* valPackParam)
    {
        SLANG_ASSERT(valPackParam->parameterIndex != -1);

        if (getArgState(valPackParam) == SolvedArgState::Known)
        {
            c.satisfied = true;
            return WorkResult::Done;
        }

        auto& solvedArg = m_solvedArgsInfo[valPackParam];
        auto oldState = solvedArg.state;
        auto oldArg = getCurrentArg(valPackParam);
        Val* val =
            solvedArg.state == SolvedArgState::Solved ? getCurrentArg(valPackParam) : nullptr;

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

        if (val)
        {
            if (!setCurrentArg(valPackParam, val))
                return WorkResult::Failed;
            solvedArg.state = SolvedArgState::Solved;
        }

        c.satisfied = true;
        if (!val)
            return WorkResult::Done;
        if (oldState != SolvedArgState::Solved)
            return WorkResult::Solved;
        if (!oldArg || !oldArg->equals(val))
            return WorkResult::Solved;
        return WorkResult::Done;
    }

    // Merge one integer/value constraint into a value parameter slot. For
    // `Foo<int N>(N)`, this records the inferred `IntVal`; if two same-priority
    // constraints disagree, the candidate fails.
    WorkResult solveValueParamConstraint(Constraint& c, GenericValueParamDecl* valParam)
    {
        SLANG_ASSERT(valParam->parameterIndex != -1);

        if (getArgState(valParam) == SolvedArgState::Known)
        {
            c.satisfied = true;
            return WorkResult::Done;
        }

        auto& solvedArg = m_solvedArgsInfo[valParam];
        auto oldState = solvedArg.state;
        auto oldArg = getCurrentArg(valParam);
        auto oldPriority = solvedArg.priority;
        Val* val = solvedArg.state == SolvedArgState::Solved ? getCurrentArg(valParam) : nullptr;
        ConstraintPriority& valPriority = solvedArg.priority;

        auto cVal = as<IntVal>(c.val);
        SLANG_RELEASE_ASSERT(cVal);

        if (!val || (c.priority < valPriority))
        {
            val = cVal;
            valPriority = c.priority;
        }
        else if (valPriority == c.priority && !val->equals(cVal))
        {
            return WorkResult::Failed;
        }

        if (val && !setCurrentArg(valParam, val))
            return WorkResult::Failed;
        solvedArg.state = SolvedArgState::Solved;

        c.satisfied = true;
        if (oldState != SolvedArgState::Solved || oldPriority != valPriority)
            return WorkResult::Solved;
        if (!oldArg || !oldArg->equals(val))
            return WorkResult::Solved;
        return WorkResult::Done;
    }

    // Check whether a witness requirement mentions unsolved parameter slots.
    // A `T : IFoo<U>` witness waits for `T` and `U`, and a non-empty-pack
    // witness waits for the referenced pack parameter to be representable.
    bool hasBlockersForWitness(Decl* constraintDecl)
    {
        if (auto genericTypeConstraintDecl = as<GenericTypeConstraintDecl>(constraintDecl))
        {
            return hasBlockersForVal(genericTypeConstraintDecl->sub.type) ||
                   hasBlockersForVal(genericTypeConstraintDecl->sup.type);
        }
        if (auto typeCoercionConstraintDecl = as<TypeCoercionConstraintDecl>(constraintDecl))
        {
            return hasBlockersForVal(typeCoercionConstraintDecl->fromType.type) ||
                   hasBlockersForVal(typeCoercionConstraintDecl->toType.type);
        }
        if (auto nonEmptyConstraintDecl = as<NonEmptyPackConstraintDecl>(constraintDecl))
        {
            if (auto declRefExpr = as<DeclRefExpr>(nonEmptyConstraintDecl->packExpr))
                return !isSlotReady(getDeclRef(m_astBuilder, declRefExpr).getDecl());
            return true;
        }
        if (auto hasDiffTypeInfoConstraintDecl = as<HasDiffTypeInfoConstraintDecl>(constraintDecl))
        {
            return hasBlockersForVal(hasDiffTypeInfoConstraintDecl->type.type);
        }
        return false;
    }

    // Check whether a value still depends on an unsolved slot. The optional
    // subject parameter lets a conformance requirement such as `T : IFoo`
    // inspect its own witness shape without treating the constraint on `T` as a
    // blocker.
    bool hasBlockersForVal(Val* val, Decl* subjectParamDecl = nullptr)
    {
        auto& dependencies = getValDependencies(val);
        for (auto dependencyDecl : dependencies)
        {
            if (!isDependencyReady(dependencyDecl, subjectParamDecl))
                return true;
        }
        return false;
    }

    // Return the structural generic slots mentioned by a value. The shared cache
    // stores only syntax-level dependencies, so `T.A` has the same entry before
    // and after `T` is solved, and the same walk can be reused by later
    // specializations. `Dictionary` is backed by `unordered_dense::map`, where
    // insertion can relocate values, so callers must treat the returned
    // reference as short-lived: inspect it immediately and do not store it or
    // use it across any call that could populate this cache for another value.
    ShortList<Decl*>& getValDependencies(Val* val)
    {
        auto shared = m_visitor->getShared();
        if (!val)
            return shared->m_emptyGenericSolverValDependencies;

        if (auto cachedDependencies = shared->m_genericSolverValDependencyCache.tryGetValue(val))
            return *cachedDependencies;

        ShortList<Decl*> dependencies;
        HashSet<Val*> visitedVals;
        collectValDependencies(val, visitedVals, dependencies);
        shared->m_genericSolverValDependencyCache.add(val, _Move(dependencies));
        return *shared->m_genericSolverValDependencyCache.tryGetValue(val);
    }

    // Walk a value tree and collect the generic slots it mentions. For example,
    // an associated-type lookup like `T.A` mentions the type slot `T` and also
    // the witness slot used to prove `T : IFoo` before selecting `A`. The walk
    // deliberately does not ask whether those slots belong to this solver; that
    // relevance check happens later when the current specialization tests
    // whether a dependency is ready.
    void collectValDependencies(
        Val* val,
        HashSet<Val*>& ioVisitedVals,
        ShortList<Decl*>& ioDependencies)
    {
        if (!val || ioVisitedVals.contains(val))
            return;
        ioVisitedVals.add(val);

        // The dependency protocol is local to decl-ref nodes. A `DeclRefType`,
        // `DeclRefIntVal`, or `DeclaredSubtypeWitness` carries its decl-ref as a
        // value operand, so the recursive walk reaches this one branch for type
        // parameters, value parameters, and hidden witness constraints alike.
        if (auto declRef = as<DeclRefBase>(val))
        {
            auto decl = declRef->getDecl();
            if (as<GenericTypeParamDeclBase>(decl))
                addValDependency(ioDependencies, decl);
            else if (isGenericValueParam(decl))
                addValDependency(ioDependencies, decl);
            else if (isTypeConstraintDecl(decl))
                addValDependency(ioDependencies, decl);
        }

        for (auto operand : val->m_operands)
        {
            if (operand.kind == ValNodeOperandKind::ValNode)
                collectValDependencies(operand.getVal(), ioVisitedVals, ioDependencies);
        }
    }

    // Add a dependency once so cached dependency lists stay compact. A value can
    // mention the same slot through several operands, but one entry is enough to
    // decide whether the value still has an unsolved blocker.
    void addValDependency(ShortList<Decl*>& ioDependencies, Decl* dependencyDecl)
    {
        if (dependencyDecl && !ioDependencies.contains(dependencyDecl))
            ioDependencies.add(dependencyDecl);
    }

    // Decide whether a cached dependency still prevents progress. Parameter and
    // witness dependencies both use the same slot-state readiness; a constraint
    // targeting the subject parameter is treated as ready for self-conformance
    // inference.
    bool isDependencyReady(Decl* dependencyDecl, Decl* subjectParamDecl)
    {
        if (!dependencyDecl || dependencyDecl == subjectParamDecl)
            return true;

        if (isConstraintForGenericParam(dependencyDecl, subjectParamDecl))
            return true;
        return isSlotReady(dependencyDecl);
    }

    // Recognize a generic type constraint whose subject is the given parameter.
    // This is the cached-dependency form of the `T : IFoo` self-conformance
    // special case used while inferring from a parameter's own conformance
    // requirement.
    bool isConstraintForGenericParam(Decl* constraintDecl, Decl* paramDecl)
    {
        if (!paramDecl)
            return false;

        auto typeConstraintDecl = as<GenericTypeConstraintDecl>(constraintDecl);
        if (!typeConstraintDecl)
            return false;

        auto subDeclRef = isDeclRefTypeOf<Decl>(typeConstraintDecl->sub.type);
        return subDeclRef && subDeclRef.getDecl() == paramDecl;
    }

    // Decide whether a generic-application slot can be used in a partial
    // substitution. Visible parameters and hidden witnesses share the same
    // solved-state test; the only extra work here is deciding whether the slot
    // belongs to this solver. Slots from outer or unrelated generics are already
    // fixed by their own substitutions and do not block this one.
    bool isSlotReady(Decl* slotDecl)
    {
        if (!slotDecl)
            return true;

        if (getGenericParamIndex(slotDecl) >= 0 || isTypeConstraintDecl(slotDecl))
        {
            if (!isGenericDeclInCurrentSolution(slotDecl->parentDecl))
                return true;
            return isReadyArgState(getArgState(slotDecl));
        }

        return true;
    }

    // Return a visible argument supplied by the caller, if one exists for this
    // parameter. Generic applications may pass the parameter itself, such as
    // `<T>` or `<N>`, while checking a generic declaration in its own scope.
    // Those identity arguments are still caller-provided arguments, so this
    // helper returns them and lets the initialization pass record that they are
    // known but still dependent.
    bool tryGetKnownGenericArg(Decl* paramDecl, Val*& outArg)
    {
        // Some callers seed inference with a default substitution such as
        // `<T, N, T : IFoo>`.  Those self-reference values keep decl-ref shapes
        // well formed, but they are not explicit user answers; the work list
        // must still be allowed to replace them with inferred arguments.
        auto genericDecl = as<GenericDecl>(paramDecl->parentDecl);
        if (!genericDecl || genericDecl != m_genericDecls[0])
            return false;

        Index knownArgIndex = getGenericParamIndex(paramDecl);
        if (knownArgIndex < 0 || knownArgIndex >= m_knownGenericArgs.getCount())
            return false;

        outArg = m_knownGenericArgs[knownArgIndex];
        return true;
    }

    // Map an ordinary generic parameter declaration to its visible parameter
    // index. Hidden witness arguments deliberately have no index here because
    // the solver fills them through type-constraint work items.
    Index getGenericParamIndex(Decl* genericParamDecl)
    {
        if (auto typeParamDecl = as<GenericTypeParamDeclBase>(genericParamDecl))
            return typeParamDecl->parameterIndex;
        if (auto valuePackParamDecl = as<GenericValuePackParamDecl>(genericParamDecl))
            return valuePackParamDecl->parameterIndex;
        if (auto valueParamDecl = as<GenericValueParamDecl>(genericParamDecl))
            return valueParamDecl->parameterIndex;
        return -1;
    }

    // Test whether an argument is the identity placeholder for the same
    // parameter. Examples are `DeclRefType(T)` for a type parameter `T` and a
    // `DeclRefIntVal(N)` for a value parameter `N`.
    bool isSelfReferencePlaceholder(Decl* paramDecl, Val* arg)
    {
        if (auto declRefType = as<DeclRefType>(arg))
            return declRefType->getDeclRef().getDecl() == paramDecl;
        if (auto intVal = as<IntVal>(arg))
        {
            if (auto declRefIntVal = getDeclRefIntValUnderCast(intVal))
                return declRefIntVal->getDeclRef().getDecl() == paramDecl;
        }
        return false;
    }

    // Read the recorded state for a slot. Missing metadata means the current
    // `m_args` value is still only the initial placeholder.
    SolvedArgState getArgState(Decl* slotDecl)
    {
        auto solvedInfo = m_solvedArgsInfo.tryGetValue(slotDecl);
        return solvedInfo ? solvedInfo->state : SolvedArgState::Placeholder;
    }

    // Update only the state metadata for a slot. The value itself is stored by
    // `setCurrentArg()`, which keeps all generic-application arguments in one
    // declaration-order array.
    void setArgState(Decl* slotDecl, SolvedArgState state)
    {
        m_solvedArgsInfo[slotDecl].state = state;
    }

    // Dependencies can proceed once a slot has some stable substitution value.
    // Concrete parameter solutions, defaults, omitted packs, and solved
    // witnesses are all ready. Dependent known placeholders are also ready
    // because they came from the caller and there may be no more local work
    // that can refine them.
    bool isReadyArgState(SolvedArgState state)
    {
        switch (state)
        {
        case SolvedArgState::Known:
        case SolvedArgState::KnownDependent:
        case SolvedArgState::Solved:
        case SolvedArgState::Default:
        case SolvedArgState::OmittedPack:
        case SolvedArgState::Witness:
            return true;
        case SolvedArgState::Placeholder:
            return false;
        }
        return false;
    }

    // The solver only blocks on slots that belong to the generic declaration
    // chain whose arguments it is currently building. A cached dependency may
    // mention a parameter or witness from some outer, already-substituted scope;
    // those declarations are considered ready here because this solver has no
    // work item that can change them.
    bool isGenericDeclInCurrentSolution(Decl* maybeGenericDecl)
    {
        auto genericDecl = as<GenericDecl>(maybeGenericDecl);
        return genericDecl && m_args.containsKey(genericDecl);
    }

    // Defaults should only run while a parameter is still replaceable. Explicit
    // arguments and ordinary inference are non-default solutions; dependent
    // placeholders, omitted packs, and previous defaults are still replaceable.
    bool hasNonDefaultSolution(Decl* paramDecl)
    {
        auto state = getArgState(paramDecl);
        return state == SolvedArgState::Known || state == SolvedArgState::Solved;
    }

    // Return the solved hidden witness value for one constraint slot, if that
    // work item has already proved the requirement.
    Val* getSolvedWitnessArg(Decl* constraintDecl)
    {
        if (getArgState(constraintDecl) != SolvedArgState::Witness)
            return nullptr;
        return getCurrentArg(constraintDecl);
    }

    // Return the current serialized argument for one solved slot.
    Val* getCurrentArg(Decl* slotDecl)
    {
        auto genericDecl = as<GenericDecl>(slotDecl->parentDecl);
        if (!genericDecl)
            return nullptr;

        auto args = m_args.tryGetValue(genericDecl);
        if (!args)
            return nullptr;
        return getCurrentArg(slotDecl, *args);
    }

    // Return the current serialized argument for one solved slot from an
    // already-selected argument list.
    Val* getCurrentArg(Decl* slotDecl, ShortList<Val*> const& args)
    {
        Index argIndex = getSerializedArgIndex(slotDecl);
        if (argIndex < 0 || argIndex >= args.getCount())
            return nullptr;
        return args[argIndex];
    }

    // Store a solved value into the serialized argument list for its owning
    // generic declaration. Type/value parameters use their declaration
    // `parameterIndex`; hidden witnesses use the position they occupy after all
    // visible arguments in the generic application.
    bool setCurrentArg(Decl* slotDecl, Val* arg)
    {
        auto genericDecl = as<GenericDecl>(slotDecl->parentDecl);
        if (!genericDecl || !arg)
            return false;

        Index argIndex = getSerializedArgIndex(slotDecl);
        if (argIndex < 0)
            return false;

        auto& args = m_args[genericDecl];
        if (args.getCount() <= argIndex)
            args.setCount(argIndex + 1);
        args[argIndex] = arg;
        return true;
    }

    // Record that a slot has a solved answer and store that answer in the
    // serialized argument list.
    bool setSolvedCurrentArg(Decl* slotDecl, Val* arg)
    {
        setArgState(slotDecl, SolvedArgState::Witness);
        return setCurrentArg(slotDecl, arg);
    }

    // Map a declaration that owns a generic-application slot to the index it
    // occupies in `m_args`.
    Index getSerializedArgIndex(Decl* slotDecl)
    {
        Index paramIndex = getGenericParamIndex(slotDecl);
        if (paramIndex >= 0)
            return paramIndex;

        auto genericDecl = as<GenericDecl>(slotDecl->parentDecl);
        if (!genericDecl || !isTypeConstraintDecl(slotDecl))
            return -1;

        Index argIndex = 0;
        for (auto member : genericDecl->getDirectMemberDecls())
        {
            if (as<GenericTypeParamDeclBase>(member) || as<GenericValueParamDecl>(member) ||
                as<GenericValuePackParamDecl>(member))
            {
                argIndex++;
            }
        }

        for (auto member : genericDecl->getDirectMemberDecls())
        {
            if (!isTypeConstraintDecl(member))
                continue;
            if (member == slotDecl)
                return argIndex;
            argIndex++;
        }

        return -1;
    }

    // Confirm that the live argument arrays no longer contain unresolved
    // required slots. The solver keeps `m_args` up to date as work items solve,
    // so finalization does not rebuild anything; it only checks that each
    // visible slot is in a ready state and that every local hidden requirement
    // has recorded its witness.
    bool validateFinalArgs()
    {
        for (auto genericDecl : m_genericDecls)
        {
            if (!m_args.containsKey(genericDecl))
                return false;

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
                    if (isSlotReady(member))
                        continue;
                    return false;
                }
            }

            for (auto member : genericDecl->getDirectMemberDecls())
            {
                if (!isTypeConstraintDecl(member))
                    continue;
                if (!isSlotReady(member))
                    return false;
            }
        }
        return true;
    }

    // Serialize a type-pack slot. An unsolved pack becomes an empty type pack,
    // while a solved pack is rebuilt from the per-element type constraints.
    Val* getTypePackParamArg(GenericTypeParamDeclBase* typeParam)
    {
        auto solvedArg = m_solvedArgsInfo.tryGetValue(typeParam);
        if (!solvedArg)
            return m_astBuilder->getTypePack(ArrayView<Type*>());

        auto& types = solvedArg->types;
        for (auto t : types)
        {
            if (!t)
                return nullptr;
        }

        if (types.getCount() == 1 && isTypePack(types[0]))
            return types[0].type;

        ShortList<Type*> typeList;
        for (auto t : types)
            typeList.add(t.type);
        return m_astBuilder->getTypePack(typeList.getArrayView().arrayView);
    }

    // Build a decl-ref for a generic member using the current serialized
    // arguments. Passing a constraint member produces the substituted decl-ref
    // for that witness slot; passing `inner` produces the solved generic body.
    DeclRef<Decl> getSubstDeclRef(Decl* constraintDecl)
    {
        auto constraintParent = as<GenericDecl>(constraintDecl->parentDecl);
        DeclRef<Decl> substDeclRef;

        for (auto genericDecl : m_genericDecls)
        {
            DeclRef<GenericDecl> genericDeclRef =
                substDeclRef ? substDeclRef.as<GenericDecl>() : getRootGenericDeclRef(genericDecl);

            substDeclRef = m_astBuilder->getGenericAppDeclRef(
                genericDeclRef,
                m_args[genericDecl].getArrayView().arrayView,
                genericDecl == constraintParent ? constraintDecl : genericDecl->inner);

            if (genericDecl == constraintParent)
                break;
        }
        return substDeclRef;
    }

    // Choose the root decl-ref used to form a nested generic application.
    // Direct generic refs can use a fresh direct ref, while lookup/member refs
    // must preserve the original lookup path.
    DeclRef<GenericDecl> getRootGenericDeclRef(GenericDecl* genericDecl)
    {
        if (as<DirectDeclRef>(m_genericDeclRef.declRefBase))
            return makeDeclRef(genericDecl);
        return m_genericDeclRef;
    }

    // Find the proof for one hidden witness requirement and record it in the
    // solved argument list. The dispatch is by constraint kind, but every
    // branch answers the same question: what witness value satisfies this
    // where-clause under the current generic arguments?
    bool findWitnessForConstraint(GenericDecl* genericDecl, Decl* constraintDecl)
    {
        if (auto genericTypeConstraintDecl = as<GenericTypeConstraintDecl>(constraintDecl))
            return findSubtypeWitnessForConstraint(genericDecl, genericTypeConstraintDecl);
        if (auto typeCoercionConstraintDecl = as<TypeCoercionConstraintDecl>(constraintDecl))
            return findTypeCoercionWitnessForConstraint(genericDecl, typeCoercionConstraintDecl);
        if (auto nonEmptyConstraintDecl = as<NonEmptyPackConstraintDecl>(constraintDecl))
            return findNonEmptyPackWitnessForConstraint(genericDecl, nonEmptyConstraintDecl);
        if (auto hasDiffTypeInfoConstraintDecl = as<HasDiffTypeInfoConstraintDecl>(constraintDecl))
            return findDiffTypeInfoWitnessForConstraint(genericDecl, hasDiffTypeInfoConstraintDecl);
        return true;
    }

    // Find a witness for a subtype/equality requirement such as `T : IFoo` or
    // `A == B`. The substituted constraint decl-ref gives the witness the same
    // generic arguments that will appear in the final application.
    bool findSubtypeWitnessForConstraint(
        GenericDecl* genericDecl,
        GenericTypeConstraintDecl* constraintDecl)
    {
        auto constraintDeclRef = getSubstDeclRef(constraintDecl).as<GenericTypeConstraintDecl>();
        auto sub = getSub(m_astBuilder, constraintDeclRef);
        auto sup = getSup(m_astBuilder, constraintDeclRef);

        if (!recordConstrainedGenericParam(constraintDeclRef))
            return false;
        if (sub->equals(sup) && isDeclRefTypeOf<InterfaceDecl>(sup))
            return false;

        // Subtype witness construction belongs to `isSubtype()`. That path owns
        // facet linearization, generic-parameter conformances, associated-type
        // constraints, concrete conformances, and cached answers; the solver
        // only supplies the fully substituted `sub` and `sup` for this witness
        // slot.
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
        if (m_system.additionalSubtypeWitnesses && sub == m_system.subTypeForAdditionalWitnesses &&
            !isDeclRefTypeOf<InterfaceDecl>(sub))
        {
            m_system.additionalSubtypeWitnesses->tryGetValue(sup, subTypeWitness);
        }
        else
        {
            subTypeWitness = m_visitor->isSubtype(
                sub,
                sup,
                m_system.additionalSubtypeWitnesses ? IsSubTypeOptions::NoCaching
                                                    : IsSubTypeOptions::None);
        }

        if (constraintDecl->isEqualityConstraint && !isTypeEqualityWitness(subTypeWitness))
            subTypeWitness = nullptr;
        return recordSubtypeWitnessArg(genericDecl, constraintDecl, subTypeWitness);
    }

    // Record that a type parameter is constrained by a subtype requirement so
    // overload ranking does not charge it as unconstrained. `each T : IFoo`
    // records the element parameter inside the pack.
    bool recordConstrainedGenericParam(DeclRef<GenericTypeConstraintDecl> constraintDeclRef)
    {
        return recordConstrainedGenericParamFromType(constraintDeclRef.getDecl()->sub.type);
    }

    // Record a raw constraint-side type parameter for overload ranking. The
    // final witness is computed from substituted types, but the specificity
    // cost asks a different question: did the generic declaration write a
    // where-clause that constrains this parameter at all?
    bool recordConstrainedGenericParamFromType(Val* typeVal)
    {
        if (auto declRefType = as<DeclRefType>(typeVal))
        {
            markConstrainedGenericParam(declRefType->getDeclRef().getDecl());
            return true;
        }

        if (auto eachType = as<EachType>(typeVal))
        {
            if (auto elementDeclRefType = eachType->getElementDeclRefType())
            {
                markConstrainedGenericParam(elementDeclRefType->getDeclRef().getDecl());
                return true;
            }
            return false;
        }

        return true;
    }

    // Type-coercion requirements constrain any generic parameter that appears
    // directly on either side of the conversion. Recording that fact here keeps
    // overload ranking local to the solver instead of hiding it inside the
    // shared helper that only knows how to append the witness argument.
    bool recordConstrainedGenericParams(TypeCoercionConstraintDecl* constraintDecl)
    {
        return recordConstrainedGenericParamFromType(constraintDecl->fromType.type) &&
               recordConstrainedGenericParamFromType(constraintDecl->toType.type);
    }

    // Differentiation metadata requirements also make a type parameter
    // meaningfully constrained. The raw declaration type is inspected rather
    // than the substituted type, because after solving `T` may already be
    // `float`, and the ranking rule still needs to remember that the generic
    // declaration constrained `T`.
    bool recordConstrainedGenericParam(HasDiffTypeInfoConstraintDecl* constraintDecl)
    {
        return recordConstrainedGenericParamFromType(constraintDecl->type.type);
    }

    // Remember that a generic type parameter appeared on the subject side of a
    // where-clause style requirement. This is not a solved argument value, but
    // it is still metadata about the same parameter slot, so it lives beside
    // the slot's solution state in `m_solvedArgsInfo`.
    void markConstrainedGenericParam(Decl* genericParamDecl)
    {
        if (!genericParamDecl)
            return;
        m_solvedArgsInfo[genericParamDecl].isConstrainedGenericParam = true;
    }

    // Read the overload-ranking bit recorded by `markConstrainedGenericParam`.
    // Missing solved-arg info means no where-clause requirement has marked this
    // parameter, so the old unconstrained-parameter penalty still applies.
    bool isConstrainedGenericParam(Decl* genericParamDecl)
    {
        auto solvedInfo = m_solvedArgsInfo.tryGetValue(genericParamDecl);
        return solvedInfo && solvedInfo->isConstrainedGenericParam;
    }

    // Store a subtype witness answer. Optional constraints can record
    // `NoneWitness` when no real proof exists; required constraints fail
    // instead. Overload cost is read from the final witness arguments after the
    // work list is quiet, so retried witness items cannot double-count it.
    bool recordSubtypeWitnessArg(
        GenericDecl* genericDecl,
        GenericTypeConstraintDecl* constraintDecl,
        SubtypeWitness* subTypeWitness)
    {
        SLANG_UNUSED(genericDecl);

        bool witnessIsOptional = m_visitor->isWitnessUncheckedOptional(subTypeWitness);
        bool constraintIsOptional = constraintDecl->hasModifier<OptionalConstraintModifier>();

        if (subTypeWitness && (!witnessIsOptional || constraintIsOptional))
        {
            if (!setSolvedCurrentArg(constraintDecl, subTypeWitness))
                return false;
            return true;
        }

        if (!subTypeWitness && constraintIsOptional)
        {
            auto noneWitness = m_astBuilder->getOrCreate<NoneWitness>();
            if (!setSolvedCurrentArg(constraintDecl, noneWitness))
                return false;
            return true;
        }

        return false;
    }

    // Find a hidden type-coercion witness, then record it in the solved
    // argument list used to substitute later hidden constraints in this same
    // generic application.
    bool findTypeCoercionWitnessForConstraint(
        GenericDecl* genericDecl,
        TypeCoercionConstraintDecl* constraintDecl)
    {
        if (!recordConstrainedGenericParams(constraintDecl))
            return false;

        auto& genericArgs = m_args[genericDecl];
        TypeCoercionWitness* witness = nullptr;
        if (!Slang::findTypeCoercionWitnessForConstraint(
                m_astBuilder,
                m_visitor,
                constraintDecl,
                m_genericDeclRef,
                nullptr,
                genericArgs.getArrayView().arrayView,
                witness,
                false))
            return false;

        if (!setSolvedCurrentArg(constraintDecl, witness))
            return false;
        return true;
    }

    // Find a non-empty-pack witness once the referenced pack argument is known
    // and proven non-empty. For an omitted pack this fails, while a pack with at
    // least one element records a `NonEmptyPackWitness`.
    bool findNonEmptyPackWitnessForConstraint(
        GenericDecl* genericDecl,
        NonEmptyPackConstraintDecl* constraintDecl)
    {
        Decl* constrainedPackDecl = nullptr;
        if (auto declRefExpr = as<DeclRefExpr>(constraintDecl->packExpr))
            constrainedPackDecl = getDeclRef(m_astBuilder, declRefExpr).getDecl();

        Val* constrainedArg = nullptr;
        if (auto typePackDecl = as<GenericTypePackParamDecl>(constrainedPackDecl))
        {
            if (typePackDecl->parameterIndex < m_args[genericDecl].getCount())
                constrainedArg = m_args[genericDecl][typePackDecl->parameterIndex];
        }
        else if (auto valuePackDecl = as<GenericValuePackParamDecl>(constrainedPackDecl))
        {
            if (valuePackDecl->parameterIndex < m_args[genericDecl].getCount())
                constrainedArg = m_args[genericDecl][valuePackDecl->parameterIndex];
        }

        if (!constrainedArg || !m_visitor->isKnownNonEmptyPack(constrainedArg))
            return false;

        if (!setSolvedCurrentArg(
                constraintDecl,
                m_astBuilder->getNonEmptyPackWitness(constrainedArg)))
            return false;
        return true;
    }

    // Find a differentiation type-info witness, then record it in the solved
    // argument list used to substitute later hidden constraints in this same
    // generic application.
    bool findDiffTypeInfoWitnessForConstraint(
        GenericDecl* genericDecl,
        HasDiffTypeInfoConstraintDecl* constraintDecl)
    {
        if (!recordConstrainedGenericParam(constraintDecl))
            return false;

        auto& genericArgs = m_args[genericDecl];
        Witness* witness = nullptr;
        if (!Slang::findDiffTypeInfoWitnessForConstraint(
                m_astBuilder,
                m_visitor,
                constraintDecl,
                m_genericDeclRef,
                nullptr,
                genericArgs.getArrayView().arrayView,
                witness,
                false))
            return false;

        if (!setSolvedCurrentArg(constraintDecl, witness))
            return false;
        return true;
    }

    // Confirm that all ordinary unification constraints reached a terminal
    // state. Hidden witnesses are verified by final serialization, which fails
    // if any required witness slot remains empty.
    bool verifyOrdinaryConstraintsSatisfied()
    {
        for (auto const& workItem : m_solverWorkItems)
        {
            if (workItem.kind != SolverWorkItem::Kind::OrdinaryConstraint)
                continue;
            if (!workItem.constraint.satisfied)
                return false;
        }
        return true;
    }

    // Add the overload penalty for type parameters that were not constrained by
    // any where-clause requirement. This preserves the old ranking rule where a
    // more constrained generic candidate is preferred.
    void addUnconstrainedGenericParamCost()
    {
        for (auto genericDecl : m_genericDecls)
        {
            for (auto typeParamDecl : genericDecl->getMembersOfType<GenericTypeParamDecl>())
            {
                if (!isConstrainedGenericParam(typeParamDecl))
                    m_outBaseCost += kConversionCost_UnconstraintGenericParam;
            }
        }
    }

    // Compute witness cost once from the final solved witness slots. Witness
    // work items may be retried after unrelated progress because the solver
    // wakes conservatively, so cost must not be accumulated as a retry-time
    // side effect. The final argument list already contains exactly the witness
    // values that overload resolution should rank.
    ConversionCost getFinalWitnessCost()
    {
        ConversionCost cost = kConversionCost_None;
        for (auto const& workItem : m_solverWorkItems)
        {
            if (workItem.kind != SolverWorkItem::Kind::Witness)
                continue;
            if (!as<GenericTypeConstraintDecl>(workItem.decl))
                continue;

            auto witnessArg = getSolvedWitnessArg(workItem.decl);
            if (as<NoneWitness>(witnessArg))
                cost += kConversionCost_FailedOptionalConstraint;
            else if (auto subTypeWitness = as<SubtypeWitness>(witnessArg))
                cost += subTypeWitness->getOverloadResolutionCost();
        }
        return cost;
    }

private:
    // Semantic visitor used for all checker operations that need broader
    // context: unification, subtype checks, default lookup, diagnostics, and
    // access to shared semantic caches.
    SemanticsVisitor* m_visitor = nullptr;

    // AST builder paired with `m_visitor`. The solver creates substituted
    // decl-refs, placeholder witnesses, packs, and canonical values through this
    // builder so all generated values belong to the active AST arena.
    ASTBuilder* m_astBuilder = nullptr;

    // Shared unification context owned by the solver while it runs. Its
    // `constraints` list is only a transient inbox for helpers that cannot know
    // about `SolverWorkItem`; every pending constraint is drained into
    // `m_solverWorkItems` before the work-list loop observes it.
    ConstraintSystem m_system;

    // Decl-ref for the generic being specialized. The solver preserves this
    // lookup path when it forms substituted decl-refs for the generic body or
    // for hidden constraint declarations.
    DeclRef<GenericDecl> m_genericDeclRef;

    // Visible generic arguments already supplied at the call site. These are
    // stored as an array view because the caller owns the backing list for the
    // duration of solving.
    ArrayView<Val*> m_knownGenericArgs;

    // Output overload cost owned by the caller. The solver writes the final
    // accumulated cost after all ordinary arguments and witnesses are solved.
    ConversionCost& m_outBaseCost;

    // Nested generic declarations from outermost to innermost. A substituted
    // decl-ref for `Outer<T>.Inner<U>` consumes arguments for `Outer` before it
    // can name `Inner`, so the solver stores the declarations in that order.
    List<GenericDecl*> m_genericDecls;

    // Live generic-application arguments keyed by generic declaration. This is
    // the declaration-order view consumed by `getGenericAppDeclRef`: visible
    // type/value arguments first, then hidden witness arguments. The arrays
    // begin with placeholders and each work item directly replaces the slot it
    // solves.
    Dictionary<Decl*, ShortList<Val*>> m_args;

    // Solved-argument metadata keyed by the declaration that owns the
    // generic-application slot. Visible type/value parameters, pack parameters,
    // defaulted parameters, and hidden witness constraint declarations all
    // store their metadata here; `m_args` stores the current declaration-order
    // argument values used by substitution.
    Dictionary<Decl*, SolvedArgInfo> m_solvedArgsInfo;

    // Stable storage for all work collected by the solver: ordinary unification
    // constraints, default arguments, and hidden witness requirements. The
    // queue stores indices into this table.
    ShortList<SolverWorkItem, 16> m_solverWorkItems;

    // Append-only queue of work-item indices. New and reawakened items are added
    // at the end, and `m_workListReadIndex` advances through this list.
    ShortList<Index, 16> m_workList;

    // Cursor into `m_workList` for the next queued item to try.
    Index m_workListReadIndex = 0;
};

// Entry point from overload resolution into the generic solver. It prepares the
// generic declaration for lookup, then moves the accumulated constraints into
// `GenericConstraintSystemSolver` so the caller only sees the final substituted
// decl-ref and accumulated overload cost.
DeclRef<Decl> SemanticsVisitor::trySolveConstraintSystem(
    ConstraintSystem&& system,
    DeclRef<GenericDecl> genericDeclRef,
    ArrayView<Val*> knownGenericArgs,
    ConversionCost& outBaseCost)
{
    // The solver reads generic members while collecting conformance
    // requirements, defaults, and witnesses, so the declaration must be ready
    // for lookup before we enter it.
    ensureDecl(genericDeclRef.getDecl(), DeclCheckState::ReadyForLookup);

    // All state for the two-phase algorithm lives in the solver object.  The
    // public entry point stays as a small adapter from semantic checking to the
    // generic-argument solver.
    GenericConstraintSystemSolver
        solver(this, _Move(system), genericDeclRef, knownGenericArgs, outBaseCost);
    return solver.solve();
}

bool SemanticsVisitor::TryUnifyVals(
    ConstraintSystem& constraints,
    ValUnificationContext unifyCtx,
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
                unifyCtx,
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
        auto fstParam = getDeclRefIntValUnderCast(fstInt);
        auto sndParam = getDeclRefIntValUnderCast(sndInt);

        bool okay = false;
        if (fstParam)
            okay |= TryUnifyIntParam(constraints, unifyCtx, fstParam->getDeclRef(), sndInt);
        if (sndParam)
            okay |= TryUnifyIntParam(constraints, unifyCtx, sndParam->getDeclRef(), fstInt);
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
                unifyCtx,
                getSup(m_astBuilder, constraintDecl1),
                getSup(m_astBuilder, constraintDecl2));
        }
    }

    if (auto fstWit = as<TypeCoercionWitness>(fst); fstWit)
    {
        if (auto sndWit = as<TypeCoercionWitness>(snd); sndWit)
        {
            // Ignore unification for coercion constraints for now,
            // they will be checked later anyway.
            //
            return true;
        }
    }

    if (as<TypeEqualityWitness>(fst) && as<DeclaredSubtypeWitness>(snd))
    {
        if (as<DeclaredSubtypeWitness>(snd)->isEquality())
        {
            // Try to unify both the sub and sup types of equality witnesses,
            // but a failure doesn't mean the unification fails, since
            // we could have associated type lookups taht aren't used for
            // inference, but will still be checked for validity in
            // trySolveConstraintSystem.
            //
            TryUnifyTypes(
                constraints,
                unifyCtx,
                as<SubtypeWitness>(snd)->getSub(),
                as<SubtypeWitness>(fst)->getSub());
            TryUnifyTypes(
                constraints,
                unifyCtx,
                as<SubtypeWitness>(snd)->getSup(),
                as<SubtypeWitness>(fst)->getSup());
            return true;
        }
    }

    if (as<DeclaredSubtypeWitness>(fst) && as<TypeEqualityWitness>(snd))
    {
        if (as<DeclaredSubtypeWitness>(fst)->isEquality())
        {
            // Try to unify both the sub and sup types of equality witnesses,
            // but a failure doesn't mean the unification fails, since
            // we could have associated type lookups taht aren't used for
            // inference, but will still be checked for validity in
            // trySolveConstraintSystem.
            //
            TryUnifyTypes(
                constraints,
                unifyCtx,
                as<SubtypeWitness>(snd)->getSub(),
                as<SubtypeWitness>(fst)->getSub());
            TryUnifyTypes(
                constraints,
                unifyCtx,
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
            unifyCtx,
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
    ConstraintSystem& constraints,
    ValUnificationContext unifyCtx,
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
    return tryUnifyGenericAppDeclRef(constraints, unifyCtx, fstGen, fstIsLVal, sndGen, sndIsLVal);
}

bool SemanticsVisitor::tryUnifyGenericAppDeclRef(
    ConstraintSystem& constraints,
    ValUnificationContext unifyCtx,
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
                unifyCtx,
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

    if (!tryUnifyDeclRef(constraints, unifyCtx, fstBase, fstIsLVal, sndBase, sndIsLVal))
    {
        okay = false;
    }

    return okay;
}

bool SemanticsVisitor::TryUnifyTypeParam(
    ConstraintSystem& constraints,
    ValUnificationContext unificationContext,
    GenericTypeParamDeclBase* typeParamDecl,
    QualType type)
{
    // We want to constrain the given type parameter
    // to equal the given type.
    Constraint constraint;
    constraint.decl = typeParamDecl;
    constraint.indexInPack = unificationContext.indexInTypePack;
    constraint.val = type;
    constraint.isUsedAsLValue = type.isLeftValue;
    constraint.priority = unificationContext.optionalConstraint ? ConstraintPriority::Optional
                                                                : ConstraintPriority::Required;
    constraint.isEquality = unificationContext.equalityConstraint;
    constraint.isGenericParamConformance = unificationContext.genericParamConformanceConstraint;
    constraints.constraints.add(constraint);

    return true;
}

bool SemanticsVisitor::TryUnifyIntParam(
    ConstraintSystem& constraints,
    ValUnificationContext unifyCtx,
    GenericValueParamDecl* paramDecl,
    IntVal* val)
{
    SLANG_UNUSED(unifyCtx);

    // We only want to accumulate constraints on
    // the parameters of the declarations being
    // specialized (don't accidentially constrain
    // parameters of a generic function based on
    // calls in its body).
    // if (paramDecl->parentDecl != constraints.genericDecl)
    if (!isRelevantGeneric(constraints, paramDecl->parentDecl))
        return false;

    // We want to constrain the given parameter to equal the given value.
    Constraint constraint;
    constraint.decl = paramDecl;
    // If `val` is of different type than `paramDecl`, we want to insert a type cast.
    if (val->getType() != paramDecl->getType())
    {
        auto cast = m_astBuilder->getTypeCastIntVal(paramDecl->getType(), val);
        val = cast;
    }
    constraint.val = val;
    constraint.isGenericParamConformance = unifyCtx.genericParamConformanceConstraint;

    constraints.constraints.add(constraint);

    return true;
}

bool SemanticsVisitor::TryUnifyIntParam(
    ConstraintSystem& constraints,
    ValUnificationContext unifyCtx,
    DeclRef<VarDeclBase> const& varRef,
    IntVal* val)
{
    if (auto genericValueParamRef = varRef.as<GenericValueParamDecl>())
    {
        return TryUnifyIntParam(constraints, unifyCtx, genericValueParamRef.getDecl(), val);
    }
    else if (auto genericValuePackParamRef = varRef.as<GenericValuePackParamDecl>())
    {
        if (genericValuePackParamRef.getDecl()->parentDecl != constraints.genericDecl)
            return false;
        Constraint constraint;
        constraint.decl = genericValuePackParamRef.getDecl();
        constraint.val = val;
        constraint.priority = unifyCtx.optionalConstraint ? ConstraintPriority::Optional
                                                          : ConstraintPriority::Required;
        constraint.isGenericParamConformance = unifyCtx.genericParamConformanceConstraint;
        constraints.constraints.add(constraint);
        return true;
    }
    else
    {
        return false;
    }
}

bool SemanticsVisitor::TryUnifyFunctorByStructuralMatch(
    ConstraintSystem& constraints,
    ValUnificationContext unifyCtx,
    StructDecl* fstStructDecl,
    FuncType* sndFuncType)
{
    // Here we just need to find an invocation method for our functor
    // to perform unification with.
    // We do not validate the validity of the functor at this step,
    // we only need to perform a reasonable unification so that constraints
    // can correctly solve.
    FuncDecl* functorInvokeMethod =
        as<FuncDecl>(fstStructDecl->findLastDirectMemberDeclOfName(getName("()")));
    if (!functorInvokeMethod)
        return false;

    return TryUnifyFuncTypesByStructuralMatch(
        constraints,
        unifyCtx,
        getFuncType(this->getASTBuilder(), functorInvokeMethod),
        sndFuncType);
}

bool SemanticsVisitor::TryUnifyFuncTypesByStructuralMatch(
    ConstraintSystem& constraints,
    ValUnificationContext unifyCtx,
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
                unifyCtx,
                fstFunType->getParamTypeWithModeWrapper(i),
                sndFunType->getParamTypeWithModeWrapper(i)))
            return false;
    }
    return TryUnifyTypes(
        constraints,
        unifyCtx,
        fstFunType->getResultType(),
        sndFunType->getResultType());
}

bool SemanticsVisitor::TryUnifyTypesByStructuralMatch(
    ConstraintSystem& constraints,
    ValUnificationContext unifyCtx,
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
                    unifyCtx,
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
                    unifyCtx,
                    fstStructDecl.getDecl(),
                    sndFunType);
        }

        if (auto typeParamDecl = as<GenericTypeParamDecl>(fstDeclRef.getDecl()))
            // if (typeParamDecl->parentDecl == constraints.genericDecl)
            if (isRelevantGeneric(constraints, typeParamDecl->parentDecl))
                return TryUnifyTypeParam(constraints, unifyCtx, typeParamDecl, snd);

        if (auto sndDeclRefType = as<DeclRefType>(snd))
        {
            auto sndDeclRef = sndDeclRefType->getDeclRef();

            if (auto typeParamDecl = as<GenericTypeParamDecl>(sndDeclRef.getDecl()))
                if (isRelevantGeneric(constraints, typeParamDecl->parentDecl))
                    return TryUnifyTypeParam(constraints, unifyCtx, typeParamDecl, fst);

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
                    unifyCtx,
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
                unifyCtx,
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
                unifyCtx,
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
                unifyCtx,
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
                        unifyCtx,
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
    ConstraintSystem& constraints,
    ValUnificationContext unifyCtx,
    QualType fst,
    QualType snd)
{
    // Unifying a type `A & B` with `T` amounts to unifying
    // `A` with `T` and also `B` with `T` while
    // unifying a type `T` with `A & B` amounts to either
    // unifying `T` with `A` or `T` with `B`
    //
    // If either unification is impossible, then the full
    // case is also impossible.
    //
    if (auto fstAndType = as<AndType>(fst))
    {
        return TryUnifyTypes(
                   constraints,
                   unifyCtx,
                   QualType(fstAndType->getLeft(), fst.isLeftValue),
                   snd) &&
               TryUnifyTypes(
                   constraints,
                   unifyCtx,
                   QualType(fstAndType->getRight(), fst.isLeftValue),
                   snd);
    }
    else if (auto sndAndType = as<AndType>(snd))
    {
        return TryUnifyTypes(
                   constraints,
                   unifyCtx,
                   fst,
                   QualType(sndAndType->getLeft(), snd.isLeftValue)) ||
               TryUnifyTypes(
                   constraints,
                   unifyCtx,
                   fst,
                   QualType(sndAndType->getRight(), snd.isLeftValue));
    }
    else
        return false;
}

void SemanticsVisitor::maybeUnifyUnconstraintIntParam(
    ConstraintSystem& constraints,
    ValUnificationContext unifyCtx,
    IntVal* param,
    IntVal* arg,
    bool paramIsLVal)
{
    SLANG_UNUSED(unifyCtx);

    // If `param` is an unconstrained integer val param, and `arg` is a const int val,
    // we add a constraint to the system that `param` must be equal to `arg`.
    // If `param` is already constrained, ignore and do nothing.
    if (auto typeCastParam = as<TypeCastIntVal>(param))
    {
        param = as<IntVal>(typeCastParam->getBase());
    }
    auto intParam = as<DeclRefIntVal>(param);
    if (!intParam)
        return;
    for (auto c : constraints.constraints)
        if (c.decl == intParam->getDeclRef().getDecl())
            return;
    Constraint c;
    c.decl = intParam->getDeclRef().getDecl();
    c.isUsedAsLValue = paramIsLVal;
    c.val = arg;
    c.priority = ConstraintPriority::Optional;
    constraints.constraints.add(c);
}

struct IndexSpan
{
    Index index;
    Index count;

    IndexSpan()
        : index(0), count(0)
    {
    }
    IndexSpan(Index idx, Index cnt)
        : index(idx), count(cnt)
    {
    }
};

struct IndexSpanPair
{
    IndexSpan first;
    IndexSpan second;

    IndexSpanPair() {}
    IndexSpanPair(IndexSpan f, IndexSpan s)
        : first(f), second(s)
    {
    }
};

// Convert one flattened tuple/pack mapping span back into the `QualType` used
// by recursive unification. If the opposite side is pack-shaped, a span like
// `[A, B]` must stay bundled as a type pack; otherwise the span represents one
// ordinary type.
QualType getMappedQualTypeForConstraint(
    ASTBuilder* astBuilder,
    const IndexSpan& span,
    ShortList<Type*>& typeList,
    bool isLeftValue,
    Type* otherType)
{
    // When the opposite side is a pack parameter or expansion, the mapped span
    // stays bundled so recursive unification sees a pack-shaped value.
    if (isDeclRefTypeOf<GenericTypePackParamDecl>(otherType) || as<ExpandType>(otherType))
    {
        auto typesView = makeArrayView(&typeList[span.index], span.count);
        auto typePack = astBuilder->getTypePack(typesView);
        return QualType(typePack, isLeftValue);
    }

    // Non-pack mappings are valid only for a single element; the caller already
    // asserted the mapping cardinality before asking for a `QualType`.
    SLANG_ASSERT(span.count == 1);
    return QualType(typeList[span.index], isLeftValue);
}

// Helper function to unwrap a type and count expandable types
static void unwrapTypeAndCountExpandable(
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

// Helper function to map type arguments between two types, handling expandable types
static bool matchTypeArgMapping(
    Type* firstType,
    Type* secondType,
    ShortList<Type*>& outFlattenedFirst,
    ShortList<Type*>& outFlattenedSecond,
    ShortList<IndexSpanPair>& outMapping)
{
    // Unwrap and flatten the types
    ShortList<Type*>& firstTypes = outFlattenedFirst;
    ShortList<Type*>& secondTypes = outFlattenedSecond;

    // Count expandable types as we unwrap
    int firstExpandableCount = 0;
    int secondExpandableCount = 0;

    // Unwrap both types using the helper function
    unwrapTypeAndCountExpandable(firstType, firstTypes, firstExpandableCount);
    unwrapTypeAndCountExpandable(secondType, secondTypes, secondExpandableCount);

    // We need to figure out which side should be expanding.
    // Consider the following cases,
    //
    //   left = [ expand, expand ]
    //   right = [ int, float, expand ]
    // when one side has more non-expandable types, the other side should expand to match it.
    // in this case, "left" should expand to cover "int" and "float".
    //
    //   left = [ int, float, expand, expand ]
    //   right = [ int, float, expand ]
    // when the number of the non-expandable types are same, we want to expand side that has
    // fewer expandable types. In this case, "right" should expand to cover the first "expand".
    //
    //   left = ConcreteTypePack(ExpandType, ExpandType)
    //   right = ConcreteTypePack(int, bool, float, double).
    // In this case, we shouldn't be mapping the first ExpandType to int and the second
    // ExpandType to bool, float, double. Instead, they should evenly divide the second type
    // pack, so we map first ExpandType with int, bool, and second ExpandType to float, double.
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

    // We need to figure out how much types should match per each expandable type.
    int typesPerExpand = 0;
    if (shouldExpandSecond)
    {
        // More types on first, need to expand second
        int countToMatch = countDifference + firstExpandableCount;
        SLANG_ASSERT(secondExpandableCount != 0);
        if (countToMatch % secondExpandableCount != 0)
            return false;
        typesPerExpand = countToMatch / secondExpandableCount;
    }
    else if (shouldExpandFirst)
    {
        // More types on second, need to expand first
        int countToMatch = -countDifference + secondExpandableCount;
        SLANG_ASSERT(firstExpandableCount != 0);
        if (countToMatch % firstExpandableCount != 0)
            return false;
        typesPerExpand = countToMatch / firstExpandableCount;
    }
    // If countDifference == 0, no expansion needed

    // Generate the mapping
    Index firstIndex = 0;
    Index secondIndex = 0;

    while (firstIndex < firstCount && secondIndex < secondCount)
    {
        IndexSpanPair mapping;

        // Determine spans based on expandable types and count difference
        if (shouldExpandFirst)
        {
            // Expanding first to match second
            if (isAbstractTypePack(firstTypes[firstIndex]))
            {
                mapping.first = IndexSpan(firstIndex, 1);
                mapping.second = IndexSpan(secondIndex, typesPerExpand);
                secondIndex += typesPerExpand;
            }
            else
            {
                mapping.first = IndexSpan(firstIndex, 1);
                mapping.second = IndexSpan(secondIndex, 1);
                secondIndex++;
            }
            firstIndex++;
        }
        else if (shouldExpandSecond)
        {
            // Expanding second to match first
            if (isAbstractTypePack(secondTypes[secondIndex]))
            {
                mapping.first = IndexSpan(firstIndex, typesPerExpand);
                mapping.second = IndexSpan(secondIndex, 1);
                firstIndex += typesPerExpand;
            }
            else
            {
                mapping.first = IndexSpan(firstIndex, 1);
                mapping.second = IndexSpan(secondIndex, 1);
                firstIndex++;
            }
            secondIndex++;
        }
        else
        {
            // No expansion needed
            mapping.first = IndexSpan(firstIndex, 1);
            mapping.second = IndexSpan(secondIndex, 1);
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
    ConstraintSystem& constraints,
    ValUnificationContext unifyCtx,
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
        return TryUnifyConjunctionType(constraints, unifyCtx, fst, snd);
    }

    // Unwrap ConcreteTypePack and call TryUnifyTypes recursively.
    ShortList<IndexSpanPair> typeMapping;
    ShortList<Type*> flattenedFirst;
    ShortList<Type*> flattenedSecond;
    if (matchTypeArgMapping(fst, snd, flattenedFirst, flattenedSecond, typeMapping) &&
        typeMapping.getCount() > 1)
    {
        // Apply unification based on the mapping
        for (const auto& mapping : typeMapping)
        {
            // Make sure it is one of three cases: 1:1, 1:N or N:1
            SLANG_ASSERT(mapping.first.count > 0 && mapping.second.count > 0);
            SLANG_ASSERT(mapping.first.count == 1 || mapping.second.count == 1);

            // Get the types directly from the mapping
            QualType firstArg = getMappedQualTypeForConstraint(
                m_astBuilder,
                mapping.first,
                flattenedFirst,
                fst.isLeftValue,
                flattenedSecond[mapping.second.index]);
            QualType secondArg = getMappedQualTypeForConstraint(
                m_astBuilder,
                mapping.second,
                flattenedSecond,
                snd.isLeftValue,
                flattenedFirst[mapping.first.index]);

            // Perform the unification
            if (!TryUnifyTypes(constraints, unifyCtx, firstArg, secondArg))
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
                        unifyCtx,
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
                ValUnificationContext subUnifyCtx = unifyCtx;
                subUnifyCtx.indexInTypePack = i;
                if (!TryUnifyTypes(
                        constraints,
                        subUnifyCtx,
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
                ValUnificationContext subUnifyCtx = unifyCtx;
                subUnifyCtx.indexInTypePack = i;
                if (!TryUnifyTypes(
                        constraints,
                        subUnifyCtx,
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

        if (auto typeParamDecl = as<GenericTypeParamDecl>(fstDeclRef.getDecl()))
        {
            if (isRelevantGeneric(constraints, typeParamDecl->parentDecl))
                return TryUnifyTypeParam(constraints, unifyCtx, typeParamDecl, snd);
        }
        else if (auto typePackParamDecl = as<GenericTypePackParamDecl>(fstDeclRef.getDecl()))
        {
            if (isRelevantGeneric(constraints, typePackParamDecl->parentDecl) && isTypePack(snd))
                return TryUnifyTypeParam(constraints, unifyCtx, typePackParamDecl, snd);
        }
    }

    if (auto sndDeclRefType = as<DeclRefType>(snd))
    {
        auto sndDeclRef = sndDeclRefType->getDeclRef();

        if (auto typeParamDecl = as<GenericTypeParamDeclBase>(sndDeclRef.getDecl()))
        {
            if (isRelevantGeneric(constraints, typeParamDecl->parentDecl))
                return TryUnifyTypeParam(constraints, unifyCtx, typeParamDecl, fst);
        }
        else if (auto typePackParamDecl = as<GenericTypePackParamDecl>(sndDeclRef.getDecl()))
        {
            if (isRelevantGeneric(constraints, typePackParamDecl->parentDecl) && isTypePack(fst))
                return TryUnifyTypeParam(constraints, unifyCtx, typePackParamDecl, fst);
        }
    }

    // If we can unify the types structurally, then we are golden
    if (TryUnifyTypesByStructuralMatch(constraints, unifyCtx, fst, snd))
        return true;

    // Now we need to consider cases where coercion might
    // need to be applied. For now we can try to do this
    // in a completely ad hoc fashion, but eventually we'd
    // want to do it more formally.

    if (auto fstVectorType = as<VectorExpressionType>(fst))
    {
        if (auto sndScalarType = as<BasicExpressionType>(snd))
        {
            // Try unify the vector count param. In case the vector count is defined by a generic
            // value parameter, we want to be able to infer that parameter should be 1. However, we
            // don't want a failed unification to fail the entire generic argument inference,
            // because a scalar can still be casted into a vector of any length.

            maybeUnifyUnconstraintIntParam(
                constraints,
                unifyCtx,
                fstVectorType->getElementCount(),
                m_astBuilder->getIntVal(m_astBuilder->getIntType(), 1),
                fst.isLeftValue);
            return TryUnifyTypes(
                constraints,
                unifyCtx,
                QualType(fstVectorType->getElementType(), fst.isLeftValue),
                QualType(sndScalarType, snd.isLeftValue));
        }
    }

    if (auto fstScalarType = as<BasicExpressionType>(fst))
    {
        if (auto sndVectorType = as<VectorExpressionType>(snd))
        {
            maybeUnifyUnconstraintIntParam(
                constraints,
                unifyCtx,
                sndVectorType->getElementCount(),
                m_astBuilder->getIntVal(m_astBuilder->getIntType(), 1),
                snd.isLeftValue);
            return TryUnifyTypes(
                constraints,
                unifyCtx,
                QualType(fstScalarType, fst.isLeftValue),
                QualType(sndVectorType->getElementType(), snd.isLeftValue));
        }
    }

    if (auto fstUniformParamGroupType = as<UniformParameterGroupType>(fst))
        return TryUnifyTypes(
            constraints,
            unifyCtx,
            QualType(fstUniformParamGroupType->getElementType(), fst.isLeftValue),
            snd);
    if (auto sndUniformParamGroupType = as<UniformParameterGroupType>(snd))
        return TryUnifyTypes(
            constraints,
            unifyCtx,
            fst,
            QualType(sndUniformParamGroupType->getElementType(), snd.isLeftValue));

    // Each T can coerce with any DeclRefType.
    if (auto eachSnd = as<EachType>(snd))
    {
        if (auto innerSnd = eachSnd->getElementDeclRefType())
        {
            if (auto sndTypePackParamDecl =
                    as<GenericTypePackParamDecl>(innerSnd->getDeclRef().getDecl()))
            {
                if (isRelevantGeneric(constraints, innerSnd->getDeclRef().getDecl()->parentDecl))
                {
                    return TryUnifyTypeParam(constraints, unifyCtx, sndTypePackParamDecl, fst);
                }
            }
        }
    }
    if (auto eachFst = as<EachType>(fst))
    {
        if (auto innerFst = eachFst->getElementDeclRefType())
        {
            if (auto fstTypePackParamDecl =
                    as<GenericTypePackParamDecl>(innerFst->getDeclRef().getDecl()))
            {
                if (isRelevantGeneric(constraints, innerFst->getDeclRef().getDecl()->parentDecl))
                {
                    return TryUnifyTypeParam(constraints, unifyCtx, fstTypePackParamDecl, snd);
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
            unifyCtx,
            QualType(fstModifiedType ? fstModifiedType->getBase() : fst.type, fst.isLeftValue),
            QualType(sndModifiedType ? sndModifiedType->getBase() : snd.type, snd.isLeftValue));
    }
    return false;
}


} // namespace Slang
