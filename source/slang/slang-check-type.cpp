// slang-check-type.cpp
#include "slang-check-impl.h"

// This file implements semantic checking logic related to types
// and type expressions (aka `TypeRepr`).

namespace Slang
{
    RefPtr<Type> checkProperType(
        Linkage*        linkage,
        TypeExp         typeExp,
        DiagnosticSink* sink)
    {
        SharedSemanticsContext sharedSemanticsContext(
            linkage,
            sink);
        SemanticsVisitor visitor(&sharedSemanticsContext);


        auto typeOut = visitor.CheckProperType(typeExp);
        return typeOut.type;
    }

    RefPtr<Expr> SemanticsVisitor::TranslateTypeNodeImpl(const RefPtr<Expr> & node)
    {
        if (!node) return nullptr;

        auto expr = CheckTerm(node);
        expr = ExpectATypeRepr(expr);
        return expr;
    }

    RefPtr<Type> SemanticsVisitor::ExtractTypeFromTypeRepr(const RefPtr<Expr>& typeRepr)
    {
        if (!typeRepr) return nullptr;
        if (auto typeType = as<TypeType>(typeRepr->type))
        {
            return typeType->type;
        }
        return getSession()->getErrorType();
    }

    RefPtr<Type> SemanticsVisitor::TranslateTypeNode(const RefPtr<Expr> & node)
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

    RefPtr<Expr> SemanticsVisitor::ExpectATypeRepr(RefPtr<Expr> expr)
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

    RefPtr<Type> SemanticsVisitor::ExpectAType(RefPtr<Expr> expr)
    {
        auto typeRepr = ExpectATypeRepr(expr);
        if (auto typeType = as<TypeType>(typeRepr->type))
        {
            return typeType->type;
        }
        return getSession()->getErrorType();
    }

    RefPtr<Type> SemanticsVisitor::ExtractGenericArgType(RefPtr<Expr> exp)
    {
        return ExpectAType(exp);
    }

    RefPtr<IntVal> SemanticsVisitor::ExtractGenericArgInteger(RefPtr<Expr> exp, DiagnosticSink* sink)
    {
        RefPtr<IntVal> val = CheckIntegerConstantExpression(exp.Ptr(), sink);
        if(val) return val;

        // If the argument expression could not be coerced to an integer
        // constant expression in context, then we will instead construct
        // a dummy "error" value to represent the result.
        //
        val = new ErrorIntVal();
        return val;
    }

    RefPtr<IntVal> SemanticsVisitor::ExtractGenericArgInteger(RefPtr<Expr> exp)
    {
        return ExtractGenericArgInteger(exp, getSink());
    }

    RefPtr<Val> SemanticsVisitor::ExtractGenericArgVal(RefPtr<Expr> exp)
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
            return ExtractGenericArgInteger(exp);
        }
    }

    RefPtr<Type> SemanticsVisitor::InstantiateGenericType(
        DeclRef<GenericDecl>        genericDeclRef,
        List<RefPtr<Expr>> const&   args)
    {
        RefPtr<GenericSubstitution> subst = new GenericSubstitution();
        subst->genericDecl = genericDeclRef.getDecl();
        subst->outer = genericDeclRef.substitutions.substitutions;

        for (auto argExpr : args)
        {
            subst->args.add(ExtractGenericArgVal(argExpr));
        }

        DeclRef<Decl> innerDeclRef;
        innerDeclRef.decl = GetInner(genericDeclRef);
        innerDeclRef.substitutions = SubstitutionSet(subst);

        return DeclRefType::Create(
            getSession(),
            innerDeclRef);
    }

    bool SemanticsVisitor::CoerceToProperTypeImpl(
        TypeExp const&  typeExp,
        RefPtr<Type>*   outProperType,
        DiagnosticSink* diagSink)
    {
        Type* type = typeExp.type.Ptr();
        if(!type && typeExp.exp)
        {
            auto expr = typeExp.exp;

            expr = maybeResolveOverloadedExpr(expr, LookupMask::type, diagSink);

            if(auto typeType = as<TypeType>(expr->type))
            {
                type = typeType->type;
            }
        }

        if (!type)
        {
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
            List<RefPtr<Expr>> args;
            for (RefPtr<Decl> member : genericDeclRef.getDecl()->members)
            {
                if (auto typeParam = as<GenericTypeParamDecl>(member))
                {
                    if (!typeParam->initType.exp)
                    {
                        if (diagSink)
                        {
                            diagSink->diagnose(typeExp.exp.Ptr(), Diagnostics::genericTypeNeedsArgs, typeExp);
                            *outProperType = getSession()->getErrorType();
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
                            diagSink->diagnose(typeExp.exp.Ptr(), Diagnostics::unimplemented, "can't fill in default for generic type parameter");
                            *outProperType = getSession()->getErrorType();
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
        Type* type = result.type.Ptr();
        if (auto basicType = as<BasicExpressionType>(type))
        {
            // TODO: `void` shouldn't be a basic type, to make this easier to avoid
            if (basicType->baseType == BaseType::Void)
            {
                // TODO(tfoley): pick the right diagnostic message
                getSink()->diagnose(result.exp.Ptr(), Diagnostics::invalidTypeVoid);
                result.type = getSession()->getErrorType();
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
        RefPtr<IntVal> left,
        RefPtr<IntVal> right)
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

    RefPtr<VectorExpressionType> SemanticsVisitor::createVectorType(
        RefPtr<Type>  elementType,
        RefPtr<IntVal>          elementCount)
    {
        auto session = getSession();
        auto vectorGenericDecl = findMagicDecl(
            session, "Vector").as<GenericDecl>();
        auto vectorTypeDecl = vectorGenericDecl->inner;

        auto substitutions = new GenericSubstitution();
        substitutions->genericDecl = vectorGenericDecl.Ptr();
        substitutions->args.add(elementType);
        substitutions->args.add(elementCount);

        auto declRef = DeclRef<Decl>(vectorTypeDecl.Ptr(), substitutions);

        return DeclRefType::Create(
            session,
            declRef).as<VectorExpressionType>();
    }

    RefPtr<Expr> SemanticsExprVisitor::visitSharedTypeExpr(SharedTypeExpr* expr)
    {
        if (!expr->type.Ptr())
        {
            expr->base = CheckProperType(expr->base);
            expr->type = expr->base.exp->type;
        }
        return expr;
    }

    RefPtr<Expr> SemanticsExprVisitor::visitTaggedUnionTypeExpr(TaggedUnionTypeExpr* expr)
    {
        // We have an expression of the form `__TaggedUnion(A, B, ...)`
        // which will evaluate to a tagged-union type over `A`, `B`, etc.
        //
        RefPtr<TaggedUnionType> type = new TaggedUnionType();
        expr->type = QualType(getTypeType(type));

        for( auto& caseTypeExpr : expr->caseTypes )
        {
            caseTypeExpr = CheckProperType(caseTypeExpr);
            type->caseTypes.add(caseTypeExpr.type);
        }

        return expr;
    }


}
