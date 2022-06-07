// slang-check-type.cpp
#include "slang-check-impl.h"

// This file implements semantic checking logic related to types
// and type expressions (aka `TypeRepr`).

namespace Slang
{
    Type* checkProperType(
        Linkage*        linkage,
        TypeExp         typeExp,
        DiagnosticSink* sink)
    {
        SharedSemanticsContext sharedSemanticsContext(
            linkage,
            nullptr,
            sink);
        SemanticsVisitor visitor(&sharedSemanticsContext);


        auto typeOut = visitor.CheckProperType(typeExp);
        return typeOut.type;
    }

    Expr* SemanticsVisitor::TranslateTypeNodeImpl(Expr* node)
    {
        if (!node) return nullptr;

        auto expr = CheckTerm(node);
        expr = ExpectATypeRepr(expr);
        return expr;
    }

    Type* SemanticsVisitor::ExtractTypeFromTypeRepr(Expr* typeRepr)
    {
        if (!typeRepr) return nullptr;
        if (auto typeType = as<TypeType>(typeRepr->type))
        {
            return typeType->type;
        }
        return m_astBuilder->getErrorType();
    }

    Type* SemanticsVisitor::TranslateTypeNode(Expr* node)
    {
        if (!node) return nullptr;
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

    Expr* SemanticsVisitor::ExpectATypeRepr(Expr* expr)
    {
        if (auto overloadedExpr = as<OverloadedExpr>(expr))
        {
            expr = resolveOverloadedExpr(overloadedExpr, LookupMask::type);
        }

        if (auto typeType = as<TypeType>(expr->type))
        {
            return expr;
        }
        else if (auto errorType = as<ErrorType>(expr->type))
        {
            return expr;
        }

        getSink()->diagnose(expr, Diagnostics::expectedAType, expr->type);
        return CreateErrorExpr(expr);
    }

    Type* SemanticsVisitor::ExpectAType(Expr* expr)
    {
        auto typeRepr = ExpectATypeRepr(expr);
        if (auto typeType = as<TypeType>(typeRepr->type))
        {
            return typeType->type;
        }
        return m_astBuilder->getErrorType();
    }

    Type* SemanticsVisitor::ExtractGenericArgType(Expr* exp)
    {
        return ExpectAType(exp);
    }

    IntVal* SemanticsVisitor::ExtractGenericArgInteger(Expr* exp, DiagnosticSink* sink)
    {
        IntVal* val = CheckIntegerConstantExpression(exp, sink);
        if(val) return val;

        // If the argument expression could not be coerced to an integer
        // constant expression in context, then we will instead construct
        // a dummy "error" value to represent the result.
        //
        val = m_astBuilder->create<ErrorIntVal>();
        return val;
    }

    IntVal* SemanticsVisitor::ExtractGenericArgInteger(Expr* exp)
    {
        return ExtractGenericArgInteger(exp, getSink());
    }

    Val* SemanticsVisitor::ExtractGenericArgVal(Expr* exp)
    {
        if (auto overloadedExpr = as<OverloadedExpr>(exp))
        {
            // assume that if it is overloaded, we want a type
            exp = resolveOverloadedExpr(overloadedExpr, LookupMask::type);
        }
        if (auto typeType = as<TypeType>(exp->type))
        {
            return typeType->type;
        }
        else if (auto errorType = as<ErrorType>(exp->type))
        {
            return exp->type.type;
        }
        else
        {
            if (!exp->type.type)
            {
                CheckExpr(exp);
            }
            return ExtractGenericArgInteger(exp);
        }
    }

    Type* SemanticsVisitor::InstantiateGenericType(
        DeclRef<GenericDecl>        genericDeclRef,
        List<Expr*> const&   args)
    {
        GenericSubstitution* subst = m_astBuilder->create<GenericSubstitution>();
        subst->genericDecl = genericDeclRef.getDecl();
        subst->outer = genericDeclRef.substitutions.substitutions;

        for (auto argExpr : args)
        {
            subst->args.add(ExtractGenericArgVal(argExpr));
        }

        DeclRef<Decl> innerDeclRef;
        innerDeclRef.decl = getInner(genericDeclRef);
        innerDeclRef.substitutions = SubstitutionSet(subst);

        return DeclRefType::create(m_astBuilder, innerDeclRef);
    }

    bool SemanticsVisitor::CoerceToProperTypeImpl(
        TypeExp const&  typeExp,
        Type**   outProperType,
        DiagnosticSink* diagSink)
    {
        Type* type = typeExp.type;
        auto originalExpr = typeExp.exp;
        auto expr = originalExpr;

        if(!type && expr)
        {
            expr = maybeResolveOverloadedExpr(expr, LookupMask::type, diagSink);

            if(auto typeType = as<TypeType>(expr->type))
            {
                type = typeType->type;
            }
        }

        if (!type)
        {
            // Only output diagnostic if we have a sink.
            if (diagSink)
            {
                // This function *can* be called with typeExp with both exp and type = nullptr.
                // Previous behavior didn't output a diagnostic if originalExpr was null, so this keeps that behavior.
                // 
                // Additional we check for ErrorType on expr, because if it's set a diagnostic has already been output via
                // previous code or via maybeResolveOverloadedExpr.
                if (originalExpr && (expr == nullptr || as<ErrorType>(expr->type) == nullptr))
                {
                    // The diagnostic for expectedAType wants to say what it 'got'.
                    // The solution given here, currently is to just use the node name.
                    // How useful that might be could depend, and perhaps some other mechanism
                    // that catagorized 'what' the wrong thing was is. For now this seems sufficient.
                    //
                    // Note that use originalExpr (not expr) because we want original expr for diagnostic.

                    // Get the AST node type info, so we can output a 'got' name
                    auto info = ASTClassInfo::getInfo(originalExpr->astNodeType);
                    diagSink->diagnose(originalExpr, Diagnostics::expectedAType, info->m_name);
                }
            }

            if (outProperType)
            {
                *outProperType = nullptr;
            }
            return false;
        }

        if (auto genericDeclRefType = as<GenericDeclRefType>(type))
        {
            // We are using a reference to a generic declaration as a concrete
            // type. This means we should substitute in any default parameter values
            // if they are available.
            //
            // TODO(tfoley): A more expressive type system would substitute in
            // "fresh" variables and then solve for their values...
            //

            auto genericDeclRef = genericDeclRefType->getDeclRef();
            ensureDecl(genericDeclRef, DeclCheckState::CanSpecializeGeneric);
            List<Expr*> args;
            for (Decl* member : genericDeclRef.getDecl()->members)
            {
                if (auto typeParam = as<GenericTypeParamDecl>(member))
                {
                    if (!typeParam->initType.exp)
                    {
                        if (diagSink)
                        {
                            diagSink->diagnose(typeExp.exp, Diagnostics::genericTypeNeedsArgs, typeExp);
                            *outProperType = m_astBuilder->getErrorType();
                        }
                        return false;
                    }

                    // TODO: this is one place where syntax should get cloned!
                    if (outProperType)
                        args.add(typeParam->initType.exp);
                }
                else if (auto valParam = as<GenericValueParamDecl>(member))
                {
                    if (!valParam->initExpr)
                    {
                        if (diagSink)
                        {
                            diagSink->diagnose(typeExp.exp, Diagnostics::unimplemented, "can't fill in default for generic type parameter");
                            *outProperType = m_astBuilder->getErrorType();
                        }
                        return false;
                    }

                    // TODO: this is one place where syntax should get cloned!
                    if (outProperType)
                        args.add(valParam->initExpr);
                }
                else
                {
                    // ignore non-parameter members
                }
            }
            if (outProperType)
            {
                *outProperType = InstantiateGenericType(genericDeclRef, args);
            }
            return true;
        }
            
        // default case: we expect this to already be a proper type
        if (outProperType)
        {
            *outProperType = type;
        }
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
        if(!CoerceToProperTypeImpl(typeExp, &result.type, nullptr))
            return TypeExp();
        return result;
    }

    TypeExp SemanticsVisitor::CheckProperType(TypeExp typeExp)
    {
        return CoerceToProperType(TranslateTypeNode(typeExp));
    }

    TypeExp SemanticsVisitor::CoerceToUsableType(TypeExp const& typeExp)
    {
        TypeExp result = CoerceToProperType(typeExp);
        Type* type = result.type;
        if (auto basicType = as<BasicExpressionType>(type))
        {
            // TODO: `void` shouldn't be a basic type, to make this easier to avoid
            if (basicType->baseType == BaseType::Void)
            {
                // TODO(tfoley): pick the right diagnostic message
                getSink()->diagnose(result.exp, Diagnostics::invalidTypeVoid);
                result.type = m_astBuilder->getErrorType();
                return result;
            }
        }
        return result;
    }

    TypeExp SemanticsVisitor::CheckUsableType(TypeExp typeExp)
    {
        return CoerceToUsableType(TranslateTypeNode(typeExp));
    }

    bool SemanticsVisitor::ValuesAreEqual(
        IntVal* left,
        IntVal* right)
    {
        if(left == right) return true;

        if(auto leftConst = as<ConstantIntVal>(left))
        {
            if(auto rightConst = as<ConstantIntVal>(right))
            {
                return leftConst->value == rightConst->value;
            }
        }

        if(auto leftVar = as<GenericParamIntVal>(left))
        {
            if(auto rightVar = as<GenericParamIntVal>(right))
            {
                return leftVar->declRef.equals(rightVar->declRef);
            }
        }

        return false;
    }

    VectorExpressionType* SemanticsVisitor::createVectorType(
        Type*  elementType,
        IntVal*          elementCount)
    {
        auto vectorGenericDecl = as<GenericDecl>(m_astBuilder->getSharedASTBuilder()->findMagicDecl("Vector"));
            
        auto vectorTypeDecl = vectorGenericDecl->inner;

        auto substitutions = m_astBuilder->create<GenericSubstitution>();
        substitutions->genericDecl = vectorGenericDecl;
        substitutions->args.add(elementType);
        substitutions->args.add(elementCount);

        auto declRef = DeclRef<Decl>(vectorTypeDecl, substitutions);

        return as<VectorExpressionType>(DeclRefType::create(m_astBuilder, declRef));
    }

    Expr* SemanticsExprVisitor::visitSharedTypeExpr(SharedTypeExpr* expr)
    {
        if (!expr->type.Ptr())
        {
            expr->base = CheckProperType(expr->base);
            expr->type = expr->base.exp->type;
        }
        return expr;
    }

    Expr* SemanticsExprVisitor::visitTaggedUnionTypeExpr(TaggedUnionTypeExpr* expr)
    {
        // We have an expression of the form `__TaggedUnion(A, B, ...)`
        // which will evaluate to a tagged-union type over `A`, `B`, etc.
        //
        TaggedUnionType* type = m_astBuilder->create<TaggedUnionType>();
        expr->type = QualType(m_astBuilder->getTypeType(type));

        for( auto& caseTypeExpr : expr->caseTypes )
        {
            caseTypeExpr = CheckProperType(caseTypeExpr);
            type->caseTypes.add(caseTypeExpr.type);
        }

        return expr;
    }


}
