#include "syntax-visitors.h"

#include "lookup.h"
#include "compiler.h"
#include "visitor.h"

#include "../core/secure-crt.h"
#include <assert.h>

namespace Slang
{
    bool IsNumeric(BaseType t)
    {
        return t == BaseType::Int || t == BaseType::Float || t == BaseType::UInt;
    }

    String TranslateHLSLTypeNames(String name)
    {
        if (name == "float2" || name == "half2")
            return "vec2";
        else if (name == "float3" || name == "half3")
            return "vec3";
        else if (name == "float4" || name == "half4")
            return "vec4";
        else if (name == "half")
            return "float";
        else if (name == "int2")
            return "ivec2";
        else if (name == "int3")
            return "ivec3";
        else if (name == "int4")
            return "ivec4";
        else if (name == "uint2")
            return "uvec2";
        else if (name == "uint3")
            return "uvec3";
        else if (name == "uint4")
            return "uvec4";
        else if (name == "float3x3" || name == "half3x3")
            return "mat3";
        else if (name == "float4x4" || name == "half4x4")
            return "mat4";
        else
            return name;
    }

    enum class CheckingPhase
    {
        Header, Body
    };

    struct SemanticsVisitor
        : ExprVisitor<SemanticsVisitor, RefPtr<Expr>>
        , StmtVisitor<SemanticsVisitor>
        , DeclVisitor<SemanticsVisitor>
    {
        CheckingPhase checkingPhase = CheckingPhase::Header;
        DeclCheckState getCheckedState()
        {
            if (checkingPhase == CheckingPhase::Body)
                return DeclCheckState::Checked;
            else
                return DeclCheckState::CheckedHeader;
        }
        DiagnosticSink* sink = nullptr;
        DiagnosticSink* getSink()
        {
            return sink;
        }

//        ModuleDecl * program = nullptr;
        FuncDecl * function = nullptr;

        CompileRequest* request = nullptr;
        TranslationUnitRequest* translationUnit = nullptr;

        SourceLanguage getSourceLanguage()
        {
            return translationUnit->sourceLanguage;
        }

        // lexical outer statements
        List<Stmt*> outerStmts;

        // We need to track what has been `import`ed,
        // to avoid importing the same thing more than once
        //
        // TODO: a smarter approach might be to filter
        // out duplicate references during lookup.
        HashSet<ModuleDecl*> importedModules;

    public:
        SemanticsVisitor(
            DiagnosticSink*         sink,
            CompileRequest*         request,
            TranslationUnitRequest* translationUnit)
            : sink(sink)
            , request(request)
            , translationUnit(translationUnit)
        {
        }

        CompileRequest* getCompileRequest() { return request; }
        TranslationUnitRequest* getTranslationUnit() { return translationUnit; }
        Session* getSession()
        {
            return getCompileRequest()->mSession;
        }

    public:
        // Translate Types
        RefPtr<Type> typeResult;
        RefPtr<Expr> TranslateTypeNodeImpl(const RefPtr<Expr> & node)
        {
            if (!node) return nullptr;

            auto expr = CheckTerm(node);
            expr = ExpectATypeRepr(expr);
            return expr;
        }
        RefPtr<Type> ExtractTypeFromTypeRepr(const RefPtr<Expr>& typeRepr)
        {
            if (!typeRepr) return nullptr;
            if (auto typeType = typeRepr->type->As<TypeType>())
            {
                return typeType->type;
            }
            return getSession()->getErrorType();
        }
        RefPtr<Type> TranslateTypeNode(const RefPtr<Expr> & node)
        {
            if (!node) return nullptr;
            auto typeRepr = TranslateTypeNodeImpl(node);
            return ExtractTypeFromTypeRepr(typeRepr);
        }
        TypeExp TranslateTypeNodeForced(TypeExp const& typeExp)
        {
            auto typeRepr = TranslateTypeNodeImpl(typeExp.exp);

            TypeExp result;
            result.exp = typeRepr;
            result.type = ExtractTypeFromTypeRepr(typeRepr);
            return result;
        }
        TypeExp TranslateTypeNode(TypeExp const& typeExp)
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

        RefPtr<DeclRefType> getExprDeclRefType(Expr * expr)
        {
            if (auto typetype = expr->type->As<TypeType>())
                return typetype->type.As<DeclRefType>();
            else
                return expr->type->As<DeclRefType>();
        }

        RefPtr<Expr> ConstructDeclRefExpr(
            DeclRef<Decl>   declRef,
            RefPtr<Expr>    baseExpr,
            SourceLoc       loc)
        {
            if (baseExpr)
            {
                RefPtr<Expr> expr;
                DeclRef<Decl> *declRefOut;
                if (baseExpr->type->As<TypeType>())
                {
                    auto sexpr = new StaticMemberExpr();
                    sexpr->loc = loc;
                    sexpr->BaseExpression = baseExpr;
                    sexpr->name = declRef.GetName();
                    sexpr->declRef = declRef;
                    declRefOut = &sexpr->declRef;
                    expr = sexpr;
                }
                else
                {
                    auto sexpr = new MemberExpr();
                    sexpr->loc = loc;
                    sexpr->BaseExpression = baseExpr;
                    sexpr->name = declRef.GetName();
                    sexpr->declRef = declRef;
                    declRefOut = &sexpr->declRef;
                    expr = sexpr;
                }

                RefPtr<ThisTypeSubstitution> baseThisTypeSubst;
                if (auto baseDeclRefExpr = baseExpr->As<DeclRefExpr>())
                {
                    baseThisTypeSubst = getThisTypeSubst(baseDeclRefExpr->declRef, false);
                }
                if (declRef.As<TypeConstraintDecl>())
                {
                    // if this is a reference to type constraint, insert a this-type substitution
                    RefPtr<Type> expType;
                    expType = baseExpr->type;
                    if (auto baseExprTT = baseExpr->type->As<TypeType>())
                        expType = baseExprTT->type;
                    auto thisTypeSubst = getNewThisTypeSubst(*declRefOut);
                    thisTypeSubst->sourceType = expType;
                    baseThisTypeSubst = nullptr;
                }
                // propagate "this-type" substitutions
                if (baseThisTypeSubst)
                {
                    if (auto declRefExpr = expr.As<DeclRefExpr>())
                    {
                        getNewThisTypeSubst(declRefExpr->declRef)->sourceType = baseThisTypeSubst->sourceType;
                    }
                }
                expr->type = GetTypeForDeclRef(*declRefOut);
                return expr;
            }
            else
            {
                auto expr = new VarExpr();
                expr->loc = loc;
                expr->name = declRef.GetName();
                expr->type = GetTypeForDeclRef(declRef);
                expr->declRef = declRef;
                return expr;
            }
        }

        RefPtr<Expr> ConstructDerefExpr(
            RefPtr<Expr>    base,
            SourceLoc       loc)
        {
            auto ptrLikeType = base->type->As<PointerLikeType>();
            SLANG_ASSERT(ptrLikeType);

            auto derefExpr = new DerefExpr();
            derefExpr->loc = loc;
            derefExpr->base = base;
            derefExpr->type = QualType(ptrLikeType->elementType);

            // TODO(tfoley): handle l-value status here

            return derefExpr;
        }

        RefPtr<Expr> createImplicitThisMemberExpr(
            Type*       type,
            SourceLoc   loc)
        {
            RefPtr<ThisExpr> expr = new ThisExpr();
            expr->type = type;
            expr->loc = loc;
            return expr;
        }

        RefPtr<Expr> ConstructLookupResultExpr(
            LookupResultItem const& item,
            RefPtr<Expr>            baseExpr,
            SourceLoc               loc)
        {
            // If we collected any breadcrumbs, then these represent
            // additional segments of the lookup path that we need
            // to expand here.
            auto bb = baseExpr;
            for (auto breadcrumb = item.breadcrumbs; breadcrumb; breadcrumb = breadcrumb->next)
            {
                switch (breadcrumb->kind)
                {
                case LookupResultItem::Breadcrumb::Kind::Member:
                    bb = ConstructDeclRefExpr(breadcrumb->declRef, bb, loc);
                    break;

                case LookupResultItem::Breadcrumb::Kind::Deref:
                    bb = ConstructDerefExpr(bb, loc);
                    break;

                case LookupResultItem::Breadcrumb::Kind::Constraint:
                    {
                        // TODO: do we need to make something more
                        // explicit here?
                        bb = ConstructDeclRefExpr(
                            breadcrumb->declRef,
                            bb,
                            loc);
                    }
                    break;

                case LookupResultItem::Breadcrumb::Kind::This:
                    {
                        // We expect a `this` to always come
                        // at the start of a chain.
                        SLANG_ASSERT(bb == nullptr);

                        // The member was looked up via a `this` expression,
                        // so we need to create one here.
                        if (auto extensionDeclRef = breadcrumb->declRef.As<ExtensionDecl>())
                        {
                            bb = createImplicitThisMemberExpr(
                                GetTargetType(extensionDeclRef),
                                loc);
                        }
                        else
                        {
                            auto type = DeclRefType::Create(getSession(), breadcrumb->declRef);
                            bb = createImplicitThisMemberExpr(
                                type,
                                loc);
                        }
                    }
                    break;

                default:
                    SLANG_UNREACHABLE("all cases handle");
                }
            }

            return ConstructDeclRefExpr(item.declRef, bb, loc);
        }

        RefPtr<Expr> createLookupResultExpr(
            LookupResult const&     lookupResult,
            RefPtr<Expr>            baseExpr,
            SourceLoc               loc)
        {
            if (lookupResult.isOverloaded())
            {
                auto overloadedExpr = new OverloadedExpr();
                overloadedExpr->loc = loc;
                overloadedExpr->type = QualType(
                    getSession()->getOverloadedType());
                overloadedExpr->base = baseExpr;
                overloadedExpr->lookupResult2 = lookupResult;
                return overloadedExpr;
            }
            else
            {
                return ConstructLookupResultExpr(lookupResult.item, baseExpr, loc);
            }
        }

        RefPtr<Expr> ResolveOverloadedExpr(RefPtr<OverloadedExpr> overloadedExpr, LookupMask mask)
        {
            auto lookupResult = overloadedExpr->lookupResult2;
            SLANG_RELEASE_ASSERT(lookupResult.isValid() && lookupResult.isOverloaded());

            // Take the lookup result we had, and refine it based on what is expected in context.
            lookupResult = refineLookup(lookupResult, mask);

            if (!lookupResult.isValid())
            {
                // If we didn't find any symbols after filtering, then just
                // use the original and report errors that way
                return overloadedExpr;
            }

            if (lookupResult.isOverloaded())
            {
                // We had an ambiguity anyway, so report it.
                getSink()->diagnose(overloadedExpr, Diagnostics::ambiguousReference, lookupResult.items[0].declRef.GetName());

                for(auto item : lookupResult.items)
                {
                    String declString = getDeclSignatureString(item);
                    getSink()->diagnose(item.declRef, Diagnostics::overloadCandidate, declString);
                }

                // TODO(tfoley): should we construct a new ErrorExpr here?
                return CreateErrorExpr(overloadedExpr);
            }

            // otherwise, we had a single decl and it was valid, hooray!
            return ConstructLookupResultExpr(lookupResult.item, overloadedExpr->base, overloadedExpr->loc);
        }

        RefPtr<Expr> ExpectATypeRepr(RefPtr<Expr> expr)
        {
            if (auto overloadedExpr = expr.As<OverloadedExpr>())
            {
                expr = ResolveOverloadedExpr(overloadedExpr, LookupMask::type);
            }

            if (auto typeType = expr->type.type->As<TypeType>())
            {
                return expr;
            }
            else if (auto errorType = expr->type.type->As<ErrorType>())
            {
                return expr;
            }

            getSink()->diagnose(expr, Diagnostics::unimplemented, "expected a type");
            return CreateErrorExpr(expr);
        }

        RefPtr<Type> ExpectAType(RefPtr<Expr> expr)
        {
            auto typeRepr = ExpectATypeRepr(expr);
            if (auto typeType = typeRepr->type->As<TypeType>())
            {
                return typeType->type;
            }
            return getSession()->getErrorType();
        }

        RefPtr<Type> ExtractGenericArgType(RefPtr<Expr> exp)
        {
            return ExpectAType(exp);
        }

        RefPtr<IntVal> ExtractGenericArgInteger(RefPtr<Expr> exp)
        {
            return CheckIntegerConstantExpression(exp.Ptr());
        }

        RefPtr<Val> ExtractGenericArgVal(RefPtr<Expr> exp)
        {
            if (auto overloadedExpr = exp.As<OverloadedExpr>())
            {
                // assume that if it is overloaded, we want a type
                exp = ResolveOverloadedExpr(overloadedExpr, LookupMask::type);
            }

            if (auto typeType = exp->type->As<TypeType>())
            {
                return typeType->type;
            }
            else if (auto errorType = exp->type->As<ErrorType>())
            {
                return exp->type.type;
            }
            else
            {
                return ExtractGenericArgInteger(exp);
            }
        }

        // Construct a type reprsenting the instantiation of
        // the given generic declaration for the given arguments.
        // The arguments should already be checked against
        // the declaration.
        RefPtr<Type> InstantiateGenericType(
            DeclRef<GenericDecl>						genericDeclRef,
            List<RefPtr<Expr>> const&	args)
        {
            RefPtr<GenericSubstitution> subst = new GenericSubstitution();
            subst->genericDecl = genericDeclRef.getDecl();
            subst->outer = genericDeclRef.substitutions.genericSubstitutions;

            for (auto argExpr : args)
            {
                subst->args.Add(ExtractGenericArgVal(argExpr));
            }

            DeclRef<Decl> innerDeclRef;
            innerDeclRef.decl = GetInner(genericDeclRef);
            innerDeclRef.substitutions = SubstitutionSet(subst, genericDeclRef.substitutions.thisTypeSubstitution,
                genericDeclRef.substitutions.globalGenParamSubstitutions);

            return DeclRefType::Create(
                getSession(),
                innerDeclRef);
        }

        // Make sure a declaration has been checked, so we can refer to it.
        // Note that this may lead to us recursively invoking checking,
        // so this may not be the best way to handle things.
        void EnsureDecl(RefPtr<Decl> decl, DeclCheckState state)
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
                sink->diagnose(decl, Diagnostics::cyclicReference, decl);
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
            if (auto varDecl = decl.As<Variable>())
            {
                if (auto parenScope = varDecl->ParentDecl->As<ScopeDecl>())
                {
                    // TODO: This diagnostic should be emitted on the line that is referencing
                    // the declaration. That requires `EnsureDecl` to take the requesting
                    // location as a parameter.
                    sink->diagnose(decl, Diagnostics::localVariableUsedBeforeDeclared, decl);
                    return;
                }
            }

            if (DeclCheckState::CheckingHeader > decl->checkState)
            {
                decl->SetCheckState(DeclCheckState::CheckingHeader);
            }

            // Use visitor pattern to dispatch to correct case
            DeclVisitor::dispatch(decl);
            decl->SetCheckState(state);
        }

        void EnusreAllDeclsRec(RefPtr<Decl> decl)
        {
            checkDecl(decl);
            if (auto containerDecl = decl.As<ContainerDecl>())
            {
                for (auto m : containerDecl->Members)
                {
                    EnusreAllDeclsRec(m);
                }
            }
        }

        // A "proper" type is one that can be used as the type of an expression.
        // Put simply, it can be a concrete type like `int`, or a generic
        // type that is applied to arguments, like `Texture2D<float4>`.
        // The type `void` is also a proper type, since we can have expressions
        // that return a `void` result (e.g., many function calls).
        //
        // A "non-proper" type is any type that can't actually have values.
        // A simple example of this in C++ is `std::vector` - you can't have
        // a value of this type.
        //
        // Part of what this function does is give errors if somebody tries
        // to use a non-proper type as the type of a variable (or anything
        // else that needs a proper type).
        //
        // The other thing it handles is the fact that HLSL lets you use
        // the name of a non-proper type, and then have the compiler fill
        // in the default values for its type arguments (e.g., a variable
        // given type `Texture2D` will actually have type `Texture2D<float4>`).
        bool CoerceToProperTypeImpl(
            TypeExp const&  typeExp,
            RefPtr<Type>*   outProperType,
            DiagnosticSink* diagSink)
        {
            Type* type = typeExp.type.Ptr();
            if(!type && typeExp.exp)
            {
                if(auto typeType = typeExp.exp->type.type.As<TypeType>())
                {
                    type = typeType->type;
                }
            }

            if (auto genericDeclRefType = type->As<GenericDeclRefType>())
            {
                // We are using a reference to a generic declaration as a concrete
                // type. This means we should substitute in any default parameter values
                // if they are available.
                //
                // TODO(tfoley): A more expressive type system would substitute in
                // "fresh" variables and then solve for their values...
                //

                auto genericDeclRef = genericDeclRefType->GetDeclRef();
                checkDecl(genericDeclRef.decl);
                List<RefPtr<Expr>> args;
                for (RefPtr<Decl> member : genericDeclRef.getDecl()->Members)
                {
                    if (auto typeParam = member.As<GenericTypeParamDecl>())
                    {
                        if (!typeParam->initType.exp)
                        {
                            if (diagSink)
                            {
                                diagSink->diagnose(typeExp.exp.Ptr(), Diagnostics::unimplemented, "can't fill in default for generic type parameter");
                                *outProperType = getSession()->getErrorType();
                            }
                            return false;
                        }

                        // TODO: this is one place where syntax should get cloned!
                        if(outProperType)
                            args.Add(typeParam->initType.exp);
                    }
                    else if (auto valParam = member.As<GenericValueParamDecl>())
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
                        if(outProperType)
                            args.Add(valParam->initExpr);
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
            else
            {
                // default case: we expect this to already be a proper type
                if (outProperType)
                {
                    *outProperType = type;
                }
                return true;
            }
        }



        TypeExp CoerceToProperType(TypeExp const& typeExp)
        {
            TypeExp result = typeExp;
            CoerceToProperTypeImpl(typeExp, &result.type, getSink());
            return result;
        }

        TypeExp tryCoerceToProperType(TypeExp const& typeExp)
        {
            TypeExp result = typeExp;
            if(!CoerceToProperTypeImpl(typeExp, &result.type, nullptr))
                return TypeExp();
            return result;
        }

        // Check a type, and coerce it to be proper
        TypeExp CheckProperType(TypeExp typeExp)
        {
            return CoerceToProperType(TranslateTypeNode(typeExp));
        }

        // For our purposes, a "usable" type is one that can be
        // used to declare a function parameter, variable, etc.
        // These turn out to be all the proper types except
        // `void`.
        //
        // TODO(tfoley): consider just allowing `void` as a
        // simple example of a "unit" type, and get rid of
        // this check.
        TypeExp CoerceToUsableType(TypeExp const& typeExp)
        {
            TypeExp result = CoerceToProperType(typeExp);
            Type* type = result.type.Ptr();
            if (auto basicType = type->As<BasicExpressionType>())
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

        // Check a type, and coerce it to be usable
        TypeExp CheckUsableType(TypeExp typeExp)
        {
            return CoerceToUsableType(TranslateTypeNode(typeExp));
        }

        RefPtr<Expr> CheckTerm(RefPtr<Expr> term)
        {
            if (!term) return nullptr;
            return ExprVisitor::dispatch(term);
        }

        RefPtr<Expr> CreateErrorExpr(Expr* expr)
        {
            expr->type = QualType(getSession()->getErrorType());
            return expr;
        }

        bool IsErrorExpr(RefPtr<Expr> expr)
        {
            // TODO: we may want other cases here...

            if (auto errorType = expr->type->As<ErrorType>())
                return true;

            return false;
        }

        // Capture the "base" expression in case this is a member reference
        RefPtr<Expr> GetBaseExpr(RefPtr<Expr> expr)
        {
            if (auto memberExpr = expr.As<MemberExpr>())
            {
                return memberExpr->BaseExpression;
            }
            else if(auto overloadedExpr = expr.As<OverloadedExpr>())
            {
                return overloadedExpr->base;
            }
            return nullptr;
        }

    public:

        bool ValuesAreEqual(
            RefPtr<IntVal> left,
            RefPtr<IntVal> right)
        {
            if(left == right) return true;

            if(auto leftConst = left.As<ConstantIntVal>())
            {
                if(auto rightConst = right.As<ConstantIntVal>())
                {
                    return leftConst->value == rightConst->value;
                }
            }

            if(auto leftVar = left.As<GenericParamIntVal>())
            {
                if(auto rightVar = right.As<GenericParamIntVal>())
                {
                    return leftVar->declRef.Equals(rightVar->declRef);
                }
            }

            return false;
        }

        // Compute the cost of using a particular declaration to
        // perform implicit type conversion.
        ConversionCost getImplicitConversionCost(
            Decl* decl)
        {
            if(auto modifier = decl->FindModifier<ImplicitConversionModifier>())
            {
                return modifier->cost;
            }

            return kConversionCost_Explicit;
        }

        // Central engine for implementing implicit coercion logic
        bool TryCoerceImpl(
            RefPtr<Type>			toType,		// the target type for conversion
            RefPtr<Expr>*	outToExpr,	// (optional) a place to stuff the target expression
            RefPtr<Type>			fromType,	// the source type for the conversion
            RefPtr<Expr>	fromExpr,	// the source expression
            ConversionCost*					outCost)	// (optional) a place to stuff the conversion cost
        {
            // Easy case: the types are equal
            if (toType->Equals(fromType))
            {
                if (outToExpr)
                    *outToExpr = fromExpr;
                if (outCost)
                    *outCost = kConversionCost_None;
                return true;
            }

            // If either type is an error, then let things pass.
            if (toType->As<ErrorType>() || fromType->As<ErrorType>())
            {
                if (outToExpr)
                    *outToExpr = CreateImplicitCastExpr(toType, fromExpr);
                if (outCost)
                    *outCost = kConversionCost_None;
                return true;
            }

            // Coercion from an initializer list is allowed for many types
            if( auto fromInitializerListExpr = fromExpr.As<InitializerListExpr>())
            {
                auto argCount = fromInitializerListExpr->args.Count();

                // In the case where we need to build a reuslt expression,
                // we will collect the new arguments here
                List<RefPtr<Expr>> coercedArgs;

                if (auto toVecType = toType->As<VectorExpressionType>())
                {
                    auto toElementCount = toVecType->elementCount;
                    auto toElementType = toVecType->elementType;

                    UInt elementCount = 0;
                    if (auto constElementCount = toElementCount.As<ConstantIntVal>())
                    {
                        elementCount = (UInt) constElementCount->value;
                    }
                    else
                    {
                        // We don't know the element count statically,
                        // so what are we supposed to be doing?
                        elementCount = fromInitializerListExpr->args.Count();
                    }

                    // TODO: need to check that the element count
                    // for the vector type matches the argument
                    // count for the initializer list, or else
                    // fix them up to match.

                    for(auto arg : fromInitializerListExpr->args)
                    {
                        RefPtr<Expr> coercedArg;
                        ConversionCost argCost;

                        bool argResult = TryCoerceImpl(
                            toElementType,
                            outToExpr ? &coercedArg : nullptr,
                            arg->type,
                            arg,
                            outCost ? &argCost : nullptr);

                        // No point in trying further if any argument fails
                        if(!argResult)
                            return false;

                        // TODO(tfoley): what to do with cost?
                        // This only matters if/when we allow an initializer list as an argument to
                        // an overloaded call.

                        if( outToExpr )
                        {
                            coercedArgs.Add(coercedArg);
                        }
                    }
                }
                //
                // TODO(tfoley): How to handle matrices here?
                // Should they expect individual scalars, or support
                // vectors for the rows?
                //
                if(auto toDeclRefType = toType->As<DeclRefType>())
                {
                    auto toTypeDeclRef = toDeclRefType->declRef;
                    if(auto toStructDeclRef = toTypeDeclRef.As<StructDecl>())
                    {
                        // Trying to initialize a `struct` type given an initializer list.
                        // We will go through the fields in order and try to match them
                        // up with initializer arguments.


                        UInt argIndex = 0;
                        for(auto fieldDeclRef : getMembersOfType<StructField>(toStructDeclRef))
                        {
                            if(argIndex >= argCount)
                            {
                                // We've consumed all the arguments, so we should stop
                                break;
                            }

                            auto arg = fromInitializerListExpr->args[argIndex++];

                            // 
                            RefPtr<Expr> coercedArg;
                            ConversionCost argCost;

                            bool argResult = TryCoerceImpl(
                                GetType(fieldDeclRef),
                                outToExpr ? &coercedArg : nullptr,
                                arg->type,
                                arg,
                                outCost ? &argCost : nullptr);

                            // No point in trying further if any argument fails
                            if(!argResult)
                                return false;

                            // TODO(tfoley): what to do with cost?
                            // This only matters if/when we allow an initializer list as an argument to
                            // an overloaded call.

                            if( outToExpr )
                            {
                                coercedArgs.Add(coercedArg);
                            }
                        }
                    }
                }
                else if(auto toArrayType = toType->As<ArrayExpressionType>())
                {
                    // TODO(tfoley): If we can compute the size of the array statically,
                    // then we want to check that there aren't too many initializers present

                    auto toElementType = toArrayType->baseType;

                    for(auto& arg : fromInitializerListExpr->args)
                    {
                        RefPtr<Expr> coercedArg;
                        ConversionCost argCost;

                        bool argResult = TryCoerceImpl(
                                toElementType,
                                outToExpr ? &coercedArg : nullptr,
                                arg->type,
                                arg,
                                outCost ? &argCost : nullptr);

                        // No point in trying further if any argument fails
                        if(!argResult)
                            return false;

                        if( outToExpr )
                        {
                            coercedArgs.Add(coercedArg);
                        }
                    }
                }
                else
                {
                    // By default, we don't allow a type to be initialized using
                    // an initializer list.
                    return false;
                }

                // For now, coercion from an initializer list has no cost
                if(outCost)
                {
                    *outCost = kConversionCost_None;
                }

                // We were able to coerce all the arguments given, and so
                // we need to construct a suitable expression to remember the result
                if(outToExpr)
                {
                    auto toInitializerListExpr = new InitializerListExpr();
                    toInitializerListExpr->loc = fromInitializerListExpr->loc;
                    toInitializerListExpr->type = QualType(toType);
                    toInitializerListExpr->args = coercedArgs;


                    *outToExpr = toInitializerListExpr;
                }

                return true;
            }

            //
            if (auto toDeclRefType = toType->As<DeclRefType>())
            {
                auto toTypeDeclRef = toDeclRefType->declRef;
                if (auto interfaceDeclRef = toTypeDeclRef.As<InterfaceDecl>())
                {
                    // Trying to convert to an interface type.
                    //
                    // We will allow this if the type conforms to the interface.
                    if (DoesTypeConformToInterface(fromType, interfaceDeclRef))
                    {
                        if (outToExpr)
                            *outToExpr = CreateImplicitCastExpr(toType, fromExpr);
                        if (outCost)
                            *outCost = kConversionCost_CastToInterface;
                        return true;
                    }
                }
                else if (auto genParamDeclRef = toTypeDeclRef.As<GenericTypeParamDecl>())
                {
                    // We need to enumerate the constraints placed on this type by its outer
                    // generic declaration, and see if any of them guarantees that we
                    // satisfy the given interface..
                    auto genericDeclRef = genParamDeclRef.GetParent().As<GenericDecl>();
                    SLANG_ASSERT(genericDeclRef);

                    for (auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(genericDeclRef))
                    {
                        auto sub = GetSub(constraintDeclRef);
                        auto sup = GetSup(constraintDeclRef);

                        auto subDeclRef = sub->As<DeclRefType>();
                        if (!subDeclRef)
                            continue;
                        if (subDeclRef->declRef != genParamDeclRef)
                            continue;
                        auto supDeclRefType = sup->As<DeclRefType>();
                        if (supDeclRefType)
                        {
                            auto toInterfaceDeclRef = supDeclRefType->declRef.As<InterfaceDecl>();
                            if (DoesTypeConformToInterface(fromType, toInterfaceDeclRef))
                            {
                                if (outToExpr)
                                    *outToExpr = CreateImplicitCastExpr(toType, fromExpr);
                                if (outCost)
                                    *outCost = kConversionCost_CastToInterface;
                                return true;
                            }
                        }
                    }

                }

            }

            // Are we converting from a parameter group type to its element type?
            if(auto fromParameterGroupType = fromType->As<ParameterGroupType>())
            {
                auto fromElementType = fromParameterGroupType->getElementType();

                // If we have, e.g., `ConstantBuffer<A>` and we want to convert
                // to `B`, where conversion from `A` to `B` is possible, then
                // we will do so here.

                ConversionCost subCost = 0;
                if(CanCoerce(toType, fromElementType, &subCost))
                {
                    if(outCost)
                        *outCost = subCost + kConversionCost_ImplicitDereference;

                    if(outToExpr)
                    {
                        auto derefExpr = new DerefExpr();
                        derefExpr->base = fromExpr;
                        derefExpr->type = QualType(fromElementType);

                        return TryCoerceImpl(
                            toType,
                            outToExpr,
                            fromElementType,
                            derefExpr,
                            nullptr);
                    }
                    return true;
                }
            }


            // Look for an initializer/constructor declaration in the target type,
            // which is marked as usable for implicit conversion, and which takes
            // the source type as an argument.

            OverloadResolveContext overloadContext;

            overloadContext.disallowNestedConversions = true;
            overloadContext.argCount = 1;
            overloadContext.argTypes = &fromType;

            overloadContext.originalExpr = nullptr;
            if(fromExpr)
            {
                overloadContext.loc = fromExpr->loc;
                overloadContext.funcLoc = fromExpr->loc;
                overloadContext.args = &fromExpr;
            }

            overloadContext.baseExpr = nullptr;
            overloadContext.mode = OverloadResolveContext::Mode::JustTrying;
            
            AddTypeOverloadCandidates(toType, overloadContext, toType);

            if(overloadContext.bestCandidates.Count() != 0)
            {
                // There were multiple candidates that were equally good.

                // First, we will check if these candidates are even applicable.
                // If they aren't, then they can't be used for conversion.
                if(overloadContext.bestCandidates[0].status != OverloadCandidate::Status::Appicable)
                    return false;

                // If we reach this point, then we have multiple candidates which are
                // all equally applicable, which means we have an ambiguity.
                // If the user is just querying whether a conversion is possible, we
                // will tell them it is, because ambiguity should trigger an ambiguity
                // error, and not a "no conversion possible" error.

                // We will compute a nominal conversion cost as the minimum over
                // all the conversions available.
                ConversionCost cost = kConversionCost_GeneralConversion;
                for(auto candidate : overloadContext.bestCandidates)
                {
                    ConversionCost candidateCost = getImplicitConversionCost(
                        candidate.item.declRef.getDecl());

                    if(candidateCost < cost)
                        cost = candidateCost;
                }

                if(outCost)
                    *outCost = cost;

                if(outToExpr)
                {
                    // The user is asking for us to actually perform the conversion,
                    // so we need to generate an appropriate expression here.

                    // YONGH: I am confused why we are not hitting this case before
                    //throw "foo bar baz";
                    // YONGH: temporary work around, may need to create the actual
                    // invocation expr to the constructor call
                    *outToExpr = fromExpr;
                }

                return true;
            }
            else if(overloadContext.bestCandidate)
            {
                // There is a single best candidate for conversion.

                // It might not actually be usable, so let's check that first.
                if(overloadContext.bestCandidate->status != OverloadCandidate::Status::Appicable)
                    return false;

                // Okay, it is applicable, and we just need to let the user
                // know about it, and optionally construct a call.

                // We need to extract the conversion cost from the candidate we found.
                ConversionCost cost = getImplicitConversionCost(
                        overloadContext.bestCandidate->item.declRef.getDecl());;

                if(outCost)
                    *outCost = cost;

                if(outToExpr)
                {
                    // The logic here is a bit ugly, to deal with the fact that
                    // `CompleteOverloadCandidate` will, left to its own devices,
                    // construct a vanilla `InvokeExpr` to represent the call
                    // to the initializer we found, while we *want* it to
                    // create some variety of `ImplicitCastExpr`.
                    //
                    // Now, it just so happens that `CompleteOverloadCandidate`
                    // will use the "original" expression if one is available,
                    // so we'll create one and initialize it here.
                    // We fill in the location and arguments, but not the
                    // base expression (the callee), since that will come
                    // from the selected overload candidate.
                    //
                    auto castExpr = createImplicitCastExpr();
                    castExpr->loc = fromExpr->loc;
                    castExpr->Arguments.Add(fromExpr);
                    //
                    // Next we need to set our cast expression as the "original"
                    // expression and then complete the overload process.
                    //
                    overloadContext.originalExpr = castExpr;
                    *outToExpr = CompleteOverloadCandidate(overloadContext, *overloadContext.bestCandidate);
                    //
                    // However, the above isn't *quite* enough, because
                    // the process of completing the overload candidate
                    // might overwrite the argument list that was passed
                    // in to overload resolution, and in this case that
                    // "argument list" was just a pointer to `fromExpr`.
                    //
                    // That means we need to clear the argument list and
                    // reload it from `fromExpr` to make sure that we
                    // got the arguments *after* any transformations
                    // were applied.
                    // For right now this probably doesn't matter,
                    // because we don't allow nested implicit conversions,
                    // but I'd rather play it safe.
                    //
                    castExpr->Arguments.Clear();
                    castExpr->Arguments.Add(fromExpr);
                }

                return true;
            }

            return false;
        }

        // Check whether a type coercion is possible
        bool CanCoerce(
            RefPtr<Type>			toType,			// the target type for conversion
            RefPtr<Type>			fromType,		// the source type for the conversion
            ConversionCost*					outCost = 0)	// (optional) a place to stuff the conversion cost
        {
            return TryCoerceImpl(
                toType,
                nullptr,
                fromType,
                nullptr,
                outCost);
        }

        RefPtr<TypeCastExpr> createImplicitCastExpr()
        {
            return new ImplicitCastExpr();
        }

        RefPtr<Expr> CreateImplicitCastExpr(
            RefPtr<Type>	toType,
            RefPtr<Expr>	fromExpr)
        {
            RefPtr<TypeCastExpr> castExpr = createImplicitCastExpr();

            auto typeType = new TypeType();
            typeType->setSession(getSession());
            typeType->type = toType;

            auto typeExpr = new SharedTypeExpr();
            typeExpr->type.type = typeType;
            typeExpr->base.type = toType;

            castExpr->loc = fromExpr->loc;
            castExpr->FunctionExpr = typeExpr;
            castExpr->type = QualType(toType);
            castExpr->Arguments.Add(fromExpr);
            return castExpr;
        }

        // Perform type coercion, and emit errors if it isn't possible
        RefPtr<Expr> Coerce(
            RefPtr<Type>			toType,
            RefPtr<Expr>	fromExpr)
        {
            RefPtr<Expr> expr;
            if (!TryCoerceImpl(
                toType,
                &expr,
                fromExpr->type.Ptr(),
                fromExpr.Ptr(),
                nullptr))
            {
                getSink()->diagnose(fromExpr->loc, Diagnostics::typeMismatch, toType, fromExpr->type);

                // Note(tfoley): We don't call `CreateErrorExpr` here, because that would
                // clobber the type on `fromExpr`, and an invariant here is that coercion
                // really shouldn't *change* the expression that is passed in, but should
                // introduce new AST nodes to coerce its value to a different type...
                return CreateImplicitCastExpr(
                    getSession()->getErrorType(),
                    fromExpr);
            }
            return expr;
        }

        void CheckVarDeclCommon(RefPtr<VarDeclBase> varDecl)
        {
            // Check the type, if one was given
            TypeExp type = CheckUsableType(varDecl->type);

            // TODO: Additional validation rules on types should go here,
            // but we need to deal with the fact that some cases might be
            // allowed in one context (e.g., an unsized array parameter)
            // but not in othters (e.g., an unsized array field in a struct).

            // Check the initializers, if one was given
            RefPtr<Expr> initExpr = CheckTerm(varDecl->initExpr);

            // If a type was given, ...
            if (type.Ptr())
            {
                // then coerce any initializer to the type
                if (initExpr)
                {
                    initExpr = Coerce(type.Ptr(), initExpr);
                }
            }
            else
            {
                // TODO: infer a type from the initializers

                if (!initExpr)
                {
                    getSink()->diagnose(varDecl, Diagnostics::unimplemented, "variable declaration with no type must have initializer");
                }
                else
                {
                    getSink()->diagnose(varDecl, Diagnostics::unimplemented, "type inference for variable declaration");
                }
            }

            varDecl->type = type;
            varDecl->initExpr = initExpr;
        }

        // Fill in default substitutions for the 'subtype' part of a type constraint decl
        void CheckConstraintSubType(TypeExp & typeExp)
        {
            if (auto sharedTypeExpr = typeExp.exp.As<SharedTypeExpr>())
            {
                if (auto declRefType = sharedTypeExpr->base->AsDeclRefType())
                {
                    declRefType->declRef.substitutions = createDefaultSubstitutions(getSession(), declRefType->declRef.getDecl());
                    if (auto typetype = typeExp.exp->type.type.As<TypeType>())
                        typetype->type = declRefType;
                }
            }
        }

        void CheckGenericConstraintDecl(GenericTypeConstraintDecl* decl)
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

        void checkDecl(Decl* decl)
        {
            EnsureDecl(decl, checkingPhase == CheckingPhase::Header ? DeclCheckState::CheckedHeader : DeclCheckState::Checked);
        }

        void checkGenericDeclHeader(GenericDecl* genericDecl)
        {
            if (genericDecl->IsChecked(DeclCheckState::CheckedHeader))
                return;
            // check the parameters
            for (auto m : genericDecl->Members)
            {
                if (auto typeParam = m.As<GenericTypeParamDecl>())
                {
                    typeParam->initType = CheckProperType(typeParam->initType);
                }
                else if (auto valParam = m.As<GenericValueParamDecl>())
                {
                    // TODO: some real checking here...
                    CheckVarDeclCommon(valParam);
                }
                else if (auto constraint = m.As<GenericTypeConstraintDecl>())
                {
                    CheckGenericConstraintDecl(constraint.Ptr());
                }
            }

            genericDecl->SetCheckState(DeclCheckState::CheckedHeader);
        }

        void visitGenericDecl(GenericDecl* genericDecl)
        {
            checkGenericDeclHeader(genericDecl);

            // check the nested declaration
            // TODO: this needs to be done in an appropriate environment...
            checkDecl(genericDecl->inner);
            genericDecl->SetCheckState(getCheckedState());
        }

        void visitGenericTypeConstraintDecl(GenericTypeConstraintDecl * genericConstraintDecl)
        {
            if (genericConstraintDecl->IsChecked(DeclCheckState::CheckedHeader))
                return;
            // check the type being inherited from
            auto base = genericConstraintDecl->sup;
            base = TranslateTypeNode(base);
            genericConstraintDecl->sup = base;
        }

        void visitInheritanceDecl(InheritanceDecl* inheritanceDecl)
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

            if(auto declRefType = base.type->As<DeclRefType>())
            {
                if(auto interfaceDeclRef = declRefType->declRef.As<InterfaceDecl>())
                {
                    return;
                }
            }

            // If type expression didn't name an interface, we'll emit an error here
            // TODO: deal with the case of an error in the type expression (don't cascade)
            getSink()->diagnose( base.exp, Diagnostics::expectedAnInterfaceGot, base.type);
        }

        RefPtr<ConstantIntVal> checkConstantIntVal(
            RefPtr<Expr>    expr)
        {
            // First type-check the expression as normal
            expr = CheckExpr(expr);

            auto intVal = CheckIntegerConstantExpression(expr.Ptr());
            if(!intVal)
                return nullptr;

            auto constIntVal = intVal.As<ConstantIntVal>();
            if(!constIntVal)
            {
                getSink()->diagnose(expr->loc, Diagnostics::expectedIntegerConstantNotLiteral);
                return nullptr;
            }
            return constIntVal;
        }

        // Check an expression, coerce it to the `String` type, and then
        // ensure that it has a literal (not just compile-time constant) value.
        bool checkLiteralStringVal(
            RefPtr<Expr>    expr,
            String*         outVal)
        {
            // TODO: This should actually perform semantic checking, etc.,
            // but for now we are just going to look for a direct string
            // literal AST node.

            if(auto stringLitExpr = expr.As<StringLiteralExpr>())
            {
                if(outVal)
                {
                    *outVal = stringLitExpr->value;
                }
                return true;
            }

            getSink()->diagnose(expr, Diagnostics::expectedAStringLiteral);

            return false;
        }

        void visitSyntaxDecl(SyntaxDecl*)
        {
            // These are only used in the stdlib, so no checking is needed
        }

        void visitAttributeDecl(AttributeDecl*)
        {
            // These are only used in the stdlib, so no checking is needed
        }

        void visitGenericTypeParamDecl(GenericTypeParamDecl*)
        {
            // These are only used in the stdlib, so no checking is needed for now
        }

        void visitGenericValueParamDecl(GenericValueParamDecl*)
        {
            // These are only used in the stdlib, so no checking is needed for now
        }

        void visitModifier(Modifier*)
        {
            // Do nothing with modifiers for now
        }

        AttributeDecl* lookUpAttributeDecl(Name* attributeName, Scope* scope)
        {
            // Look up the name and see what we find.
            //
            // TODO: This needs to have some special filtering or naming
            // rules to keep us from seeing shadowing variable declarations.
            auto lookupResult = lookUp(getSession(), this, attributeName, scope, LookupMask::Attribute);

            // If we didn't find anything, or the result was overloaded,
            // then we aren't going to be able to extract a single decl.
            if(!lookupResult.isValid() || lookupResult.isOverloaded())
                return nullptr;

            auto decl = lookupResult.item.declRef.getDecl();
            if( auto attributeDecl = dynamic_cast<AttributeDecl*>(decl) )
            {
                return attributeDecl;
            }
            else
            {
                return nullptr;
            }
        }

        Stage findStageByName(String const& name)
        {
            static const struct
            {
                char const* name;
                Stage       stage;
            } kStages[] =
            {
            #define PROFILE_STAGE(ID, NAME, ENUM) \
                { #NAME,    Stage::ID },

            #define PROFILE_STAGE_ALIAS(ID, NAME, VAL) \
                { #NAME,    Stage::ID },

            #include "profile-defs.h"
            };

            for(auto entry : kStages)
            {
                if(name == entry.name)
                {
                    return entry.stage;
                }
            }

            return Stage::Unknown;
        }

        bool validateAttribute(RefPtr<Attribute> attr)
        {
                if(auto numThreadsAttr = attr.As<NumThreadsAttribute>())
                {
                    SLANG_ASSERT(attr->args.Count() == 3);
                    auto xVal = checkConstantIntVal(attr->args[0]);
                    auto yVal = checkConstantIntVal(attr->args[1]);
                    auto zVal = checkConstantIntVal(attr->args[2]);

                    if(!xVal) return false;
                    if(!yVal) return false;
                    if(!zVal) return false;

                    numThreadsAttr->x          = (int32_t) xVal->value;
                    numThreadsAttr->y          = (int32_t) yVal->value;
                    numThreadsAttr->z          = (int32_t) zVal->value;
                }
                else if (auto maxVertexCountAttr = attr.As<MaxVertexCountAttribute>())
                {
                    SLANG_ASSERT(attr->args.Count() == 1);
                    auto val = checkConstantIntVal(attr->args[0]);

                    maxVertexCountAttr->value = (int32_t)val->value;
                }
                else if(auto instanceAttr = attr.As<InstanceAttribute>())
                {
                    SLANG_ASSERT(attr->args.Count() == 1);
                    auto val = checkConstantIntVal(attr->args[0]);

                    instanceAttr->value = (int32_t)val->value;
                }
                else if(auto entryPointAttr = attr.As<EntryPointAttribute>())
                {
                    SLANG_ASSERT(attr->args.Count() == 1);

                    String stageName;
                    if(!checkLiteralStringVal(attr->args[0], &stageName))
                    {
                        return false;
                    }

                    auto stage = findStageByName(stageName);
                    if(stage == Stage::Unknown)
                    {
                        getSink()->diagnose(attr->args[0], Diagnostics::unknownStageName, stageName);
                    }

                    entryPointAttr->stage = stage;
                }
                else
                {
                    if(attr->args.Count() == 0)
                    {
                        // If the attribute took no arguments, then we will
                        // assume it is valid as written.
                    }
                    else
                    {
                        // We should be special-casing the checking of any attribute
                        // with a non-zero number of arguments.
                        SLANG_DIAGNOSE_UNEXPECTED(getSink(), attr, "unhandled attribute");
                        return false;
                    }
                }

                return true;
        }

        RefPtr<AttributeBase> checkAttribute(
            UncheckedAttribute*     uncheckedAttr,
            ModifiableSyntaxNode*   attrTarget)
        {
            auto attrName = uncheckedAttr->getName();
            auto attrDecl = lookUpAttributeDecl(
                attrName,
                uncheckedAttr->scope);

            if(!attrDecl)
            {
                getSink()->diagnose(uncheckedAttr, Diagnostics::unknownAttributeName, attrName);
                return uncheckedAttr;
            }

            if(!attrDecl->syntaxClass.isSubClassOf<Attribute>())
            {
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), attrDecl, "attribute declaration does not reference an attribute class");
                return uncheckedAttr;
            }

            RefPtr<RefObject> attrObj = attrDecl->syntaxClass.createInstance();
            auto attr = attrObj.As<Attribute>();
            if(!attr)
            {
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), attrDecl, "attribute class did not yield an attribute object");
                return uncheckedAttr;
            }

            // We are going to replace the unchecked attribute with the checked one.

            // First copy all of the state over from the original attribute.
            attr->name  = uncheckedAttr->name;
            attr->args  = uncheckedAttr->args;
            attr->loc   = uncheckedAttr->loc;

            // We will start with checking steps that can be applied independent
            // of the concrete attribute type that was selected. These only need
            // us to look at the attribute declaration itself.
            //
            // Start by doing argument/parameter matching
            UInt argCount = attr->args.Count();
            UInt paramCounter = 0;
            bool mismatch = false;
            for(auto paramDecl : attrDecl->getMembersOfType<ParamDecl>())
            {
                UInt paramIndex = paramCounter++;
                if( paramIndex < argCount )
                {
                    auto arg = attr->args[paramIndex];

                    // TODO: support checking the argument against the declared
                    // type for the parameter.

                }
                else
                {
                    // We didn't have enough arguments for the
                    // number of parameters declared.
                    if(auto defaultArg = paramDecl->initExpr)
                    {
                        // The attribute declaration provided a default,
                        // so we should use that.
                        //
                        // TODO: we need to figure out how to hook up
                        // default arguments as needed.
                    }
                    else
                    {
                        mismatch = true;
                    }
                }
            }
            UInt paramCount = paramCounter;

            if(mismatch)
            {
                getSink()->diagnose(attr, Diagnostics::attributeArgumentCountMismatch, attrName, paramCount, argCount);
                return uncheckedAttr;
            }

            // The next bit of validation that we can apply semi-generically
            // is to validate that the target for this attribute is a valid
            // one for the chosen attribute.
            //
            // The attribute declaration will have one or more `AttributeTargetModifier`s
            // that each specify a syntax class that the attribute can be applied to.
            // If any of these match `attrTarget`, then we are good.
            //
            bool validTarget = false;
            for(auto attrTargetMod : attrDecl->GetModifiersOfType<AttributeTargetModifier>())
            {
                if(attrTarget->getClass().isSubClassOf(attrTargetMod->syntaxClass))
                {
                    validTarget = true;
                    break;
                }
            }
            if(!validTarget)
            {
                getSink()->diagnose(attr, Diagnostics::attributeNotApplicable, attrName);
                return uncheckedAttr;
            }

            // Now apply type-specific validation to the attribute.
            if(!validateAttribute(attr))
            {
                return uncheckedAttr;
            }


            return attr;
        }

        RefPtr<Modifier> checkModifier(
            RefPtr<Modifier>        m,
            ModifiableSyntaxNode*   syntaxNode)
        {
            if(auto hlslUncheckedAttribute = m.As<UncheckedAttribute>())
            {
                // We have an HLSL `[name(arg,...)]` attribute, and we'd like
                // to check that it is provides all the expected arguments
                //
                // First, look up the attribute name in the current scope to find
                // the right syntax class to instantiate.
                //

                return checkAttribute(hlslUncheckedAttribute, syntaxNode);
            }
            // Default behavior is to leave things as they are,
            // and assume that modifiers are mostly already checked.
            //
            // TODO: This would be a good place to validate that
            // a modifier is actually valid for the thing it is
            // being applied to, and potentially to check that
            // it isn't in conflict with any other modifiers
            // on the same declaration.

            return m;
        }


        void checkModifiers(ModifiableSyntaxNode* syntaxNode)
        {
            // TODO(tfoley): need to make sure this only
            // performs semantic checks on a `SharedModifier` once...

            // The process of checking a modifier may produce a new modifier in its place,
            // so we will build up a new linked list of modifiers that will replace
            // the old list.
            RefPtr<Modifier> resultModifiers;
            RefPtr<Modifier>* resultModifierLink = &resultModifiers;

            RefPtr<Modifier> modifier = syntaxNode->modifiers.first;
            while(modifier)
            {
                // Because we are rewriting the list in place, we need to extract
                // the next modifier here (not at the end of the loop).
                auto next = modifier->next;

                // We also go ahead and clobber the `next` field on the modifier
                // itself, so that the default behavior of `checkModifier()` can
                // be to return a single unlinked modifier.
                modifier->next = nullptr;

                auto checkedModifier = checkModifier(modifier, syntaxNode);
                if(checkedModifier)
                {
                    // If checking gave us a modifier to add, then we
                    // had better add it.

                    // Just in case `checkModifier` ever returns multiple
                    // modifiers, lets advance to the end of the list we
                    // are building.
                    while(*resultModifierLink)
                        resultModifierLink = &(*resultModifierLink)->next;

                    // attach the new modifier at the end of the list,
                    // and now set the "link" to point to its `next` field
                    *resultModifierLink = checkedModifier;
                    resultModifierLink = &checkedModifier->next;
                }

                // Move along to the next modifier
                modifier = next;
            }

            // Whether we actually re-wrote anything or note, lets
            // install the new list of modifiers on the declaration
            syntaxNode->modifiers.first = resultModifiers;
        }

        void visitModuleDecl(ModuleDecl* programNode)
        {
            // Try to register all the builtin decls
            for (auto decl : programNode->Members)
            {
                auto inner = decl;
                if (auto genericDecl = decl.As<GenericDecl>())
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
                if (auto extDecl = g->inner->As<ExtensionDecl>())
                {
                    checkGenericDeclHeader(g);
                    registerExtension(extDecl);
                }
            }
            // check types
            for (auto & s : programNode->getMembersOfType<TypeDefDecl>())
                checkDecl(s.Ptr());

            for (int pass = 0; pass < 2; pass++)
            {
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

                if (sink->GetErrorCount() != 0)
                    return;

                // Force everything to be fully checked, just in case
                // Note that we don't just call this on the program,
                // because we'd end up recursing into this very code path...
                for (auto d : programNode->Members)
                {
                    EnusreAllDeclsRec(d);
                }

                // Do any semantic checking required on modifiers?
                for (auto d : programNode->Members)
                {
                    checkModifiers(d.Ptr());
                }
                
                if (pass == 0)
                {
                    // now we can check all interface conformances
                    for (auto & s : programNode->getMembersOfType<AggTypeDecl>())
                        checkAggTypeConformance(s);
                    for (auto & s : programNode->getMembersOfType<ExtensionDecl>())
                        checkExtensionConformance(s);
                    for (auto & g : programNode->getMembersOfType<GenericDecl>())
                    {
                        if (auto innerAggDecl = g->inner->As<AggTypeDecl>())
                            checkAggTypeConformance(innerAggDecl);
                        else if (auto innerExtDecl = g->inner->As<ExtensionDecl>())
                            checkExtensionConformance(innerExtDecl);
                    }
                }
            }
        }

        void visitStructField(StructField* field)
        {
            if (field->IsChecked(DeclCheckState::CheckedHeader))
                return;
            // TODO: bottleneck through general-case variable checking

            field->type = CheckUsableType(field->type);
            field->SetCheckState(getCheckedState());
        }

        bool doesSignatureMatchRequirement(
            DeclRef<CallableDecl>           memberDecl,
            DeclRef<CallableDecl>   requiredMemberDeclRef,
            Dictionary<DeclRef<Decl>, DeclRef<Decl>> & requirementDict)
        {
            // TODO: actually implement matching here. For now we'll
            // just pretend that things are satisfied in order to make progress..
            requirementDict.AddIfNotExists(requiredMemberDeclRef, memberDecl);
            return true;
        }

        bool doesGenericSignatureMatchRequirement(
            DeclRef<GenericDecl> genDecl,
            DeclRef<GenericDecl> requirementGenDecl,
            Dictionary<DeclRef<Decl>, DeclRef<Decl>> & requirementDict)
        {
            if (genDecl.getDecl()->Members.Count() != requirementGenDecl.getDecl()->Members.Count())
                return false;
            for (UInt i = 0; i < genDecl.getDecl()->Members.Count(); i++)
            {
                auto genMbr = genDecl.getDecl()->Members[i];
                auto requiredGenMbr = genDecl.getDecl()->Members[i];
                if (auto genTypeMbr = genMbr.As<GenericTypeParamDecl>())
                {
                    if (auto requiredGenTypeMbr = requiredGenMbr.As<GenericTypeParamDecl>())
                    {
                    }
                    else
                        return false;
                }
                else if (auto genValMbr = genMbr.As<GenericValueParamDecl>())
                {
                    if (auto requiredGenValMbr = requiredGenMbr.As<GenericValueParamDecl>())
                    {
                        if (!genValMbr->type->Equals(requiredGenValMbr->type))
                            return false;
                    }
                    else
                        return false;
                }
                else if (auto genTypeConstraintMbr = genMbr.As<GenericTypeConstraintDecl>())
                {
                    if (auto requiredTypeConstraintMbr = requiredGenMbr.As<GenericTypeConstraintDecl>())
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
            return doesMemberSatisfyRequirement(DeclRef<Decl>(genDecl.getDecl()->inner.Ptr(), genDecl.substitutions),
                DeclRef<Decl>(requirementGenDecl.getDecl()->inner.Ptr(), requirementGenDecl.substitutions),
                requirementDict);
        }

        // Does the given `memberDecl` work as an implementation
        // to satisfy the requirement `requiredMemberDeclRef`
        // from an interface?
        bool doesMemberSatisfyRequirement(
            DeclRef<Decl>   memberDeclRef,
            DeclRef<Decl>   requiredMemberDeclRef,
            Dictionary<DeclRef<Decl>, DeclRef<Decl>> & requirementDictionary)
        {
            // At a high level, we want to chack that the
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
            auto checkSubTypeMember = [&](DeclRef<ContainerDecl> subStructTypeDeclRef) -> bool
            {
                checkDecl(subStructTypeDeclRef.getDecl());
                // this is a sub type (e.g. nested struct declaration) in an aggregate type
                // check if this sub type declaration satisfies the constraints defined by the associated type
                if (auto requiredTypeDeclRef = requiredMemberDeclRef.As<AssocTypeDecl>())
                {
                    bool conformance = true;
                    auto inheritanceReqDeclRefs = getMembersOfType<TypeConstraintDecl>(requiredTypeDeclRef);
                    for (auto inheritanceReqDeclRef : inheritanceReqDeclRefs)
                    {
                        auto interfaceDeclRefType = inheritanceReqDeclRef.getDecl()->getSup().type.As<DeclRefType>();
                        SLANG_ASSERT(interfaceDeclRefType);
                        auto interfaceDeclRef = interfaceDeclRefType->declRef.As<InterfaceDecl>();
                        SLANG_ASSERT(interfaceDeclRef);
                        RefPtr<DeclRefType> declRefType = new DeclRefType();
                        declRefType->declRef = subStructTypeDeclRef;
                        auto witness = tryGetInterfaceConformanceWitness(declRefType,
                            interfaceDeclRef).As<SubtypeWitness>();
                        if (witness)
                            requirementDictionary.Add(inheritanceReqDeclRef, witness->getLastStepDeclRef());
                        else
                            conformance = false;
                    }
                    return conformance;
                }
                return false;
            };
            if (auto memberFuncDecl = memberDeclRef.As<FuncDecl>())
            {
                if (auto requiredFuncDeclRef = requiredMemberDeclRef.As<FuncDecl>())
                {
                    // Check signature match.
                    return doesSignatureMatchRequirement(
                        memberFuncDecl,
                        requiredFuncDeclRef,
                        requirementDictionary);
                }
            }
            else if (auto memberInitDecl = memberDeclRef.As<ConstructorDecl>())
            {
                if (auto requiredInitDecl = requiredMemberDeclRef.As<ConstructorDecl>())
                {
                    // Check signature match.
                    return doesSignatureMatchRequirement(
                        memberInitDecl,
                        requiredInitDecl,
                        requirementDictionary);
                }
            }
            else if (auto genDecl = memberDeclRef.As<GenericDecl>())
            {
                if (auto requiredGenDeclRef = requiredMemberDeclRef.As<GenericDecl>())
                {
                    return doesGenericSignatureMatchRequirement(genDecl, requiredGenDeclRef, requirementDictionary);
                }
            }
            else if (auto subStructTypeDeclRef = memberDeclRef.As<AggTypeDecl>())
            {
                return checkSubTypeMember(subStructTypeDeclRef);
            }
            else if (auto typedefDeclRef = memberDeclRef.As<TypeDefDecl>())
            {
                // this is a type-def decl in an aggregate type
                // check if the specified type satisfies the constraints defined by the associated type
                if (auto requiredTypeDeclRef = requiredMemberDeclRef.As<AssocTypeDecl>())
                {
                    auto declRefType = GetType(typedefDeclRef)->GetCanonicalType()->As<DeclRefType>();
                    if (!declRefType)
                        return false;

                    if (auto genTypeParamDeclRef = declRefType->declRef.As<GenericTypeParamDecl>())
                    {
                        // TODO: check generic type parameter satisfies constraints
                        return true;
                    }
                    

                    auto containerDeclRef = declRefType->declRef.As<ContainerDecl>();
                    if (!containerDeclRef)
                        return false;

                    return checkSubTypeMember(containerDeclRef);
                }
            }
            // Default: just assume that thing aren't being satisfied.
            return false;
        }

        // Find the appropriate member of a declared type to
        // satisfy a requirement of an interface the type
        // claims to conform to.
        //
        // The type declaration `typeDecl` has declared that it
        // conforms to the interface `interfaceDeclRef`, and
        // `requiredMemberDeclRef` is a required member of
        // the interface.
        RefPtr<Decl> findWitnessForInterfaceRequirement(
            DeclRef<AggTypeDeclBase>    typeDeclRef,
            InheritanceDecl*        inheritanceDecl,
            DeclRef<InterfaceDecl>  interfaceDeclRef,
            DeclRef<Decl>           requiredMemberDeclRef,
            Dictionary<DeclRef<Decl>, DeclRef<Decl>> & requirementWitness)
        {
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
                return nullptr;
            }

            // Iterate over the members and look for one that matches
            // the expected signature for the requirement.
            for (auto member : lookupResult)
            {
                if (doesMemberSatisfyRequirement(member.declRef, requiredMemberDeclRef, requirementWitness))
                    return member.declRef.getDecl();
            }
            
            // No suitable member found, although there were candidates.
            //
            // TODO: Eventually we might want something akin to the current
            // overload resolution logic, where we keep track of a list
            // of "candidates" for satisfaction of the requirement,
            // and if nothing is found we print the candidates

            getSink()->diagnose(inheritanceDecl, Diagnostics::typeDoesntImplementInterfaceRequirement, typeDeclRef, requiredMemberDeclRef);
            return nullptr;
        }

        // Check that the type declaration `typeDecl`, which
        // declares conformance to the interface `interfaceDeclRef`,
        // (via the given `inheritanceDecl`) actually provides
        // members to satisfy all the requirements in the interface.
        bool checkInterfaceConformance(
            HashSet<DeclRef<InterfaceDecl>> & checkedInterfaceDeclRef,
            DeclRef<AggTypeDeclBase>        typeDeclRef,
            InheritanceDecl*    inheritanceDecl,
            DeclRef<InterfaceDecl>  interfaceDeclRef)
        {
            if (!checkedInterfaceDeclRef.Contains(interfaceDeclRef))
                checkedInterfaceDeclRef.Add(interfaceDeclRef);
            else
                return true;

            bool result = true;

            // We need to check the declaration of the interface
            // before we can check that we conform to it.
            checkDecl(interfaceDeclRef.getDecl());

            // TODO: If we ever allow for implementation inheritance,
            // then we will need to consider the case where a type
            // declares that it conforms to an interface, but one of
            // its (non-interface) base types already conforms to
            // that interface, so that all of the requirements are
            // already satisfied with inherited implementations...
            auto allMembers = getMembersWithExt(interfaceDeclRef);
            for (auto requiredMemberDeclRef : allMembers)
            {
                // Some members of the interface don't actually represent
                // things that we required of the implementing type.
                // For example, when the interface declares that
                // it inherits from another interface, we don't look for
                // a matching inheritance clause on the type, but
                // instead require that it also conforms to that
                // interface.
                if (auto requiredInheritanceDeclRef = requiredMemberDeclRef.As<InheritanceDecl>())
                {
                    // Recursively check that the type conforms
                    // to the inherited interface.
                    //
                    // TODO: we *really* need a linearization step here!!!!
                    result = result && checkConformanceToType(
                        checkedInterfaceDeclRef,
                        typeDeclRef,
                        inheritanceDecl,
                        getBaseType(requiredInheritanceDeclRef));
                    continue;
                }

                // Look for a member in the type that can satisfy the
                // interface requirement.
                auto isConformanceSatisfied = findWitnessForInterfaceRequirement(
                    typeDeclRef,
                    inheritanceDecl,
                    interfaceDeclRef,
                    requiredMemberDeclRef,
                    inheritanceDecl->requirementWitnesses);

                if (!isConformanceSatisfied)
                {
                    result = false;
                    continue;
                }
            }
            return result;
        }

        bool checkConformanceToType(
            HashSet<DeclRef<InterfaceDecl>>& checkedInterfaceDeclRefs,
            DeclRef<AggTypeDeclBase> typeDeclRef,
            InheritanceDecl*     inheritanceDecl,
            Type*                baseType)
        {
            if (auto baseDeclRefType = baseType->As<DeclRefType>())
            {
                auto baseTypeDeclRef = baseDeclRefType->declRef;
                if (auto baseInterfaceDeclRef = baseTypeDeclRef.As<InterfaceDecl>())
                {
                    // The type is stating that it conforms to an interface.
                    // We need to check that it provides all of the members
                    // required by that interface.
                    return checkInterfaceConformance(
                        checkedInterfaceDeclRefs,
                        typeDeclRef,
                        inheritanceDecl,
                        baseInterfaceDeclRef);
                }
            }

            getSink()->diagnose(inheritanceDecl, Diagnostics::unimplemented, "type not supported for inheritance");
            return false;
        }

        // Check that the type declaration `typeDecl`, which
        // declares that it inherits from another type via
        // `inheritanceDecl` actually does what it needs to
        // for that inheritance to be valid.
        bool checkConformance(
            DeclRef<AggTypeDeclBase>        typeDecl,
            InheritanceDecl*            inheritanceDecl)
        {
            // Look at the type being inherited from, and validate
            // appropriately.
            auto baseType = inheritanceDecl->base.type;
            HashSet<DeclRef<InterfaceDecl>> checkdInterfaceDeclRefs;
            return checkConformanceToType(checkdInterfaceDeclRefs, typeDecl, inheritanceDecl, baseType.As<Type>());
        }

        bool checkConformance(
            AggTypeDeclBase*        typeDecl,
            InheritanceDecl*    inheritanceDecl)
        {
            return checkConformance(DeclRef<AggTypeDeclBase>(typeDecl, SubstitutionSet()), inheritanceDecl);
        }

        void checkExtensionConformance(ExtensionDecl* decl)
        {
            DeclRef<AggTypeDecl> aggTypeDeclRef;
            if (auto targetDeclRefType = decl->targetType->As<DeclRefType>())
            {
                if (aggTypeDeclRef = targetDeclRefType->declRef.As<AggTypeDecl>())
                {
                    for (auto inheritanceDecl : decl->getMembersOfType<InheritanceDecl>())
                    {
                        checkConformance(aggTypeDeclRef.getDecl(), inheritanceDecl);
                    }
                }
            }
        }

        void checkAggTypeConformance(AggTypeDecl* decl)
        {
            // After we've checked members, we need to go through
            // any inheritance clauses on the type itself, and
            // confirm that the type actually provides whatever
            // those clauses require.

            if (auto interfaceDecl = dynamic_cast<InterfaceDecl*>(decl))
            {
                // Don't check that an interface conforms to the
                // things it inherits from.
            }
            else if (auto assocTypeDecl = dynamic_cast<AssocTypeDecl*>(decl))
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
                    checkConformance(decl, inheritanceDecl);
                }
            }
        }

        void visitAggTypeDecl(AggTypeDecl* decl)
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
            // expands them to include implicitic inheritance,
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

        void visitDeclGroup(DeclGroup* declGroup)
        {
            for (auto decl : declGroup->decls)
            {
                DeclVisitor::dispatch(decl);
            }
        }

        void visitTypeDefDecl(TypeDefDecl* decl)
        {
            if (decl->IsChecked(getCheckedState())) return;
            if (checkingPhase == CheckingPhase::Header)
            {
                decl->type = CheckProperType(decl->type);
            }
            decl->SetCheckState(getCheckedState());
        }

        void visitGlobalGenericParamDecl(GlobalGenericParamDecl * decl)
        {
            if (decl->IsChecked(getCheckedState())) return;
            if (checkingPhase == CheckingPhase::Header)
            {
                decl->SetCheckState(DeclCheckState::CheckedHeader);
                // global generic param only allowed in global scope
                auto program = decl->ParentDecl->As<ModuleDecl>();
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

        void visitAssocTypeDecl(AssocTypeDecl* decl)
        {
            if (decl->IsChecked(getCheckedState())) return;
            if (checkingPhase == CheckingPhase::Header)
            {
                decl->SetCheckState(DeclCheckState::CheckedHeader);

                // assoctype only allowed in an interface
                auto interfaceDecl = decl->ParentDecl->As<InterfaceDecl>();
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

        void checkStmt(Stmt* stmt)
        {
            if (!stmt) return;
            StmtVisitor::dispatch(stmt);
            checkModifiers(stmt);
        }

        void visitFuncDecl(FuncDecl *functionNode)
        {
            if (functionNode->IsChecked(getCheckedState()))
                return;

            if (checkingPhase == CheckingPhase::Header)
            {
                VisitFunctionDeclaration(functionNode);
            }
            // TODO: This should really only set "checked header"
            functionNode->SetCheckState(getCheckedState());

            if (checkingPhase == CheckingPhase::Body)
            {
                // TODO: should put the checking of the body onto a "work list"
                // to avoid recursion here.
                if (functionNode->Body)
                {
                    auto oldFunc = function;
                    this->function = functionNode;
                    checkStmt(functionNode->Body);
                    this->function = oldFunc;
                }
            }
        }

        void getGenericParams(
            GenericDecl*                        decl,
            List<Decl*>&                        outParams,
            List<GenericTypeConstraintDecl*>    outConstraints)
        {
            for (auto dd : decl->Members)
            {
                if (dd == decl->inner)
                    continue;

                if (auto typeParamDecl = dd.As<GenericTypeParamDecl>())
                    outParams.Add(typeParamDecl);
                else if (auto valueParamDecl = dd.As<GenericValueParamDecl>())
                    outParams.Add(valueParamDecl);
                else if (auto constraintDecl = dd.As<GenericTypeConstraintDecl>())
                    outConstraints.Add(constraintDecl);
            }
        }

        bool doGenericSignaturesMatch(
            GenericDecl*    fst,
            GenericDecl*    snd)
        {
            // First we'll extract the parameters and constraints
            // in each generic signature. We will consider parameters
            // and constraints separately so that we are independent
            // of the order in which constraints are given (that is,
            // a constraint like `<T : IFoo>` whould be considered
            // the same as `<T>` with a later `where T : IFoo`.

            List<Decl*> fstParams;
            List<GenericTypeConstraintDecl*> fstConstraints;
            getGenericParams(fst, fstParams, fstConstraints);

            List<Decl*> sndParams;
            List<GenericTypeConstraintDecl*> sndConstraints;
            getGenericParams(snd, sndParams, sndConstraints);

            // For there to be any hope of a match, the
            // two need to have the same number of parameters.
            UInt paramCount = fstParams.Count();
            if (paramCount != sndParams.Count())
                return false;

            // Now we'll walk through the parameters.
            for (UInt pp = 0; pp < paramCount; ++pp)
            {
                Decl* fstParam = fstParams[pp];
                Decl* sndParam = sndParams[pp];

                if (auto fstTypeParam = dynamic_cast<GenericTypeParamDecl*>(fstParam))
                {
                    if (auto sndTypeParam = dynamic_cast<GenericTypeParamDecl*>(sndParam))
                    {
                        // TODO: is there any validation that needs to be peformed here?
                    }
                    else
                    {
                        // Type and non-type parameters can't match.
                        return false;
                    }
                }
                else if (auto fstValueParam = dynamic_cast<GenericValueParamDecl*>(fstParam))
                {
                    if (auto sndValueParam = dynamic_cast<GenericValueParamDecl*>(sndParam))
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
            UInt constraintCount = fstConstraints.Count();
            if(constraintCount != sndConstraints.Count())
                return false;

            for (UInt cc = 0; cc < constraintCount; ++cc)
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

        // Check if two functions have the same signature for the purposes
        // of overload resolution.
        bool doFunctionSignaturesMatch(
            DeclRef<FuncDecl> fst,
            DeclRef<FuncDecl> snd)
        {

            // TODO(tfoley): This copies the parameter array, which is bad for performance.
            auto fstParams = GetParameters(fst).ToArray();
            auto sndParams = GetParameters(snd).ToArray();

            // If the functions have different numbers of parameters, then
            // their signatures trivially don't match.
            auto fstParamCount = fstParams.Count();
            auto sndParamCount = sndParams.Count();
            if (fstParamCount != sndParamCount)
                return false;

            for (UInt ii = 0; ii < fstParamCount; ++ii)
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
            }

            // Note(tfoley): return type doesn't enter into it, because we can't take
            // calling context into account during overload resolution.

            return true;
        }

        RefPtr<GenericSubstitution> createDummySubstitutions(
            GenericDecl* genericDecl)
        {
            RefPtr<GenericSubstitution> subst = new GenericSubstitution();
            subst->genericDecl = genericDecl;
            for (auto dd : genericDecl->Members)
            {
                if (dd == genericDecl->inner)
                    continue;

                if (auto typeParam = dd.As<GenericTypeParamDecl>())
                {
                    auto type = DeclRefType::Create(getSession(),
                        makeDeclRef(typeParam.Ptr()));
                    subst->args.Add(type);
                }
                else if (auto valueParam = dd.As<GenericValueParamDecl>())
                {
                    auto val = new GenericParamIntVal(
                        makeDeclRef(valueParam.Ptr()));
                    subst->args.Add(val);
                }
                // TODO: need to handle constraints here?
            }
            return subst;
        }

        void ValidateFunctionRedeclaration(FuncDecl* funcDecl)
        {
            auto parentDecl = funcDecl->ParentDecl;
            SLANG_RELEASE_ASSERT(parentDecl);
            if (!parentDecl) return;

            Decl* childDecl = funcDecl;

            // If this is a generic function (that is, its parent
            // declaration is a generic), then we need to look
            // for sibling declarations of the parent.
            auto genericDecl = dynamic_cast<GenericDecl*>(parentDecl);
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
                auto prevGenericDecl = dynamic_cast<GenericDecl*>(prevDecl);
                if (prevGenericDecl)
                    prevDecl = prevGenericDecl->inner.Ptr();

                // We only care about previously-declared functions
                // Note(tfoley): although we should really error out if the
                // name is already in use for something else, like a variable...
                auto prevFuncDecl = dynamic_cast<FuncDecl*>(prevDecl);
                if (!prevFuncDecl)
                    continue;

                // If one declaration is a prefix/postfix operator, and the
                // other is not a matching operator, then don't consider these
                // to be redeclarations.
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
                    assert(prevGenericDecl);
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
                    prevFuncDeclRef.substitutions.genericSubstitutions = subst;
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
                    // Bad dedeclaration
                    getSink()->diagnose(funcDecl, Diagnostics::unimplemented, "redeclaration has a different return type");

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
                    getSink()->diagnose(funcDecl, Diagnostics::unimplemented, "function redefinition");

                    // Don't bother emitting other errors
                    break;
                }

                // At this point we've processed the redeclaration and
                // put it into a group, so there is no reason to keep
                // looping and looking at prior declarations.
                return;
            }
        }

        void visitScopeDecl(ScopeDecl*)
        {
            // Nothing to do
        }

        void visitParamDecl(ParamDecl* para)
        {
            // TODO: This needs to bottleneck through the common variable checks

            if(para->type.exp)
            {
                para->type = CheckUsableType(para->type);
            
                if (para->type.Equals(getSession()->getVoidType()))
                {
                    getSink()->diagnose(para, Diagnostics::parameterCannotBeVoid);
                }
            }
        }

        void VisitFunctionDeclaration(FuncDecl *functionNode)
        {
            if (functionNode->IsChecked(DeclCheckState::CheckedHeader)) return;
            functionNode->SetCheckState(DeclCheckState::CheckingHeader);
            auto oldFunc = this->function;
            this->function = functionNode;
            auto returnType = CheckProperType(functionNode->ReturnType);
            functionNode->ReturnType = returnType;
            HashSet<Name*> paraNames;
            for (auto & para : functionNode->GetParameters())
            {
                checkDecl(para);

                if (paraNames.Contains(para->getName()))
                {
                    getSink()->diagnose(para, Diagnostics::parameterAlreadyDefined, para->getName());
                }
                else
                    paraNames.Add(para->getName());
            }
            this->function = oldFunc;
            functionNode->SetCheckState(DeclCheckState::CheckedHeader);

            // One last bit of validation: check if we are redeclaring an existing function
            ValidateFunctionRedeclaration(functionNode);
        }

        void visitDeclStmt(DeclStmt* stmt)
        {
            // We directly dispatch here instead of using `EnsureDecl()` for two
            // reasons:
            //
            // 1. We expect that a local declaration won't have been referenced
            // before it is declared, so that we can just check things in-order
            //
            // 2. `EnsureDecl()` is specialized for `Decl*` instead of `DeclBase*`
            // and trying to special case `DeclGroup*` here feels silly.
            //
            DeclVisitor::dispatch(stmt->decl);
        }

        void visitBlockStmt(BlockStmt* stmt)
        {
            checkStmt(stmt->body);
        }

        void visitSeqStmt(SeqStmt* stmt)
        {
            for(auto ss : stmt->stmts)
            {
                checkStmt(ss);
            }
        }

        template<typename T>
        T* FindOuterStmt()
        {
            UInt outerStmtCount = outerStmts.Count();
            for (UInt ii = outerStmtCount; ii > 0; --ii)
            {
                auto outerStmt = outerStmts[ii-1];
                auto found = dynamic_cast<T*>(outerStmt);
                if (found)
                    return found;
            }
            return nullptr;
        }

        void visitBreakStmt(BreakStmt *stmt)
        {
            auto outer = FindOuterStmt<BreakableStmt>();
            if (!outer)
            {
                getSink()->diagnose(stmt, Diagnostics::breakOutsideLoop);
            }
            stmt->parentStmt = outer;
        }
        void visitContinueStmt(ContinueStmt *stmt)
        {
            auto outer = FindOuterStmt<LoopStmt>();
            if (!outer)
            {
                getSink()->diagnose(stmt, Diagnostics::continueOutsideLoop);
            }
            stmt->parentStmt = outer;
        }

        void PushOuterStmt(Stmt* stmt)
        {
            outerStmts.Add(stmt);
        }

        void PopOuterStmt(Stmt* /*stmt*/)
        {
            outerStmts.RemoveAt(outerStmts.Count() - 1);
        }

        RefPtr<Expr> checkPredicateExpr(Expr* expr)
        {
            RefPtr<Expr> e = expr;
            e = CheckTerm(e);
            e = Coerce(getSession()->getBoolType(), e);
            return e;
        }

        void visitDoWhileStmt(DoWhileStmt *stmt)
        {
            PushOuterStmt(stmt);
            stmt->Predicate = checkPredicateExpr(stmt->Predicate);
            checkStmt(stmt->Statement);

            PopOuterStmt(stmt);
        }
        void visitForStmt(ForStmt *stmt)
        {
            PushOuterStmt(stmt);
            checkStmt(stmt->InitialStatement);
            if (stmt->PredicateExpression)
            {
                stmt->PredicateExpression = checkPredicateExpr(stmt->PredicateExpression);
            }
            if (stmt->SideEffectExpression)
            {
                stmt->SideEffectExpression = CheckExpr(stmt->SideEffectExpression);
            }
            checkStmt(stmt->Statement);

            PopOuterStmt(stmt);
        }

        RefPtr<Expr> checkExpressionAndExpectIntegerConstant(RefPtr<Expr> expr, RefPtr<IntVal>* outIntVal)
        {
            expr = CheckExpr(expr);
            auto intVal = CheckIntegerConstantExpression(expr);
            if (outIntVal)
                *outIntVal = intVal;
            return expr;
        }

        void visitCompileTimeForStmt(CompileTimeForStmt* stmt)
        {
            PushOuterStmt(stmt);

            stmt->varDecl->type.type = getSession()->getIntType();
            addModifier(stmt->varDecl, new ConstModifier());
            stmt->varDecl->SetCheckState(DeclCheckState::Checked);

            RefPtr<IntVal> rangeBeginVal;
            RefPtr<IntVal> rangeEndVal;

            if (stmt->rangeBeginExpr)
            {
                stmt->rangeBeginExpr = checkExpressionAndExpectIntegerConstant(stmt->rangeBeginExpr, &rangeBeginVal);
            }
            else
            {
                RefPtr<ConstantIntVal> rangeBeginConst = new ConstantIntVal();
                rangeBeginConst->value = 0;
                rangeBeginVal = rangeBeginConst;
            }

            stmt->rangeEndExpr = checkExpressionAndExpectIntegerConstant(stmt->rangeEndExpr, &rangeEndVal);

            stmt->rangeBeginVal = rangeBeginVal;
            stmt->rangeEndVal = rangeEndVal;

            checkStmt(stmt->body);


            PopOuterStmt(stmt);
        }

        void visitSwitchStmt(SwitchStmt* stmt)
        {
            PushOuterStmt(stmt);
            // TODO(tfoley): need to coerce condition to an integral type...
            stmt->condition = CheckExpr(stmt->condition);
            checkStmt(stmt->body);

            // TODO(tfoley): need to check that all case tags are unique

            // TODO(tfoley): check that there is at most one `default` clause

            PopOuterStmt(stmt);
        }
        void visitCaseStmt(CaseStmt* stmt)
        {
            // TODO(tfoley): Need to coerce to type being switch on,
            // and ensure that value is a compile-time constant
            auto expr = CheckExpr(stmt->expr);
            auto switchStmt = FindOuterStmt<SwitchStmt>();

            if (!switchStmt)
            {
                getSink()->diagnose(stmt, Diagnostics::caseOutsideSwitch);
            }
            else
            {
                // TODO: need to do some basic matching to ensure the type
                // for the `case` is consistent with the type for the `switch`...
            }

            stmt->expr = expr;
            stmt->parentStmt = switchStmt;
        }
        void visitDefaultStmt(DefaultStmt* stmt)
        {
            auto switchStmt = FindOuterStmt<SwitchStmt>();
            if (!switchStmt)
            {
                getSink()->diagnose(stmt, Diagnostics::defaultOutsideSwitch);
            }
            stmt->parentStmt = switchStmt;
        }
        void visitIfStmt(IfStmt *stmt)
        {
            stmt->Predicate = checkPredicateExpr(stmt->Predicate);
            checkStmt(stmt->PositiveStatement);
            checkStmt(stmt->NegativeStatement);
        }

        void visitUnparsedStmt(UnparsedStmt*)
        {
            // Nothing to do
        }

        void visitEmptyStmt(EmptyStmt*)
        {
            // Nothing to do
        }

        void visitDiscardStmt(DiscardStmt*)
        {
            // Nothing to do
        }

        void visitReturnStmt(ReturnStmt *stmt)
        {
            if (!stmt->Expression)
            {
                if (function && !function->ReturnType.Equals(getSession()->getVoidType()))
                {
                    getSink()->diagnose(stmt, Diagnostics::returnNeedsExpression);
                }
            }
            else
            {
                stmt->Expression = CheckTerm(stmt->Expression);
                if (!stmt->Expression->type->Equals(getSession()->getErrorType()))
                {
                    if (function)
                    {
                        stmt->Expression = Coerce(function->ReturnType.Ptr(), stmt->Expression);
                    }
                    else
                    {
                        // TODO(tfoley): this case currently gets triggered for member functions,
                        // which aren't being checked consistently (because of the whole symbol
                        // table idea getting in the way).

//							getSink()->diagnose(stmt, Diagnostics::unimplemented, "case for return stmt");
                    }
                }
            }
        }

        IntegerLiteralValue GetMinBound(RefPtr<IntVal> val)
        {
            if (auto constantVal = val.As<ConstantIntVal>())
                return constantVal->value;

            // TODO(tfoley): Need to track intervals so that this isn't just a lie...
            return 1;
        }

        void maybeInferArraySizeForVariable(Variable* varDecl)
        {
            // Not an array?
            auto arrayType = varDecl->type->AsArrayType();
            if (!arrayType) return;

            // Explicit element count given?
            auto elementCount = arrayType->ArrayLength;
            if (elementCount) return;

            // No initializer?
            auto initExpr = varDecl->initExpr;
            if(!initExpr) return;

            // Is the initializer an initializer list?
            if(auto initializerListExpr = initExpr.As<InitializerListExpr>())
            {
                auto argCount = initializerListExpr->args.Count();
                elementCount = new ConstantIntVal(argCount);
            }
            // Is the type of the initializer an array type?
            else if(auto arrayInitType = initExpr->type->As<ArrayExpressionType>())
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

        void ValidateArraySizeForVariable(Variable* varDecl)
        {
            auto arrayType = varDecl->type->AsArrayType();
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

        void visitVariable(Variable* varDecl)
        {
            if (function || checkingPhase == CheckingPhase::Header)
            {
                TypeExp typeExp = CheckUsableType(varDecl->type);
#if 0
                if (typeExp.type->GetBindableResourceType() != BindableResourceType::NonBindable)
                {
                    // We don't want to allow bindable resource types as local variables (at least for now).
                    auto parentDecl = varDecl->ParentDecl;
                    if (auto parentScopeDecl = dynamic_cast<ScopeDecl*>(parentDecl))
                    {
                        getSink()->diagnose(varDecl->type, Diagnostics::invalidTypeForLocalVariable);
                    }
                }
#endif
                varDecl->type = typeExp;
                if (varDecl->type.Equals(getSession()->getVoidType()))
                {
                    getSink()->diagnose(varDecl, Diagnostics::invalidTypeVoid);
                }
            }

            if (checkingPhase == CheckingPhase::Body)
            {
                if (auto initExpr = varDecl->initExpr)
                {
                    initExpr = CheckTerm(initExpr);
                    varDecl->initExpr = initExpr;
                }

                // If this is an array variable, then we first want to give
                // it a chance to infer an array size from its initializer
                //
                // TODO(tfoley): May need to extend this to handle the
                // multi-dimensional case...
                maybeInferArraySizeForVariable(varDecl);
                //
                // Next we want to make sure that the declared (or inferred)
                // size for the array meets whatever language-specific
                // constraints we want to enforce (e.g., disallow empty
                // arrays in specific cases)
                ValidateArraySizeForVariable(varDecl);


                if (auto initExpr = varDecl->initExpr)
                {
                    // TODO(tfoley): should coercion of initializer lists be special-cased
                    // here, or handled as a general case for coercion?

                    initExpr = Coerce(varDecl->type.Ptr(), initExpr);
                    varDecl->initExpr = initExpr;
                }
            }
            varDecl->SetCheckState(getCheckedState());
        }

        void visitWhileStmt(WhileStmt *stmt)
        {
            PushOuterStmt(stmt);
            stmt->Predicate = checkPredicateExpr(stmt->Predicate);
            checkStmt(stmt->Statement);
            PopOuterStmt(stmt);
        }
        void visitExpressionStmt(ExpressionStmt *stmt)
        {
            stmt->Expression = CheckExpr(stmt->Expression);
        }

        RefPtr<Expr> visitBoolLiteralExpr(BoolLiteralExpr* expr)
        {
            expr->type = getSession()->getBoolType();
            return expr;
        }

        RefPtr<Expr> visitIntegerLiteralExpr(IntegerLiteralExpr* expr)
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
                expr->type = getSession()->getIntType();
            }
            return expr;
        }

        RefPtr<Expr> visitFloatingPointLiteralExpr(FloatingPointLiteralExpr* expr)
        {
            if(!expr->type.type)
            {
                expr->type = getSession()->getFloatType();
            }
            return expr;
        }

        RefPtr<Expr> visitStringLiteralExpr(StringLiteralExpr* expr)
        {
            expr->type = getSession()->getStringType();
            return expr;
        }

        IntVal* GetIntVal(IntegerLiteralExpr* expr)
        {
            // TODO(tfoley): don't keep allocating here!
            return new ConstantIntVal(expr->value);
        }

        Name* getName(String const& text)
        {
            return getCompileRequest()->getNamePool()->getName(text);
        }

        RefPtr<IntVal> TryConstantFoldExpr(
            InvokeExpr* invokeExpr)
        {
            // We need all the operands to the expression

            // Check if the callee is an operation that is amenable to constant-folding.
            //
            // For right now we will look for calls to intrinsic functions, and then inspect
            // their names (this is bad and slow).
            auto funcDeclRefExpr = invokeExpr->FunctionExpr.As<DeclRefExpr>();
            if (!funcDeclRefExpr) return nullptr;

            auto funcDeclRef = funcDeclRefExpr->declRef;
            auto intrinsicMod = funcDeclRef.getDecl()->FindModifier<IntrinsicOpModifier>();
            if (!intrinsicMod)
            {
                // We can't constant fold anything that doesn't map to a builtin
                // operation right now.
                //
                // TODO: we should really allow constant-folding for anything
                // that can be lowerd to our bytecode...
                return nullptr;
            }



            // Let's not constant-fold operations with more than a certain number of arguments, for simplicity
            static const int kMaxArgs = 8;
            if (invokeExpr->Arguments.Count() > kMaxArgs)
                return nullptr;

            // Before checking the operation name, let's look at the arguments
            RefPtr<IntVal> argVals[kMaxArgs];
            IntegerLiteralValue constArgVals[kMaxArgs];
            int argCount = 0;
            bool allConst = true;
            for (auto argExpr : invokeExpr->Arguments)
            {
                auto argVal = TryCheckIntegerConstantExpression(argExpr.Ptr());
                if (!argVal)
                    return nullptr;

                argVals[argCount] = argVal;

                if (auto constArgVal = argVal.As<ConstantIntVal>())
                {
                    constArgVals[argCount] = constArgVal->value;
                }
                else
                {
                    allConst = false;
                }
                argCount++;
            }

            if (!allConst)
            {
                // TODO(tfoley): We probably want to support a very limited number of operations
                // on "constants" that aren't actually known, to be able to handle a generic
                // that takes an integer `N` but then constructs a vector of size `N+1`.
                //
                // The hard part there is implementing the rules for value unification in the
                // presence of more complicated `IntVal` subclasses, like `SumIntVal`. You'd
                // need inference to be smart enough to know that `2 + N` and `N + 2` are the
                // same value, as are `N + M + 1 + 1` and `M + 2 + N`.
                //
                // For now we can just bail in this case.
                return nullptr;
            }

            // At this point, all the operands had simple integer values, so we are golden.
            IntegerLiteralValue resultValue = 0;
            auto opName = funcDeclRef.GetName();

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

            // simple binary operators
#define CASE(OP)                                                    \
            else if(opName == getName(#OP)) do {                    \
                if(argCount != 2) return nullptr;                   \
                resultValue = constArgVals[0] OP constArgVals[1];   \
            } while(0)

            CASE(+); // TODO: this can also be unary...
            CASE(*);
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

            // TODO(tfoley): more cases
            else
            {
                return nullptr;
            }

            RefPtr<IntVal> result = new ConstantIntVal(resultValue);
            return result;
        }

        RefPtr<IntVal> TryConstantFoldExpr(
            Expr* expr)
        {
            // Unwrap any "identity" expressions
            while (auto parenExpr = dynamic_cast<ParenExpr*>(expr))
            {
                expr = parenExpr->base;
            }

            // TODO(tfoley): more serious constant folding here
            if (auto intLitExpr = dynamic_cast<IntegerLiteralExpr*>(expr))
            {
                return GetIntVal(intLitExpr);
            }

            // it is possible that we are referring to a generic value param
            if (auto declRefExpr = dynamic_cast<DeclRefExpr*>(expr))
            {
                auto declRef = declRefExpr->declRef;

                if (auto genericValParamRef = declRef.As<GenericValueParamDecl>())
                {
                    // TODO(tfoley): handle the case of non-`int` value parameters...
                    return new GenericParamIntVal(genericValParamRef);
                }

                // We may also need to check for references to variables that
                // are defined in a way that can be used as a constant expression:
                if(auto varRef = declRef.As<VarDeclBase>())
                {
                    auto varDecl = varRef.getDecl();

                    switch(getSourceLanguage())
                    {
                    case SourceLanguage::Slang:
                    case SourceLanguage::HLSL:
                        // HLSL: `static const` is used to mark compile-time constant expressions
                        if(auto staticAttr = varDecl->FindModifier<HLSLStaticModifier>())
                        {
                            if(auto constAttr = varDecl->FindModifier<ConstModifier>())
                            {
                                // HLSL `static const` can be used as a constant expression
                                if(auto initExpr = getInitExpr(varRef))
                                {
                                    return TryConstantFoldExpr(initExpr.Ptr());
                                }
                            }
                        }
                        break;

                    case SourceLanguage::GLSL:
                        // GLSL: `const` indicates compile-time constant expression
                        //
                        // TODO(tfoley): The current logic here isn't robust against
                        // GLSL "specialization constants" - we will extract the
                        // initializer for a `const` variable and use it to extract
                        // a value, when we really should be using an opaque
                        // reference to the variable.
                        if(auto constAttr = varDecl->FindModifier<ConstModifier>())
                        {
                            // We need to handle a "specialization constant" (with a `constant_id` layout modifier)
                            // differently from an ordinary compile-time constant. The latter can/should be reduced
                            // to a value, while the former should be kept as a symbolic reference

                            if(auto constantIDModifier = varDecl->FindModifier<GLSLConstantIDLayoutModifier>())
                            {
                                // Retain the specialization constant as a symbolic reference
                                //
                                // TODO(tfoley): handle the case of non-`int` value parameters...
                                //
                                // TODO(tfoley): this is cloned from the case above that handles generic value parameters
                                return new GenericParamIntVal(varRef);
                            }
                            else if(auto initExpr = getInitExpr(varRef))
                            {
                                // This is an ordinary constant, and not a specialization constant, so we
                                // can try to fold its value right now.
                                return TryConstantFoldExpr(initExpr.Ptr());
                            }
                        }
                        break;
                    }

                }
            }

            if(auto castExpr = dynamic_cast<TypeCastExpr*>(expr))
            {
                auto val = TryConstantFoldExpr(castExpr->Arguments[0].Ptr());
                if(val)
                    return val;
            }
            else if (auto invokeExpr = dynamic_cast<InvokeExpr*>(expr))
            {
                auto val = TryConstantFoldExpr(invokeExpr);
                if (val)
                    return val;
            }

            return nullptr;
        }

        // Try to check an integer constant expression, either returning the value,
        // or NULL if the expression isn't recognized as a constant.
        RefPtr<IntVal> TryCheckIntegerConstantExpression(Expr* exp)
        {
            if (!exp->type.type->Equals(getSession()->getIntType()))
            {
                return nullptr;
            }



            // Otherwise, we need to consider operations that we might be able to constant-fold...
            return TryConstantFoldExpr(exp);
        }

        // Enforce that an expression resolves to an integer constant, and get its value
        RefPtr<IntVal> CheckIntegerConstantExpression(Expr* inExpr)
        {
            // First coerce the expression to the expected type
            auto expr = Coerce(getSession()->getIntType(),inExpr);
            auto result = TryCheckIntegerConstantExpression(expr.Ptr());
            if (!result)
            {
                getSink()->diagnose(expr, Diagnostics::expectedIntegerConstantNotConstant);
            }
            return result;
        }



        RefPtr<Expr> CheckSimpleSubscriptExpr(
            RefPtr<IndexExpr>   subscriptExpr,
            RefPtr<Type>              elementType)
        {
            auto baseExpr = subscriptExpr->BaseExpression;
            auto indexExpr = subscriptExpr->IndexExpression;

            if (!indexExpr->type->Equals(getSession()->getIntType()) &&
                !indexExpr->type->Equals(getSession()->getUIntType()))
            {
                getSink()->diagnose(indexExpr, Diagnostics::subscriptIndexNonInteger);
                return CreateErrorExpr(subscriptExpr.Ptr());
            }

            subscriptExpr->type = QualType(elementType);

            // TODO(tfoley): need to be more careful about this stuff
            subscriptExpr->type.IsLeftValue = baseExpr->type.IsLeftValue;

            return subscriptExpr;
        }

        // The way that we have designed out type system, pretyt much *every*
        // type is a reference to some declaration in the standard library.
        // That means that when we construct a new type on the fly, we need
        // to make sure that it is wired up to reference the appropriate
        // declaration, or else it won't compare as equal to other types
        // that *do* reference the declaration.
        //
        // This function is used to construct a `vector<T,N>` type
        // programmatically, so that it will work just like a type of
        // that form constructed by the user.
        RefPtr<VectorExpressionType> createVectorType(
            RefPtr<Type>  elementType,
            RefPtr<IntVal>          elementCount)
        {
            auto session = getSession();
            auto vectorGenericDecl = findMagicDecl(
                session, "Vector").As<GenericDecl>();
            auto vectorTypeDecl = vectorGenericDecl->inner;
               
            auto substitutions = new GenericSubstitution();
            substitutions->genericDecl = vectorGenericDecl.Ptr();
            substitutions->args.Add(elementType);
            substitutions->args.Add(elementCount);

            auto declRef = DeclRef<Decl>(vectorTypeDecl.Ptr(), substitutions);

            return DeclRefType::Create(
                session,
                declRef)->As<VectorExpressionType>();
        }

        RefPtr<Expr> visitIndexExpr(IndexExpr* subscriptExpr)
        {
            auto baseExpr = subscriptExpr->BaseExpression;
            baseExpr = CheckExpr(baseExpr);

            RefPtr<Expr> indexExpr = subscriptExpr->IndexExpression;
            if (indexExpr)
            {
                indexExpr = CheckExpr(indexExpr);
            }

            subscriptExpr->BaseExpression = baseExpr;
            subscriptExpr->IndexExpression = indexExpr;

            // If anything went wrong in the base expression,
            // then just move along...
            if (IsErrorExpr(baseExpr))
                return CreateErrorExpr(subscriptExpr);

            // Otherwise, we need to look at the type of the base expression,
            // to figure out how subscripting should work.
            auto baseType = baseExpr->type.Ptr();
            if (auto baseTypeType = baseType->As<TypeType>())
            {
                // We are trying to "index" into a type, so we have an expression like `float[2]`
                // which should be interpreted as resolving to an array type.

                RefPtr<IntVal> elementCount = nullptr;
                if (indexExpr)
                {
                    elementCount = CheckIntegerConstantExpression(indexExpr.Ptr());
                }

                auto elementType = CoerceToUsableType(TypeExp(baseExpr, baseTypeType->type));
                auto arrayType = getArrayType(
                    elementType,
                    elementCount);

                typeResult = arrayType;
                subscriptExpr->type = QualType(getTypeType(arrayType));
                return subscriptExpr;
            }
            else if (auto baseArrayType = baseType->As<ArrayExpressionType>())
            {
                return CheckSimpleSubscriptExpr(
                    subscriptExpr,
                    baseArrayType->baseType);
            }
            else if (auto vecType = baseType->As<VectorExpressionType>())
            {
                return CheckSimpleSubscriptExpr(
                    subscriptExpr,
                    vecType->elementType);
            }
            else if (auto matType = baseType->As<MatrixExpressionType>())
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

            {
                LookupResult lookupResult = lookUpMember(
                    getSession(),
                    this,
                    getName("operator[]"),
                    baseType);
                if (!lookupResult.isValid())
                {
                    goto fail;
                }

                // Now that we know there is at least one subscript member,
                // we will construct a reference to it and try to call it.
                //
                // Note: the expression may be an `OverloadedExpr`, in which
                // case the attempt to call it will trigger overload
                // resolution.
                RefPtr<Expr> subscriptFuncExpr = createLookupResultExpr(
                    lookupResult, subscriptExpr->BaseExpression, subscriptExpr->loc);

                RefPtr<InvokeExpr> subscriptCallExpr = new InvokeExpr();
                subscriptCallExpr->loc = subscriptExpr->loc;
                subscriptCallExpr->FunctionExpr = subscriptFuncExpr;

                // TODO(tfoley): This path can support multiple arguments easily
                subscriptCallExpr->Arguments.Add(subscriptExpr->IndexExpression);

                return CheckInvokeExprWithCheckedOperands(subscriptCallExpr.Ptr());
            }

        fail:
            {
                getSink()->diagnose(subscriptExpr, Diagnostics::subscriptNonArray, baseType);
                return CreateErrorExpr(subscriptExpr);
            }
        }

        bool MatchArguments(FuncDecl * functionNode, List <RefPtr<Expr>> &args)
        {
            if (functionNode->GetParameters().Count() != args.Count())
                return false;
            int i = 0;
            for (auto param : functionNode->GetParameters())
            {
                if (!param->type.Equals(args[i]->type.Ptr()))
                    return false;
                i++;
            }
            return true;
        }

        // Coerce an expression to a specific  type that it is expected to have in context
        RefPtr<Expr> CoerceExprToType(
            RefPtr<Expr>	expr,
            RefPtr<Type>			type)
        {
            // TODO(tfoley): clean this up so there is only one version...
            return Coerce(type, expr);
        }

        RefPtr<Expr> visitParenExpr(ParenExpr* expr)
        {
            auto base = expr->base;
            base = CheckTerm(base);

            expr->base = base;
            expr->type = base->type;
            return expr;
        }

        //

        RefPtr<Expr> visitAssignExpr(AssignExpr* expr)
        {
            expr->left = CheckExpr(expr->left);

            auto type = expr->left->type;

            expr->right = Coerce(type, CheckTerm(expr->right));

            if (!type.IsLeftValue)
            {
                if (type->As<ErrorType>())
                {
                    // Don't report an l-value issue on an errorneous expression
                }
                else
                {
                    getSink()->diagnose(expr, Diagnostics::assignNonLValue);
                }
            }
            expr->type = type;
            return expr;
        }

        void registerExtension(ExtensionDecl* decl)
        {
            if (decl->IsChecked(DeclCheckState::CheckedHeader))
                return;

            decl->SetCheckState(DeclCheckState::CheckingHeader);
            decl->targetType = CheckProperType(decl->targetType);
            decl->SetCheckState(DeclCheckState::CheckedHeader);

            // TODO: need to check that the target type names a declaration...

            DeclRef<AggTypeDecl> aggTypeDeclRef;
            if (auto targetDeclRefType = decl->targetType->As<DeclRefType>())
            {
                // Attach our extension to that type as a candidate...
                if (aggTypeDeclRef = targetDeclRefType->declRef.As<AggTypeDecl>())
                {
                    auto aggTypeDecl = aggTypeDeclRef.getDecl();
                    decl->nextCandidateExtension = aggTypeDecl->candidateExtensions;
                    aggTypeDecl->candidateExtensions = decl;
                    return;
                }
            }
            getSink()->diagnose(decl->targetType.exp, Diagnostics::unimplemented, "expected a nominal type here");
        }

        void visitExtensionDecl(ExtensionDecl* decl)
        {
            if (decl->IsChecked(getCheckedState())) return;

            if (!decl->targetType->As<DeclRefType>())
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

        // Figure out what type an initializer/constructor declaration
        // is supposed to return. In most cases this is just the type
        // declaration that its declaration is nested inside.
        RefPtr<Type> findResultTypeForConstructorDecl(ConstructorDecl* decl)
        {
            // We want to look at the parent of the declaration,
            // but if the declaration is generic, the parent will be
            // the `GenericDecl` and we need to skip past that to
            // the grandparent.
            //
            auto parent = decl->ParentDecl;
            auto genericParent = dynamic_cast<GenericDecl*>(parent);
            if (genericParent)
            {
                parent = genericParent->ParentDecl;
            }

            // Now look at the type of the parent (or grandparent).
            if (auto aggTypeDecl = dynamic_cast<AggTypeDecl*>(parent))
            {
                // We are nested in an aggregate type declaration,
                // so the result type of the initializer will just
                // be the surrounding type.
                return DeclRefType::Create(
                    getSession(),
                    makeDeclRef(aggTypeDecl));
            }
            else if (auto extDecl = dynamic_cast<ExtensionDecl*>(parent))
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

        void visitConstructorDecl(ConstructorDecl* decl)
        {
            if (decl->IsChecked(getCheckedState())) return;
            if (checkingPhase == CheckingPhase::Header)
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


        void visitSubscriptDecl(SubscriptDecl* decl)
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
                decl->Members.Add(getterDecl);
            }

            for(auto mm : decl->Members)
            {
                checkDecl(mm);
            }

            decl->SetCheckState(getCheckedState());
        }

        void visitAccessorDecl(AccessorDecl* decl)
        {
            if (checkingPhase == CheckingPhase::Header)
            {
                // An acessor must appear nested inside a subscript declaration (today),
                // or a property declaration (when we add them). It will derive
                // its return type from the outer declaration, so we handle both
                // of these checks at the same place.
                auto parent = decl->ParentDecl;
                if (auto parentSubscript = dynamic_cast<SubscriptDecl*>(parent))
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


        //

        struct Constraint
        {
            Decl*		decl; // the declaration of the thing being constraints
            RefPtr<Val>	val; // the value to which we are constraining it
            bool satisfied = false; // Has this constraint been met?
        };

        // A collection of constraints that will need to be satisified (solved)
        // in order for checking to suceed.
        struct ConstraintSystem
        {
            // The generic declaration whose parameters we
            // are trying to solve for.
            RefPtr<GenericDecl> genericDecl;

            // Constraints we have accumulated, which constrain
            // the possible arguments for those parameters.
            List<Constraint> constraints;
        };

        RefPtr<Type> TryJoinVectorAndScalarType(
            RefPtr<VectorExpressionType> vectorType,
            RefPtr<BasicExpressionType>  scalarType)
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

        struct TypeWitnessBreadcrumb
        {
            TypeWitnessBreadcrumb*  prev;

            RefPtr<Type>            sub;
            RefPtr<Type>            sup;
            DeclRef<Decl>           declRef;
        };

        // Crete a subtype witness based on the declared relationship
        // found in a single breadcrumb
        RefPtr<SubtypeWitness> createSimplSubtypeWitness(
            TypeWitnessBreadcrumb*  breadcrumb)
        {
            RefPtr<DeclaredSubtypeWitness> witness = new DeclaredSubtypeWitness();
            witness->sub = breadcrumb->sub;
            witness->sup = breadcrumb->sup;
            witness->declRef = breadcrumb->declRef;
            return witness;
        }

        RefPtr<Val> createTypeWitness(
            RefPtr<Type>            type,
            DeclRef<InterfaceDecl>  interfaceDeclRef,
            TypeWitnessBreadcrumb*  inBreadcrumbs)
        {
            if(!inBreadcrumbs)
            {
                // We need to construct a witness to the fact
                // that `type` has been proven to be equal
                // to `interfaceDeclRef`.
                //
                SLANG_UNEXPECTED("reflexive type witness");
                UNREACHABLE_RETURN(nullptr);
            }

            // We might have one or more steps in the breadcrumb trail, e.g.:
            //
            //      (A : B) (B : C) (C : D)
            //
            // The chain is stored as a reversed linked list, so that
            // the first entry would be the `(C : D)` relationship
            // above.
            //
            // We are going to walk the list and build up a suitable
            // subtype witness.
            auto bb = inBreadcrumbs;

            // Create a witness for the last step in the chain
            RefPtr<SubtypeWitness> witness = createSimplSubtypeWitness(bb);
            bb = bb->prev;

            // Now, as long as we have more entries to deal with,
            // we'll be in a situation like:
            //
            //      ... (B : C) <witness>
            //
            // and we want to wrap up one more link in our chain.

            while (bb)
            {
                // Create simple witness for the step in the chain
                RefPtr<SubtypeWitness> link = createSimplSubtypeWitness(bb);

                // Now join the link onto the existing chain represented
                // by `witness`.
                RefPtr<TransitiveSubtypeWitness> transitiveWitness = new TransitiveSubtypeWitness();
                transitiveWitness->sub = link->sub;
                transitiveWitness->sup = witness->sup;
                transitiveWitness->subToMid = link;
                transitiveWitness->midToSup = witness;

                witness = transitiveWitness;
                bb = bb->prev;
            }

            return witness;
        }

        bool doesTypeConformToInterfaceImpl(
            RefPtr<Type>            originalType,
            RefPtr<Type>            type,
            DeclRef<InterfaceDecl>  interfaceDeclRef,
            RefPtr<Val>*            outWitness,
            TypeWitnessBreadcrumb*  inBreadcrumbs)
        {
            // for now look up a conformance member...
            if(auto declRefType = type->As<DeclRefType>())
            {
                auto declRef = declRefType->declRef;

                // Easy case: a type conforms to itself.
                //
                // TODO: This is actually a bit more complicated, as
                // the interface needs to be "object-safe" for us to
                // really make this determination...
                if(declRef == interfaceDeclRef)
                {
                    if(outWitness)
                    {
                        *outWitness = createTypeWitness(originalType, interfaceDeclRef, inBreadcrumbs);
                    }
                    return true;
                }

                if( auto aggTypeDeclRef = declRef.As<AggTypeDecl>() )
                {
                    for( auto inheritanceDeclRef : getMembersOfTypeWithExt<InheritanceDecl>(aggTypeDeclRef))
                    {
                        checkDecl(inheritanceDeclRef.getDecl());

                        // Here we will recursively look up conformance on the type
                        // that is being inherited from. This is dangerous because
                        // it might lead to infinite loops.
                        //
                        // TODO: A better appraoch would be to create a linearized list
                        // of all the interfaces that a given type direclty or indirectly
                        // inheirts, and store it with the type, so that we don't have
                        // to recurse in places like this (and can maybe catch infinite
                        // loops better). This would also help avoid checking multiply-inherited
                        // conformances multiple times.

                        auto inheritedType = getBaseType(inheritanceDeclRef);

                        // We need to ensure that the witness that gets created
                        // is a composite one, reflecting lookup through
                        // the inheritance declaration.
                        TypeWitnessBreadcrumb breadcrumb;
                        breadcrumb.prev = inBreadcrumbs;

                        breadcrumb.sub = type;
                        breadcrumb.sup = inheritedType;
                        breadcrumb.declRef = inheritanceDeclRef;

                        if(doesTypeConformToInterfaceImpl(originalType, inheritedType, interfaceDeclRef, outWitness, &breadcrumb))
                        {
                            return true;
                        }
                    }
                    // if an inheritance decl is not found, try to find a GenericTypeConstraintDecl
                    for (auto genConstraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(aggTypeDeclRef))
                    {
                        checkDecl(genConstraintDeclRef.getDecl());
                        auto inheritedType = GetSup(genConstraintDeclRef);
                        TypeWitnessBreadcrumb breadcrumb;
                        breadcrumb.prev = inBreadcrumbs;
                        breadcrumb.sub = type;
                        breadcrumb.sup = inheritedType;
                        breadcrumb.declRef = genConstraintDeclRef;
                        if (doesTypeConformToInterfaceImpl(originalType, inheritedType, interfaceDeclRef, outWitness, &breadcrumb))
                        {
                            return true;
                        }
                    }
                }
                else if( auto genericTypeParamDeclRef = declRef.As<GenericTypeParamDecl>() )
                {
                    // We need to enumerate the constraints placed on this type by its outer
                    // generic declaration, and see if any of them guarantees that we
                    // satisfy the given interface..
                    auto genericDeclRef = genericTypeParamDeclRef.GetParent().As<GenericDecl>();
                    SLANG_ASSERT(genericDeclRef);

                    for( auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(genericDeclRef) )
                    {
                        auto sub = GetSub(constraintDeclRef);
                        auto sup = GetSup(constraintDeclRef);

                        auto subDeclRef = sub->As<DeclRefType>();
                        if(!subDeclRef)
                            continue;
                        if(subDeclRef->declRef != genericTypeParamDeclRef)
                            continue;

                        // The witness that we create needs to reflect that
                        // it found the needed conformance by lookup through
                        // a generic type constraint.

                        TypeWitnessBreadcrumb breadcrumb;
                        breadcrumb.prev = inBreadcrumbs;
                        breadcrumb.sub = sub;
                        breadcrumb.sup = sup;
                        breadcrumb.declRef = constraintDeclRef;

                        if(doesTypeConformToInterfaceImpl(originalType, sup, interfaceDeclRef, outWitness, &breadcrumb))
                        {
                            return true;
                        }
                    }
                }
            }

            // default is failure
            return false;
        }

        bool DoesTypeConformToInterface(
            RefPtr<Type>  type,
            DeclRef<InterfaceDecl>        interfaceDeclRef)
        {
            return doesTypeConformToInterfaceImpl(type, type, interfaceDeclRef, nullptr, nullptr);
        }

        RefPtr<Val> tryGetInterfaceConformanceWitness(
            RefPtr<Type>  type,
            DeclRef<InterfaceDecl>        interfaceDeclRef)
        {
            RefPtr<Val> result;
            doesTypeConformToInterfaceImpl(type, type, interfaceDeclRef, &result, nullptr);
            return result;
        }

        RefPtr<Type> TryJoinTypeWithInterface(
            RefPtr<Type>  type,
            DeclRef<InterfaceDecl>        interfaceDeclRef)
        {
            // The most basic test here should be: does the type declare conformance to the trait.
            if(DoesTypeConformToInterface(type, interfaceDeclRef))
                return type;

            // There is a more nuanced case if `type` is a builtin type, and we need to make it
            // conform to a trait that some but not all builtin types support (the main problem
            // here is when an operation wants an integer type, but one of our operands is a `float`.
            // The HLSL rules will allow that, with implicit conversion, but our default join rules
            // will end up picking `float` and we don't want that...).

            // For now we don't handle the hard case and just bail
            return nullptr;
        }

        // Try to compute the "join" between two types
        RefPtr<Type> TryJoinTypes(
            RefPtr<Type>  left,
            RefPtr<Type>  right)
        {
            // Easy case: they are the same type!
            if (left->Equals(right))
                return left;

            // We can join two basic types by picking the "better" of the two
            if (auto leftBasic = left->As<BasicExpressionType>())
            {
                if (auto rightBasic = right->As<BasicExpressionType>())
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
                        SLANG_ASSERT(rightFlavor > leftFlavor);
                        return right;
                    }
                }

                // We can also join a vector and a scalar
                if(auto rightVector = right->As<VectorExpressionType>())
                {
                    return TryJoinVectorAndScalarType(rightVector, leftBasic);
                }
            }

            // We can join two vector types by joining their element types
            // (and also their sizes...)
            if( auto leftVector = left->As<VectorExpressionType>())
            {
                if(auto rightVector = right->As<VectorExpressionType>())
                {
                    // Check if the vector sizes match
                    if(!leftVector->elementCount->EqualsVal(rightVector->elementCount.Ptr()))
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
                if(auto rightBasic = right->As<BasicExpressionType>())
                {
                    return TryJoinVectorAndScalarType(leftVector, rightBasic);
                }
            }

            // HACK: trying to work trait types in here...
            if(auto leftDeclRefType = left->As<DeclRefType>())
            {
                if( auto leftInterfaceRef = leftDeclRefType->declRef.As<InterfaceDecl>() )
                {
                    // 
                    return TryJoinTypeWithInterface(right, leftInterfaceRef);
                }
            }
            if(auto rightDeclRefType = right->As<DeclRefType>())
            {
                if( auto rightInterfaceRef = rightDeclRefType->declRef.As<InterfaceDecl>() )
                {
                    // 
                    return TryJoinTypeWithInterface(left, rightInterfaceRef);
                }
            }

            // TODO: all the cases for vectors apply to matrices too!

            // Default case is that we just fail.
            return nullptr;
        }

        // Try to solve a system of generic constraints.
        // The `system` argument provides the constraints.
        // The `varSubst` argument provides the list of constraint
        // variables that were created for the system.
        //
        // Returns a new substitution representing the values that
        // we solved for along the way.
        SubstitutionSet TrySolveConstraintSystem(
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
                if(!TryUnifyTypes(*system, GetSub(constraintDeclRef), GetSup(constraintDeclRef)))
                    return SubstitutionSet();
            }
            SubstitutionSet resultSubst = genericDeclRef.substitutions;
            // We will loop over the generic parameters, and for
            // each we will try to find a way to satisfy all
            // the constraints for that parameter
            List<RefPtr<Val>> args;
            for (auto m : getMembers(genericDeclRef))
            {
                if (auto typeParam = m.As<GenericTypeParamDecl>())
                {
                    RefPtr<Type> type = nullptr;
                    for (auto& c : system->constraints)
                    {
                        if (c.decl != typeParam.getDecl())
                            continue;

                        auto cType = c.val.As<Type>();
                        SLANG_RELEASE_ASSERT(cType.Ptr());

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
                    args.Add(type);
                }
                else if (auto valParam = m.As<GenericValueParamDecl>())
                {
                    // TODO(tfoley): maybe support more than integers some day?
                    // TODO(tfoley): figure out how this needs to interact with
                    // compile-time integers that aren't just constants...
                    RefPtr<IntVal> val = nullptr;
                    for (auto& c : system->constraints)
                    {
                        if (c.decl != valParam.getDecl())
                            continue;

                        auto cVal = c.val.As<IntVal>();
                        SLANG_RELEASE_ASSERT(cVal.Ptr());

                        if (!val)
                        {
                            val = cVal;
                        }
                        else
                        {
                            if(!val->EqualsVal(cVal.Ptr()))
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
                    args.Add(val);
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

            RefPtr<GenericSubstitution> solvedSubst = new GenericSubstitution();
            solvedSubst->genericDecl = genericDeclRef.getDecl();
            solvedSubst->outer = genericDeclRef.substitutions.genericSubstitutions;
            solvedSubst->args = args;
            resultSubst.genericSubstitutions = solvedSubst;

            for( auto constraintDecl : genericDeclRef.getDecl()->getMembersOfType<GenericTypeConstraintDecl>() )
            {
                DeclRef<GenericTypeConstraintDecl> constraintDeclRef(
                    constraintDecl,
                    solvedSubst);

                // Extract the (substituted) sub- and super-type from the constraint.
                auto sub = GetSub(constraintDeclRef);
                auto sup = GetSup(constraintDeclRef);

                // Search for a witness that shows the constraint is satisfied.
                auto subTypeWitness = tryGetSubtypeWitness(sub, sup);
                if(subTypeWitness)
                {
                    // We found a witness, so it will become an (implicit) argument.
                    solvedSubst->args.Add(subTypeWitness);
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

        //

        struct OverloadCandidate
        {
            enum class Flavor
            {
                Func,
                Generic,
                UnspecializedGeneric,
            };
            Flavor flavor;

            enum class Status
            {
                GenericArgumentInferenceFailed,
                Unchecked,
                ArityChecked,
                FixityChecked,
                TypeChecked,
                DirectionChecked,
                Appicable,
            };
            Status status = Status::Unchecked;

            // Reference to the declaration being applied
            LookupResultItem item;

            // The type of the result expression if this candidate is selected
            RefPtr<Type>	resultType;

            // A system for tracking constraints introduced on generic parameters
//            ConstraintSystem constraintSystem;

            // How much conversion cost should be considered for this overload,
            // when ranking candidates.
            ConversionCost conversionCostSum = kConversionCost_None;

            // When required, a candidate can store a pre-checked list of
            // arguments so that we don't have to repeat work across checking
            // phases. Currently this is only needed for generics.
            RefPtr<Substitutions>   subst;
        };



        // State related to overload resolution for a call
        // to an overloaded symbol
        struct OverloadResolveContext
        {
            enum class Mode
            {
                // We are just checking if a candidate works or not
                JustTrying,

                // We want to actually update the AST for a chosen candidate
                ForReal,
            };

            // Location to use when reporting overload-resolution errors.
            SourceLoc loc;

            // The original expression (if any) that triggered things
            RefPtr<Expr> originalExpr;

            // Source location of the "function" part of the expression, if any
            SourceLoc       funcLoc;

            // The original arguments to the call
            UInt argCount = 0;
            RefPtr<Expr>* args = nullptr;
            RefPtr<Type>* argTypes = nullptr;

            UInt getArgCount() { return argCount; }
            RefPtr<Expr>& getArg(UInt index) { return args[index]; }
            RefPtr<Type>& getArgType(UInt index)
            {
                if(argTypes)
                    return argTypes[index];
                else
                    return getArg(index)->type.type;
            }

            bool disallowNestedConversions = false;

            RefPtr<Expr> baseExpr;

            // Are we still trying out candidates, or are we
            // checking the chosen one for real?
            Mode mode = Mode::JustTrying;

            // We store one candidate directly, so that we don't
            // need to do dynamic allocation on the list every time
            OverloadCandidate bestCandidateStorage;
            OverloadCandidate*	bestCandidate = nullptr;

            // Full list of all candidates being considered, in the ambiguous case
            List<OverloadCandidate> bestCandidates;
        };

        struct ParamCounts
        {
            UInt required;
            UInt allowed;
        };

        // count the number of parameters required/allowed for a callable
        ParamCounts CountParameters(FilteredMemberRefList<ParamDecl> params)
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

        // count the number of parameters required/allowed for a generic
        ParamCounts CountParameters(DeclRef<GenericDecl> genericRef)
        {
            ParamCounts counts = { 0, 0 };
            for (auto m : genericRef.getDecl()->Members)
            {
                if (auto typeParam = m.As<GenericTypeParamDecl>())
                {
                    counts.allowed++;
                    if (!typeParam->initType.Ptr())
                    {
                        counts.required++;
                    }
                }
                else if (auto valParam = m.As<GenericValueParamDecl>())
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

        bool TryCheckOverloadCandidateArity(
            OverloadResolveContext&		context,
            OverloadCandidate const&	candidate)
        {
            UInt argCount = context.getArgCount();
            ParamCounts paramCounts = { 0, 0 };
            switch (candidate.flavor)
            {
            case OverloadCandidate::Flavor::Func:
                paramCounts = CountParameters(GetParameters(candidate.item.declRef.As<CallableDecl>()));
                break;

            case OverloadCandidate::Flavor::Generic:
                paramCounts = CountParameters(candidate.item.declRef.As<GenericDecl>());
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

        bool TryCheckOverloadCandidateFixity(
            OverloadResolveContext&		context,
            OverloadCandidate const&	candidate)
        {
            auto expr = context.originalExpr;

            auto decl = candidate.item.declRef.decl;

            if(auto prefixExpr = expr.As<PrefixExpr>())
            {
                if(decl->HasModifier<PrefixModifier>())
                    return true;

                if (context.mode != OverloadResolveContext::Mode::JustTrying)
                {
                    getSink()->diagnose(context.loc, Diagnostics::expectedPrefixOperator);
                    getSink()->diagnose(decl, Diagnostics::seeDefinitionOf, decl->getName());
                }

                return false;
            }
            else if(auto postfixExpr = expr.As<PostfixExpr>())
            {
                if(decl->HasModifier<PostfixModifier>())
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

            return false;
        }

        bool TryCheckGenericOverloadCandidateTypes(
            OverloadResolveContext&	context,
            OverloadCandidate&		candidate)
        {
            auto genericDeclRef = candidate.item.declRef.As<GenericDecl>();

            // We will go ahead and hang onto the arguments that we've
            // already checked, since downstream validation might need
            // them.
            auto genSubst = new GenericSubstitution();
            candidate.subst = genSubst;
            auto& checkedArgs = genSubst->args;

            int aa = 0;
            for (auto memberRef : getMembers(genericDeclRef))
            {
                if (auto typeParamRef = memberRef.As<GenericTypeParamDecl>())
                {
                    auto arg = context.getArg(aa++);

                    TypeExp typeExp;
                    if (context.mode == OverloadResolveContext::Mode::JustTrying)
                    {
                        typeExp = tryCoerceToProperType(TypeExp(arg));
                        if(!typeExp.type)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        typeExp = CoerceToProperType(TypeExp(arg));
                    }
                    checkedArgs.Add(typeExp.type);
                }
                else if (auto valParamRef = memberRef.As<GenericValueParamDecl>())
                {
                    auto arg = context.getArg(aa++);

                    if (context.mode == OverloadResolveContext::Mode::JustTrying)
                    {
                        ConversionCost cost = kConversionCost_None;
                        if (!CanCoerce(GetType(valParamRef), arg->type, &cost))
                        {
                            return false;
                        }
                        candidate.conversionCostSum += cost;
                    }

                    arg = Coerce(GetType(valParamRef), arg);
                    auto val = ExtractGenericArgInteger(arg);
                    checkedArgs.Add(val);
                }
                else
                {
                    continue;
                }
            }

            // Okay, we've made it!
            return true;
        }

        bool TryCheckOverloadCandidateTypes(
            OverloadResolveContext&	context,
            OverloadCandidate&		candidate)
        {
            UInt argCount = context.getArgCount();

            List<DeclRef<ParamDecl>> params;
            switch (candidate.flavor)
            {
            case OverloadCandidate::Flavor::Func:
                params = GetParameters(candidate.item.declRef.As<CallableDecl>()).ToArray();
                break;

            case OverloadCandidate::Flavor::Generic:
                return TryCheckGenericOverloadCandidateTypes(context, candidate);

            default:
                SLANG_UNEXPECTED("unknown flavor of overload candidate");
                break;
            }

            // Note(tfoley): We might have fewer arguments than parameters in the
            // case where one or more parameters had defaults.
            SLANG_RELEASE_ASSERT(argCount <= params.Count());

            for (UInt ii = 0; ii < argCount; ++ii)
            {
                auto& arg = context.getArg(ii);
                auto argType = context.getArgType(ii);
                auto param = params[ii];

                if (context.mode == OverloadResolveContext::Mode::JustTrying)
                {
                    ConversionCost cost = kConversionCost_None;
                    if( context.disallowNestedConversions )
                    {
                        // We need an exact match in this case.
                        if(!GetType(param)->Equals(argType))
                            return false;
                    }
                    else if (!CanCoerce(GetType(param), argType, &cost))
                    {
                        return false;
                    }
                    candidate.conversionCostSum += cost;
                }
                else
                {
                    arg = Coerce(GetType(param), arg);
                }
            }
            return true;
        }

        bool TryCheckOverloadCandidateDirections(
            OverloadResolveContext&		/*context*/,
            OverloadCandidate const&	/*candidate*/)
        {
            // TODO(tfoley): check `in` and `out` markers, as needed.
            return true;
        }

        // Create a witness that attests to the fact that `type`
        // is equal to itself.
        RefPtr<Val> createTypeEqualityWitness(
            Type*  type)
        {
            RefPtr<TypeEqualityWitness> rs = new TypeEqualityWitness();
            rs->sub = type;
            rs->sup = type;
            return rs;
        }

        // If `sub` is a subtype of `sup`, then return a value that
        // can serve as a "witness" for that fact.
        RefPtr<Val> tryGetSubtypeWitness(
            RefPtr<Type>    sub,
            RefPtr<Type>    sup)
        {
            if(sub->Equals(sup))
            {
                // They are the same type, so we just need a witness
                // for type equality.
                return createTypeEqualityWitness(sub);
            }

            if(auto supDeclRefType = sup->As<DeclRefType>())
            {
                auto supDeclRef = supDeclRefType->declRef;
                if(auto supInterfaceDeclRef = supDeclRef.As<InterfaceDecl>())
                {
                    if(auto witness = tryGetInterfaceConformanceWitness(sub, supInterfaceDeclRef))
                    {
                        return witness;
                    }
                }
            }

            return nullptr;
        }

        // In the case where we are explicitly applying a generic
        // to arguments (e.g., `G<A,B>`) check that the constraints
        // on those parameters are satisfied.
        //
        // Note: the constraints actually work as additional parameters/arguments
        // of the generic, and so we need to reify them into the final
        // argument list.
        //
        bool TryCheckOverloadCandidateConstraints(
            OverloadResolveContext&		context,
            OverloadCandidate const&	candidate)
        {
            // We only need this step for generics, so always succeed on
            // everything else.
            if(candidate.flavor != OverloadCandidate::Flavor::Generic)
                return true;

            auto genericDeclRef = candidate.item.declRef.As<GenericDecl>();
            assert(genericDeclRef);

            // We should have the existing arguments to the generic
            // handy, so that we can construct a substitution list.

            RefPtr<GenericSubstitution> subst = candidate.subst.As<GenericSubstitution>();
            assert(subst);

            subst->genericDecl = genericDeclRef.getDecl();
            subst->outer = genericDeclRef.substitutions.genericSubstitutions;

            for( auto constraintDecl : genericDeclRef.getDecl()->getMembersOfType<GenericTypeConstraintDecl>() )
            {
                auto subset = genericDeclRef.substitutions;
                subset.genericSubstitutions = subst;
                DeclRef<GenericTypeConstraintDecl> constraintDeclRef(
                    constraintDecl, subset);

                auto sub = GetSub(constraintDeclRef);
                auto sup = GetSup(constraintDeclRef);

                auto subTypeWitness = tryGetSubtypeWitness(sub, sup);
                if(subTypeWitness)
                {
                    subst->args.Add(subTypeWitness);
                }
                else
                {
                    if(context.mode != OverloadResolveContext::Mode::JustTrying)
                    {
                        // TODO: diagnose a problem here
                        getSink()->diagnose(context.loc, Diagnostics::unimplemented, "generic constraint not satisfied");
                    }
                    return false;
                }
            }

            // Done checking all the constraints, hooray.
            return true;
        }

        // Try to check an overload candidate, but bail out
        // if any step fails
        void TryCheckOverloadCandidate(
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

            candidate.status = OverloadCandidate::Status::Appicable;
        }

        // Create the representation of a given generic applied to some arguments
        RefPtr<Expr> createGenericDeclRef(
            RefPtr<Expr>            baseExpr,
            RefPtr<Expr>            originalExpr,
            RefPtr<GenericSubstitution>   subst)
        {
            auto baseDeclRefExpr = baseExpr.As<DeclRefExpr>();
            if (!baseDeclRefExpr)
            {
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), baseExpr, "expected a reference to a generic declaration");
                return CreateErrorExpr(originalExpr);
            }
            auto baseGenericRef = baseDeclRefExpr->declRef.As<GenericDecl>();
            if (!baseGenericRef)
            {
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), baseExpr, "expected a reference to a generic declaration");
                return CreateErrorExpr(originalExpr);
            }

            subst->genericDecl = baseGenericRef.getDecl();
            subst->outer = baseGenericRef.substitutions.genericSubstitutions;

            DeclRef<Decl> innerDeclRef(GetInner(baseGenericRef), subst);

            RefPtr<Expr> base;
            if (auto mbrExpr = baseExpr.As<MemberExpr>())
                base = mbrExpr->BaseExpression;

            return ConstructDeclRefExpr(
                innerDeclRef,
                base,
                originalExpr->loc);
        }

        // Take an overload candidate that previously got through
        // `TryCheckOverloadCandidate` above, and try to finish
        // up the work and turn it into a real expression.
        //
        // If the candidate isn't actually applicable, this is
        // where we'd start reporting the issue(s).
        RefPtr<Expr> CompleteOverloadCandidate(
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

                String declString = getDeclSignatureString(candidate.item);
                getSink()->diagnose(candidate.item.declRef, Diagnostics::genericSignatureTried, declString);
                goto error;
            }

            context.mode = OverloadResolveContext::Mode::ForReal;

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
                auto baseExpr = ConstructLookupResultExpr(
                    candidate.item, context.baseExpr, context.funcLoc);

                switch(candidate.flavor)
                {
                case OverloadCandidate::Flavor::Func:
                    {
                        RefPtr<AppExprBase> callExpr = context.originalExpr.As<InvokeExpr>();
                        if(!callExpr)
                        {
                            callExpr = new InvokeExpr();
                            callExpr->loc = context.loc;

                            for(UInt aa = 0; aa < context.argCount; ++aa)
                                callExpr->Arguments.Add(context.getArg(aa));
                        }


                        callExpr->FunctionExpr = baseExpr;
                        callExpr->type = QualType(candidate.resultType);

                        // A call may yield an l-value, and we should take a look at the candidate to be sure
                        if(auto subscriptDeclRef = candidate.item.declRef.As<SubscriptDecl>())
                        {
                            for(auto setter : subscriptDeclRef.getDecl()->getMembersOfType<SetterDecl>())
                            {
                                callExpr->type.IsLeftValue = true;
                            }
                            for(auto refAccessor : subscriptDeclRef.getDecl()->getMembersOfType<RefAccessorDecl>())
                            {
                                callExpr->type.IsLeftValue = true;
                            }
                        }

                        // TODO: there may be other cases that confer l-value-ness

                        return callExpr;
                    }

                    break;

                case OverloadCandidate::Flavor::Generic:
                    return createGenericDeclRef(
                        baseExpr,
                        context.originalExpr,
                        candidate.subst.As<GenericSubstitution>());
                    break;

                default:
                    SLANG_DIAGNOSE_UNEXPECTED(getSink(), context.loc, "unknown overload candidate flavor");
                    break;
                }
            }


        error:

            if(context.originalExpr)
            {
                return CreateErrorExpr(context.originalExpr.Ptr());
            }
            else
            {
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), context.loc, "no original expression for overload result");
                return nullptr;
            }
        }

        // Implement a comparison operation between overload candidates,
        // so that the better candidate compares as less-than the other
        int CompareOverloadCandidates(
            OverloadCandidate*	left,
            OverloadCandidate*	right)
        {
            // If one candidate got further along in validation, pick it
            if (left->status != right->status)
                return int(right->status) - int(left->status);

            // If both candidates are applicable, then we need to compare
            // the costs of their type conversion sequences
            if(left->status == OverloadCandidate::Status::Appicable)
            {
                if (left->conversionCostSum != right->conversionCostSum)
                    return left->conversionCostSum - right->conversionCostSum;
            }

            return 0;
        }

        void AddOverloadCandidateInner(
            OverloadResolveContext& context,
            OverloadCandidate&		candidate)
        {
            // Filter our existing candidates, to remove any that are worse than our new one

            bool keepThisCandidate = true; // should this candidate be kept?

            if (context.bestCandidates.Count() != 0)
            {
                // We have multiple candidates right now, so filter them.
                bool anyFiltered = false;
                // Note that we are querying the list length on every iteration,
                // because we might remove things.
                for (UInt cc = 0; cc < context.bestCandidates.Count(); ++cc)
                {
                    int cmp = CompareOverloadCandidates(&candidate, &context.bestCandidates[cc]);
                    if (cmp < 0)
                    {
                        // our new candidate is better!

                        // remove it from the list (by swapping in a later one)
                        context.bestCandidates.FastRemoveAt(cc);
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
            if (context.bestCandidates.Count() > 0)
            {
                // There were already multiple candidates, and we are adding one more
                context.bestCandidates.Add(candidate);
            }
            else if (context.bestCandidate)
            {
                // There was a unique best candidate, but now we are ambiguous
                context.bestCandidates.Add(*context.bestCandidate);
                context.bestCandidates.Add(candidate);
                context.bestCandidate = nullptr;
            }
            else
            {
                // This is the only candidate worthe keeping track of right now
                context.bestCandidateStorage = candidate;
                context.bestCandidate = &context.bestCandidateStorage;
            }
        }

        void AddOverloadCandidate(
            OverloadResolveContext& context,
            OverloadCandidate&		candidate)
        {
            // Try the candidate out, to see if it is applicable at all.
            TryCheckOverloadCandidate(context, candidate);

            // Now (potentially) add it to the set of candidate overloads to consider.
            AddOverloadCandidateInner(context, candidate);
        }

        void AddFuncOverloadCandidate(
            LookupResultItem			item,
            DeclRef<CallableDecl>             funcDeclRef,
            OverloadResolveContext&		context)
        {
            auto funcDecl = funcDeclRef.getDecl();
            checkDecl(funcDecl);

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
            candidate.resultType = GetResultType(funcDeclRef);

            AddOverloadCandidate(context, candidate);
        }

        void AddFuncOverloadCandidate(
            RefPtr<FuncType>		/*funcType*/,
            OverloadResolveContext&	/*context*/)
        {
#if 0
            if (funcType->decl)
            {
                AddFuncOverloadCandidate(funcType->decl, context);
            }
            else if (funcType->Func)
            {
                AddFuncOverloadCandidate(funcType->Func->SyntaxNode, context);
            }
            else if (funcType->Component)
            {
                AddComponentFuncOverloadCandidate(funcType->Component, context);
            }
#else
            throw "unimplemented";
#endif
        }

        // Add a candidate callee for overload resolution, based on
        // calling a particular `ConstructorDecl`.
        void AddCtorOverloadCandidate(
            LookupResultItem            typeItem,
            RefPtr<Type>                type,
            DeclRef<ConstructorDecl>    ctorDeclRef,
            OverloadResolveContext&     context,
            RefPtr<Type>                resultType)
        {
            checkDecl(ctorDeclRef.getDecl());

            // `typeItem` refers to the type being constructed (the thing
            // that was applied as a function) so we need to construct
            // a `LookupResultItem` that refers to the constructor instead

            LookupResultItem ctorItem;
            ctorItem.declRef = ctorDeclRef;
            ctorItem.breadcrumbs = new LookupResultItem::Breadcrumb(LookupResultItem::Breadcrumb::Kind::Member, typeItem.declRef, typeItem.breadcrumbs);

            OverloadCandidate candidate;
            candidate.flavor = OverloadCandidate::Flavor::Func;
            candidate.item = ctorItem;
            candidate.resultType = resultType;

            AddOverloadCandidate(context, candidate);
        }

        // If the given declaration has generic parameters, then
        // return the corresponding `GenericDecl` that holds the
        // parameters, etc.
        GenericDecl* GetOuterGeneric(Decl* decl)
        {
            auto parentDecl = decl->ParentDecl;
            if (!parentDecl) return nullptr;
            auto parentGeneric = dynamic_cast<GenericDecl*>(parentDecl);
            return parentGeneric;
        }

        // Try to find a unification for two values
        bool TryUnifyVals(
            ConstraintSystem&	constraints,
            RefPtr<Val>			fst,
            RefPtr<Val>			snd)
        {
            // if both values are types, then unify types
            if (auto fstType = fst.As<Type>())
            {
                if (auto sndType = snd.As<Type>())
                {
                    return TryUnifyTypes(constraints, fstType, sndType);
                }
            }

            // if both values are constant integers, then compare them
            if (auto fstIntVal = fst.As<ConstantIntVal>())
            {
                if (auto sndIntVal = snd.As<ConstantIntVal>())
                {
                    return fstIntVal->value == sndIntVal->value;
                }
            }

            // Check if both are integer values in general
            if (auto fstInt = fst.As<IntVal>())
            {
                if (auto sndInt = snd.As<IntVal>())
                {
                    auto fstParam = fstInt.As<GenericParamIntVal>();
                    auto sndParam = sndInt.As<GenericParamIntVal>();

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

            if (auto fstWit = fst.As<DeclaredSubtypeWitness>())
            {
                if (auto sndWit = snd.As<DeclaredSubtypeWitness>())
                {
                    auto constraintDecl1 = fstWit->declRef.As<TypeConstraintDecl>();
                    auto constraintDecl2 = sndWit->declRef.As<TypeConstraintDecl>();
                    assert(constraintDecl1);
                    assert(constraintDecl2);
                    return TryUnifyTypes(constraints, 
                        constraintDecl1.getDecl()->getSup().type,
                        constraintDecl2.getDecl()->getSup().type);
                }
            }

            throw "unimplemented";

            // default: fail
            return false;
        }
        
        bool TryUnifySubstitutions(
            ConstraintSystem&		constraints,
            RefPtr<GenericSubstitution>	fst,
            RefPtr<GenericSubstitution>	snd)
        {
            // They must both be NULL or non-NULL
            if (!fst || !snd)
                return fst == snd;
            auto fstGen = fst;
            auto sndGen = snd;
            // They must be specializing the same generic
            if (fstGen->genericDecl != sndGen->genericDecl)
                return false;

            // Their arguments must unify
            SLANG_RELEASE_ASSERT(fstGen->args.Count() == sndGen->args.Count());
            UInt argCount = fstGen->args.Count();
            for (UInt aa = 0; aa < argCount; ++aa)
            {
                if (!TryUnifyVals(constraints, fstGen->args[aa], sndGen->args[aa]))
                    return false;
            }

            // Their "base" specializations must unify
            if (!TryUnifySubstitutions(constraints, fstGen->outer, sndGen->outer))
                return false;

            return true;
        }

        bool TryUnifyTypeParam(
            ConstraintSystem&				constraints,
            RefPtr<GenericTypeParamDecl>	typeParamDecl,
            RefPtr<Type>			type)
        {
            // We want to constrain the given type parameter
            // to equal the given type.
            Constraint constraint;
            constraint.decl = typeParamDecl.Ptr();
            constraint.val = type;

            constraints.constraints.Add(constraint);

            return true;
        }

        bool TryUnifyIntParam(
            ConstraintSystem&               constraints,
            RefPtr<GenericValueParamDecl>	paramDecl,
            RefPtr<IntVal>                  val)
        {
            // We only want to accumulate constraints on
            // the parameters of the declarations being
            // specialized (don't accidentially constrain
            // parameters of a generic function based on
            // calls in its body).
            if(paramDecl->ParentDecl != constraints.genericDecl)
                return false;

            // We want to constrain the given parameter to equal the given value.
            Constraint constraint;
            constraint.decl = paramDecl.Ptr();
            constraint.val = val;

            constraints.constraints.Add(constraint);

            return true;
        }

        bool TryUnifyIntParam(
            ConstraintSystem&       constraints,
            DeclRef<VarDeclBase> const&   varRef,
            RefPtr<IntVal>          val)
        {
            if(auto genericValueParamRef = varRef.As<GenericValueParamDecl>())
            {
                return TryUnifyIntParam(constraints, RefPtr<GenericValueParamDecl>(genericValueParamRef.getDecl()), val);
            }
            else
            {
                return false;
            }
        }

        bool TryUnifyTypesByStructuralMatch(
            ConstraintSystem&       constraints,
            RefPtr<Type>  fst,
            RefPtr<Type>  snd)
        {
            if (auto fstDeclRefType = fst->As<DeclRefType>())
            {
                auto fstDeclRef = fstDeclRefType->declRef;

                if (auto typeParamDecl = dynamic_cast<GenericTypeParamDecl*>(fstDeclRef.getDecl()))
                    return TryUnifyTypeParam(constraints, typeParamDecl, snd);

                if (auto sndDeclRefType = snd->As<DeclRefType>())
                {
                    auto sndDeclRef = sndDeclRefType->declRef;

                    if (auto typeParamDecl = dynamic_cast<GenericTypeParamDecl*>(sndDeclRef.getDecl()))
                        return TryUnifyTypeParam(constraints, typeParamDecl, fst);

                    // can't be unified if they refer to differnt declarations.
                    if (fstDeclRef.getDecl() != sndDeclRef.getDecl()) return false;

                    // next we need to unify the substitutions applied
                    // to each decalration reference.
                    if (!TryUnifySubstitutions(
                        constraints,
                        fstDeclRef.substitutions.genericSubstitutions,
                        sndDeclRef.substitutions.genericSubstitutions))
                    {
                        return false;
                    }

                    return true;
                }
            }

            return false;
        }

        bool TryUnifyTypes(
            ConstraintSystem&       constraints,
            RefPtr<Type>  fst,
            RefPtr<Type>  snd)
        {
            if (fst->Equals(snd)) return true;

            // An error type can unify with anything, just so we avoid cascading errors.

            if (auto fstErrorType = fst->As<ErrorType>())
                return true;

            if (auto sndErrorType = snd->As<ErrorType>())
                return true;

            // A generic parameter type can unify with anything.
            // TODO: there actually needs to be some kind of "occurs check" sort
            // of thing here...

            if (auto fstDeclRefType = fst->As<DeclRefType>())
            {
                auto fstDeclRef = fstDeclRefType->declRef;

                if (auto typeParamDecl = dynamic_cast<GenericTypeParamDecl*>(fstDeclRef.getDecl()))
                {
                    if(typeParamDecl->ParentDecl == constraints.genericDecl )
                        return TryUnifyTypeParam(constraints, typeParamDecl, snd);
                }
            }

            if (auto sndDeclRefType = snd->As<DeclRefType>())
            {
                auto sndDeclRef = sndDeclRefType->declRef;

                if (auto typeParamDecl = dynamic_cast<GenericTypeParamDecl*>(sndDeclRef.getDecl()))
                {
                    if(typeParamDecl->ParentDecl == constraints.genericDecl )
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

            if(auto fstVectorType = fst->As<VectorExpressionType>())
            {
                if(auto sndScalarType = snd->As<BasicExpressionType>())
                {
                    return TryUnifyTypes(
                        constraints,
                        fstVectorType->elementType,
                        sndScalarType);
                }
            }

            if(auto fstScalarType = fst->As<BasicExpressionType>())
            {
                if(auto sndVectorType = snd->As<VectorExpressionType>())
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

        // Is the candidate extension declaration actually applicable to the given type
        DeclRef<ExtensionDecl> ApplyExtensionToType(
            ExtensionDecl*			extDecl,
            RefPtr<Type>	type)
        {
            if (auto extGenericDecl = GetOuterGeneric(extDecl))
            {
                ConstraintSystem constraints;
                constraints.genericDecl = extGenericDecl;

                if (!TryUnifyTypes(constraints, extDecl->targetType.Ptr(), type))
                    return DeclRef<Decl>().As<ExtensionDecl>();

                auto constraintSubst = TrySolveConstraintSystem(&constraints, DeclRef<Decl>(extGenericDecl, nullptr).As<GenericDecl>());
                if (!constraintSubst)
                {
                    return DeclRef<Decl>().As<ExtensionDecl>();
                }

                // Consruct a reference to the extension with our constraint variables
                // set as they were found by solving the constraint system.
                DeclRef<ExtensionDecl> extDeclRef = DeclRef<Decl>(extDecl, constraintSubst).As<ExtensionDecl>();

                // We expect/require that the result of unification is such that
                // the target types are now equal
                SLANG_ASSERT(GetTargetType(extDeclRef)->Equals(type));

                return extDeclRef;
            }
            else
            {
                // The easy case is when the extension isn't generic:
                // either it applies to the type or not.
                if (!type->Equals(extDecl->targetType))
                    return DeclRef<Decl>().As<ExtensionDecl>();
                return DeclRef<Decl>(extDecl, nullptr).As<ExtensionDecl>();
            }
        }

#if 0
        bool TryUnifyArgAndParamTypes(
            ConstraintSystem&				system,
            RefPtr<Expr>	argExpr,
            DeclRef<ParamDecl>					paramDeclRef)
        {
            // TODO(tfoley): potentially need a bit more
            // nuance in case where argument might be
            // an overload group...
            return TryUnifyTypes(system, argExpr->type, GetType(paramDeclRef));
        }
#endif

        // Take a generic declaration and try to specialize its parameters
        // so that the resulting inner declaration can be applicable in
        // a particular context...
        DeclRef<Decl> SpecializeGenericForOverload(
            DeclRef<GenericDecl>			genericDeclRef,
            OverloadResolveContext&	context)
        {
            checkDecl(genericDeclRef.getDecl());

            ConstraintSystem constraints;
            constraints.genericDecl = genericDeclRef.getDecl();

            // Construct a reference to the inner declaration that has any generic
            // parameter substitutions in place already, but *not* any substutions
            // for the generic declaration we are currently trying to infer.
            auto innerDecl = GetInner(genericDeclRef);
            DeclRef<Decl> unspecializedInnerRef = DeclRef<Decl>(innerDecl, genericDeclRef.substitutions);

            // Check what type of declaration we are dealing with, and then try
            // to match it up with the arguments accordingly...
            if (auto funcDeclRef = unspecializedInnerRef.As<CallableDecl>())
            {
                auto params = GetParameters(funcDeclRef).ToArray();

                UInt argCount = context.getArgCount();
                UInt paramCount = params.Count();

                // Bail out on mismatch.
                // TODO(tfoley): need more nuance here
                if (argCount != paramCount)
                {
                    return DeclRef<Decl>(nullptr, nullptr);
                }

                for (UInt aa = 0; aa < argCount; ++aa)
                {
#if 0
                    if (!TryUnifyArgAndParamTypes(constraints, args[aa], params[aa]))
                        return DeclRef<Decl>(nullptr, nullptr);
#else
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

                    TryUnifyTypes(constraints, context.getArgType(aa), GetType(params[aa]));
#endif
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

        void AddAggTypeOverloadCandidates(
            LookupResultItem        typeItem,
            RefPtr<Type>            type,
            DeclRef<AggTypeDecl>    aggTypeDeclRef,
            OverloadResolveContext& context,
            RefPtr<Type>            resultType)
        {
            for (auto ctorDeclRef : getMembersOfType<ConstructorDecl>(aggTypeDeclRef))
            {
                // now work through this candidate...
                AddCtorOverloadCandidate(typeItem, type, ctorDeclRef, context, resultType);
            }

            // Now walk through any extensions we can find for this types
            for (auto ext = GetCandidateExtensions(aggTypeDeclRef); ext; ext = ext->nextCandidateExtension)
            {
                auto extDeclRef = ApplyExtensionToType(ext, type);
                if (!extDeclRef)
                    continue;

                for (auto ctorDeclRef : getMembersOfType<ConstructorDecl>(extDeclRef))
                {
                    // TODO(tfoley): `typeItem` here should really reference the extension...

                    // now work through this candidate...
                    AddCtorOverloadCandidate(typeItem, type, ctorDeclRef, context, resultType);
                }

                // Also check for generic constructors
                for (auto genericDeclRef : getMembersOfType<GenericDecl>(extDeclRef))
                {
                    if (auto ctorDecl = genericDeclRef.getDecl()->inner.As<ConstructorDecl>())
                    {
                        DeclRef<Decl> innerRef = SpecializeGenericForOverload(genericDeclRef, context);
                        if (!innerRef)
                            continue;

                        DeclRef<ConstructorDecl> innerCtorRef = innerRef.As<ConstructorDecl>();

                        AddCtorOverloadCandidate(typeItem, type, innerCtorRef, context, resultType);

                        // TODO(tfoley): need a way to do the solving step for the constraint system
                    }
                }
            }
        }

        void addGenericTypeParamOverloadCandidates(
            DeclRef<GenericTypeParamDecl>   typeDeclRef,
            OverloadResolveContext&         context,
            RefPtr<Type>                    resultType)
        {
            // We need to look for any constraints placed on the generic
            // type parameter, since they will give us information on
            // interfaces that the type must conform to.

            // We expect the parent of the generic type parameter to be a generic...
            auto genericDeclRef = typeDeclRef.GetParent().As<GenericDecl>();
            assert(genericDeclRef);

            for(auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(genericDeclRef))
            {
                // Does this constraint pertain to the type we are working on?
                //
                // We want constraints of the form `T : Foo` where `T` is the
                // generic parameter in question, and `Foo` is whatever we are
                // constraining it to.
                auto subType = GetSub(constraintDeclRef);
                auto subDeclRefType = subType->As<DeclRefType>();
                if(!subDeclRefType)
                    continue;
                if(!subDeclRefType->declRef.Equals(typeDeclRef))
                    continue;

                // The super-type in the constraint (e.g., `Foo` in `T : Foo`)
                // will tell us a type we should use for lookup.
                auto bound = GetSup(constraintDeclRef);

                // Go ahead and use the target type:
                //
                // TODO: Need to consider case where this might recurse infinitely.
                AddTypeOverloadCandidates(bound, context, resultType);
            }
        }

        void AddTypeOverloadCandidates(
            RefPtr<Type>	        type,
            OverloadResolveContext&	context,
            RefPtr<Type>            resultType)
        {
            if (auto declRefType = type->As<DeclRefType>())
            {
                auto declRef = declRefType->declRef;
                if (auto aggTypeDeclRef = declRef.As<AggTypeDecl>())
                {
                    AddAggTypeOverloadCandidates(LookupResultItem(aggTypeDeclRef), type, aggTypeDeclRef, context, resultType);
                }
                else if(auto genericTypeParamDeclRef = declRef.As<GenericTypeParamDecl>())
                {
                    addGenericTypeParamOverloadCandidates(
                        genericTypeParamDeclRef,
                        context,
                        resultType);
                }
            }
        }

        void AddDeclRefOverloadCandidates(
            LookupResultItem		item,
            OverloadResolveContext&	context)
        {
            auto declRef = item.declRef;

            if (auto funcDeclRef = item.declRef.As<CallableDecl>())
            {
                AddFuncOverloadCandidate(item, funcDeclRef, context);
            }
            else if (auto aggTypeDeclRef = item.declRef.As<AggTypeDecl>())
            {
                auto type = DeclRefType::Create(
                    getSession(),
                    aggTypeDeclRef);
                AddAggTypeOverloadCandidates(item, type, aggTypeDeclRef, context, type);
            }
            else if (auto genericDeclRef = item.declRef.As<GenericDecl>())
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
            else if( auto typeDefDeclRef = item.declRef.As<TypeDefDecl>() )
            {
                auto type = getNamedType(getSession(), typeDefDeclRef);
                AddTypeOverloadCandidates(GetType(typeDefDeclRef), context, type);
            }
            else if( auto genericTypeParamDeclRef = item.declRef.As<GenericTypeParamDecl>() )
            {
                auto type = DeclRefType::Create(
                    getSession(),
                    genericTypeParamDeclRef);
                addGenericTypeParamOverloadCandidates(genericTypeParamDeclRef, context, type);
            }
            else
            {
                // TODO(tfoley): any other cases needed here?
            }
        }

        void AddOverloadCandidates(
            RefPtr<Expr>	funcExpr,
            OverloadResolveContext&			context)
        {
            auto funcExprType = funcExpr->type;

            if (auto declRefExpr = funcExpr.As<DeclRefExpr>())
            {
                // The expression directly referenced a declaration,
                // so we can use that declaration directly to look
                // for anything applicable.
                AddDeclRefOverloadCandidates(LookupResultItem(declRefExpr->declRef), context);
            }
            else if (auto funcType = funcExprType->As<FuncType>())
            {
                // TODO(tfoley): deprecate this path...
                AddFuncOverloadCandidate(funcType, context);
            }
            else if (auto overloadedExpr = funcExpr.As<OverloadedExpr>())
            {
                auto lookupResult = overloadedExpr->lookupResult2;
                SLANG_RELEASE_ASSERT(lookupResult.isOverloaded());
                for(auto item : lookupResult.items)
                {
                    AddDeclRefOverloadCandidates(item, context);
                }
            }
            else if (auto overloadedExpr2 = funcExpr.As<OverloadedExpr2>())
            {
                for (auto item : overloadedExpr2->candidiateExprs)
                {
                    AddOverloadCandidates(item, context);
                }
            }
            else if (auto typeType = funcExprType->As<TypeType>())
            {
                // If none of the above cases matched, but we are
                // looking at a type, then I suppose we have
                // a constructor call on our hands.
                //
                // TODO(tfoley): are there any meaningful types left
                // that aren't declaration references?
                auto type = typeType->type;
                AddTypeOverloadCandidates(type, context, type);
                return;
            }
        }

        void formatType(StringBuilder& sb, RefPtr<Type> type)
        {
            sb << type->ToString();
        }

        void formatVal(StringBuilder& sb, RefPtr<Val> val)
        {
            sb << val->ToString();
        }

        void formatDeclPath(StringBuilder& sb, DeclRef<Decl> declRef)
        {
            // Find the parent declaration
            auto parentDeclRef = declRef.GetParent();

            // If the immediate parent is a generic, then we probably
            // want the declaration above that...
            auto parentGenericDeclRef = parentDeclRef.As<GenericDecl>();
            if(parentGenericDeclRef)
            {
                parentDeclRef = parentGenericDeclRef.GetParent();
            }

            // Depending on what the parent is, we may want to format things specially
            if(auto aggTypeDeclRef = parentDeclRef.As<AggTypeDecl>())
            {
                formatDeclPath(sb, aggTypeDeclRef);
                sb << ".";
            }

            sb << getText(declRef.GetName());

            // If the parent declaration is a generic, then we need to print out its
            // signature
            if( parentGenericDeclRef )
            {
                SLANG_RELEASE_ASSERT(declRef.substitutions);
                auto genSubst = declRef.substitutions.genericSubstitutions;
                SLANG_RELEASE_ASSERT(genSubst->genericDecl == parentGenericDeclRef.getDecl());

                sb << "<";
                bool first = true;
                for(auto arg : genSubst->args)
                {
                    if(!first) sb << ", ";
                    formatVal(sb, arg);
                    first = false;
                }
                sb << ">";
            }
        }

        void formatDeclParams(StringBuilder& sb, DeclRef<Decl> declRef)
        {
            if (auto funcDeclRef = declRef.As<CallableDecl>())
            {

                // This is something callable, so we need to also print parameter types for overloading
                sb << "(";

                bool first = true;
                for (auto paramDeclRef : GetParameters(funcDeclRef))
                {
                    if (!first) sb << ", ";

                    formatType(sb, GetType(paramDeclRef));

                    first = false;

                }

                sb << ")";
            }
            else if(auto genericDeclRef = declRef.As<GenericDecl>())
            {
                sb << "<";
                bool first = true;
                for (auto paramDeclRef : getMembers(genericDeclRef))
                {
                    if(auto genericTypeParam = paramDeclRef.As<GenericTypeParamDecl>())
                    {
                        if (!first) sb << ", ";
                        first = false;

                        sb << getText(genericTypeParam.GetName());
                    }
                    else if(auto genericValParam = paramDeclRef.As<GenericValueParamDecl>())
                    {
                        if (!first) sb << ", ";
                        first = false;

                        formatType(sb, GetType(genericValParam));
                        sb << " ";
                        sb << getText(genericValParam.GetName());
                    }
                    else
                    {}
                }
                sb << ">";

                formatDeclParams(sb, DeclRef<Decl>(GetInner(genericDeclRef), genericDeclRef.substitutions));
            }
            else
            {
            }
        }

        void formatDeclSignature(StringBuilder& sb, DeclRef<Decl> declRef)
        {
            formatDeclPath(sb, declRef);
            formatDeclParams(sb, declRef);
        }

        String getDeclSignatureString(DeclRef<Decl> declRef)
        {
            StringBuilder sb;
            formatDeclSignature(sb, declRef);
            return sb.ProduceString();
        }

        String getDeclSignatureString(LookupResultItem item)
        {
            return getDeclSignatureString(item.declRef);
        }

        String getCallSignatureString(
            OverloadResolveContext&     context)
        {
            StringBuilder argsListBuilder;
            argsListBuilder << "(";

            UInt argCount = context.getArgCount();
            for( UInt aa = 0; aa < argCount; ++aa )
            {
                if(aa != 0) argsListBuilder << ", ";
                argsListBuilder << context.getArgType(aa)->ToString();
            }
            argsListBuilder << ")";
            return argsListBuilder.ProduceString();
        }

#if 0
        String GetCallSignatureString(RefPtr<AppExprBase> expr)
        {
            return getCallSignatureString(expr->Arguments);
        }
#endif

        RefPtr<Expr> ResolveInvoke(InvokeExpr * expr)
        {
            // Look at the base expression for the call, and figure out how to invoke it.
            auto funcExpr = expr->FunctionExpr;
            auto funcExprType = funcExpr->type;

            // If we are trying to apply an erroroneous expression, then just bail out now.
            if(IsErrorExpr(funcExpr))
            {
                return CreateErrorExpr(expr);
            }
            // If any of the arguments is an error, then we should bail out, to avoid
            // cascading errors where we successfully pick an overload, but not the one
            // the user meant.
            for (auto arg : expr->Arguments)
            {
                if (IsErrorExpr(arg))
                    return CreateErrorExpr(expr);
            }

            OverloadResolveContext context;

            context.originalExpr = expr;
            context.funcLoc = funcExpr->loc;

            context.argCount = expr->Arguments.Count();
            context.args = expr->Arguments.Buffer();
            context.loc = expr->loc;

            if (auto funcMemberExpr = funcExpr.As<MemberExpr>())
            {
                context.baseExpr = funcMemberExpr->BaseExpression;
            }
            else if (auto funcOverloadExpr = funcExpr.As<OverloadedExpr>())
            {
                context.baseExpr = funcOverloadExpr->base;
            }
            else if (auto funcOverloadExpr2 = funcExpr.As<OverloadedExpr2>())
            {
                context.baseExpr = funcOverloadExpr2->base;
            }
            AddOverloadCandidates(funcExpr, context);

            if (context.bestCandidates.Count() > 0)
            {
                // Things were ambiguous.

                // It might be that things were only ambiguous because
                // one of the argument expressions had an error, and
                // so a bunch of candidates could match at that position.
                //
                // If any argument was an error, we skip out on printing
                // another message, to avoid cascading errors.
                for (auto arg : expr->Arguments)
                {
                    if (IsErrorExpr(arg))
                    {
                        return CreateErrorExpr(expr);
                    }
                }

                Name* funcName = nullptr;
                if (auto baseVar = funcExpr.As<VarExpr>())
                    funcName = baseVar->name;
                else if(auto baseMemberRef = funcExpr.As<MemberExpr>())
                    funcName = baseMemberRef->name;

                String argsList = getCallSignatureString(context);

                if (context.bestCandidates[0].status != OverloadCandidate::Status::Appicable)
                {
                    // There were multple equally-good candidates, but none actually usable.
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
                    UInt candidateCount = context.bestCandidates.Count();
                    UInt maxCandidatesToPrint = 10; // don't show too many candidates at once...
                    UInt candidateIndex = 0;
                    for (auto candidate : context.bestCandidates)
                    {
                        String declString = getDeclSignatureString(candidate.item);

//                        declString = declString + "[" + String(candidate.conversionCostSum) + "]";

#if 0
                        // Debugging: ensure that we don't consider multiple declarations of the same operation
                        if (auto decl = dynamic_cast<CallableDecl*>(candidate.item.declRef.decl))
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
                return CompleteOverloadCandidate(context, *context.bestCandidate);
            }
            else
            {
                // Nothing at all was found that we could even consider invoking
                getSink()->diagnose(expr->FunctionExpr, Diagnostics::expectedFunction);
                expr->type = QualType(getSession()->getErrorType());
                return expr;
            }
        }

        void AddGenericOverloadCandidate(
            LookupResultItem		baseItem,
            OverloadResolveContext&	context)
        {
            if (auto genericDeclRef = baseItem.declRef.As<GenericDecl>())
            {
                checkDecl(genericDeclRef.getDecl());

                OverloadCandidate candidate;
                candidate.flavor = OverloadCandidate::Flavor::Generic;
                candidate.item = baseItem;
                candidate.resultType = nullptr;

                AddOverloadCandidate(context, candidate);
            }
        }

        void AddGenericOverloadCandidates(
            RefPtr<Expr>	baseExpr,
            OverloadResolveContext&			context)
        {
            if(auto baseDeclRefExpr = baseExpr.As<DeclRefExpr>())
            {
                auto declRef = baseDeclRefExpr->declRef;
                AddGenericOverloadCandidate(LookupResultItem(declRef), context);
            }
            else if (auto overloadedExpr = baseExpr.As<OverloadedExpr>())
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

        RefPtr<Expr> visitGenericAppExpr(GenericAppExpr * genericAppExpr)
        {
            // We are applying a generic to arguments, but there might be multiple generic
            // declarations with the same name, so this becomes a specialized case of
            // overload resolution.

            // Start by checking the base expression and arguments.
            auto& baseExpr = genericAppExpr->FunctionExpr;
            baseExpr = CheckTerm(baseExpr);
            auto& args = genericAppExpr->Arguments;
            for (auto& arg : args)
            {
                arg = CheckTerm(arg);
            }

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
            context.argCount = args.Count();
            context.args = args.Buffer();
            context.loc = genericAppExpr->loc;

            context.baseExpr = GetBaseExpr(baseExpr);

            AddGenericOverloadCandidates(baseExpr, context);

            if (context.bestCandidates.Count() > 0)
            {
                // Things were ambiguous.
                if (context.bestCandidates[0].status != OverloadCandidate::Status::Appicable)
                {
                    // There were multple equally-good candidates, but none actually usable.
                    // We will construct a diagnostic message to help out.

                    // TODO(tfoley): print a reasonable message here...

                    getSink()->diagnose(genericAppExpr, Diagnostics::unimplemented, "no applicable generic");

                    return CreateErrorExpr(genericAppExpr);
                }
                else
                {
                    // There were multiple viable candidates, but that isn't an error: we just need
                    // to complete all of them and create an overloaded expression as a result.

                    auto overloadedExpr = new OverloadedExpr2();
                    overloadedExpr->base = context.baseExpr;
                    for (auto candidate : context.bestCandidates)
                    {
                        auto candidateExpr = CompleteOverloadCandidate(context, candidate);
                        overloadedExpr->candidiateExprs.Add(candidateExpr);
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
                getSink()->diagnose(genericAppExpr, Diagnostics::unimplemented, "expected a generic");
                return CreateErrorExpr(genericAppExpr);
            }
        }

        RefPtr<Expr> visitSharedTypeExpr(SharedTypeExpr* expr)
        {
            if (!expr->type.Ptr())
            {
                expr->base = CheckProperType(expr->base);
                expr->type = expr->base.exp->type;
            }
            return expr;
        }




        RefPtr<Expr> CheckExpr(RefPtr<Expr> expr)
        {
            auto term = CheckTerm(expr);

            // TODO(tfoley): Need a step here to ensure that the term actually
            // resolves to a (single) expression with a real type.

            return term;
        }

        RefPtr<Expr> CheckInvokeExprWithCheckedOperands(InvokeExpr *expr)
        {
            auto rs = ResolveInvoke(expr);
            if (auto invoke = dynamic_cast<InvokeExpr*>(rs.Ptr()))
            {
                // if this is still an invoke expression, test arguments passed to inout/out parameter are LValues
                if(auto funcType = invoke->FunctionExpr->type->As<FuncType>())
                {
                    UInt paramCount = funcType->getParamCount();
                    for (UInt pp = 0; pp < paramCount; ++pp)
                    {
                        auto paramType = funcType->getParamType(pp);
                        if (auto outParamType = paramType->As<OutTypeBase>())
                        {
                            if (pp < expr->Arguments.Count()
                                && !expr->Arguments[pp]->type.IsLeftValue)
                            {
                                getSink()->diagnose(
                                    expr->Arguments[pp],
                                    Diagnostics::argumentExpectedLValue,
                                    pp);
                            }
                        }
                    }
                }
            }
            return rs;
        }

        RefPtr<Expr> visitInvokeExpr(InvokeExpr *expr)
        {
            // check the base expression first
            expr->FunctionExpr = CheckExpr(expr->FunctionExpr);
            // Next check the argument expressions
            for (auto & arg : expr->Arguments)
            {
                arg = CheckExpr(arg);
            }

            return CheckInvokeExprWithCheckedOperands(expr);
        }


        RefPtr<Expr> visitVarExpr(VarExpr *expr)
        {
            // If we've already resolved this expression, don't try again.
            if (expr->declRef)
                return expr;

            expr->type = QualType(getSession()->getErrorType());
            auto lookupResult = lookUp(
                getSession(),
                this, expr->name, expr->scope);
            if (lookupResult.isValid())
            {
                return createLookupResultExpr(
                    lookupResult,
                    nullptr,
                    expr->loc);
            }

            getSink()->diagnose(expr, Diagnostics::undefinedIdentifier2, expr->name);

            return expr;
        }

        RefPtr<Expr> visitTypeCastExpr(TypeCastExpr * expr)
        {
            // Check the term we are applying first
            auto funcExpr = expr->FunctionExpr;
            funcExpr = CheckTerm(funcExpr);

            // Now ensure that the term represnets a (proper) type.
            TypeExp typeExp;
            typeExp.exp = funcExpr;
            typeExp = CheckProperType(typeExp);

            expr->FunctionExpr = typeExp.exp;
            expr->type.type = typeExp.type;

            // Next check the argument expression (there should be only one)
            for (auto & arg : expr->Arguments)
            {
                arg = CheckExpr(arg);
            }

            // Now process this like any other explicit call (so casts
            // and constructor calls are semantically equivalent).
            return CheckInvokeExprWithCheckedOperands(expr);

#if 0
            expr->Expression = CheckTerm(expr->Expression);
            auto targetType = CheckProperType(expr->TargetType);
            expr->TargetType = targetType;

            // The way to perform casting depends on the types involved
            if (expr->Expression->type->Equals(getSession()->getErrorType()))
            {
                // If the expression being casted has an error type, then just silently succeed
                expr->type = targetType.Ptr();
                return expr;
            }
            else if (auto targetArithType = targetType->AsArithmeticType())
            {
                if (auto exprArithType = expr->Expression->type->AsArithmeticType())
                {
                    // Both source and destination types are arithmetic, so we might
                    // have a valid cast
                    auto targetScalarType = targetArithType->GetScalarType();
                    auto exprScalarType = exprArithType->GetScalarType();

                    if (!IsNumeric(exprScalarType->baseType)) goto fail;
                    if (!IsNumeric(targetScalarType->baseType)) goto fail;

                    // TODO(tfoley): this checking is incomplete here, and could
                    // lead to downstream compilation failures
                    expr->type = targetType.Ptr();
                    return expr;
                }
            }
            // TODO: other cases? Should we allow a cast to succeeed whenever
            // a single-argument constructor for the target type would work?

        fail:
            // Default: in no other case succeds, then the cast failed and we emit a diagnostic.
            getSink()->diagnose(expr, Diagnostics::invalidTypeCast, expr->Expression->type, targetType->ToString());
            expr->type = QualType(getSession()->getErrorType());
            return expr;
#endif
        }

        // Get the type to use when referencing a declaration
        QualType GetTypeForDeclRef(DeclRef<Decl> declRef)
        {
            return getTypeForDeclRef(
                getSession(),
                this,
                getSink(),
                declRef,
                &typeResult);
        }

        //
        // Some syntax nodes should not occur in the concrete input syntax,
        // and will only appear *after* checking is complete. We need to
        // deal with this cases here, even if they are no-ops.
        //

        RefPtr<Expr> visitDerefExpr(DerefExpr* expr)
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), expr, "should not appear in input syntax");
            return expr;
        }

        RefPtr<Expr> visitSwizzleExpr(SwizzleExpr* expr)
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), expr, "should not appear in input syntax");
            return expr;
        }

        RefPtr<Expr> visitOverloadedExpr(OverloadedExpr* expr)
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), expr, "should not appear in input syntax");
            return expr;
        }

        RefPtr<Expr> visitOverloadedExpr2(OverloadedExpr2* expr)
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), expr, "should not appear in input syntax");
            return expr;
        }


        RefPtr<Expr> visitAggTypeCtorExpr(AggTypeCtorExpr* expr)
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), expr, "should not appear in input syntax");
            return expr;
        }

        //
        //
        //

        RefPtr<Expr> MaybeDereference(RefPtr<Expr> inExpr)
        {
            RefPtr<Expr> expr = inExpr;
            for (;;)
            {
                auto& type = expr->type;
                if (auto pointerLikeType = type->As<PointerLikeType>())
                {
                    type = QualType(pointerLikeType->elementType);

                    auto derefExpr = new DerefExpr();
                    derefExpr->base = expr;
                    derefExpr->type = QualType(pointerLikeType->elementType);

                    // TODO(tfoley): deal with l-value-ness here

                    expr = derefExpr;
                    continue;
                }

                // Default case: just use the expression as-is
                return expr;
            }
        }

        RefPtr<Expr> CheckSwizzleExpr(
            MemberExpr* memberRefExpr,
            RefPtr<Type>      baseElementType,
            IntegerLiteralValue         baseElementCount)
        {
            RefPtr<SwizzleExpr> swizExpr = new SwizzleExpr();
            swizExpr->loc = memberRefExpr->loc;
            swizExpr->base = memberRefExpr->BaseExpression;

            IntegerLiteralValue limitElement = baseElementCount;

            int elementIndices[4];
            int elementCount = 0;

            bool elementUsed[4] = { false, false, false, false };
            bool anyDuplicates = false;
            bool anyError = false;

            auto swizzleText = getText(memberRefExpr->name);

            for (UInt i = 0; i < swizzleText.Length(); i++)
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
                    getSink()->diagnose(swizExpr, Diagnostics::invalidSwizzleExpr, swizzleText, baseElementType->ToString());
                    anyError = true;
                    continue;
                }

                // TODO(tfoley): GLSL requires that all component names
                // come from the same "family"...

                // Make sure the index is in range for the source type
                if (elementIndex >= limitElement)
                {
                    getSink()->diagnose(swizExpr, Diagnostics::invalidSwizzleExpr, swizzleText, baseElementType->ToString());
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
                    new ConstantIntVal(elementCount)));
            }

            // A swizzle can be used as an l-value as long as there
            // were no duplicates in the list of components
            swizExpr->type.IsLeftValue = !anyDuplicates;

            return swizExpr;
        }

        RefPtr<Expr> CheckSwizzleExpr(
            MemberExpr*	memberRefExpr,
            RefPtr<Type>		baseElementType,
            RefPtr<IntVal>				baseElementCount)
        {
            if (auto constantElementCount = baseElementCount.As<ConstantIntVal>())
            {
                return CheckSwizzleExpr(memberRefExpr, baseElementType, constantElementCount->value);
            }
            else
            {
                getSink()->diagnose(memberRefExpr, Diagnostics::unimplemented, "swizzle on vector of unknown size");
                return CreateErrorExpr(memberRefExpr);
            }
        }

        RefPtr<Expr> visitStaticMemberExpr(StaticMemberExpr* /*expr*/)
        {
            SLANG_UNEXPECTED("should not occur in unchecked AST");
            UNREACHABLE_RETURN(nullptr);
        }

        RefPtr<Expr> lookupResultFailure(
            MemberExpr*     expr,
            QualType const& baseType)
        {
            getSink()->diagnose(expr, Diagnostics::noMemberOfNameInType, expr->name, baseType);
            expr->type = QualType(getSession()->getErrorType());
            return expr;

        }

        RefPtr<Expr> visitMemberExpr(MemberExpr * expr)
        {
            expr->BaseExpression = CheckExpr(expr->BaseExpression);

            expr->BaseExpression = MaybeDereference(expr->BaseExpression);

            auto & baseType = expr->BaseExpression->type;

            // Note: Checking for vector types before declaration-reference types,
            // because vectors are also declaration reference types...
            //
            // Also note: the way this is done right now means that the ability
            // to swizzle vectors interferes with any chance of looking up
            // members via extension, for vector or scalar types.
            //
            // TODO: Matrix swizzles probably need to be handled at some point.
            if (auto baseVecType = baseType->AsVectorType())
            {
                return CheckSwizzleExpr(
                    expr,
                    baseVecType->elementType,
                    baseVecType->elementCount);
            }
            else if(auto baseScalarType = baseType->AsBasicType())
            {
                // Treat scalar like a 1-element vector when swizzling
                return CheckSwizzleExpr(
                    expr,
                    baseScalarType,
                    1);
            }
            else if(auto typeType = baseType->As<TypeType>())
            {
                // We are looking up a member inside a type.
                // We want to be careful here because we should only find members
                // that are implicitly or explicitly `static`.
                //
                // TODO: this duplicates a *lot* of logic with the case below.
                // We need to fix that.
                auto type = typeType->type;

                if (type->As<ErrorType>())
                {
                    return CreateErrorExpr(expr);
                }

                LookupResult lookupResult = lookUpMember(
                    getSession(),
                    this,
                    expr->name,
                    type);
                if (!lookupResult.isValid())
                {
                    return lookupResultFailure(expr, baseType);
                }

                // TODO: need to filter for declarations that are valid to refer
                // to in this context...

                return createLookupResultExpr(
                    lookupResult,
                    expr->BaseExpression,
                    expr->loc);
            }
            else if (baseType->As<ErrorType>())
            {
                return CreateErrorExpr(expr);
            }
            else
            {
                LookupResult lookupResult = lookUpMember(
                    getSession(),
                    this,
                    expr->name,
                    baseType.Ptr());
                if (!lookupResult.isValid())
                {
                    return lookupResultFailure(expr, baseType);
                }

                // TODO: need to filter for declarations that are valid to refer
                // to in this context...

                return createLookupResultExpr(
                    lookupResult,
                    expr->BaseExpression,
                    expr->loc);
            }
        }
        SemanticsVisitor & operator = (const SemanticsVisitor &) = delete;


        //

        RefPtr<Expr> visitInitializerListExpr(InitializerListExpr* expr)
        {
            // When faced with an initializer list, we first just check the sub-expressions blindly.
            // Actually making them conform to a desired type will wait for when we know the desired
            // type based on context.

            for( auto& arg : expr->args )
            {
                arg = CheckTerm(arg);
            }

            expr->type = getSession()->getInitializerListType();

            return expr;
        }

        void importModuleIntoScope(Scope* scope, ModuleDecl* moduleDecl)
        {
            // If we've imported this one already, then
            // skip the step where we modify the current scope.
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

        void visitEmptyDecl(EmptyDecl* /*decl*/)
        {
            // nothing to do
        }

        void visitImportDecl(ImportDecl* decl)
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
            auto importedModuleDecl = findOrImportModule(request, name, decl->moduleNameAndLoc.loc);

            // If we didn't find a matching module, then bail out
            if (!importedModuleDecl)
                return;

            // Record the module that was imported, so that we can use
            // it later during code generation.
            decl->importedModuleDecl = importedModuleDecl;

            importModuleIntoScope(scope.Ptr(), importedModuleDecl.Ptr());

            decl->SetCheckState(getCheckedState());
        }

        // Perform semantic checking of an object-oriented `this`
        // expression.
        RefPtr<Expr> visitThisExpr(ThisExpr* expr)
        {
            // We will do an upwards search starting in the current
            // scope, looking for a surrounding type (or `extension`)
            // declaration that could be the referrant of the expression.
            auto scope = expr->scope;
            while (scope)
            {
                auto containerDecl = scope->containerDecl;
                if (auto aggTypeDecl = containerDecl->As<AggTypeDecl>())
                {
                    checkDecl(aggTypeDecl);

                    // Okay, we are using `this` in the context of an
                    // aggregate type, so the expression should be
                    // of the corresponding type.
                    expr->type = DeclRefType::Create(
                        getSession(),
                        makeDeclRef(aggTypeDecl));
                    return expr;
                }
                else if (auto extensionDecl = containerDecl->As<ExtensionDecl>())
                {
                    checkDecl(extensionDecl);

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
                    expr->type = QualType(extensionDecl->targetType.type);
                    return expr;
                }

                scope = scope->parent;
            }

            getSink()->diagnose(expr, Diagnostics::thisExpressionOutsideOfTypeDecl);
            return CreateErrorExpr(expr);
        }
    };

    bool isPrimaryDecl(
        CallableDecl*   decl)
    {
        assert(decl);
        return (!decl->primaryDecl) || (decl == decl->primaryDecl);
    }

    RefPtr<Type> checkProperType(TranslationUnitRequest * tu, TypeExp typeExp)
    {
        RefPtr<Type> type;
        DiagnosticSink nSink;
        nSink.sourceManager = tu->compileRequest->sourceManager;
        SemanticsVisitor visitor(
            &nSink,
            tu->compileRequest,
            tu);
        auto typeOut = visitor.CheckProperType(typeExp);
        if (!nSink.errorCount)
        {
            type = typeOut.type;
        }
        return type;
    }

    // Validate that an entry point function conforms to any additional
    // constraints based on the stage (and profile?) it specifies.
    void validateEntryPoint(
        EntryPointRequest*  /*entryPoint*/)
    {
        // TODO: We currently don't do any checking here, but this is the
        // right place to perform the following validation checks:
        //
        // * Does the entry point specify all of the attributes required
        //   by the chosen stage (e.g., a `[domain(...)]` attribute for]
        //   a hull shader.
        //
        // * Are the function input/output parameters and result type
        //   all valid for the chosen stage? (e.g., there shouldn't be
        //   an `OutputStream<X>` type in a vertex shader signature)
        //
        // * For any varying input/output, are there semantics specified
        //   (Note: this potentially overlaps with layout logic...), and
        //   are the system-value semantics valid for the given stage?
        //
        //   There's actually a lot of detail to semantic checking, in
        //   that the AST-level code should probably be validating the
        //   use of system-value semantics by linking them to explicit
        //   declarations in the standard library. We should also be
        //   using profile information on those declarations to infer
        //   appropriate profile restrictions on the entry point.
        //
        // * Is the entry point actually usable on the given stage/profile?
        //   E.g., if we have a vertex shader that (transitively) calls
        //   `Texture2D.Sample`, then that should produce an error because
        //   that function is specific to the fragment profile/stage.
        //
    }

    // Given an `EntryPointRequest` specified via API or command line options,
    // attempt to find a matching AST declaration that implements the specified
    // entry point. If such a function is found, then validate that it actually
    // meets the requirements for the selected stage/profile.
    //
    void findAndValidateEntryPoint(
        EntryPointRequest*  entryPoint)
    {
        // The first step in validating the entry point is to find
        // the (unique) function declaration that matches its name.

        auto translationUnit = entryPoint->getTranslationUnit();
        auto sink = &entryPoint->compileRequest->mSink;
        auto translationUnitSyntax = translationUnit->SyntaxNode;


        // Make sure we've got a query-able member dictionary
        buildMemberDictionary(translationUnitSyntax);

        // We will look up any global-scope declarations in the translation
        // unit that match the name of our entry point.
        Decl* firstDeclWithName = nullptr;
        if( !translationUnitSyntax->memberDictionary.TryGetValue(entryPoint->name, firstDeclWithName) )
        {
            // If there doesn't appear to be any such declaration, then
            // we need to diagnose it as an error, and then bail out.
            sink->diagnose(translationUnitSyntax, Diagnostics::entryPointFunctionNotFound, entryPoint->name);
            return;
        }

        // We found at least one global-scope declaration with the right name,
        // but (1) it might not be a function, and (2) there might be
        // more than one function.
        //
        // We'll walk the linked list of declarations with the same name,
        // to see what we find. Along the way we'll keep track of the
        // first function declaration we find, if any:
        FuncDecl* entryPointFuncDecl = nullptr;
        for(auto ee = firstDeclWithName; ee; ee = ee->nextInContainerWithSameName)
        {
            // Is this declaration a function?
            if (auto funcDecl = dynamic_cast<FuncDecl*>(ee))
            {
                // Skip non-primary declarations, so that
                // we don't give an error when an entry
                // point is forward-declared.
                if (!isPrimaryDecl(funcDecl))
                    continue;

                // is this the first one we've seen?
                if (!entryPointFuncDecl)
                {
                    // If so, this is a candidate to be
                    // the entry point function.
                    entryPointFuncDecl = funcDecl;
                }
                else
                {
                    // Uh-oh! We've already seen a function declaration with this
                    // name before, so the whole thing is ambiguous. We need
                    // to diagnose and bail out.

                    sink->diagnose(translationUnitSyntax, Diagnostics::ambiguousEntryPoint, entryPoint->name);

                    // List all of the declarations that the user *might* mean
                    for (auto ff = firstDeclWithName; ff; ff = ff->nextInContainerWithSameName)
                    {
                        if (auto candidate = dynamic_cast<FuncDecl*>(ff))
                        {
                            sink->diagnose(candidate, Diagnostics::entryPointCandidate, candidate->getName());
                        }
                    }

                    // Bail out.
                    return;
                }
            }
        }

        // Did we find a function declaration in our search?
        if(!entryPointFuncDecl)
        {
            // If not, then we need to diagnose the error.
            // For convenience, we will point to the first
            // declaration with the right name, that wasn't a function.
            sink->diagnose(firstDeclWithName, Diagnostics::entryPointSymbolNotAFunction, entryPoint->name);
            return;
        }

        // TODO: it is possible that the entry point was declared with
        // profile or target overloading. Is there anything that we need
        // to do at this point to filter out declarations that aren't
        // relevant to the selected profile for the entry point?

        // Phew, we have at least found a suitable decl.
        // Let's record that in the entry-point request so
        // that we don't have to re-do this effort again later.
        entryPoint->decl = entryPointFuncDecl;

        // Lookup generic parameter types in global scope
        List<RefPtr<Scope>> scopesToTry;
        scopesToTry.Add(entryPoint->getTranslationUnit()->SyntaxNode->scope);
        for (auto & module : entryPoint->compileRequest->loadedModulesList)
            scopesToTry.Add(module->moduleDecl->scope);
        for (auto name : entryPoint->genericParameterTypeNames)
        {   
            // parse type name
            RefPtr<Type> type;
            for (auto & s : scopesToTry)
            {
                RefPtr<Expr> typeExpr = entryPoint->compileRequest->parseTypeString(entryPoint->getTranslationUnit(),
                    name, s);
                type = checkProperType(translationUnit, TypeExp(typeExpr));
                if (type)
                {
                    break;
                }
            }
            if (!type)
            {
                sink->diagnose(firstDeclWithName, Diagnostics::entryPointTypeSymbolNotAType, name);
                return;
            }
            entryPoint->genericParameterTypes.Add(type);
        }
        
        // validate global type arguments only when we are generating code
        if ((entryPoint->compileRequest->compileFlags & SLANG_COMPILE_FLAG_NO_CODEGEN) == 0)
        {
            // check that user-provioded type arguments conforms to the generic type
            // parameter declaration of this translation unit

            // collect global generic parameters from all imported modules
            List<RefPtr<GlobalGenericParamDecl>> globalGenericParams;
            // add current translation unit first
            {
                auto globalGenParams = translationUnit->SyntaxNode->getMembersOfType<GlobalGenericParamDecl>();
                for (auto p : globalGenParams)
                    globalGenericParams.Add(p);
            }
            // add imported modules
            for (auto loadedModule : entryPoint->compileRequest->loadedModulesList)
            {
                auto moduleDecl = loadedModule->moduleDecl;
                auto globalGenParams = moduleDecl->getMembersOfType<GlobalGenericParamDecl>();
                for (auto p : globalGenParams)
                    globalGenericParams.Add(p);
            }
            if (globalGenericParams.Count() != entryPoint->genericParameterTypes.Count())
            {
                sink->diagnose(entryPoint->decl, Diagnostics::mismatchEntryPointTypeArgument, globalGenericParams.Count(),
                    entryPoint->genericParameterTypes.Count());
                return;
            }
            // if entry-point type arguments matches parameters, try find
            // SubtypeWitness for each argument
            int index = 0;
            for (auto & gParam : globalGenericParams)
            {
                for (auto constraint : gParam->getMembersOfType<GenericTypeConstraintDecl>())
                {
                    auto interfaceType = GetSup(DeclRef<GenericTypeConstraintDecl>(constraint, nullptr));
                    SemanticsVisitor visitor(sink, entryPoint->compileRequest, translationUnit);
                    auto witness = visitor.tryGetSubtypeWitness(entryPoint->genericParameterTypes[index], interfaceType);
                    if (!witness)
                    {
                        sink->diagnose(gParam,
                            Diagnostics::typeArgumentDoesNotConformToInterface, gParam->nameAndLoc.name, entryPoint->genericParameterTypes[index],
                            interfaceType);
                    }
                    entryPoint->genericParameterWitnesses.Add(witness);
                }
                index++;
            }
        }
        if (sink->errorCount != 0)
            return;

        // Now that we've *found* the entry point, it is time to validate
        // that it actually meets the constraints for the chosen stage/profile.
        validateEntryPoint(entryPoint);
    }

    void validateEntryPoints(
        CompileRequest* compileRequest)
    {
        // The validation of entry points here will be modal, and controlled
        // by whether the user specified any entry points directly via
        // API or command-line options.
        //
        // TODO: We may want to make this choice explicit rather than implicit.
        //
        // First, check if the user request any entry points explicitly via
        // the API or command line.
        //
        bool anyExplicitEntryPointRequests = false;
        for (auto& translationUnit : compileRequest->translationUnits)
        {
            if( translationUnit->entryPoints.Count() != 0)
            {
                anyExplicitEntryPointRequests = true;
                break;
            }
        }

        if( anyExplicitEntryPointRequests )
        {
            // If there were any explicit requests for entry points to be
            // checked, then we will *only* check those.

            for (auto& translationUnit : compileRequest->translationUnits)
            {
                for (auto entryPoint : translationUnit->entryPoints)
                {
                    findAndValidateEntryPoint(entryPoint);
                }
            }
        }
        else
        {
            // Otherwise, scan for any `[shader(...)]` attributes in
            // the user's code, and construct `EntryPointRequest`s to
            // represent them.
            //
            // This ensures that downstream code only has to consider
            // the central list of entry point requests, and doesn't
            // have to know where they came from.

            // TODO: A comprehensive approach here would need to search
            // recursively for entry points, because they might appear
            // as, e.g., member function of a `struct` type.
            //
            // For now we'll start with an extremely basic approach that
            // should work for typical HLSL code.
            //
            UInt translationUnitCount = compileRequest->translationUnits.Count();
            for(UInt tt = 0; tt < translationUnitCount; ++tt)
            {
                auto translationUnit = compileRequest->translationUnits[tt];
                for( auto globalDecl : translationUnit->SyntaxNode->Members )
                {
                    auto maybeFuncDecl = globalDecl;
                    if( auto genericDecl = maybeFuncDecl->As<GenericDecl>() )
                    {
                        maybeFuncDecl = genericDecl->inner;
                    }

                    auto funcDecl = maybeFuncDecl->As<FuncDecl>();
                    if(!funcDecl)
                        continue;

                    auto entryPointAttr = funcDecl->FindModifier<EntryPointAttribute>();
                    if(!entryPointAttr)
                        continue;

                    // We've discovered a valid entry point. It is a function (possibly
                    // generic) that has a `[shader(...)]` attribute to mark it as an
                    // entry point.
                    //
                    // We will now register that entry point as an `EntryPointRequest`
                    // with an appropriately chosen profile.
                    //
                    // The profile will only include a stage, so that the profile "family"
                    // and "version" are left unspecified. Downstream code will need
                    // to be able to handle this case.
                    //
                    Profile profile;
                    profile.setStage(entryPointAttr->stage);

                    // We manually fill in the entry point request object.
                    RefPtr<EntryPointRequest> entryPointReq = new EntryPointRequest();
                    entryPointReq->compileRequest = compileRequest;
                    entryPointReq->translationUnitIndex = int(tt);
                    entryPointReq->decl = funcDecl;
                    entryPointReq->name = funcDecl->getName();
                    entryPointReq->profile = profile;

                    // Apply the common validation logic to this entry point.
                    validateEntryPoint(entryPointReq);

                    // Add the entry point to the list in the translation unit,
                    // and also the global list in the compile request.
                    translationUnit->entryPoints.Add(entryPointReq);
                    compileRequest->entryPoints.Add(entryPointReq);
                }
            }
        }
    }

    void checkTranslationUnit(
        TranslationUnitRequest* translationUnit)
    {
        SemanticsVisitor visitor(
            &translationUnit->compileRequest->mSink,
            translationUnit->compileRequest,
            translationUnit);

        // Apply the visitor to do the main semantic
        // checking that is required on all declarations
        // in the translation unit.
        visitor.checkDecl(translationUnit->SyntaxNode);
    }


    //

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
        if (auto varDeclRef = declRef.As<VarDeclBase>())
        {
            QualType qualType;
            qualType.type = GetType(varDeclRef);
            qualType.IsLeftValue = true; // TODO(tfoley): allow explicit `const` or `let` variables
            return qualType;
        }
        else if (auto typeAliasDeclRef = declRef.As<TypeDefDecl>())
        {
            auto type = getNamedType(session, typeAliasDeclRef);
            *outTypeResult = type;
            return QualType(getTypeType(type));
        }
        else if (auto aggTypeDeclRef = declRef.As<AggTypeDecl>())
        {
            auto type = DeclRefType::Create(session, aggTypeDeclRef);
            *outTypeResult = type;
            return QualType(getTypeType(type));
        }
        else if (auto simpleTypeDeclRef = declRef.As<SimpleTypeDecl>())
        {
            auto type = DeclRefType::Create(session, simpleTypeDeclRef);
            *outTypeResult = type;
            return QualType(getTypeType(type));
        }
        else if (auto genericDeclRef = declRef.As<GenericDecl>())
        {
            auto type = getGenericDeclRefType(session, genericDeclRef);
            *outTypeResult = type;
            return QualType(getTypeType(type));
        }
        else if (auto funcDeclRef = declRef.As<CallableDecl>())
        {
            auto type = getFuncType(session, funcDeclRef);
            return QualType(type);
        }
        else if (auto constraintDeclRef = declRef.As<TypeConstraintDecl>())
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

    // Sometimes we need to refer to a declaration the way that it would be specialized
    // inside the context where it is declared (e.g., with generic parameters filled in
    // using their archetypes).
    //
    SubstitutionSet createDefaultSubstitutions(
        Session*        session,
        Decl*           decl,
        SubstitutionSet parentSubst)
    {
        SubstitutionSet resultSubst = parentSubst;
        if (auto interfaceDecl = dynamic_cast<InterfaceDecl*>(decl))
        {
            resultSubst.thisTypeSubstitution = new ThisTypeSubstitution();
        }
        auto dd = decl->ParentDecl;
        if( auto genericDecl = dynamic_cast<GenericDecl*>(dd) )
        {
            // We don't want to specialize references to anything
            // other than the "inner" declaration itself.
            if(decl != genericDecl->inner)
                return resultSubst;

            RefPtr<GenericSubstitution> subst = new GenericSubstitution();
            subst->genericDecl = genericDecl;
            subst->outer = parentSubst.genericSubstitutions;
            resultSubst.genericSubstitutions = subst;
            SubstitutionSet outerSubst = resultSubst;
            outerSubst.genericSubstitutions = outerSubst.genericSubstitutions?outerSubst.genericSubstitutions->outer:nullptr;
            for( auto mm : genericDecl->Members )
            {
                if( auto genericTypeParamDecl = mm.As<GenericTypeParamDecl>() )
                {
                    subst->args.Add(DeclRefType::Create(session, DeclRef<Decl>(genericTypeParamDecl.Ptr(), outerSubst)));
                }
                else if( auto genericValueParamDecl = mm.As<GenericValueParamDecl>() )
                {
                    subst->args.Add(new GenericParamIntVal(DeclRef<GenericValueParamDecl>(genericValueParamDecl.Ptr(), outerSubst)));
                }
            }

            // create default substitution arguments for constraints
            for (auto mm : genericDecl->Members)
            {
                if (auto genericTypeConstraintDecl = mm.As<GenericTypeConstraintDecl>())
                {
                    RefPtr<DeclaredSubtypeWitness> witness = new DeclaredSubtypeWitness();
                    witness->declRef = DeclRef<Decl>(genericTypeConstraintDecl.Ptr(), outerSubst);
                    witness->sub = genericTypeConstraintDecl->sub.type;
                    witness->sup = genericTypeConstraintDecl->sup.type;
                    subst->args.Add(witness);
                }
            }
        }
        return resultSubst;
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
}
