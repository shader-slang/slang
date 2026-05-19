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
                // own, but its where-clause bounds are still evidence.  When
                // `InArray : IArrayAccessor<V>` is compared against
                // `IArrayAccessor<U>`, unifying the two interface shapes gives
                // the generic solver the missing ordinary constraint `U = V`.
                if (auto typeParamDecl =
                        as<GenericTypeParamDeclBase>(typeDeclRefType->getDeclRef().getDecl()))
                {
                    if (auto genericDecl = as<GenericDecl>(typeParamDecl->parentDecl))
                    {
                        if (!isRelevantGeneric(*constraints, genericDecl))
                        {
                            for (auto member : genericDecl->getDirectMemberDecls())
                            {
                                auto boundDecl = as<GenericTypeConstraintDecl>(member);
                                if (!boundDecl)
                                    continue;

                                auto boundDeclRef = createDefaultSubstitutionsIfNeeded(
                                                        m_astBuilder,
                                                        this,
                                                        boundDecl->getDefaultDeclRef())
                                                        .as<GenericTypeConstraintDecl>();
                                if (!boundDeclRef)
                                    continue;

                                auto boundSub = getSub(m_astBuilder, boundDeclRef);
                                auto boundSup = getSup(m_astBuilder, boundDeclRef);
                                if (!boundSub || !boundSup || !boundSub->equals(type))
                                    continue;

                                auto unificationResult = TryUnifyTypes(
                                    *constraints,
                                    ValUnificationContext(),
                                    QualType(boundSup),
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

bool addTypeCoercionWitnessToArgs(
    ASTBuilder* astBuilder,
    SemanticsVisitor* visitor,
    TypeCoercionConstraintDecl* constraintDecl,
    DeclRef<GenericDecl> genericDeclRef,
    SemanticsVisitor::OverloadResolveContext* maybeContext,
    HashSet<Decl*>* maybeConstrainedGenericParams,
    ShortList<Val*>& args,
    bool shouldEmitError)
{
    // To emit an error, `maybeContext` must be provided.
    SLANG_ASSERT(!shouldEmitError || shouldEmitError && maybeContext);

    DeclRef<TypeCoercionConstraintDecl> constraintDeclRef =
        astBuilder
            ->getGenericAppDeclRef(genericDeclRef, args.getArrayView().arrayView, constraintDecl)
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

    if (maybeConstrainedGenericParams)
    {
        if (auto fromDecl = isDeclRefTypeOf<Decl>(constraintDecl->fromType))
        {
            maybeConstrainedGenericParams->add(fromDecl.getDecl());
        }
        if (auto toDecl = isDeclRefTypeOf<Decl>(constraintDecl->toType))
        {
            maybeConstrainedGenericParams->add(toDecl.getDecl());
        }
    }

    // If `_coerce` accepted the conversion but didn't provide an explicit
    // witness object, treat it as a builtin conversion witness so later
    // lowering still has a concrete generic argument to pass through.
    if (!typeCoercionWitness)
        typeCoercionWitness = astBuilder->getBuiltinTypeCoercionWitness(fromType, toType);

    args.add(typeCoercionWitness);
    return true;
}

bool addHasDiffTypeInfoWitnessToArgs(
    ASTBuilder* astBuilder,
    SemanticsVisitor* visitor,
    HasDiffTypeInfoConstraintDecl* constraintDecl,
    DeclRef<GenericDecl> genericDeclRef,
    SemanticsVisitor::OverloadResolveContext* maybeContext,
    HashSet<Decl*>* maybeConstrainedGenericParams,
    ShortList<Val*>& args,
    bool shouldEmitError)
{
    SLANG_ASSERT(!shouldEmitError || maybeContext);

    auto constraintDeclRef =
        astBuilder
            ->getGenericAppDeclRef(genericDeclRef, args.getArrayView().arrayView, constraintDecl)
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
    if (maybeConstrainedGenericParams)
    {
        if (auto declRefType = as<DeclRefType>(constrainedType))
            maybeConstrainedGenericParams->add(declRefType->getDeclRef().getDecl());
    }

    if (auto witness = visitor->getDiffTypeInfoWitness(constrainedType))
    {
        args.add(witness);
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

struct SolvedArg
{
    Val* val = nullptr;
    SemanticsVisitor::ConstraintPriority priority = SemanticsVisitor::ConstraintPriority::Default;
    bool hasNonDefaultNonBoundConstraint = false;
    ShortList<QualType, 8> types;
};

bool constraintSystemSolverUnpackArgs(
    SemanticsVisitor* visitor,
    List<GenericDecl*>& genericDecls,
    ArrayView<Val*> knownGenericArgs,
    Dictionary<Decl*, ShortList<SolvedArg>>& solvedArgs,
    Dictionary<Decl*, ShortList<Val*>>& args)
{
    ASTBuilder* astBuilder = visitor->getASTBuilder();

    args.clear();
    for (auto arg : knownGenericArgs)
        args[genericDecls[0]].add(arg);

    Count knownGenericArgCount = knownGenericArgs.getCount();
    for (auto _genericDecl : genericDecls)
    {
        for (auto member : _genericDecl->getDirectMemberDecls())
        {
            if (auto typeParam = as<GenericTypeParamDeclBase>(member))
            {
                SLANG_ASSERT(typeParam->parameterIndex != -1);

                if (typeParam->parameterIndex < knownGenericArgCount)
                    continue;
                bool isPack = as<GenericTypePackParamDecl>(typeParam) != nullptr;
                if (typeParam->parameterIndex >= solvedArgs[_genericDecl].getCount())
                {
                    // If the parameter is not a type pack and we don't have a
                    // resolved type for it, we should fail.
                    if (!isPack)
                        return false;
                    // If the parameter is a type pack, we should add an empty
                    // type list to solvedTypes.
                    solvedArgs[_genericDecl].setCount(typeParam->parameterIndex + 1);
                }
                auto& types = solvedArgs[_genericDecl][typeParam->parameterIndex].types;
                // Fail if any of the resolved type element is empty.
                for (auto t : types)
                {
                    if (!t)
                        return false;
                }
                if (!isPack)
                {
                    // If the generic parameter is not a pack, we can simply add the first type.
                    if (types.getCount() != 1)
                        return false;

                    args[_genericDecl].add(types[0]);
                }
                else
                {
                    // If the generic parameter is a pack, and we are supplying one single pack
                    // argument, we can use it as is.
                    if (types.getCount() == 1 && isTypePack(types[0]))
                    {
                        args[_genericDecl].add(types[0]);
                    }
                    else
                    {
                        // If we are supplying 0 or multiple arguments for the pack, we need to
                        // create a type pack and add it to the argument list.
                        ShortList<Type*> typeList;
                        bool isLVal = true;
                        for (auto t : types)
                        {
                            typeList.add(t);
                            isLVal = isLVal && t.isLeftValue;
                        }
                        args[_genericDecl].add(QualType(
                            astBuilder->getTypePack(typeList.getArrayView().arrayView),
                            isLVal));
                    }
                }
            }
            else if (auto valPackParam = as<GenericValuePackParamDecl>(member))
            {
                SLANG_ASSERT(valPackParam->parameterIndex != -1);

                if (valPackParam->parameterIndex < knownGenericArgCount)
                    continue;

                if (valPackParam->parameterIndex >= solvedArgs[_genericDecl].getCount())
                {
                    // Empty pack.
                    args[_genericDecl].add(astBuilder->getIntValPack(ArrayView<IntVal*>()));
                    continue;
                }

                auto val = solvedArgs[_genericDecl][valPackParam->parameterIndex].val;
                if (!val)
                {
                    args[_genericDecl].add(astBuilder->getIntValPack(ArrayView<IntVal*>()));
                }
                else
                {
                    args[_genericDecl].add(val);
                }
            }
            else if (auto valParam = as<GenericValueParamDecl>(member))
            {
                SLANG_ASSERT(valParam->parameterIndex != -1);

                if (valParam->parameterIndex < knownGenericArgCount)
                    continue;

                if (valParam->parameterIndex >= solvedArgs[_genericDecl].getCount())
                    return false;

                auto val = solvedArgs[_genericDecl][valParam->parameterIndex].val;
                if (!val)
                {
                    // failure!
                    return false;
                }
                args[_genericDecl].add(val);
            }
        }
    }
    return true;
}

Val* tryGetDefaultGenericArgVal(SemanticsVisitor* visitor, Decl* paramDecl)
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

class GenericConstraintSystemSolver
{
public:
    using Constraint = SemanticsVisitor::Constraint;
    using ConstraintPriority = SemanticsVisitor::ConstraintPriority;
    using ConstraintSystem = SemanticsVisitor::ConstraintSystem;
    using ValUnificationContext = SemanticsVisitor::ValUnificationContext;

    struct SolverWorkItem
    {
        enum class Kind
        {
            OrdinaryConstraint,
            DefaultArg,
            Witness,
        };

        Kind kind = Kind::OrdinaryConstraint;
        GenericDecl* genericDecl = nullptr;
        Decl* decl = nullptr;
        Index constraintIndex = -1;
        Val* val = nullptr;
    };

    enum class WorkResult
    {
        Done,
        Solved,
        Blocked,
        Failed,
    };

    GenericConstraintSystemSolver(
        SemanticsVisitor* visitor,
        ConstraintSystem* system,
        DeclRef<GenericDecl> genericDeclRef,
        ArrayView<Val*> knownGenericArgs,
        ConversionCost& outBaseCost)
        : m_visitor(visitor)
        , m_astBuilder(visitor->getASTBuilder())
        , m_system(system)
        , m_genericDeclRef(genericDeclRef)
        , m_knownGenericArgs(knownGenericArgs)
        , m_knownGenericArgCount(knownGenericArgs.getCount())
        , m_outBaseCost(outBaseCost)
    {
    }

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
        // hidden witness slot must have an answer.  We then serialize those slot
        // answers back into the generic argument order expected by decl refs.
        if (!verifyOrdinaryConstraintsSatisfied())
            return DeclRef<Decl>();
        if (!rebuildArgsFromCurrentAnswers(false))
            return DeclRef<Decl>();

        // The final cost mirrors the previous solver: unconstrained generic
        // parameters are less specific, and witness/type-promotion work
        // contributes to overload ranking.
        addUnconstrainedGenericParamCost();
        m_outBaseCost += m_system->typePromotionCost + m_finalWitnessCost;

        return getSubstDeclRef(m_genericDeclRef.getDecl()->inner);
    }

private:
    bool collectConstraints()
    {
        // Generic substitutions are nested, so the solver starts by recording the
        // chain from the outermost generic to the generic being applied.
        collectGenericDeclChain();

        // Bounds still provide useful shape information for legacy cases such
        // as `This`-based interface members.  They enter the work list as
        // ordinary hints, while their real validity is checked by the witness
        // items collected below.
        if (!collectGenericParamBoundConstraints())
            return false;

        // The constraint system already contains inference facts collected by
        // argument/parameter unification.  Each one becomes a work item that can
        // fill a type or value parameter slot.
        collectOrdinaryConstraintsFrom(0);

        // Defaults and where-clauses are collected after ordinary constraints,
        // but they are not solved in a later phase.  They are simply more work
        // items in the same loop.
        collectGenericMemberWorkItems();
        return true;
    }

    void collectGenericDeclChain()
    {
        for (auto genericDecl = m_genericDeclRef.getDecl(); genericDecl;
             genericDecl = as<GenericDecl>(genericDecl->parentDecl))
        {
            m_genericDecls.add(genericDecl);
        }
        m_genericDecls.reverse();
    }

    bool collectGenericParamBoundConstraints()
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
                ctx.genericParamBoundConstraint = true;

                if (!m_visitor->TryUnifyTypes(
                        *m_system,
                        ctx,
                        getSub(m_astBuilder, constraintDeclRef),
                        getSup(m_astBuilder, constraintDeclRef)))
                    return false;
            }
        }
        return true;
    }

    void collectOrdinaryConstraintsFrom(Index firstConstraintIndex)
    {
        for (Index constraintIndex = firstConstraintIndex;
             constraintIndex < m_system->constraints.getCount();
             constraintIndex++)
        {
            SolverWorkItem workItem;
            workItem.kind = SolverWorkItem::Kind::OrdinaryConstraint;
            workItem.constraintIndex = constraintIndex;
            addWorkItem(workItem);
        }
        m_collectedConstraintCount = m_system->constraints.getCount();
    }

    void collectGenericMemberWorkItems()
    {
        for (auto genericDecl : m_genericDecls)
            collectGenericMemberWorkItems(genericDecl);
    }

    void collectGenericMemberWorkItems(GenericDecl* genericDecl)
    {
        for (auto member : genericDecl->getDirectMemberDecls())
        {
            if (isDefaultableParam(member))
                addDefaultArgWorkItem(genericDecl, member);
            else if (isImplicitWitnessConstraint(member))
                addWitnessWorkItem(genericDecl, member);
        }
    }

    bool isDefaultableParam(Decl* member)
    {
        if (as<GenericTypePackParamDecl>(member) || as<GenericValuePackParamDecl>(member))
            return false;
        return as<GenericTypeParamDecl>(member) || as<GenericValueParamDecl>(member);
    }

    bool isImplicitWitnessConstraint(Decl* member)
    {
        return as<GenericTypeConstraintDecl>(member) || as<TypeCoercionConstraintDecl>(member) ||
               as<NonEmptyPackConstraintDecl>(member) || as<HasDiffTypeInfoConstraintDecl>(member);
    }

    void addDefaultArgWorkItem(GenericDecl* genericDecl, Decl* paramDecl)
    {
        Val* defaultVal = tryGetDefaultGenericArgVal(m_visitor, paramDecl);
        if (!defaultVal)
            return;

        SolverWorkItem workItem;
        workItem.kind = SolverWorkItem::Kind::DefaultArg;
        workItem.genericDecl = genericDecl;
        workItem.decl = paramDecl;
        workItem.val = defaultVal;
        m_defaultArgWorkItemIndexByDecl[paramDecl] = m_solverWorkItems.getCount();
        addWorkItem(workItem);
    }

    void addWitnessWorkItem(GenericDecl* genericDecl, Decl* constraintDecl)
    {
        SolverWorkItem workItem;
        workItem.kind = SolverWorkItem::Kind::Witness;
        workItem.genericDecl = genericDecl;
        workItem.decl = constraintDecl;
        m_witnessWorkItemIndexByDecl[constraintDecl] = m_solverWorkItems.getCount();
        addWorkItem(workItem);
    }

    void addWorkItem(SolverWorkItem const& workItem)
    {
        Index itemIndex = m_solverWorkItems.getCount();
        m_solverWorkItems.add(workItem);
        m_workItemDone.add(0);
        m_workItemQueued.add(0);
        enqueueWorkItem(itemIndex);
    }

    void enqueueWorkItem(Index itemIndex)
    {
        if (m_workItemDone[itemIndex] || m_workItemQueued[itemIndex])
            return;
        m_workItemQueued[itemIndex] = 1;
        m_workList.add(itemIndex);
    }

    void wakeWorkItems()
    {
        for (Index itemIndex = 0; itemIndex < m_solverWorkItems.getCount(); itemIndex++)
        {
            m_workItemDone[itemIndex] = 0;
            enqueueWorkItem(itemIndex);
        }
    }

    bool runWorkList()
    {
        // The work list is intentionally not split by answer kind.  A default
        // that needs a witness simply blocks, and the witness constraint is free
        // to run next.  Conversely, a witness that needs a defaulted type blocks
        // until that type slot receives an answer.
        while (m_workListReadIndex < m_workList.getCount())
        {
            Index itemIndex = m_workList[m_workListReadIndex++];
            m_workItemQueued[itemIndex] = 0;

            if (m_workItemDone[itemIndex])
                continue;

            WorkResult result = tryApplyWorkItem(itemIndex);
            if (result == WorkResult::Failed)
                return false;
            if (result == WorkResult::Blocked)
                continue;

            m_workItemDone[itemIndex] = 1;
            if (result == WorkResult::Solved)
                wakeWorkItems();
        }

        // If the queue drained while an item remained unsolved, then all
        // remaining work is waiting on slots that never acquired answers.
        for (Index itemIndex = 0; itemIndex < m_solverWorkItems.getCount(); itemIndex++)
        {
            if (!m_workItemDone[itemIndex])
                return false;
        }
        return true;
    }

    WorkResult tryApplyWorkItem(Index itemIndex)
    {
        auto const& workItem = m_solverWorkItems[itemIndex];
        switch (workItem.kind)
        {
        case SolverWorkItem::Kind::OrdinaryConstraint:
            return applyOrdinaryConstraint(workItem);
        case SolverWorkItem::Kind::DefaultArg:
            return applyDefaultArg(workItem);
        case SolverWorkItem::Kind::Witness:
            return applyWitness(workItem);
        }
        return WorkResult::Failed;
    }

    WorkResult applyOrdinaryConstraint(SolverWorkItem const& workItem)
    {
        if (workItem.constraintIndex < 0 ||
            workItem.constraintIndex >= m_system->constraints.getCount())
            return WorkResult::Failed;

        Constraint c = m_system->constraints[workItem.constraintIndex];
        if (c.satisfied)
            return WorkResult::Done;
        if (c.isGenericParamBound && !c.isEquality)
            return applyGenericParamBoundConstraint(workItem, c);
        if (hasBlockersForVal(c.val, c.decl))
            return WorkResult::Blocked;

        if (!rebuildArgsFromCurrentAnswers(true))
            return WorkResult::Failed;
        if (!substitutePotentiallyDependentConstraint(c))
            return WorkResult::Failed;

        bool succeeded = true;
        if (auto typeParam = as<GenericTypeParamDeclBase>(c.decl))
        {
            succeeded = processTypeParamConstraint(c, typeParam);
        }
        else if (auto valPackParam = as<GenericValuePackParamDecl>(c.decl))
        {
            succeeded = processValuePackParamConstraint(c, valPackParam);
        }
        else if (auto valParam = as<GenericValueParamDecl>(c.decl))
        {
            succeeded = processValueParamConstraint(c, valParam);
        }

        if (!succeeded)
            return WorkResult::Failed;

        m_system->constraints[workItem.constraintIndex].satisfied = c.satisfied;
        if (m_collectedConstraintCount < m_system->constraints.getCount())
            collectOrdinaryConstraintsFrom(m_collectedConstraintCount);
        return WorkResult::Solved;
    }

    WorkResult applyGenericParamBoundConstraint(SolverWorkItem const& workItem, Constraint& c)
    {
        // A subtype bound is not an answer for the parameter it mentions.  It is
        // an edge that becomes informative after the subject slot has an answer:
        // `InputArray : IArrayAccessor<U>` can then compare the known
        // `InputArray` against that interface shape and may discover `U`.
        if (isSelfReferenceKnownGenericArg(c.decl))
        {
            c.satisfied = true;
            m_system->constraints[workItem.constraintIndex].satisfied = true;
            return WorkResult::Done;
        }
        if (!hasConcreteSubjectForGenericParamBound(c.decl))
            return WorkResult::Blocked;

        // The interface side of the bound is allowed to mention parameters that
        // this same bound is trying to infer.  Rebuilding with placeholders keeps
        // those references visible to `TryJoinTypes`, which can add the real
        // ordinary constraints when it recognizes an existing generic witness.
        if (!rebuildArgsFromCurrentAnswers(true))
            return WorkResult::Failed;
        if (!substitutePotentiallyDependentConstraint(c))
            return WorkResult::Failed;

        Count oldConstraintCount = m_system->constraints.getCount();
        bool succeeded = true;
        if (auto typeParam = as<GenericTypeParamDeclBase>(c.decl))
            succeeded = processTypeParamConstraint(c, typeParam);
        else if (auto valPackParam = as<GenericValuePackParamDecl>(c.decl))
            succeeded = processValuePackParamConstraint(c, valPackParam);
        else if (auto valParam = as<GenericValueParamDecl>(c.decl))
            succeeded = processValueParamConstraint(c, valParam);

        // Bound-shape failures are left to the witness item, which can produce
        // the actual proof or fail the specialization.  The ordinary bound item
        // only exists to contribute any inference that falls out of comparing
        // generic interface arguments.
        m_system->constraints[workItem.constraintIndex].satisfied = c.satisfied;
        if (m_collectedConstraintCount < m_system->constraints.getCount())
            collectOrdinaryConstraintsFrom(m_collectedConstraintCount);
        if (!succeeded && oldConstraintCount == m_system->constraints.getCount())
        {
            m_system->constraints[workItem.constraintIndex].satisfied = true;
            return WorkResult::Done;
        }
        return WorkResult::Solved;
    }

    bool isSelfReferenceKnownGenericArg(Decl* paramDecl)
    {
        // Default substitutions sometimes pass a parameter back as its own
        // argument so later code can keep a complete generic-argument list.
        // That placeholder is useful for shape, but it is not evidence that
        // the parameter has been inferred.
        Val* knownArg = nullptr;
        if (!tryGetKnownGenericArg(paramDecl, knownArg, true))
            return false;
        return isSelfReferencePlaceholder(paramDecl, knownArg);
    }

    bool hasConcreteSubjectForGenericParamBound(Decl* paramDecl)
    {
        // Bound inference needs a real subject type, not the declaration-shaped
        // placeholder used to keep partial substitutions well formed.  A known
        // explicit argument, an ordinary solved slot, or an already-reduced
        // default is concrete enough to compare against the bound interface.
        Val* knownArg = nullptr;
        if (tryGetKnownGenericArg(paramDecl, knownArg))
            return true;
        if (hasConcreteSolvedArg(paramDecl))
            return true;
        return m_defaultArgValues.containsKey(paramDecl);
    }

    WorkResult applyDefaultArg(SolverWorkItem const& workItem)
    {
        if (isKnownGenericArg(workItem.decl) || hasNonDefaultNonBoundConstraint(workItem.decl))
            return WorkResult::Done;
        if (hasBlockersForVal(workItem.val))
            return WorkResult::Blocked;
        if (!rebuildArgsFromCurrentAnswers(true))
            return WorkResult::Failed;

        auto fullSubst = SubstitutionSet(getSubstDeclRef(m_genericDeclRef.getDecl()->inner));
        int diff = 0;
        auto substArg = workItem.val->substituteImpl(m_astBuilder, fullSubst, &diff);
        if (!substArg)
            return WorkResult::Failed;

        substArg = substArg->resolve();
        if (auto substArgType = as<Type>(substArg))
            substArg = substArgType->getCanonicalType();

        Val* oldArg = nullptr;
        if (m_defaultArgValues.tryGetValue(workItem.decl, oldArg) && oldArg->equals(substArg))
            return WorkResult::Done;

        m_defaultArgValues[workItem.decl] = substArg;
        return WorkResult::Solved;
    }

    WorkResult applyWitness(SolverWorkItem const& workItem)
    {
        if (hasBlockersForWitness(workItem.decl))
            return WorkResult::Blocked;
        if (!rebuildArgsFromCurrentAnswers(true))
            return WorkResult::Failed;

        Val* oldWitnessArg = nullptr;
        bool hadOldWitnessArg = m_witnessArgValues.tryGetValue(workItem.decl, oldWitnessArg);
        if (!appendImplicitWitnessArg(workItem.genericDecl, workItem.decl))
            return WorkResult::Failed;

        Val* newWitnessArg = nullptr;
        if (!m_witnessArgValues.tryGetValue(workItem.decl, newWitnessArg))
            return WorkResult::Failed;
        if (hadOldWitnessArg && oldWitnessArg->equals(newWitnessArg))
            return WorkResult::Done;

        return WorkResult::Solved;
    }

    bool substitutePotentiallyDependentConstraint(Constraint& c)
    {
        if (!c.potentiallyDependent)
            return true;

        // Potentially dependent ordinary constraints are replayed through the
        // current partial substitution.  The work-list blocker check has already
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

    bool processTypeParamConstraint(Constraint& c, GenericTypeParamDeclBase* typeParam)
    {
        SLANG_ASSERT(typeParam->parameterIndex != -1);

        if (isKnownGenericArg(typeParam))
        {
            c.satisfied = true;
            return true;
        }

        bool isPack = as<GenericTypePackParamDecl>(typeParam) != nullptr;
        if (m_solvedArgs[typeParam->parentDecl].getCount() <= typeParam->parameterIndex)
            m_solvedArgs[typeParam->parentDecl].setCount(typeParam->parameterIndex + 1);

        auto& solvedArg = m_solvedArgs[typeParam->parentDecl][typeParam->parameterIndex];
        markNonDefaultNonBoundConstraint(solvedArg, c);
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
            if (c.isGenericParamBound && !c.isEquality)
            {
                c.satisfied = true;
                return true;
            }
            type = cType;
            solvedArg.priority = c.priority;
        }
        else if (!mergeTypeConstraint(type, solvedArg.priority, c, cType))
        {
            if (!c.isGenericParamBound)
                return false;
        }

        if (c.priority < solvedArg.priority)
            solvedArg.priority = c.priority;

        c.satisfied = true;
        return true;
    }

    bool mergeTypeConstraint(
        QualType& ioType,
        ConstraintPriority currentPriority,
        Constraint const& c,
        QualType cType)
    {
        auto joinType = m_visitor->TryJoinTypes(m_system, ioType, cType);
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

    bool processValuePackParamConstraint(Constraint& c, GenericValuePackParamDecl* valPackParam)
    {
        SLANG_ASSERT(valPackParam->parameterIndex != -1);

        if (isKnownGenericArg(valPackParam))
        {
            c.satisfied = true;
            return true;
        }

        if (m_solvedArgs[valPackParam->parentDecl].getCount() <= valPackParam->parameterIndex)
            m_solvedArgs[valPackParam->parentDecl].setCount(valPackParam->parameterIndex + 1);

        auto& solvedArg = m_solvedArgs[valPackParam->parentDecl][valPackParam->parameterIndex];
        markNonDefaultNonBoundConstraint(solvedArg, c);
        Val*& val = solvedArg.val;

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

        c.satisfied = true;
        return true;
    }

    bool processValueParamConstraint(Constraint& c, GenericValueParamDecl* valParam)
    {
        SLANG_ASSERT(valParam->parameterIndex != -1);

        if (isKnownGenericArg(valParam))
        {
            c.satisfied = true;
            return true;
        }

        if (m_solvedArgs[valParam->parentDecl].getCount() <= valParam->parameterIndex)
            m_solvedArgs[valParam->parentDecl].setCount(valParam->parameterIndex + 1);

        auto& solvedArg = m_solvedArgs[valParam->parentDecl][valParam->parameterIndex];
        markNonDefaultNonBoundConstraint(solvedArg, c);
        Val*& val = solvedArg.val;
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
            return false;
        }

        c.satisfied = true;
        return true;
    }

    void markNonDefaultNonBoundConstraint(SolvedArg& solvedArg, Constraint const& c)
    {
        if (c.priority != ConstraintPriority::Default && !c.isGenericParamBound)
            solvedArg.hasNonDefaultNonBoundConstraint = true;
    }

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
                return !isParamSlotReady(getDeclRef(m_astBuilder, declRefExpr).getDecl());
            return true;
        }
        if (auto hasDiffTypeInfoConstraintDecl = as<HasDiffTypeInfoConstraintDecl>(constraintDecl))
        {
            return hasBlockersForVal(hasDiffTypeInfoConstraintDecl->type.type);
        }
        return false;
    }

    bool hasBlockersForVal(Val* val, Decl* ignoredParamDecl = nullptr)
    {
        HashSet<Val*> visitedVals;
        return collectValBlockers(val, visitedVals, ignoredParamDecl);
    }

    bool collectValBlockers(Val* val, HashSet<Val*>& ioVisitedVals, Decl* ignoredParamDecl)
    {
        if (!val || ioVisitedVals.contains(val))
            return false;
        ioVisitedVals.add(val);

        // The dependency protocol is local to expression evaluation: a type or
        // value parameter reference waits for its parameter slot, and a declared
        // witness reference waits for the where-clause slot that will supply it.
        if (auto declRefType = as<DeclRefType>(val))
        {
            auto decl = declRefType->getDeclRef().getDecl();
            if (decl == ignoredParamDecl)
                return false;
            if (as<GenericTypeParamDeclBase>(decl) && isRelevantParamSlot(decl) &&
                !isParamSlotReady(decl))
                return true;
        }
        else if (auto declRefIntVal = as<DeclRefIntVal>(val))
        {
            auto decl = declRefIntVal->getDeclRef().getDecl();
            if (decl == ignoredParamDecl)
                return false;
            if (isGenericValueParam(decl) && isRelevantParamSlot(decl) && !isParamSlotReady(decl))
                return true;
        }
        else if (auto declaredWitness = as<DeclaredSubtypeWitness>(val))
        {
            if (isWitnessForIgnoredParam(declaredWitness, ignoredParamDecl))
                return false;
            if (!isWitnessSlotReady(declaredWitness->getDeclRef().getDecl()))
                return true;
        }

        for (auto operand : val->m_operands)
        {
            if (operand.kind == ValNodeOperandKind::ValNode &&
                collectValBlockers(operand.getVal(), ioVisitedVals, ignoredParamDecl))
                return true;
        }
        return false;
    }

    bool isWitnessForIgnoredParam(DeclaredSubtypeWitness* witness, Decl* ignoredParamDecl)
    {
        if (!ignoredParamDecl)
            return false;
        if (!witness->getDeclRef().as<GenericTypeConstraintDecl>())
            return false;

        auto witnessSubDeclRef = isDeclRefTypeOf<Decl>(witness->getSub());
        return witnessSubDeclRef && witnessSubDeclRef.getDecl() == ignoredParamDecl;
    }

    bool isParamSlotReady(Decl* paramDecl)
    {
        if (!isRelevantParamSlot(paramDecl))
            return true;
        Val* knownArg = nullptr;
        if (tryGetKnownGenericArg(paramDecl, knownArg, true))
            return true;

        // Pack parameters can always be represented as empty packs while the
        // solver is forming partial substitutions.  Non-empty-pack constraints
        // still validate the final cardinality when their witness item runs.
        if (as<GenericTypePackParamDecl>(paramDecl) || as<GenericValuePackParamDecl>(paramDecl))
            return true;

        if (hasNonDefaultNonBoundConstraint(paramDecl))
            return hasConcreteSolvedArg(paramDecl);
        if (m_defaultArgWorkItemIndexByDecl.containsKey(paramDecl))
            return m_defaultArgValues.containsKey(paramDecl);
        return hasConcreteSolvedArg(paramDecl);
    }

    bool isRelevantParamSlot(Decl* paramDecl)
    {
        return m_visitor->isRelevantGeneric(*m_system, paramDecl->parentDecl);
    }

    bool isWitnessSlotReady(Decl* constraintDecl)
    {
        if (!m_witnessWorkItemIndexByDecl.containsKey(constraintDecl))
            return true;
        return m_witnessArgValues.containsKey(constraintDecl);
    }

    bool isKnownGenericArg(Decl* paramDecl)
    {
        Val* knownArg = nullptr;
        return tryGetKnownGenericArg(paramDecl, knownArg);
    }

    bool tryGetKnownGenericArg(
        Decl* paramDecl,
        Val*& outArg,
        bool allowSelfReferencePlaceholder = false)
    {
        // Some callers seed inference with a default substitution such as
        // `<T, N, T : IFoo>`.  Those self-reference values keep decl-ref shapes
        // well formed, but they are not explicit user answers; the work list
        // must still be allowed to replace them with inferred arguments.
        auto genericDecl = as<GenericDecl>(paramDecl->parentDecl);
        if (!genericDecl || genericDecl != m_genericDecls[0])
            return false;

        Index knownArgIndex = getKnownGenericArgIndex(paramDecl);
        if (knownArgIndex < 0 || knownArgIndex >= m_knownGenericArgCount)
            return false;

        Val* knownArg = m_knownGenericArgs[knownArgIndex];
        if (!allowSelfReferencePlaceholder && isSelfReferencePlaceholder(paramDecl, knownArg))
            return false;

        outArg = knownArg;
        return true;
    }

    Index getKnownGenericArgIndex(Decl* argDecl)
    {
        // `knownGenericArgs` is the explicit visible-argument prefix supplied by
        // the call site.  Hidden witnesses still appear in completed generic
        // applications, but they are solved by witness work items and therefore
        // do not participate in this lookup.
        if (auto typeParamDecl = as<GenericTypeParamDeclBase>(argDecl))
            return typeParamDecl->parameterIndex;
        if (auto valuePackParamDecl = as<GenericValuePackParamDecl>(argDecl))
            return valuePackParamDecl->parameterIndex;
        if (auto valueParamDecl = as<GenericValueParamDecl>(argDecl))
            return valueParamDecl->parameterIndex;
        return -1;
    }

    Index getGenericArgIndex(Decl* argDecl)
    {
        auto genericDecl = as<GenericDecl>(argDecl->parentDecl);
        if (!genericDecl)
            return -1;

        Index argIndex = 0;
        for (auto member : genericDecl->getDirectMemberDecls())
        {
            if (isDefaultableParam(member) || as<GenericTypePackParamDecl>(member) ||
                as<GenericValuePackParamDecl>(member) || isImplicitWitnessConstraint(member))
            {
                if (member == argDecl)
                    return argIndex;
                argIndex++;
            }
        }
        return -1;
    }

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

    bool hasConcreteSolvedArg(Decl* paramDecl)
    {
        if (auto typeParam = as<GenericTypeParamDeclBase>(paramDecl))
        {
            Val* ignored = nullptr;
            return tryGetSolvedTypeArg(typeParam, ignored);
        }
        if (auto valPackParam = as<GenericValuePackParamDecl>(paramDecl))
            return valPackParam->parameterIndex < m_solvedArgs[valPackParam->parentDecl].getCount();
        if (auto valParam = as<GenericValueParamDecl>(paramDecl))
            return tryGetSolvedValueArg(valParam) != nullptr;
        return false;
    }

    bool hasNonDefaultNonBoundConstraint(Decl* paramDecl)
    {
        Index parameterIndex = -1;
        if (auto typeParam = as<GenericTypeParamDeclBase>(paramDecl))
            parameterIndex = typeParam->parameterIndex;
        else if (auto valPackParam = as<GenericValuePackParamDecl>(paramDecl))
            parameterIndex = valPackParam->parameterIndex;
        else if (auto valParam = as<GenericValueParamDecl>(paramDecl))
            parameterIndex = valParam->parameterIndex;
        else
            return false;

        if (parameterIndex < 0 || parameterIndex >= m_solvedArgs[paramDecl->parentDecl].getCount())
            return false;
        return m_solvedArgs[paramDecl->parentDecl][parameterIndex].hasNonDefaultNonBoundConstraint;
    }

    bool rebuildArgsFromCurrentAnswers(bool allowPlaceholders)
    {
        // This function is the single serializer from slot answers to generic
        // argument arrays.  During solving it may use placeholders so unrelated
        // slots keep their declaration-order positions; at the end placeholders
        // are disallowed and every required slot must be concrete.
        m_args.clear();

        for (auto genericDecl : m_genericDecls)
        {
            // Generic applications store the visible type and value arguments
            // before any hidden witnesses.  The declaration list may contain a
            // synthesized constraint next to the parameter that introduced it,
            // so the serializer walks ordinary parameters first and only then
            // appends the witness answers.
            for (auto member : genericDecl->getDirectMemberDecls())
            {
                if (auto typeParam = as<GenericTypeParamDeclBase>(member))
                {
                    Val* knownArg = nullptr;
                    if (tryGetKnownGenericArg(typeParam, knownArg, true))
                    {
                        m_args[genericDecl].add(knownArg);
                        continue;
                    }
                    Val* arg = getTypeParamArg(typeParam, allowPlaceholders);
                    if (!arg)
                        return false;
                    m_args[genericDecl].add(arg);
                }
                else if (auto valPackParam = as<GenericValuePackParamDecl>(member))
                {
                    Val* knownArg = nullptr;
                    if (tryGetKnownGenericArg(valPackParam, knownArg, true))
                    {
                        m_args[genericDecl].add(knownArg);
                        continue;
                    }
                    m_args[genericDecl].add(getValuePackParamArg(valPackParam));
                }
                else if (auto valParam = as<GenericValueParamDecl>(member))
                {
                    Val* knownArg = nullptr;
                    if (tryGetKnownGenericArg(valParam, knownArg, true))
                    {
                        m_args[genericDecl].add(knownArg);
                        continue;
                    }
                    Val* arg = getValueParamArg(valParam, allowPlaceholders);
                    if (!arg)
                        return false;
                    m_args[genericDecl].add(arg);
                }
            }

            // The second pass preserves the same hidden-argument order that the
            // generic declaration expects, while keeping those witnesses after
            // the ordinary argument prefix used by substitution and unification.
            for (auto member : genericDecl->getDirectMemberDecls())
            {
                if (isImplicitWitnessConstraint(member))
                {
                    Val* witnessArg = nullptr;
                    if (m_witnessArgValues.tryGetValue(member, witnessArg))
                    {
                        m_args[genericDecl].add(witnessArg);
                    }
                    else if (allowPlaceholders)
                    {
                        witnessArg = getPlaceholderWitnessArg(member);
                        if (!witnessArg)
                            return false;
                        m_args[genericDecl].add(witnessArg);
                    }
                    else if (!allowPlaceholders)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    Val* getPlaceholderWitnessArg(Decl* constraintDecl)
    {
        // Partial substitutions need every hidden witness position to remain
        // stable, even before the work-list has proved the real witness.  These
        // placeholders are the same declaration-shaped values used by default
        // substitutions elsewhere in the checker, so they preserve dependency
        // edges without claiming the constraint is solved.
        if (auto genericTypeConstraintDecl = as<GenericTypeConstraintDecl>(constraintDecl))
        {
            auto constraintDeclRef =
                getSubstDeclRef(genericTypeConstraintDecl).as<GenericTypeConstraintDecl>();
            auto sub = getSub(m_astBuilder, constraintDeclRef);
            auto sup = getSup(m_astBuilder, constraintDeclRef);
            if (!sub || !sup)
                return nullptr;
            return m_astBuilder->getDeclaredSubtypeWitness(sub, sup, constraintDeclRef);
        }

        if (auto typeCoercionConstraintDecl = as<TypeCoercionConstraintDecl>(constraintDecl))
        {
            auto constraintDeclRef =
                getSubstDeclRef(typeCoercionConstraintDecl).as<TypeCoercionConstraintDecl>();
            auto fromType = getFromType(m_astBuilder, constraintDeclRef);
            auto toType = getToType(m_astBuilder, constraintDeclRef);
            if (!fromType || !toType)
                return nullptr;
            return m_astBuilder->getBuiltinTypeCoercionWitness(fromType, toType);
        }

        if (auto nonEmptyConstraintDecl = as<NonEmptyPackConstraintDecl>(constraintDecl))
        {
            auto packVal = getNonEmptyPackPlaceholder(nonEmptyConstraintDecl);
            return packVal ? m_astBuilder->getNonEmptyPackWitness(packVal) : nullptr;
        }

        if (auto hasDiffTypeInfoConstraintDecl = as<HasDiffTypeInfoConstraintDecl>(constraintDecl))
        {
            auto constraintDeclRef =
                getSubstDeclRef(hasDiffTypeInfoConstraintDecl).as<HasDiffTypeInfoConstraintDecl>();
            return m_astBuilder->getHasDiffTypeInfoWitness(constraintDeclRef);
        }

        return nullptr;
    }

    Val* getNonEmptyPackPlaceholder(NonEmptyPackConstraintDecl* constraintDecl)
    {
        // A non-empty-pack witness is parameterized by the pack being tested.
        // While solving, the pack reference itself is the right placeholder:
        // later work may prove its cardinality and replace this declaration
        // witness with the final concrete witness.
        if (auto declRefExpr = as<DeclRefExpr>(constraintDecl->packExpr))
        {
            auto packDecl = getDeclRef(m_astBuilder, declRefExpr).getDecl();
            if (auto typePackDecl = as<GenericTypePackParamDecl>(packDecl))
                return DeclRefType::create(m_astBuilder, makeDeclRef(typePackDecl));
            if (auto valuePackDecl = as<GenericValuePackParamDecl>(packDecl))
            {
                return m_astBuilder->getOrCreate<DeclRefIntVal>(
                    valuePackDecl->getType(),
                    makeDeclRef(valuePackDecl));
            }
        }
        return nullptr;
    }

    Val* getTypeParamArg(GenericTypeParamDeclBase* typeParam, bool allowPlaceholders)
    {
        if (as<GenericTypePackParamDecl>(typeParam))
            return getTypePackParamArg(typeParam);

        Val* solvedArg = nullptr;
        bool hasSolvedArg = tryGetSolvedTypeArg(typeParam, solvedArg);
        if (hasNonDefaultNonBoundConstraint(typeParam))
            return hasSolvedArg ? solvedArg : nullptr;

        Val* defaultArg = nullptr;
        if (m_defaultArgValues.tryGetValue(typeParam, defaultArg))
            return defaultArg;
        if (m_defaultArgWorkItemIndexByDecl.containsKey(typeParam))
        {
            if (allowPlaceholders)
                return tryGetDefaultGenericArgVal(m_visitor, typeParam);
            return nullptr;
        }

        if (hasSolvedArg)
            return solvedArg;
        if (allowPlaceholders)
            return DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(typeParam));
        return nullptr;
    }

    Val* getTypePackParamArg(GenericTypeParamDeclBase* typeParam)
    {
        auto& solvedArgs = m_solvedArgs[typeParam->parentDecl];
        if (typeParam->parameterIndex >= solvedArgs.getCount())
            return m_astBuilder->getTypePack(ArrayView<Type*>());

        auto& types = solvedArgs[typeParam->parameterIndex].types;
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

    bool tryGetSolvedTypeArg(GenericTypeParamDeclBase* typeParam, Val*& outArg)
    {
        auto& solvedArgs = m_solvedArgs[typeParam->parentDecl];
        if (typeParam->parameterIndex >= solvedArgs.getCount())
            return false;

        auto& types = solvedArgs[typeParam->parameterIndex].types;
        if (types.getCount() != 1 || !types[0])
            return false;
        outArg = types[0].type;
        return true;
    }

    Val* getValuePackParamArg(GenericValuePackParamDecl* valPackParam)
    {
        auto& solvedArgs = m_solvedArgs[valPackParam->parentDecl];
        if (valPackParam->parameterIndex >= solvedArgs.getCount())
            return m_astBuilder->getIntValPack(ArrayView<IntVal*>());

        auto val = solvedArgs[valPackParam->parameterIndex].val;
        return val ? val : m_astBuilder->getIntValPack(ArrayView<IntVal*>());
    }

    Val* getValueParamArg(GenericValueParamDecl* valParam, bool allowPlaceholders)
    {
        Val* solvedArg = tryGetSolvedValueArg(valParam);
        if (hasNonDefaultNonBoundConstraint(valParam))
            return solvedArg;

        Val* defaultArg = nullptr;
        if (m_defaultArgValues.tryGetValue(valParam, defaultArg))
            return defaultArg;
        if (m_defaultArgWorkItemIndexByDecl.containsKey(valParam))
        {
            if (allowPlaceholders)
                return tryGetDefaultGenericArgVal(m_visitor, valParam);
            return nullptr;
        }

        if (solvedArg)
            return solvedArg;
        if (allowPlaceholders)
        {
            auto valParamRef = makeDeclRef(as<VarDeclBase>(valParam));
            return m_astBuilder->getOrCreate<DeclRefIntVal>(valParam->getType(), valParamRef);
        }
        return nullptr;
    }

    Val* tryGetSolvedValueArg(GenericValueParamDecl* valParam)
    {
        auto& solvedArgs = m_solvedArgs[valParam->parentDecl];
        if (valParam->parameterIndex >= solvedArgs.getCount())
            return nullptr;
        return solvedArgs[valParam->parameterIndex].val;
    }

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

    DeclRef<GenericDecl> getRootGenericDeclRef(GenericDecl* genericDecl)
    {
        if (as<DirectDeclRef>(m_genericDeclRef.declRefBase))
            return makeDeclRef(genericDecl);
        return m_genericDeclRef;
    }

    bool appendImplicitWitnessArg(GenericDecl* genericDecl, Decl* constraintDecl)
    {
        if (auto genericTypeConstraintDecl = as<GenericTypeConstraintDecl>(constraintDecl))
            return appendGenericTypeConstraintWitness(genericDecl, genericTypeConstraintDecl);
        if (auto typeCoercionConstraintDecl = as<TypeCoercionConstraintDecl>(constraintDecl))
            return appendTypeCoercionWitness(genericDecl, typeCoercionConstraintDecl);
        if (auto nonEmptyConstraintDecl = as<NonEmptyPackConstraintDecl>(constraintDecl))
            return appendNonEmptyPackWitness(genericDecl, nonEmptyConstraintDecl);
        if (auto hasDiffTypeInfoConstraintDecl = as<HasDiffTypeInfoConstraintDecl>(constraintDecl))
            return appendHasDiffTypeInfoWitness(genericDecl, hasDiffTypeInfoConstraintDecl);
        return true;
    }

    bool appendGenericTypeConstraintWitness(
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

        auto subTypeWitness = findSubtypeWitnessForConstraint(constraintDecl, sub, sup);
        return recordSubtypeWitnessArg(genericDecl, constraintDecl, subTypeWitness);
    }

    bool recordConstrainedGenericParam(DeclRef<GenericTypeConstraintDecl> constraintDeclRef)
    {
        if (auto subDeclRefType = as<DeclRefType>(constraintDeclRef.getDecl()->sub.type))
        {
            m_constrainedGenericParams.add(subDeclRefType->getDeclRef().getDecl());
            return true;
        }

        if (auto subEachType = as<EachType>(constraintDeclRef.getDecl()->sub.type))
        {
            if (auto elementDeclRefType = subEachType->getElementDeclRefType())
            {
                m_constrainedGenericParams.add(elementDeclRefType->getDeclRef().getDecl());
                return true;
            }
            return false;
        }

        return true;
    }

    SubtypeWitness* findSubtypeWitnessForConstraint(
        GenericTypeConstraintDecl* constraintDecl,
        Type* sub,
        Type* sup)
    {
        // A witness item first asks the ordinary subtype engine for a proof,
        // because concrete conformances, extension conformances, inherited
        // interfaces, packs, and cached answers all live there.
        SubtypeWitness* subTypeWitness = nullptr;
        if (sub == m_system->subTypeForAdditionalWitnesses)
        {
            m_system->additionalSubtypeWitnesses->tryGetValue(sup, subTypeWitness);
        }
        else
        {
            subTypeWitness = m_visitor->isSubtype(
                sub,
                sup,
                m_system->additionalSubtypeWitnesses ? IsSubTypeOptions::NoCaching
                                                     : IsSubTypeOptions::None);
        }

        // Generic solving also sees abstract proofs that the subtype cache is
        // not allowed to invent.  A forwarded generic parameter can reuse its
        // own bound witness, and an associated-type projection can reuse the
        // constraint declared on the associated type that introduced it.
        if (!subTypeWitness)
            subTypeWitness = findSubtypeWitnessForGenericParamBound(sub, sup);
        if (!subTypeWitness)
            subTypeWitness = findSubtypeWitnessForAssociatedTypeConstraint(sub, sup);

        if (constraintDecl->isEqualityConstraint && !isTypeEqualityWitness(subTypeWitness))
            subTypeWitness = nullptr;
        return subTypeWitness;
    }

    SubtypeWitness* findSubtypeWitnessForGenericParamBound(Type* sub, Type* sup)
    {
        // A generic type parameter can be passed as the explicit answer for a
        // different generic parameter.  When that happens, the proof that the
        // answer satisfies a required interface may already live on the source
        // parameter's own declaration, so the witness item can reuse that
        // declaration rather than waiting for a concrete type.
        auto subDeclRefType = as<DeclRefType>(sub);
        if (!subDeclRefType)
            return nullptr;

        auto typeParamDecl = as<GenericTypeParamDeclBase>(subDeclRefType->getDeclRef().getDecl());
        if (!typeParamDecl)
            return nullptr;

        auto genericDecl = as<GenericDecl>(typeParamDecl->parentDecl);
        if (!genericDecl)
            return nullptr;

        // Bound declarations are ordinary hidden witness parameters on the
        // generic that owns the source type parameter.  Default substitutions
        // give those bounds the same self-reference placeholders used while
        // checking the generic body, which is exactly the form we see when the
        // source parameter is forwarded as an argument to another generic.
        for (auto member : genericDecl->getDirectMemberDecls())
        {
            auto boundDecl = as<GenericTypeConstraintDecl>(member);
            if (!boundDecl)
                continue;

            auto boundDeclRef = createDefaultSubstitutionsIfNeeded(
                                    m_astBuilder,
                                    m_visitor,
                                    boundDecl->getDefaultDeclRef())
                                    .as<GenericTypeConstraintDecl>();
            if (!boundDeclRef)
                continue;

            auto boundSub = getSub(m_astBuilder, boundDeclRef);
            auto boundSup = getSup(m_astBuilder, boundDeclRef);
            if (!boundSub || !boundSup)
                continue;
            if (!boundSub->equals(sub) || !boundSup->equals(sup))
                continue;

            return m_astBuilder->getDeclaredSubtypeWitness(sub, sup, boundDeclRef);
        }

        return nullptr;
    }

    SubtypeWitness* findSubtypeWitnessForAssociatedTypeConstraint(Type* sub, Type* sup)
    {
        // Associated-type lookups carry the witness used to open the interface:
        // `T.A` is really `Lookup(T, witness T : IFoo, A)`.  If `A` itself
        // declares a constraint such as `associatedtype A : IBar`, that
        // constraint is a solved witness for `T.A : IBar`.
        auto subDeclRefType = as<DeclRefType>(sub);
        if (!subDeclRefType)
            return nullptr;

        auto associatedTypeDeclRef = subDeclRefType->getDeclRef().as<AssocTypeDecl>();
        if (!associatedTypeDeclRef)
            return nullptr;

        // The member constraint has to be referenced through the same lookup
        // path as the associated type.  That preserves the dependency on the
        // original `T : IFoo` witness and lets later substitution or lowering
        // follow the proof back through the right interface requirement.
        for (auto constraintDecl :
             associatedTypeDeclRef.getDecl()->getMembersOfType<GenericTypeConstraintDecl>())
        {
            auto constraintDeclRef =
                m_astBuilder->getMemberDeclRef(associatedTypeDeclRef, constraintDecl)
                    .as<GenericTypeConstraintDecl>();
            if (!constraintDeclRef)
                continue;

            auto constraintSub = getSub(m_astBuilder, constraintDeclRef);
            auto constraintSup = getSup(m_astBuilder, constraintDeclRef);
            if (!constraintSub || !constraintSup)
                continue;
            if (!constraintSub->equals(sub) || !constraintSup->equals(sup))
                continue;

            return m_astBuilder->getDeclaredSubtypeWitness(sub, sup, constraintDeclRef);
        }

        return nullptr;
    }

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
            m_witnessArgValues[constraintDecl] = subTypeWitness;
            m_finalWitnessCost += subTypeWitness->getOverloadResolutionCost();
            return true;
        }

        if (!subTypeWitness && constraintIsOptional)
        {
            auto noneWitness = m_astBuilder->getOrCreate<NoneWitness>();
            m_witnessArgValues[constraintDecl] = noneWitness;
            m_finalWitnessCost += kConversionCost_FailedOptionalConstraint;
            return true;
        }

        return false;
    }

    bool appendTypeCoercionWitness(
        GenericDecl* genericDecl,
        TypeCoercionConstraintDecl* constraintDecl)
    {
        auto& genericArgs = m_args[genericDecl];
        Count oldArgCount = genericArgs.getCount();
        if (!addTypeCoercionWitnessToArgs(
                m_astBuilder,
                m_visitor,
                constraintDecl,
                m_genericDeclRef,
                nullptr,
                &m_constrainedGenericParams,
                genericArgs,
                false))
            return false;
        if (genericArgs.getCount() <= oldArgCount)
            return false;
        m_witnessArgValues[constraintDecl] = genericArgs[genericArgs.getCount() - 1];
        return true;
    }

    bool appendNonEmptyPackWitness(
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

        m_witnessArgValues[constraintDecl] = m_astBuilder->getNonEmptyPackWitness(constrainedArg);
        return true;
    }

    bool appendHasDiffTypeInfoWitness(
        GenericDecl* genericDecl,
        HasDiffTypeInfoConstraintDecl* constraintDecl)
    {
        auto& genericArgs = m_args[genericDecl];
        Count oldArgCount = genericArgs.getCount();
        if (!addHasDiffTypeInfoWitnessToArgs(
                m_astBuilder,
                m_visitor,
                constraintDecl,
                m_genericDeclRef,
                nullptr,
                &m_constrainedGenericParams,
                genericArgs,
                false))
            return false;
        if (genericArgs.getCount() <= oldArgCount)
            return false;
        m_witnessArgValues[constraintDecl] = genericArgs[genericArgs.getCount() - 1];
        return true;
    }

    bool verifyOrdinaryConstraintsSatisfied()
    {
        for (auto c : m_system->constraints)
        {
            if (!c.satisfied)
                return false;
        }
        return true;
    }

    void addUnconstrainedGenericParamCost()
    {
        for (auto genericDecl : m_genericDecls)
        {
            for (auto typeParamDecl : genericDecl->getMembersOfType<GenericTypeParamDecl>())
            {
                if (!m_constrainedGenericParams.contains(typeParamDecl))
                    m_outBaseCost += kConversionCost_UnconstraintGenericParam;
            }
        }
    }

private:
    SemanticsVisitor* m_visitor = nullptr;
    ASTBuilder* m_astBuilder = nullptr;
    ConstraintSystem* m_system = nullptr;
    DeclRef<GenericDecl> m_genericDeclRef;
    ArrayView<Val*> m_knownGenericArgs;
    Count m_knownGenericArgCount = 0;
    ConversionCost& m_outBaseCost;

    List<GenericDecl*> m_genericDecls;
    Dictionary<Decl*, ShortList<Val*>> m_args;
    Dictionary<Decl*, ShortList<SolvedArg>> m_solvedArgs;
    Dictionary<Decl*, Val*> m_defaultArgValues;
    Dictionary<Decl*, Val*> m_witnessArgValues;
    HashSet<Decl*> m_constrainedGenericParams;

    ShortList<SolverWorkItem, 16> m_solverWorkItems;
    Dictionary<Decl*, Index> m_defaultArgWorkItemIndexByDecl;
    Dictionary<Decl*, Index> m_witnessWorkItemIndexByDecl;
    ShortList<Index, 16> m_workList;
    Index m_workListReadIndex = 0;
    ShortList<int, 16> m_workItemDone;
    ShortList<int, 16> m_workItemQueued;
    Count m_collectedConstraintCount = 0;
    ConversionCost m_finalWitnessCost = kConversionCost_None;
};

DeclRef<Decl> SemanticsVisitor::trySolveConstraintSystem(
    ConstraintSystem* system,
    DeclRef<GenericDecl> genericDeclRef,
    ArrayView<Val*> knownGenericArgs,
    ConversionCost& outBaseCost)
{
    // The solver reads generic members while collecting bounds, defaults, and
    // witnesses, so the declaration must be ready for lookup before we enter it.
    ensureDecl(genericDeclRef.getDecl(), DeclCheckState::ReadyForLookup);

    // All state for the two-phase algorithm lives in the solver object.  The
    // public entry point stays as a small adapter from semantic checking to the
    // generic-argument solver.
    GenericConstraintSystemSolver
        solver(this, system, genericDeclRef, knownGenericArgs, outBaseCost);
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
    constraint.isGenericParamBound = unificationContext.genericParamBoundConstraint;
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
    constraint.isGenericParamBound = unifyCtx.genericParamBoundConstraint;

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
        constraint.isGenericParamBound = unifyCtx.genericParamBoundConstraint;
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
