// slang-check-type.cpp
#include "slang-ast-print.h"
#include "slang-check-impl.h"
#include "slang-rich-diagnostics.h"

// This file implements semantic checking logic related to types
// and type expressions (aka `TypeRepr`).

namespace Slang
{
Type* checkProperType(Linkage* linkage, TypeExp typeExp, DiagnosticSink* sink)
{
    SharedSemanticsContext sharedSemanticsContext(
        linkage,
        linkage->m_optionSet.getLanguageVersion(),
        sink);
    SemanticsVisitor visitor(&sharedSemanticsContext);

    SLANG_AST_BUILDER_RAII(linkage->getASTBuilder());

    auto typeOut = visitor.CheckProperType(typeExp);
    return typeOut.type;
}

Type* getPointedToTypeIfCanImplicitDeref(Type* type)
{
    if (auto ptrLike = as<PointerLikeType>(type))
    {
        return ptrLike->getElementType();
    }
    else if (auto ptrType = as<PtrType>(type))
    {
        return ptrType->getValueType();
    }
    else if (auto refType = as<ExplicitRefType>(type))
    {
        return refType->getValueType();
    }
    return nullptr;
}

Expr* SemanticsVisitor::TranslateTypeNodeImpl(Expr* node)
{
    if (!node)
        return nullptr;

    auto expr = CheckTerm(node);
    expr = ExpectATypeRepr(expr);
    return expr;
}

Type* SemanticsVisitor::ExtractTypeFromTypeRepr(Expr* typeRepr)
{
    if (!typeRepr)
        return nullptr;
    if (auto typeType = as<TypeType>(typeRepr->type))
    {
        return typeType->getType();
    }
    return m_astBuilder->getErrorType();
}

Type* SemanticsVisitor::TranslateTypeNode(Expr* node)
{
    if (!node)
        return nullptr;
    auto typeRepr = TranslateTypeNodeImpl(node);
    return ExtractTypeFromTypeRepr(typeRepr);
}

TypeExp SemanticsVisitor::TranslateTypeNodeForced(TypeExp const& typeExp)
{
    auto typeRepr = TranslateTypeNodeImpl(typeExp.exp);

    TypeExp result;
    result.exp = typeRepr;
    result.type = ExtractTypeFromTypeRepr(typeRepr);
    return result;
}

TypeExp SemanticsVisitor::TranslateTypeNode(TypeExp const& typeExp)
{
    // HACK(tfoley): It seems that in some cases we end up re-checking
    // syntax that we've already checked. We need to root-cause that
    // issue, but for now a quick fix in this case is to early
    // exist if we've already got a type associated here:
    if (typeExp.type)
    {
        return typeExp;
    }
    return TranslateTypeNodeForced(typeExp);
}

Type* SemanticsVisitor::getRemovedModifierType(ModifiedType* modifiedType, ModifierVal* modifier)
{
    if (modifiedType->getModifierCount() == 1)
        return modifiedType->getBase();
    List<Val*> newModifiers;
    for (Index i = 0; i < modifiedType->getModifierCount(); i++)
    {
        auto m = modifiedType->getModifier(i);
        if (m == modifier)
            continue;
        newModifiers.add(m);
    }
    return m_astBuilder->getModifiedType(modifiedType->getBase(), newModifiers);
}

Type* SemanticsVisitor::getConstantBufferType(Type* elementType, Type* layoutType)
{
    auto iBufferDataLayoutType = m_astBuilder->getSharedASTBuilder()->getIBufferDataLayoutType();
    auto witness = isSubtype(layoutType, iBufferDataLayoutType, IsSubTypeOptions());
    return m_astBuilder->getConstantBufferType(elementType, layoutType, witness);
}

static String _getExprName(Expr* expr)
{
    if (auto overloadedExpr = as<OverloadedExpr>(expr))
    {
        if (overloadedExpr->name)
            return String(overloadedExpr->name->text);
    }
    if (auto declRefExpr = as<DeclRefExpr>(expr))
    {
        if (declRefExpr->name)
            return String(declRefExpr->name->text);
    }
    return String();
}

Expr* SemanticsVisitor::ExpectATypeRepr(Expr* expr)
{
    auto originalExpr = expr;

    if (auto overloadedExpr = as<OverloadedExpr>(expr))
    {
        expr = resolveOverloadedExpr(overloadedExpr, LookupMask::type);
    }

    if (const auto typeType = as<TypeType>(expr->type); typeType)
    {
        return expr;
    }
    else if (const auto errorType = as<ErrorType>(expr->type); errorType)
    {
        return expr;
    }

    auto name = _getExprName(originalExpr);
    if (name.getLength() == 0)
        name = _getExprName(expr);

    StringBuilder whatWeGot;
    whatWeGot << "a '" << (expr->type.type ? expr->type.type->toString() : toSlice("null")) << "'";
    if (name.getLength())
        whatWeGot << " for '" << name << "'";

    getSink()->diagnose(
        Diagnostics::ExpectedAType{.whatWeGot = whatWeGot.produceString(), .expr = expr});
    return CreateErrorExpr(expr);
}

Type* SemanticsVisitor::ExpectAType(Expr* expr)
{
    auto typeRepr = ExpectATypeRepr(expr);
    if (auto typeType = as<TypeType>(typeRepr->type))
    {
        return typeType->getType();
    }
    return m_astBuilder->getErrorType();
}

Type* SemanticsVisitor::ExtractGenericArgType(Expr* exp)
{
    return ExpectAType(exp);
}

IntVal* SemanticsVisitor::ExtractGenericArgInteger(
    Expr* exp,
    Type* genericParamType,
    ConstantFoldingKind kind,
    DiagnosticSink* sink,
    ConstantFoldingCircularityInfo* circularityInfo)
{
    IntVal* val = CheckIntegerConstantExpression(
        exp,
        genericParamType ? IntegerConstantExpressionCoercionType::SpecificType
                         : IntegerConstantExpressionCoercionType::AnyInteger,
        genericParamType,
        kind,
        sink,
        circularityInfo);
    if (val)
        return val;

    // If the argument expression could not be coerced to an integer
    // constant expression in context, then we will instead construct
    // a dummy "error" value to represent the result.
    //
    val = m_astBuilder->getOrCreate<ErrorIntVal>(m_astBuilder->getIntType());
    return val;
}

IntVal* SemanticsVisitor::ExtractGenericArgInteger(Expr* exp, Type* genericParamType)
{
    return ExtractGenericArgInteger(
        exp,
        genericParamType,
        ConstantFoldingKind::LinkTime,
        getSink(),
        nullptr);
}

Val* SemanticsVisitor::ExtractGenericArgVal(
    Expr* exp,
    ConstantFoldingCircularityInfo* circularityInfo)
{
    if (auto overloadedExpr = as<OverloadedExpr>(exp))
    {
        // assume that if it is overloaded, we want a type
        exp = resolveOverloadedExpr(overloadedExpr, LookupMask::type);
    }
    if (auto typeType = as<TypeType>(exp->type))
    {
        return typeType->getType();
    }
    else if (const auto errorType = as<ErrorType>(exp->type); errorType)
    {
        return exp->type.type;
    }
    else
    {
        if (!exp->type.type)
        {
            CheckExpr(exp);
        }
        return ExtractGenericArgInteger(
            exp,
            nullptr,
            ConstantFoldingKind::LinkTime,
            getSink(),
            circularityInfo);
    }
}

Type* SemanticsVisitor::InstantiateGenericType(
    DeclRef<GenericDecl> genericDeclRef,
    List<Expr*> const& args)
{
    List<Val*> evaledArgs;

    for (auto argExpr : args)
    {
        evaledArgs.add(ExtractGenericArgVal(argExpr));
    }

    DeclRef<Decl> innerDeclRef =
        m_astBuilder->getGenericAppDeclRef(genericDeclRef, evaledArgs.getArrayView());
    return DeclRefType::create(m_astBuilder, innerDeclRef);
}

bool isManagedType(Type* type)
{
    if (auto boxType = as<ExistentialBoxType>(type))
        type = boxType->getInterfaceType();
    if (auto declRefValueType = as<DeclRefType>(type))
    {
        auto decl = declRefValueType->getDeclRef().getDecl();
        if (as<ClassDecl>(decl))
            return true;
        if (as<InterfaceDecl>(decl) && decl->findModifier<ComInterfaceAttribute>())
            return true;
    }
    return false;
}

bool SemanticsVisitor::coerceToAliasableTypeImpl(
    TypeExp const& typeExp,
    Type** outType,
    DiagnosticSink* diagSink)
{
    SLANG_RELEASE_ASSERT(outType);
    Type* result = nullptr;
    Type* type = typeExp.type;
    auto originalExpr = typeExp.exp;
    auto expr = originalExpr;
    if (!type && expr)
    {
        expr = maybeResolveOverloadedExpr(expr, LookupMask::type, diagSink);

        if (auto typeType = as<TypeType>(expr->type))
        {
            type = typeType->getType();
        }
    }

    if (!type)
    {
        // Only output diagnostic if we have a sink.
        if (diagSink)
        {
            // This function *can* be called with typeExp with both exp and type = nullptr.
            // Previous behavior didn't output a diagnostic if originalExpr was null, so this keeps
            // that behavior.
            //
            // Additional we check for ErrorType on expr, because if it's set a diagnostic has
            // already been output via previous code or via maybeResolveOverloadedExpr.
            if (originalExpr && (expr == nullptr || as<ErrorType>(expr->type) == nullptr))
            {
                // The diagnostic for expectedAType wants to say what it 'got'.
                // The solution given here, currently is to just use the node name.
                // How useful that might be could depend, and perhaps some other mechanism
                // that catagorized 'what' the wrong thing was is. For now this seems sufficient.
                //
                // Note that use originalExpr (not expr) because we want original expr for
                // diagnostic.

                // Get the AST node type info, so we can output a 'got' name
                auto exprName = _getExprName(originalExpr);
                StringBuilder whatWeGotStr;
                whatWeGotStr << "a '" << originalExpr->getClass().getName() << "'";
                if (exprName.getLength())
                    whatWeGotStr << " for '" << exprName << "'";
                diagSink->diagnose(Diagnostics::ExpectedAType{
                    .whatWeGot = whatWeGotStr.produceString(),
                    .expr = originalExpr});
            }
        }

        *outType = nullptr;
        return false;
    }

    if (auto genericDeclRefType = as<GenericDeclRefType>(type))
    {
        // We use declared defaults when completing a reference to a generic declaration.
        // The programmer must write an argument list for a missing default or a parameter pack,
        // even when call inference could find an argument. For example, `Tuple<>` names the empty
        // tuple type, but `Tuple` without an argument list cannot be completed here.
        auto genericDeclRef = genericDeclRefType->getDeclRef();
        ensureDecl(genericDeclRef, DeclCheckState::CanSpecializeGeneric);
        for (auto member : genericDeclRef.getDecl()->getDirectMemberDecls())
        {
            bool requiresArgumentList = false;
            if (auto typeParam = as<GenericTypeParamDecl>(member))
                requiresArgumentList = !typeParam->initType.type;
            else if (auto valueParam = as<GenericValueParamDecl>(member))
                requiresArgumentList = !valueParam->initExpr;
            else
                requiresArgumentList =
                    as<GenericTypePackParamDecl>(member) || as<GenericValuePackParamDecl>(member);
            if (requiresArgumentList)
            {
                if (diagSink)
                    diagSink->diagnose(Diagnostics::GenericTypeNeedsArgs{
                        .type = type,
                        .location = typeExp.exp ? typeExp.exp->loc : SourceLoc()});
                *outType = m_astBuilder->getErrorType();
                return false;
            }
        }

        // We use `trySolveGenericArguments` to substitute dependent defaults and their
        // conformance witnesses together. A constraint's interface remains an interface here.
        GenericArgumentInferenceFailure failure;
        GenericInferenceContext inferenceContext;
        inferenceContext.genericDecl = genericDeclRef.getDecl();
        inferenceContext.failure = &failure;
        inferenceContext.applicationLoc = typeExp.exp ? typeExp.exp->loc : SourceLoc();
        ConversionCost solveCost = kConversionCost_None;
        auto specializedDeclRef = trySolveGenericArguments(
            _Move(inferenceContext),
            genericDeclRef,
            ArrayView<Val*>(),
            solveCost);
        if (!specializedDeclRef)
        {
            if (diagSink)
            {
                if (failure.kind ==
                    GenericArgumentInferenceFailure::Kind::InterfaceConformanceNotSatisfied)
                {
                    auto& conformance = failure.interfaceConformanceNotSatisfied;
                    diagSink->diagnose(Diagnostics::TypeArgumentDoesNotConformToInterface{
                        .typeArg = conformance.subType,
                        .interface = conformance.supType,
                        .location = conformance.location});
                }
                else if (
                    failure.kind ==
                    GenericArgumentInferenceFailure::Kind::GenericConstraintNotSatisfied)
                {
                    auto& constraint = failure.genericConstraintNotSatisfied;
                    diagSink->diagnose(Diagnostics::GenericArgumentDoesNotSatisfyConstraint{
                        .constraint = ASTPrinter::getGenericConstraintString(
                            constraint.constraintDecl,
                            m_astBuilder),
                        .location = constraint.location});
                    diagSink->diagnose(Diagnostics::SeeGenericConstraintDeclaration{
                        .location = constraint.constraintLoc});
                }
                else
                {
                    diagSink->diagnose(Diagnostics::GenericTypeNeedsArgs{
                        .type = type,
                        .location = typeExp.exp ? typeExp.exp->loc : SourceLoc()});
                }
            }
            *outType = m_astBuilder->getErrorType();
            return false;
        }
        result = DeclRefType::create(m_astBuilder, specializedDeclRef);
    }
    // All other inputs retain their existing type.
    if (!result)
    {
        result = type;
    }

    if (auto ptrType = as<PtrType>(result))
    {
        if (isManagedType(ptrType->getValueType()))
        {
            if (diagSink)
                diagSink->diagnose(Diagnostics::CannotDefinePtrTypeToManagedResource{
                    .location = typeExp.exp ? typeExp.exp->loc : SourceLoc()});
            *outType = m_astBuilder->getErrorType();
            return false;
        }
    }
    *outType = result;
    return true;
}

TypeExp SemanticsVisitor::coerceToAliasableType(TypeExp const& typeExp)
{
    TypeExp result = typeExp;
    coerceToAliasableTypeImpl(typeExp, &result.type, getSink());
    return result;
}

TypeExp SemanticsVisitor::checkAliasableType(TypeExp typeExp)
{
    return coerceToAliasableType(TranslateTypeNode(typeExp));
}

TypeExp SemanticsVisitor::checkInterfaceType(TypeExp typeExp)
{
    auto result = checkAliasableType(typeExp);
    if (as<ErrorType>(result.type))
        return result;
    if (!isValidGenericConstraintType(result.type))
    {
        getSink()->diagnose(Diagnostics::ExpectedInterfaceType{
            .type = result.type,
            .location = result.exp ? result.exp->loc : SourceLoc()});
        result.type = m_astBuilder->getErrorType();
    }
    return result;
}

Type* SemanticsVisitor::formExistentialBoxType(
    TypeExp const& interfaceType,
    DiagnosticSink* diagSink)
{
    if (as<ErrorType>(interfaceType.type))
        return interfaceType.type;
    if (as<AndType>(interfaceType.type))
    {
        if (diagSink)
            diagSink->diagnose(Diagnostics::ExistentialBoxConjunctionNotSupported{
                .location = interfaceType.exp ? interfaceType.exp->loc : SourceLoc()});
        return m_astBuilder->getErrorType();
    }
    return m_astBuilder->getExistentialBoxType(interfaceType.type);
}

bool SemanticsVisitor::CoerceToProperTypeImpl(
    TypeExp const& typeExp,
    Type** outProperType,
    DiagnosticSink* diagSink)
{
    if (!coerceToAliasableTypeImpl(typeExp, outProperType, diagSink))
        return false;
    auto type = *outProperType;
    if (!isValidGenericConstraintType(type))
        return true;

    // A conjunction is a valid constraint or alias, but we do not implement a box that
    // packages witnesses for several interfaces. We diagnose formation before a value use
    // could defer the failure to lowering, including when the box type is only aliased.
    if (as<AndType>(type))
    {
        *outProperType = formExistentialBoxType(TypeExp(typeExp.exp, type), diagSink);
        return false;
    }
    if (isSlang202cOrLater(this))
    {
        if (diagSink)
            diagSink->diagnose(Diagnostics::ExplicitDynRequired{
                .type = type,
                .location = typeExp.exp ? typeExp.exp->loc : SourceLoc()});
        *outProperType = m_astBuilder->getErrorType();
        return false;
    }
    *outProperType = m_astBuilder->getExistentialBoxType(type);
    return true;
}

TypeExp SemanticsVisitor::CoerceToProperType(TypeExp const& typeExp)
{
    TypeExp result = typeExp;
    CoerceToProperTypeImpl(typeExp, &result.type, getSink());
    return result;
}

TypeExp SemanticsVisitor::tryCoerceToProperType(TypeExp const& typeExp)
{
    TypeExp result = typeExp;
    if (!CoerceToProperTypeImpl(typeExp, &result.type, nullptr))
        return TypeExp();
    return result;
}

TypeExp SemanticsVisitor::CheckProperType(TypeExp typeExp)
{
    return CoerceToProperType(TranslateTypeNode(typeExp));
}

TypeExp SemanticsVisitor::CoerceToUsableType(TypeExp const& typeExp, Decl* decl)
{
    TypeExp result = CoerceToProperType(typeExp);
    Type* type = result.type;
    if (auto basicType = as<BasicExpressionType>(type))
    {
        // TODO: `void` shouldn't be a basic type, to make this easier to avoid
        if (basicType->getBaseType() == BaseType::Void)
        {
            // TODO(tfoley): pick the right diagnostic message
            getSink()->diagnose(Diagnostics::InvalidTypeVoid{.location = result.exp->loc});
            result.type = m_astBuilder->getErrorType();
            return result;
        }
    }

    // Matrix row/column dimension validation is intentionally *not* done at
    // type-construction time. Rejecting a matrix here would also reject uses
    // that may be valid on specific targets (e.g. the CPU target, or
    // capability-gated code paths). Instead, matrices with out-of-range
    // dimensions are caught at entry-point varying sites by a rule in
    // `validateEntryPoint`, and downstream codegen for other contexts is
    // responsible for its own diagnostics.

    // A type pack is not a usable type other than for defining parameters.
    if (!as<ParamDecl>(decl) && isTypePack(type))
    {
        getSink()->diagnose(
            Diagnostics::ImproperUseOfType{.type = typeExp.type, .expr = typeExp.exp});
        result.type = m_astBuilder->getErrorType();
        return result;
    }

    // Optional<T> cannot wrap a resource/opaque type (e.g. SamplerState, textures, buffers),
    // nor a struct that transitively contains such a type.
    if (auto optType = as<OptionalType>(type))
    {
        auto valueType = optType->getValueType();
        if (typeTransitivelyContainsOpaqueHandle(this, valueType))
        {
            getSink()->diagnose(Diagnostics::OptionalCannotWrapResourceType{
                .type = valueType,
                .expr = typeExp.exp});
            result.type = m_astBuilder->getErrorType();
            return result;
        }
    }

    return result;
}

TypeExp SemanticsVisitor::CheckUsableType(TypeExp typeExp, Decl* decl)
{
    return CoerceToUsableType(TranslateTypeNode(typeExp), decl);
}

bool SemanticsVisitor::ValuesAreEqual(IntVal* left, IntVal* right)
{
    if (left == right)
        return true;

    if (auto leftConst = as<ConstantIntVal>(left))
    {
        if (auto rightConst = as<ConstantIntVal>(right))
        {
            return leftConst->getValue() == rightConst->getValue();
        }
    }

    if (auto leftVar = as<DeclRefIntVal>(left))
    {
        if (auto rightVar = as<DeclRefIntVal>(right))
        {
            return leftVar->getDeclRef().equals(rightVar->getDeclRef());
        }
        else if (const auto rightPoly = as<PolynomialIntVal>(right); rightPoly)
        {
            return right->equals(leftVar);
        }
    }
    if (auto leftVar = as<PolynomialIntVal>(left))
    {
        return leftVar->equals(right);
    }
    return false;
}

VectorExpressionType* SemanticsVisitor::createVectorType(Type* elementType, IntVal* elementCount)
{
    return m_astBuilder->getVectorType(elementType, elementCount);
}

Expr* SemanticsExprVisitor::visitSharedTypeExpr(SharedTypeExpr* expr)
{
    if (!expr->type.Ptr())
    {
        expr->base = checkAliasableType(expr->base);
        expr->type = m_astBuilder->getTypeType(expr->base.type);
    }
    return expr;
}

Expr* SemanticsExprVisitor::visitHLSLUnsignedTypeExpr(HLSLUnsignedTypeExpr* expr)
{
    // This spelling denotes the built-in type directly, so a declaration named `uint` cannot
    // shadow it through ordinary name lookup.
    expr->type = m_astBuilder->getTypeType(m_astBuilder->getUIntType());
    return expr;
}

} // namespace Slang
