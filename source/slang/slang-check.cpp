#include "slang-syntax-visitors.h"

#include "slang-lookup.h"
#include "slang-compiler.h"
#include "slang-visitor.h"

#include "../core/slang-secure-crt.h"

#include "../core/slang-io.h"

#include <assert.h>

namespace Slang
{
    RefPtr<TypeType> getTypeType(
        Type* type);

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

        /// Should the given `decl` be treated as a static rather than instance declaration?
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

    // A flat representation of basic types (scalars, vectors and matrices)
    // that can be used as lookup key in caches
    struct BasicTypeKey
    {
        union
        {
            struct
            {
                unsigned char type : 4;
                unsigned char dim1 : 2;
                unsigned char dim2 : 2;
            } data;
            unsigned char aggVal;
        };
        bool fromType(Type* typeIn)
        {
            aggVal = 0;
            if (auto basicType = as<BasicExpressionType>(typeIn))
            {
                data.type = (unsigned char)basicType->baseType;
                data.dim1 = data.dim2 = 0;
            }
            else if (auto vectorType = as<VectorExpressionType>(typeIn))
            {
                if (auto elemCount = as<ConstantIntVal>(vectorType->elementCount))
                {
                    data.dim1 = elemCount->value - 1;
                    auto elementBasicType = as<BasicExpressionType>(vectorType->elementType);
                    data.type = (unsigned char)elementBasicType->baseType;
                    data.dim2 = 0;
                }
                else
                    return false;
            }
            else if (auto matrixType = as<MatrixExpressionType>(typeIn))
            {
                if (auto elemCount1 = as<ConstantIntVal>(matrixType->getRowCount()))
                {
                    if (auto elemCount2 = as<ConstantIntVal>(matrixType->getColumnCount()))
                    {
                        auto elemBasicType = as<BasicExpressionType>(matrixType->getElementType());
                        data.type = (unsigned char)elemBasicType->baseType;
                        data.dim1 = elemCount1->value - 1;
                        data.dim2 = elemCount2->value - 1;
                    }
                }
                else
                    return false;
            }
            else
                return false;
            return true;
        }
    };

    struct BasicTypeKeyPair
    {
        BasicTypeKey type1, type2;
        bool operator == (BasicTypeKeyPair p)
        {
            return type1.aggVal == p.type1.aggVal && type2.aggVal == p.type2.aggVal;
        }
        int GetHashCode()
        {
            return combineHash(type1.aggVal, type2.aggVal);
        }
    };

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
            Applicable,
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

    struct OperatorOverloadCacheKey
    {
        IROp operatorName;
        BasicTypeKey args[2];
        bool operator == (OperatorOverloadCacheKey key)
        {
            return operatorName == key.operatorName && args[0].aggVal == key.args[0].aggVal
                && args[1].aggVal == key.args[1].aggVal;
        }
        int GetHashCode()
        {
            return ((int)(UInt64)(void*)(operatorName) << 16) ^ (args[0].aggVal << 8) ^ (args[1].aggVal);
        }
        bool fromOperatorExpr(OperatorExpr* opExpr)
        {
            // First, lets see if the argument types are ones
            // that we can encode in our space of keys.
            args[0].aggVal = 0;
            args[1].aggVal = 0;
            if (opExpr->Arguments.getCount() > 2)
                return false;

            for (Index i = 0; i < opExpr->Arguments.getCount(); i++)
            {
                if (!args[i].fromType(opExpr->Arguments[i]->type.Ptr()))
                    return false;
            }

            // Next, lets see if we can find an intrinsic opcode
            // attached to an overloaded definition (filtered for
            // definitions that could conceivably apply to us).
            //
            // TODO: This should really be parsed on the operator name
            // plus fixity, rather than the intrinsic opcode...
            //
            // We will need to reject postfix definitions for prefix
            // operators, and vice versa, to ensure things work.
            //
            auto prefixExpr = as<PrefixExpr>(opExpr);
            auto postfixExpr = as<PostfixExpr>(opExpr);

            if (auto overloadedBase = as<OverloadedExpr>(opExpr->FunctionExpr))
            {
                for(auto item : overloadedBase->lookupResult2 )
                {
                    // Look at a candidate definition to be called and
                    // see if it gives us a key to work with.
                    //
                    Decl* funcDecl = overloadedBase->lookupResult2.item.declRef.decl;
                    if (auto genDecl = as<GenericDecl>(funcDecl))
                        funcDecl = genDecl->inner.Ptr();

                    // Reject definitions that have the wrong fixity.
                    //
                    if(prefixExpr && !funcDecl->FindModifier<PrefixModifier>())
                        continue;
                    if(postfixExpr && !funcDecl->FindModifier<PostfixModifier>())
                        continue;

                    if (auto intrinsicOp = funcDecl->FindModifier<IntrinsicOpModifier>())
                    {
                        operatorName = intrinsicOp->op;
                        return true;
                    }
                }
            }
            return false;
        }
    };

    struct TypeCheckingCache
    {
        Dictionary<OperatorOverloadCacheKey, OverloadCandidate> resolvedOperatorOverloadCache;
        Dictionary<BasicTypeKeyPair, ConversionCost> conversionCostCache;
    };

    TypeCheckingCache* Session::getTypeCheckingCache()
    {
        if (!typeCheckingCache)
            typeCheckingCache = new TypeCheckingCache();
        return typeCheckingCache;
    }

    void Session::destroyTypeCheckingCache()
    {
        delete typeCheckingCache;
        typeCheckingCache = nullptr;
    }

    namespace { // anonymous
    struct FunctionInfo
    {
        const char* name;
        SharedLibraryType libraryType;
    };
    } // anonymous

    static FunctionInfo _getFunctionInfo(Session::SharedLibraryFuncType funcType)
    {
        typedef Session::SharedLibraryFuncType FuncType;
        typedef SharedLibraryType LibType;

        switch (funcType)
        {
            case FuncType::Glslang_Compile:   return { "glslang_compile", LibType::Glslang } ;
            case FuncType::Fxc_D3DCompile:     return { "D3DCompile", LibType::Fxc };
            case FuncType::Fxc_D3DDisassemble: return { "D3DDisassemble", LibType::Fxc };
            case FuncType::Dxc_DxcCreateInstance:  return { "DxcCreateInstance", LibType::Dxc };
            default: return { nullptr, LibType::Unknown };
        } 
    }

    static PassThroughMode _toPassThroughMode(SharedLibraryType type)
    {
        switch (type)
        {
            case SharedLibraryType::Dxil:
            case SharedLibraryType::Dxc:
            {
                return PassThroughMode::Dxc;
            }
            case SharedLibraryType::Fxc:        return PassThroughMode::Fxc;
            case SharedLibraryType::Glslang:    return PassThroughMode::Glslang;
            default: break;
        }

        return PassThroughMode::None;    
    }

    void Session::setSharedLibrary(SharedLibraryType type, ISlangSharedLibrary* library)
    {
        sharedLibraries[int(type)] = library;
    }

    ISlangSharedLibrary* Session::getOrLoadSharedLibrary(SharedLibraryType type, DiagnosticSink* sink)
    {
        // If not loaded, try loading it
        if (!sharedLibraries[int(type)])
        {
            // Try to preload dxil first, if loading dxc
            if (type == SharedLibraryType::Dxc)
            {
                // Pass nullptr as the sink, because if it fails we don't want to report as error
                getOrLoadSharedLibrary(SharedLibraryType::Dxil, nullptr);
            }

            const char* libName = DefaultSharedLibraryLoader::getSharedLibraryNameFromType(type);

            StringBuilder builder;
            PassThroughMode passThrough = _toPassThroughMode(type);
            if (passThrough != PassThroughMode::None && m_downstreamCompilerPaths[int(passThrough)].getLength() > 0)
            {
                Path::combineIntoBuilder(m_downstreamCompilerPaths[int(passThrough)].getUnownedSlice(), UnownedStringSlice(libName), builder);
                libName = builder.getBuffer();
            }

            if (SLANG_FAILED(sharedLibraryLoader->loadSharedLibrary(libName, sharedLibraries[int(type)].writeRef())))
            {
                if (sink)
                {
                    sink->diagnose(SourceLoc(), Diagnostics::failedToLoadDynamicLibrary, libName);
                }
                return nullptr;
            }
        }
        return sharedLibraries[int(type)];
    }

    SlangFuncPtr Session::getSharedLibraryFunc(SharedLibraryFuncType type, DiagnosticSink* sink)
    {
        if (sharedLibraryFunctions[int(type)])
        {
            return sharedLibraryFunctions[int(type)];
        }
        // do we have the library
        FunctionInfo info = _getFunctionInfo(type);
        if (info.name == nullptr)
        {
            return nullptr;
        }
        // Try loading the library
        ISlangSharedLibrary* sharedLib = getOrLoadSharedLibrary(info.libraryType, sink);
        if (!sharedLib)
        {
            return nullptr;
        }

        // Okay now access the func
        SlangFuncPtr func = sharedLib->findFuncByName(info.name);
        if (!func)
        {
            const char* libName = DefaultSharedLibraryLoader::getSharedLibraryNameFromType(info.libraryType);
            sink->diagnose(SourceLoc(), Diagnostics::failedToFindFunctionInSharedLibrary, info.name, libName);
            return nullptr;
        }

        // Store in the function cache
        sharedLibraryFunctions[int(type)] = func;
        return func;
    }

    CPPCompilerSet* Session::requireCPPCompilerSet()
    {
        if (cppCompilerSet == nullptr)
        {
            cppCompilerSet = new CPPCompilerSet;

            typedef CPPCompiler::CompilerType CompilerType;
            CPPCompilerUtil::InitializeSetDesc desc;
       
            desc.paths[int(CompilerType::GCC)] = m_downstreamCompilerPaths[int(PassThroughMode::Gcc)];
            desc.paths[int(CompilerType::Clang)] = m_downstreamCompilerPaths[int(PassThroughMode::Clang)];
            desc.paths[int(CompilerType::VisualStudio)] = m_downstreamCompilerPaths[int(PassThroughMode::VisualStudio)];

            CPPCompilerUtil::initializeSet(desc, cppCompilerSet);
        }
        SLANG_ASSERT(cppCompilerSet);
        return cppCompilerSet;
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

        Linkage* m_linkage = nullptr;
        DiagnosticSink* m_sink = nullptr;

        DiagnosticSink* getSink()
        {
            return m_sink;
        }

//        ModuleDecl * program = nullptr;
        FuncDecl * function = nullptr;


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
            Linkage*        linkage,
            DiagnosticSink* sink)
            : m_linkage(linkage)
            , m_sink(sink)
        {}

        Session* getSession()
        {
            return m_linkage->getSessionImpl();
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
            if (auto typeType = as<TypeType>(typeRepr->type))
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
            if (auto typetype = as<TypeType>(expr->type))
                return typetype->type.dynamicCast<DeclRefType>();
            else
                return as<DeclRefType>(expr->type);
        }

        /// Is `decl` usable as a static member?
        bool isDeclUsableAsStaticMember(
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

        /// Is `item` usable as a static member?
        bool isUsableAsStaticMember(
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

            /// Move `expr` into a temporary variable and execute `func` on that variable.
            ///
            /// Returns an expression that wraps both the creation and initialization of
            /// the temporary, and the computation created by `func`.
            ///
        template<typename F>
        RefPtr<Expr> moveTemp(RefPtr<Expr> const& expr, F const& func)
        {
            RefPtr<VarDecl> varDecl = new VarDecl();
            varDecl->ParentDecl = nullptr; // TODO: need to fill this in somehow!
            varDecl->checkState = DeclCheckState::Checked;
            varDecl->nameAndLoc.loc = expr->loc;
            varDecl->initExpr = expr;
            varDecl->type.type = expr->type.type;

            auto varDeclRef = makeDeclRef(varDecl.Ptr());

            RefPtr<LetExpr> letExpr = new LetExpr();
            letExpr->decl = varDecl;

            auto body = func(varDeclRef);

            letExpr->body = body;
            letExpr->type = body->type;

            return letExpr;
        }

            /// Execute `func` on a variable with the value of `expr`.
            ///
            /// If `expr` is just a reference to an immutable (e.g., `let`) variable
            /// then this might use the existing variable. Otherwise it will create
            /// a new variable to hold `expr`, using `moveTemp()`.
            ///
        template<typename F>
        RefPtr<Expr> maybeMoveTemp(RefPtr<Expr> const& expr, F const& func)
        {
            if(auto varExpr = as<VarExpr>(expr))
            {
                auto declRef = varExpr->declRef;
                if(auto varDeclRef = declRef.as<LetDecl>())
                    return func(varDeclRef);
            }

            return moveTemp(expr, func);
        }

            /// Return an expression that represents "opening" the existential `expr`.
            ///
            /// The type of `expr` must be an interface type, matching `interfaceDeclRef`.
            ///
            /// If we scope down the PL theory to just the case that Slang cares about,
            /// a value of an existential type like `IMover` is a tuple of:
            ///
            ///  * a concrete type `X`
            ///  * a witness `w` of the fact that `X` implements `IMover`
            ///  * a value `v` of type `X`
            ///
            /// "Opening" an existential value is the process of decomposing a single
            /// value `e : IMover` into the pieces `X`, `w`, and `v`.
            ///
            /// Rather than return all those pieces individually, this operation
            /// returns an expression that logically corresponds to `v`: an expression
            /// of type `X`, where the type carries the knowledge that `X` implements `IMover`.
            ///
        RefPtr<Expr> openExistential(
            RefPtr<Expr>            expr,
            DeclRef<InterfaceDecl>  interfaceDeclRef)
        {
            // If `expr` refers to an immutable binding,
            // then we can use it directly. If it refers
            // to an arbitrary expression or a mutable
            // binding, we will move its value into an
            // immutable temporary so that we can use
            // it directly.
            //
            auto interfaceDecl = interfaceDeclRef.getDecl();
            return maybeMoveTemp(expr, [&](DeclRef<VarDeclBase> varDeclRef)
            {
                RefPtr<ExtractExistentialType> openedType = new ExtractExistentialType();
                openedType->declRef = varDeclRef;

                RefPtr<ExtractExistentialSubtypeWitness> openedWitness = new ExtractExistentialSubtypeWitness();
                openedWitness->sub = openedType;
                openedWitness->sup = expr->type.type;
                openedWitness->declRef = varDeclRef;

                RefPtr<ThisTypeSubstitution> openedThisType = new ThisTypeSubstitution();
                openedThisType->outer = interfaceDeclRef.substitutions.substitutions;
                openedThisType->interfaceDecl = interfaceDecl;
                openedThisType->witness = openedWitness;

                DeclRef<InterfaceDecl> substDeclRef = DeclRef<InterfaceDecl>(interfaceDecl, openedThisType);
                auto substDeclRefType = DeclRefType::Create(getSession(), substDeclRef);

                RefPtr<ExtractExistentialValueExpr> openedValue = new ExtractExistentialValueExpr();
                openedValue->declRef = varDeclRef;
                openedValue->type = QualType(substDeclRefType);

                return openedValue;
            });
        }

            /// If `expr` has existential type, then open it.
            ///
            /// Returns an expression that opens `expr` if it had existential type.
            /// Otherwise returns `expr` itself.
            ///
            /// See `openExistential` for a discussion of what "opening" an
            /// existential-type value means.
            ///
        RefPtr<Expr> maybeOpenExistential(RefPtr<Expr> expr)
        {
            auto exprType = expr->type.type;

            if(auto declRefType = as<DeclRefType>(exprType))
            {
                if(auto interfaceDeclRef = declRefType->declRef.as<InterfaceDecl>())
                {
                    // Is there an this-type substitution being applied, so that
                    // we are referencing the interface type through a concrete
                    // type (e.g., a type parameter constrained to this interface)?
                    //
                    // Because of the way that substitutions need to mirror the nesting
                    // hierarchy of declarations, any this-type substitution pertaining
                    // to the chosen interface decl must be the first substitution on
                    // the list (which is a linked list from the "inside" out).
                    //
                    auto thisTypeSubst = interfaceDeclRef.substitutions.substitutions.as<ThisTypeSubstitution>();
                    if(thisTypeSubst && thisTypeSubst->interfaceDecl == interfaceDeclRef.decl)
                    {
                        // This isn't really an existential type, because somebody
                        // has already filled in a this-type substitution.
                    }
                    else
                    {
                        // Okay, here is the case that matters.
                        //
                        return openExistential(expr, interfaceDeclRef);
                    }
                }
            }

            // Default: apply the callback to the original expression;
            return expr;
        }

        RefPtr<Expr> ConstructDeclRefExpr(
            DeclRef<Decl>   declRef,
            RefPtr<Expr>    baseExpr,
            SourceLoc       loc)
        {
            // Compute the type that this declaration reference will have in context.
            //
            auto type = GetTypeForDeclRef(declRef);

            // Construct an appropriate expression based on the structured of
            // the declaration reference.
            //
            if (baseExpr)
            {
                // If there was a base expression, we will have some kind of
                // member expression.

                // We want to check for the case where the base "expression"
                // actually names a type, because in that case we are doing
                // a static member reference.
                //
                // TODO: Should we be checking if the member is static here?
                // If it isn't, should we be automatically producing a "curried"
                // form (e.g., for a member function, return a value usable
                // for referencing it as a free function).
                //
                if (as<TypeType>(baseExpr->type))
                {
                    auto expr = new StaticMemberExpr();
                    expr->loc = loc;
                    expr->type = type;
                    expr->BaseExpression = baseExpr;
                    expr->name = declRef.GetName();
                    expr->declRef = declRef;
                    return expr;
                }
                else if(isEffectivelyStatic(declRef.getDecl()))
                {
                    // Extract the type of the baseExpr
                    auto baseExprType = baseExpr->type.type;
                    RefPtr<SharedTypeExpr> baseTypeExpr = new SharedTypeExpr();
                    baseTypeExpr->base.type = baseExprType;
                    baseTypeExpr->type.type = getTypeType(baseExprType);

                    auto expr = new StaticMemberExpr();
                    expr->loc = loc;
                    expr->type = type;
                    expr->BaseExpression = baseTypeExpr;
                    expr->name = declRef.GetName();
                    expr->declRef = declRef;
                    return expr;
                }
                else
                {
                    // If the base expression wasn't a type, then this
                    // is a normal member expression.
                    //
                    auto expr = new MemberExpr();
                    expr->loc = loc;
                    expr->type = type;
                    expr->BaseExpression = baseExpr;
                    expr->name = declRef.GetName();
                    expr->declRef = declRef;

                    // When referring to a member through an expression,
                    // the result is only an l-value if both the base
                    // expression and the member agree that it should be.
                    //
                    // We have already used the `QualType` from the member
                    // above (that is `type`), so we need to take the
                    // l-value status of the base expression into account now.
                    if(!baseExpr->type.IsLeftValue)
                    {
                        expr->type.IsLeftValue = false;
                    }

                    return expr;
                }
            }
            else
            {
                // If there is no base expression, then the result must
                // be an ordinary variable expression.
                //
                auto expr = new VarExpr();
                expr->loc = loc;
                expr->name = declRef.GetName();
                expr->type = type;
                expr->declRef = declRef;
                return expr;
            }
        }

        RefPtr<Expr> ConstructDerefExpr(
            RefPtr<Expr>    base,
            SourceLoc       loc)
        {
            auto ptrLikeType = as<PointerLikeType>(base->type);
            SLANG_ASSERT(ptrLikeType);

            auto derefExpr = new DerefExpr();
            derefExpr->loc = loc;
            derefExpr->base = base;
            derefExpr->type = QualType(ptrLikeType->elementType);

            // TODO(tfoley): handle l-value status here

            return derefExpr;
        }

        RefPtr<Expr> createImplicitThisMemberExpr(
            Type*                                           type,
            SourceLoc                                       loc,
            LookupResultItem::Breadcrumb::ThisParameterMode thisParameterMode)
        {
            RefPtr<ThisExpr> expr = new ThisExpr();
            expr->type = type;
            expr->type.IsLeftValue = thisParameterMode == LookupResultItem::Breadcrumb::ThisParameterMode::Mutating;
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
                        if (auto extensionDeclRef = breadcrumb->declRef.as<ExtensionDecl>())
                        {
                            bb = createImplicitThisMemberExpr(
                                GetTargetType(extensionDeclRef),
                                loc,
                                breadcrumb->thisParameterMode);
                        }
                        else
                        {
                            auto type = DeclRefType::Create(getSession(), breadcrumb->declRef);
                            bb = createImplicitThisMemberExpr(
                                type,
                                loc,
                                breadcrumb->thisParameterMode);
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
            if (auto overloadedExpr = as<OverloadedExpr>(expr))
            {
                expr = ResolveOverloadedExpr(overloadedExpr, LookupMask::type);
            }

            if (auto typeType = as<TypeType>(expr->type))
            {
                return expr;
            }
            else if (auto errorType = as<ErrorType>(expr->type))
            {
                return expr;
            }

            getSink()->diagnose(expr, Diagnostics::unimplemented, "expected a type");
            return CreateErrorExpr(expr);
        }

        RefPtr<Type> ExpectAType(RefPtr<Expr> expr)
        {
            auto typeRepr = ExpectATypeRepr(expr);
            if (auto typeType = as<TypeType>(typeRepr->type))
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
            if (auto overloadedExpr = as<OverloadedExpr>(exp))
            {
                // assume that if it is overloaded, we want a type
                exp = ResolveOverloadedExpr(overloadedExpr, LookupMask::type);
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

        // Construct a type representing the instantiation of
        // the given generic declaration for the given arguments.
        // The arguments should already be checked against
        // the declaration.
        RefPtr<Type> InstantiateGenericType(
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

        // This routine is a bottleneck for all declaration checking,
        // so that we can add some quality-of-life features for users
        // in cases where the compiler crashes
        void dispatchDecl(DeclBase* decl)
        {
            try
            {
                DeclVisitor::dispatch(decl);
            }
            // Don't emit any context message for an explicit `AbortCompilationException`
            // because it should only happen when an error is already emitted.
            catch(AbortCompilationException&) { throw; }
            catch(...)
            {
                getSink()->noteInternalErrorLoc(decl->loc);
                throw;
            }
        }
        void dispatchStmt(Stmt* stmt)
        {
            try
            {
                StmtVisitor::dispatch(stmt);
            }
            catch(AbortCompilationException&) { throw; }
            catch(...)
            {
                getSink()->noteInternalErrorLoc(stmt->loc);
                throw;
            }
        }
        void dispatchExpr(Expr* expr)
        {
            try
            {
                ExprVisitor::dispatch(expr);
            }
            catch(AbortCompilationException&) { throw; }
            catch(...)
            {
                getSink()->noteInternalErrorLoc(expr->loc);
                throw;
            }
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

        void EnusreAllDeclsRec(RefPtr<Decl> decl)
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
                if(auto typeType = as<TypeType>(typeExp.exp->type))
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

                auto genericDeclRef = genericDeclRefType->GetDeclRef();
                checkDecl(genericDeclRef.decl);
                List<RefPtr<Expr>> args;
                for (RefPtr<Decl> member : genericDeclRef.getDecl()->Members)
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

            if (auto errorType = as<ErrorType>(expr->type))
                return true;

            return false;
        }

        // Capture the "base" expression in case this is a member reference
        RefPtr<Expr> GetBaseExpr(RefPtr<Expr> expr)
        {
            if (auto memberExpr = as<MemberExpr>(expr))
            {
                return memberExpr->BaseExpression;
            }
            else if(auto overloadedExpr = as<OverloadedExpr>(expr))
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

        bool isEffectivelyScalarForInitializerLists(
            RefPtr<Type>    type)
        {
            if(as<ArrayExpressionType>(type)) return false;
            if(as<VectorExpressionType>(type)) return false;
            if(as<MatrixExpressionType>(type)) return false;

            if(as<BasicExpressionType>(type))
            {
                return true;
            }

            if(as<ResourceType>(type))
            {
                return true;
            }
            if(as<UntypedBufferResourceType>(type))
            {
                return true;
            }
            if(as<SamplerStateType>(type))
            {
                return true;
            }

            if(auto declRefType = as<DeclRefType>(type))
            {
                if(as<StructDecl>(declRefType->declRef))
                    return false;
            }

            return true;
        }

            /// Should the provided expression (from an initializer list) be used directly to initialize `toType`?
        bool shouldUseInitializerDirectly(
            RefPtr<Type>    toType,
            RefPtr<Expr>    fromExpr)
        {
            // A nested initializer list should always be used directly.
            //
            if(as<InitializerListExpr>(fromExpr))
            {
                return true;
            }

            // If the desired type is a scalar, then we should always initialize
            // directly, since it isn't an aggregate.
            //
            if(isEffectivelyScalarForInitializerLists(toType))
                return true;

            // If the type we are initializing isn't effectively scalar,
            // but the initialization expression *is*, then it doesn't
            // seem like direct initialization is intended.
            //
            if(isEffectivelyScalarForInitializerLists(fromExpr->type))
                return false;

            // Once the above cases are handled, the main thing
            // we want to check for is whether a direct initialization
            // is possible (a type conversion exists).
            //
            return canCoerce(toType, fromExpr->type);
        }

            /// Read a value from an initializer list expression.
            ///
            /// This reads one or more argument from the initializer list
            /// given as `fromInitializerListExpr` to initialize a value
            /// of type `toType`. This may involve reading one or
            /// more arguments from the initializer list, depending
            /// on whether `toType` is an aggregate or not, and on
            /// whether the next argument in the initializer list is
            /// itself an initializer list.
            ///
            /// This routine returns `true` if it was able to read
            /// arguments that can form a value of type `toType`,
            /// and `false` otherwise.
            ///
            /// If the routine succeeds and `outToExpr` is non-null,
            /// then it will be filled in with an expression
            /// representing the value (or type `toType`) that was read,
            /// or it will be left null to indicate that a default
            /// value should be used.
            ///
            /// If the routine fails and `outToExpr` is non-null,
            /// then a suitable diagnostic will be emitted.
            ///
        bool _readValueFromInitializerList(
            RefPtr<Type>                toType,
            RefPtr<Expr>*               outToExpr,
            RefPtr<InitializerListExpr> fromInitializerListExpr,
            UInt                       &ioInitArgIndex)
        {
            // First, we will check if we have run out of arguments
            // on the initializer list.
            //
            UInt initArgCount = fromInitializerListExpr->args.getCount();
            if(ioInitArgIndex >= initArgCount)
            {
                // If we are at the end of the initializer list,
                // then our ability to read an argument depends
                // on whether the type we are trying to read
                // is default-initializable.
                //
                // For now, we will just pretend like everything
                // is default-initializable and move along.
                return true;
            }

            // Okay, we have at least one initializer list expression,
            // so we will look at the next expression and decide
            // whether to use it to initialize the desired type
            // directly (possibly via casts), or as the first sub-expression
            // for aggregate initialization.
            //
            auto firstInitExpr = fromInitializerListExpr->args[ioInitArgIndex];
            if(shouldUseInitializerDirectly(toType, firstInitExpr))
            {
                ioInitArgIndex++;
                return _coerce(
                    toType,
                    outToExpr,
                    firstInitExpr->type,
                    firstInitExpr,
                    nullptr);
            }

            // If there is somehow an error in one of the initialization
            // expressions, then everything could be thrown off and we
            // shouldn't keep trying to read arguments.
            //
            if( IsErrorExpr(firstInitExpr) )
            {
                // Stop reading arguments, as if we'd reached
                // the end of the list.
                //
                ioInitArgIndex = initArgCount;
                return true;
            }

            // The fallback case is to recursively read the
            // type from the same list as an aggregate.
            //
            return _readAggregateValueFromInitializerList(
                toType,
                outToExpr,
                fromInitializerListExpr,
                ioInitArgIndex);
        }

            /// Read an aggregate value from an initializer list expression.
            ///
            /// This reads one or more arguments from the initializer list
            /// given as `fromInitializerListExpr` to initialize the
            /// fields/elements of a value of type `toType`.
            ///
            /// This routine returns `true` if it was able to read
            /// arguments that can form a value of type `toType`,
            /// and `false` otherwise.
            ///
            /// If the routine succeeds and `outToExpr` is non-null,
            /// then it will be filled in with an expression
            /// representing the value (or type `toType`) that was read,
            /// or it will be left null to indicate that a default
            /// value should be used.
            ///
            /// If the routine fails and `outToExpr` is non-null,
            /// then a suitable diagnostic will be emitted.
            ///
        bool _readAggregateValueFromInitializerList(
            RefPtr<Type>                inToType,
            RefPtr<Expr>*               outToExpr,
            RefPtr<InitializerListExpr> fromInitializerListExpr,
            UInt                       &ioArgIndex)
        {
            auto toType = inToType;
            UInt argCount = fromInitializerListExpr->args.getCount();

            // In the case where we need to build a result expression,
            // we will collect the new arguments here
            List<RefPtr<Expr>> coercedArgs;

            if(isEffectivelyScalarForInitializerLists(toType))
            {
                // For any type that is effectively a non-aggregate,
                // we expect to read a single value from the initializer list
                //
                if(ioArgIndex < argCount)
                {
                    auto arg = fromInitializerListExpr->args[ioArgIndex++];
                    return _coerce(
                        toType,
                        outToExpr,
                        arg->type,
                        arg,
                        nullptr);
                }
                else
                {
                    // If there wasn't an initialization
                    // expression to be found, then we need
                    // to perform default initialization here.
                    //
                    // We will let this case come through the front-end
                    // as an `InitializerListExpr` with zero arguments,
                    // and then have the IR generation logic deal with
                    // synthesizing default values.
                }
            }
            else if (auto toVecType = as<VectorExpressionType>(toType))
            {
                auto toElementCount = toVecType->elementCount;
                auto toElementType = toVecType->elementType;

                UInt elementCount = 0;
                if (auto constElementCount = as<ConstantIntVal>(toElementCount))
                {
                    elementCount = (UInt) constElementCount->value;
                }
                else
                {
                    // We don't know the element count statically,
                    // so what are we supposed to be doing?
                    //
                    if(outToExpr)
                    {
                        getSink()->diagnose(fromInitializerListExpr, Diagnostics::cannotUseInitializerListForVectorOfUnknownSize, toElementCount);
                    }
                    return false;
                }

                for(UInt ee = 0; ee < elementCount; ++ee)
                {
                    RefPtr<Expr> coercedArg;
                    bool argResult = _readValueFromInitializerList(
                        toElementType,
                        outToExpr ? &coercedArg : nullptr,
                        fromInitializerListExpr,
                        ioArgIndex);

                    // No point in trying further if any argument fails
                    if(!argResult)
                        return false;

                    if( coercedArg )
                    {
                        coercedArgs.add(coercedArg);
                    }
                }
            }
            else if(auto toArrayType = as<ArrayExpressionType>(toType))
            {
                // TODO(tfoley): If we can compute the size of the array statically,
                // then we want to check that there aren't too many initializers present

                auto toElementType = toArrayType->baseType;

                if(auto toElementCount = toArrayType->ArrayLength)
                {
                    // In the case of a sized array, we need to check that the number
                    // of elements being initialized matches what was declared.
                    //
                    UInt elementCount = 0;
                    if (auto constElementCount = as<ConstantIntVal>(toElementCount))
                    {
                        elementCount = (UInt) constElementCount->value;
                    }
                    else
                    {
                        // We don't know the element count statically,
                        // so what are we supposed to be doing?
                        //
                        if(outToExpr)
                        {
                            getSink()->diagnose(fromInitializerListExpr, Diagnostics::cannotUseInitializerListForArrayOfUnknownSize, toElementCount);
                        }
                        return false;
                    }

                    for(UInt ee = 0; ee < elementCount; ++ee)
                    {
                        RefPtr<Expr> coercedArg;
                        bool argResult = _readValueFromInitializerList(
                            toElementType,
                            outToExpr ? &coercedArg : nullptr,
                            fromInitializerListExpr,
                            ioArgIndex);

                        // No point in trying further if any argument fails
                        if(!argResult)
                            return false;

                        if( coercedArg )
                        {
                            coercedArgs.add(coercedArg);
                        }
                    }
                }
                else
                {
                    // In the case of an unsized array type, we will use the
                    // number of arguments to the initializer to determine
                    // the element count.
                    //
                    UInt elementCount = 0;
                    while(ioArgIndex < argCount)
                    {
                        RefPtr<Expr> coercedArg;
                        bool argResult = _readValueFromInitializerList(
                            toElementType,
                            outToExpr ? &coercedArg : nullptr,
                            fromInitializerListExpr,
                            ioArgIndex);

                        // No point in trying further if any argument fails
                        if(!argResult)
                            return false;

                        elementCount++;

                        if( coercedArg )
                        {
                            coercedArgs.add(coercedArg);
                        }
                    }

                    // We have a new type for the conversion, based on what
                    // we learned.
                    toType = getSession()->getArrayType(
                        toElementType,
                        new ConstantIntVal(elementCount));
                }
            }
            else if(auto toMatrixType = as<MatrixExpressionType>(toType))
            {
                // In the general case, the initializer list might comprise
                // both vectors and scalars.
                //
                // The traditional HLSL compilers treat any vectors in
                // the initializer list exactly equivalent to their sequence
                // of scalar elements, and don't care how this might, or
                // might not, align with the rows of the matrix.
                //
                // We will draw a line in the sand and say that an initializer
                // list for a matrix will act as if the matrix type were an
                // array of vectors for the rows.


                UInt rowCount = 0;
                auto toRowType = createVectorType(
                    toMatrixType->getElementType(),
                    toMatrixType->getColumnCount());

                if (auto constRowCount = as<ConstantIntVal>(toMatrixType->getRowCount()))
                {
                    rowCount = (UInt) constRowCount->value;
                }
                else
                {
                    // We don't know the element count statically,
                    // so what are we supposed to be doing?
                    //
                    if(outToExpr)
                    {
                        getSink()->diagnose(fromInitializerListExpr, Diagnostics::cannotUseInitializerListForMatrixOfUnknownSize, toMatrixType->getRowCount());
                    }
                    return false;
                }

                for(UInt rr = 0; rr < rowCount; ++rr)
                {
                    RefPtr<Expr> coercedArg;
                    bool argResult = _readValueFromInitializerList(
                        toRowType,
                        outToExpr ? &coercedArg : nullptr,
                        fromInitializerListExpr,
                        ioArgIndex);

                    // No point in trying further if any argument fails
                    if(!argResult)
                        return false;

                    if( coercedArg )
                    {
                        coercedArgs.add(coercedArg);
                    }
                }
            }
            else if(auto toDeclRefType = as<DeclRefType>(toType))
            {
                auto toTypeDeclRef = toDeclRefType->declRef;
                if(auto toStructDeclRef = toTypeDeclRef.as<StructDecl>())
                {
                    // Trying to initialize a `struct` type given an initializer list.
                    // We will go through the fields in order and try to match them
                    // up with initializer arguments.
                    //
                    for(auto fieldDeclRef : getMembersOfType<VarDecl>(toStructDeclRef))
                    {
                        RefPtr<Expr> coercedArg;
                        bool argResult = _readValueFromInitializerList(
                            GetType(fieldDeclRef),
                            outToExpr ? &coercedArg : nullptr,
                            fromInitializerListExpr,
                            ioArgIndex);

                        // No point in trying further if any argument fails
                        if(!argResult)
                            return false;

                        if( coercedArg )
                        {
                            coercedArgs.add(coercedArg);
                        }
                    }
                }
            }
            else
            {
                // We shouldn't get to this case in practice,
                // but just in case we'll consider an initializer
                // list invalid if we are trying to read something
                // off of it that wasn't handled by the cases above.
                //
                if(outToExpr)
                {
                    getSink()->diagnose(fromInitializerListExpr, Diagnostics::cannotUseInitializerListForType, inToType);
                }
                return false;
            }

            // We were able to coerce all the arguments given, and so
            // we need to construct a suitable expression to remember the result
            //
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

            /// Coerce an initializer-list expression to a specific type.
            ///
            /// This reads one or more arguments from the initializer list
            /// given as `fromInitializerListExpr` to initialize the
            /// fields/elements of a value of type `toType`.
            ///
            /// This routine returns `true` if it was able to read
            /// arguments that can form a value of type `toType`,
            /// with no arguments left over, and `false` otherwise.
            ///
            /// If the routine succeeds and `outToExpr` is non-null,
            /// then it will be filled in with an expression
            /// representing the value (or type `toType`) that was read,
            /// or it will be left null to indicate that a default
            /// value should be used.
            ///
            /// If the routine fails and `outToExpr` is non-null,
            /// then a suitable diagnostic will be emitted.
            ///
        bool _coerceInitializerList(
            RefPtr<Type>                toType,
            RefPtr<Expr>*               outToExpr,
            RefPtr<InitializerListExpr> fromInitializerListExpr)
        {
            UInt argCount = fromInitializerListExpr->args.getCount();
            UInt argIndex = 0;

            // TODO: we should handle the special case of `{0}` as an initializer
            // for arbitrary `struct` types here.

            if(!_readAggregateValueFromInitializerList(toType, outToExpr, fromInitializerListExpr, argIndex))
                return false;

            if(argIndex != argCount)
            {
                if( outToExpr )
                {
                    getSink()->diagnose(fromInitializerListExpr, Diagnostics::tooManyInitializers, argIndex, argCount);
                }
            }

            return true;
        }

            /// Report that implicit type coercion is not possible.
        bool _failedCoercion(
            RefPtr<Type>    toType,
            RefPtr<Expr>*   outToExpr,
            RefPtr<Expr>    fromExpr)
        {
            if(outToExpr)
            {
                getSink()->diagnose(fromExpr->loc, Diagnostics::typeMismatch, toType, fromExpr->type);
            }
            return false;
        }

            /// Central engine for implementing implicit coercion logic
            ///
            /// This function tries to find an implicit conversion path from
            /// `fromType` to `toType`. It returns `true` if a conversion
            /// is found, and `false` if not.
            ///
            /// If a conversion is found, then its cost will be written to `outCost`.
            ///
            /// If a `fromExpr` is provided, it must be of type `fromType`,
            /// and represent a value to be converted.
            ///
            /// If `outToExpr` is non-null, and if a conversion is found, then
            /// `*outToExpr` will be set to an expression that performs the
            /// implicit conversion of `fromExpr` (which must be non-null
            /// to `toType`).
            ///
            /// The case where `outToExpr` is non-null is used to identify
            /// when a conversion is being done "for real" so that diagnostics
            /// should be emitted on failure.
            ///
        bool _coerce(
            RefPtr<Type>    toType,
            RefPtr<Expr>*   outToExpr,
            RefPtr<Type>    fromType,
            RefPtr<Expr>    fromExpr,
            ConversionCost* outCost)
        {
            // An important and easy case is when the "to" and "from" types are equal.
            //
            if( toType->Equals(fromType) )
            {
                if(outToExpr)
                    *outToExpr = fromExpr;
                if(outCost)
                    *outCost = kConversionCost_None;
                return true;
            }

            // Another important case is when either the "to" or "from" type
            // represents an error. In such a case we must have already
            // reporeted the error, so it is better to allow the conversion
            // to pass than to report a "cascading" error that might not
            // make any sense.
            //
            if(as<ErrorType>(toType) || as<ErrorType>(fromType))
            {
                if(outToExpr)
                    *outToExpr = CreateImplicitCastExpr(toType, fromExpr);
                if(outCost)
                    *outCost = kConversionCost_None;
                return true;
            }

            // Coercion from an initializer list is allowed for many types,
            // so we will farm that out to its own subroutine.
            //
            if( auto fromInitializerListExpr = as<InitializerListExpr>(fromExpr))
            {
                if( !_coerceInitializerList(
                    toType,
                    outToExpr,
                    fromInitializerListExpr) )
                {
                    return false;
                }

                // For now, we treat coercion from an initializer list
                // as having  no cost, so that all conversions from initializer
                // lists are equally valid. This is fine given where initializer
                // lists are allowed to appear now, but might need to be made
                // more strict if we allow for initializer lists in more
                // places in the language (e.g., as function arguments).
                //
                if(outCost)
                {
                    *outCost = kConversionCost_None;
                }

                return true;
            }

            // If we are casting to an interface type, then that will succeed
            // if the "from" type conforms to the interface.
            //
            if (auto toDeclRefType = as<DeclRefType>(toType))
            {
                auto toTypeDeclRef = toDeclRefType->declRef;
                if (auto interfaceDeclRef = toTypeDeclRef.as<InterfaceDecl>())
                {
                    if(auto witness = tryGetInterfaceConformanceWitness(fromType, interfaceDeclRef))
                    {
                        if (outToExpr)
                            *outToExpr = createCastToInterfaceExpr(toType, fromExpr, witness);
                        if (outCost)
                            *outCost = kConversionCost_CastToInterface;
                        return true;
                    }
                }
            }

            // We allow implicit conversion of a parameter group type like
            // `ConstantBuffer<X>` or `ParameterBlock<X>` to its element
            // type `X`.
            //
            if(auto fromParameterGroupType = as<ParameterGroupType>(fromType))
            {
                auto fromElementType = fromParameterGroupType->getElementType();

                // If we convert, e.g., `ConstantBuffer<A> to `A`, we will allow
                // subsequent conversion of `A` to `B` if such a conversion
                // is possible.
                //
                ConversionCost subCost = kConversionCost_None;

                RefPtr<DerefExpr> derefExpr;
                if(outToExpr)
                {
                    derefExpr = new DerefExpr();
                    derefExpr->base = fromExpr;
                    derefExpr->type = QualType(fromElementType);
                }

                if(!_coerce(
                    toType,
                    outToExpr,
                    fromElementType,
                    derefExpr,
                    &subCost))
                {
                    return false;
                }

                if(outCost)
                    *outCost = subCost + kConversionCost_ImplicitDereference;
                return true;
            }

            // The main general-purpose approach for conversion is
            // using suitable marked initializer ("constructor")
            // declarations on the target type.
            //
            // This is treated as a form of overload resolution,
            // since we are effectively forming an overloaded
            // call to one of the initializers in the target type.

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

            // After all of the overload candidates have been added
            // to the context and processed, we need to see whether
            // there was one best overload or not.
            //
            if(overloadContext.bestCandidates.getCount() != 0)
            {
                // In this case there were multiple equally-good candidates to call.
                //
                // We will start by checking if the candidates are
                // even applicable, because if not, then we shouldn't
                // consider the conversion as possible.
                //
                if(overloadContext.bestCandidates[0].status != OverloadCandidate::Status::Applicable)
                    return _failedCoercion(toType, outToExpr, fromExpr);

                // If all of the candidates in `bestCandidates` are applicable,
                // then we have an ambiguity.
                //
                // We will compute a nominal conversion cost as the minimum over
                // all the conversions available.
                //
                ConversionCost bestCost = kConversionCost_Explicit;
                for(auto candidate : overloadContext.bestCandidates)
                {
                    ConversionCost candidateCost = getImplicitConversionCost(
                        candidate.item.declRef.getDecl());

                    if(candidateCost < bestCost)
                        bestCost = candidateCost;
                }

                // Conceptually, we want to treat the conversion as
                // possible, but report it as ambiguous if we actually
                // need to reify the result as an expression.
                //
                if(outToExpr)
                {
                    getSink()->diagnose(fromExpr, Diagnostics::ambiguousConversion, fromType, toType);

                    *outToExpr = CreateErrorExpr(fromExpr);
                }

                if(outCost)
                    *outCost = bestCost;

                return true;
            }
            else if(overloadContext.bestCandidate)
            {
                // If there is a single best candidate for conversion,
                // then we want to use it.
                //
                // It is possible that there was a single best candidate,
                // but it wasn't actually usable, so we will check for
                // that case first.
                //
                if(overloadContext.bestCandidate->status != OverloadCandidate::Status::Applicable)
                    return _failedCoercion(toType, outToExpr, fromExpr);

                // Next, we need to look at the implicit conversion
                // cost associated with the initializer we are invoking.
                //
                ConversionCost cost = getImplicitConversionCost(
                        overloadContext.bestCandidate->item.declRef.getDecl());;

                // If the cost is too high to be usable as an
                // implicit conversion, then we will report the
                // conversion as possible (so that an overload involving
                // this conversion will be selected over one without),
                // but then emit a diagnostic when actually reifying
                // the result expression.
                //
                if( cost >= kConversionCost_Explicit )
                {
                    if( outToExpr )
                    {
                        getSink()->diagnose(fromExpr, Diagnostics::typeMismatch, toType, fromType);
                        getSink()->diagnose(fromExpr, Diagnostics::noteExplicitConversionPossible, fromType, toType);
                    }
                }

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
                    castExpr->Arguments.add(fromExpr);
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
                    castExpr->Arguments.clear();
                    castExpr->Arguments.add(fromExpr);
                }

                return true;
            }

            return _failedCoercion(toType, outToExpr, fromExpr);
        }

            /// Check whether implicit type coercion from `fromType` to `toType` is possible.
            ///
            /// If conversion is possible, returns `true` and sets `outCost` to the cost
            /// of the conversion found (if `outCost` is non-null).
            ///
            /// If conversion is not possible, returns `false`.
            ///
        bool canCoerce(
            RefPtr<Type>    toType,
            RefPtr<Type>    fromType,
            ConversionCost* outCost = 0)
        {
            // As an optimization, we will maintain a cache of conversion results
            // for basic types such as scalars and vectors.
            //
            BasicTypeKey key1, key2;
            BasicTypeKeyPair cacheKey;
            bool shouldAddToCache = false;
            ConversionCost cost;
            TypeCheckingCache* typeCheckingCache = getSession()->getTypeCheckingCache();
            if( key1.fromType(toType.Ptr()) && key2.fromType(fromType.Ptr()) )
            {
                cacheKey.type1 = key1;
                cacheKey.type2 = key2;

                if (typeCheckingCache->conversionCostCache.TryGetValue(cacheKey, cost))
                {
                    if (outCost)
                        *outCost = cost;
                    return cost != kConversionCost_Impossible;
                }
                else
                    shouldAddToCache = true;
            }

            // If there was no suitable entry in the cache,
            // then we fall back to the general-purpose
            // conversion checking logic.
            //
            // Note that we are passing in `nullptr` as
            // the output expression to be constructed,
            // which suppresses emission of any diagnostics
            // during the coercion process.
            //
            bool rs = _coerce(
                toType,
                nullptr,
                fromType,
                nullptr,
                &cost);

            if (outCost)
                *outCost = cost;

            if (shouldAddToCache)
            {
                if (!rs)
                    cost = kConversionCost_Impossible;
                typeCheckingCache->conversionCostCache[cacheKey] = cost;
            }

            return rs;
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

            auto typeType = getTypeType(toType);

            auto typeExpr = new SharedTypeExpr();
            typeExpr->type.type = typeType;
            typeExpr->base.type = toType;

            castExpr->loc = fromExpr->loc;
            castExpr->FunctionExpr = typeExpr;
            castExpr->type = QualType(toType);
            castExpr->Arguments.add(fromExpr);
            return castExpr;
        }

            /// Create an "up-cast" from a value to an interface type
            ///
            /// This operation logically constructs an "existential" value,
            /// which packages up the value, its type, and the witness
            /// of its conformance to the interface.
            ///
        RefPtr<Expr> createCastToInterfaceExpr(
            RefPtr<Type>    toType,
            RefPtr<Expr>    fromExpr,
            RefPtr<Val>     witness)
        {
            RefPtr<CastToInterfaceExpr> expr = new CastToInterfaceExpr();
            expr->loc = fromExpr->loc;
            expr->type = QualType(toType);
            expr->valueArg = fromExpr;
            expr->witnessArg = witness;
            return expr;
        }

            /// Implicitly coerce `fromExpr` to `toType` and diagnose errors if it isn't possible
        RefPtr<Expr> coerce(
            RefPtr<Type>    toType,
            RefPtr<Expr>    fromExpr)
        {
            RefPtr<Expr> expr;
            if (!_coerce(
                toType,
                &expr,
                fromExpr->type.Ptr(),
                fromExpr.Ptr(),
                nullptr))
            {
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
                if (function || checkingPhase == CheckingPhase::Header)
                {
                    TypeExp typeExp = CheckUsableType(varDecl->type);
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
        void CheckConstraintSubType(TypeExp& typeExp)
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

        RefPtr<ConstantIntVal> checkConstantIntVal(
            RefPtr<Expr>    expr)
        {
            // First type-check the expression as normal
            expr = CheckExpr(expr);

            auto intVal = CheckIntegerConstantExpression(expr.Ptr());
            if(!intVal)
                return nullptr;

            auto constIntVal = as<ConstantIntVal>(intVal);
            if(!constIntVal)
            {
                getSink()->diagnose(expr->loc, Diagnostics::expectedIntegerConstantNotLiteral);
                return nullptr;
            }
            return constIntVal;
        }

        RefPtr<ConstantIntVal> checkConstantEnumVal(
            RefPtr<Expr>    expr)
        {
            // First type-check the expression as normal
            expr = CheckExpr(expr);

            auto intVal = CheckEnumConstantExpression(expr.Ptr());
            if(!intVal)
                return nullptr;

            auto constIntVal = as<ConstantIntVal>(intVal);
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

            if(auto stringLitExpr = as<StringLiteralExpr>(expr))
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

            // If the result was overloaded,
            // then we aren't going to be able to extract a single decl.
            if(lookupResult.isOverloaded())
                return nullptr;

            if (lookupResult.isValid())
            {
                auto decl = lookupResult.item.declRef.getDecl();
                if (auto attributeDecl = as<AttributeDecl>(decl))
                {
                    return attributeDecl;
                }
                else
                {
                    return nullptr;
                }
            }

            // If we couldn't find a system attribute, try looking up as a user defined attribute
            // A user defined attribute class is defined as a struct type with a "UserDefinedAttributeAttribute" modifier
            lookupResult = lookUp(getSession(), this, getSession()->getNameObj(attributeName->text + "Attribute"), scope, LookupMask::type);
            if (lookupResult.isOverloaded())
            {
                // see if we have already created an AttributeDecl for this attribute struct
                for (auto alt : lookupResult.items)
                {
                    if (auto adecl = alt.declRef.as<AttributeDecl>())
                        return adecl.getDecl();
                }
            }
            // If we still cannot find any thing, quit
            if (!lookupResult.isValid() || lookupResult.isOverloaded())
                return nullptr;
            // Now construct an AttributeDecl for this user defined attribute class for future lookup
            auto userDefAttribAttrib = lookupResult.item.declRef.decl->FindModifier<AttributeUsageAttribute>();
            if (!userDefAttribAttrib)
                return nullptr;
            // create an AttributeDecl for the user defined attribute
            auto structAttribDef = lookupResult.item.declRef.as<StructDecl>().getDecl();
            RefPtr<AttributeDecl> attribDecl = new AttributeDecl();
            attribDecl->nameAndLoc = structAttribDef->nameAndLoc;
            attribDecl->loc = structAttribDef->loc;
            attribDecl->nextInContainerWithSameName = structAttribDef->nextInContainerWithSameName;
            // create a __attributeTarget modifier for the attribute class definition
            RefPtr<AttributeTargetModifier> targetModifier = new AttributeTargetModifier();
            targetModifier->syntaxClass = userDefAttribAttrib->targetSyntaxClass;
            targetModifier->loc = structAttribDef->loc;
            targetModifier->next = attribDecl->modifiers.first;
            attribDecl->modifiers.first = targetModifier;
            structAttribDef->nextInContainerWithSameName = attribDecl.Ptr();
            // we should create UserDefinedAttribute nodes for all user defined attribute instances
            attribDecl->syntaxClass = getSession()->findSyntaxClass(getSession()->getNameObj("UserDefinedAttribute"));
            for (auto member : structAttribDef->Members)
            {
                if (auto varMember = as<VarDecl>(member))
                {
                    RefPtr<ParamDecl> param = new ParamDecl();
                    param->nameAndLoc = member->nameAndLoc;
                    param->type = varMember->type;
                    param->loc = member->loc;
                    attribDecl->Members.add(param);
                }
            }
            // add the attribute class definition to the syntax tree, so it can be found
            structAttribDef->ParentDecl->Members.add(attribDecl.Ptr());
            structAttribDef->ParentDecl->memberDictionaryIsValid = false;
            // do necessary checks on this newly constructed node
            checkDecl(attribDecl.Ptr());
            return attribDecl.Ptr();
        }

        bool hasIntArgs(Attribute* attr, int numArgs)
        {
            if (int(attr->args.getCount()) != numArgs)
            {
                return false;
            }
            for (int i = 0; i < numArgs; ++i)
            {
                if (!as<IntegerLiteralExpr>(attr->args[i]))
                {
                    return false;
                }
            }
            return true;
        }

        bool hasStringArgs(Attribute* attr, int numArgs)
        {
            if (int(attr->args.getCount()) != numArgs)
            {
                return false;
            }
            for (int i = 0; i < numArgs; ++i)
            {
                if (!as<StringLiteralExpr>(attr->args[i]))
                {
                    return false;
                }
            }
            return true;
        }

        bool getAttributeTargetSyntaxClasses(SyntaxClass<RefObject> & cls, uint32_t typeFlags)
        {
            if (typeFlags == (int)UserDefinedAttributeTargets::Struct)
            {
                cls = getSession()->findSyntaxClass(getSession()->getNameObj("StructDecl"));
                return true;
            }
            if (typeFlags == (int)UserDefinedAttributeTargets::Var)
            {
                cls = getSession()->findSyntaxClass(getSession()->getNameObj("VarDecl"));
                return true;
            }
            if (typeFlags == (int)UserDefinedAttributeTargets::Function)
            {
                cls = getSession()->findSyntaxClass(getSession()->getNameObj("FuncDecl"));
                return true;
            }
            return false;
        }

        bool validateAttribute(RefPtr<Attribute> attr, AttributeDecl* attribClassDecl)
        {
                if(auto numThreadsAttr = as<NumThreadsAttribute>(attr))
                {
                    SLANG_ASSERT(attr->args.getCount() == 3);
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
                else if (auto bindingAttr = as<GLSLBindingAttribute>(attr))
                {
                    // This must be vk::binding or gl::binding (as specified in core.meta.slang under vk_binding/gl_binding)
                    // Must have 2 int parameters. Ideally this would all be checked from the specification
                    // in core.meta.slang, but that's not completely implemented. So for now we check here.
                    if (attr->args.getCount() != 2)
                    {
                        return false;
                    }

                    // TODO(JS): Prior validation currently doesn't ensure both args are ints (as specified in core.meta.slang), so check here
                    // to make sure they both are
                    auto binding = checkConstantIntVal(attr->args[0]);
                    auto set = checkConstantIntVal(attr->args[1]);

                    if (binding == nullptr || set == nullptr)
                    {
                        return false;
                    }
                    
                    bindingAttr->binding = int32_t(binding->value);
                    bindingAttr->set = int32_t(set->value);
                }
                else if (auto maxVertexCountAttr = as<MaxVertexCountAttribute>(attr))
                {
                    SLANG_ASSERT(attr->args.getCount() == 1);
                    auto val = checkConstantIntVal(attr->args[0]);

                    if(!val) return false;

                    maxVertexCountAttr->value = (int32_t)val->value;
                }
                else if(auto instanceAttr = as<InstanceAttribute>(attr))
                {
                    SLANG_ASSERT(attr->args.getCount() == 1);
                    auto val = checkConstantIntVal(attr->args[0]);

                    if(!val) return false;

                    instanceAttr->value = (int32_t)val->value;
                }
                else if(auto entryPointAttr = as<EntryPointAttribute>(attr))
                {
                    SLANG_ASSERT(attr->args.getCount() == 1);

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
                else if ((as<DomainAttribute>(attr)) ||
                         (as<MaxTessFactorAttribute>(attr)) ||
                         (as<OutputTopologyAttribute>(attr)) ||
                         (as<PartitioningAttribute>(attr)) ||
                         (as<PatchConstantFuncAttribute>(attr)))
                {
                    // Let it go thru iff single string attribute
                    if (!hasStringArgs(attr, 1))
                    {
                        getSink()->diagnose(attr, Diagnostics::expectedSingleStringArg, attr->name);
                    }
                }
                else if (as<OutputControlPointsAttribute>(attr))
                {
                    // Let it go thru iff single integral attribute
                    if (!hasIntArgs(attr, 1))
                    {
                        getSink()->diagnose(attr, Diagnostics::expectedSingleIntArg, attr->name);
                    }
                }
                else if (as<PushConstantAttribute>(attr))
                {
                    // Has no args
                    SLANG_ASSERT(attr->args.getCount() == 0);
                }
                else if (as<ShaderRecordAttribute>(attr))
                {
                    // Has no args
                    SLANG_ASSERT(attr->args.getCount() == 0);
                }
                else if (as<EarlyDepthStencilAttribute>(attr))
                {
                    // Has no args
                    SLANG_ASSERT(attr->args.getCount() == 0);
                }
                else if (auto attrUsageAttr = as<AttributeUsageAttribute>(attr))
                {
                    uint32_t targetClassId = (uint32_t)UserDefinedAttributeTargets::None;
                    if (attr->args.getCount() == 1)
                    {
                        RefPtr<IntVal> outIntVal;
                        if (auto cInt = checkConstantEnumVal(attr->args[0]))
                        {
                            targetClassId = (uint32_t)(cInt->value);
                        }
                        else
                        {
                            getSink()->diagnose(attr, Diagnostics::expectedSingleIntArg, attr->name);
                            return false;
                        }
                    }
                    if (!getAttributeTargetSyntaxClasses(attrUsageAttr->targetSyntaxClass, targetClassId))
                    {
                        getSink()->diagnose(attr, Diagnostics::invalidAttributeTarget);
                        return false;
                    }
                }
                else if (auto unrollAttr = as<UnrollAttribute>(attr))
                {
                    // Check has an argument. We need this because default behavior is to give an error
                    // if an attribute has arguments, but not handled explicitly (and the default param will come through
                    // as 1 arg if nothing is specified)
                    SLANG_ASSERT(attr->args.getCount() == 1);
                }
                else if (auto userDefAttr = as<UserDefinedAttribute>(attr))
                {
                    // check arguments against attribute parameters defined in attribClassDecl
                    Index paramIndex = 0;
                    auto params = attribClassDecl->getMembersOfType<ParamDecl>();
                    for (auto paramDecl : params)
                    {
                        if (paramIndex < attr->args.getCount())
                        {
                            auto & arg = attr->args[paramIndex];
                            bool typeChecked = false;
                            if (auto basicType = as<BasicExpressionType>(paramDecl->getType()))
                            {
                                if (basicType->baseType == BaseType::Int)
                                {
                                    if (auto cint = checkConstantIntVal(arg))
                                    {
                                        attr->intArgVals[(uint32_t)paramIndex] = cint;
                                    }
                                    typeChecked = true;
                                }
                            }
                            if (!typeChecked)
                            {
                                arg = CheckExpr(arg);
                                arg = coerce(paramDecl->getType(), arg);
                            }
                        }
                        paramIndex++;
                    }
                    if (params.getCount() < attr->args.getCount())
                    {
                        getSink()->diagnose(attr, Diagnostics::tooManyArguments, attr->args.getCount(), params.getCount());
                    }
                    else if (params.getCount() > attr->args.getCount())
                    {
                        getSink()->diagnose(attr, Diagnostics::notEnoughArguments, attr->args.getCount(), params.getCount());
                    }
                }
                else if (auto formatAttr = as<FormatAttribute>(attr))
                {
                    SLANG_ASSERT(attr->args.getCount() == 1);

                    String formatName;
                    if(!checkLiteralStringVal(attr->args[0], &formatName))
                    {
                        return false;
                    }

                    ImageFormat format = ImageFormat::unknown;
                    if(!findImageFormatByName(formatName.getBuffer(), &format))
                    {
                        getSink()->diagnose(attr->args[0], Diagnostics::unknownImageFormatName, formatName);
                    }

                    formatAttr->format = format;
                }
                else if (auto allowAttr = as<AllowAttribute>(attr))
                {
                    SLANG_ASSERT(attr->args.getCount() == 1);

                    String diagnosticName;
                    if(!checkLiteralStringVal(attr->args[0], &diagnosticName))
                    {
                        return false;
                    }

                    auto diagnosticInfo = findDiagnosticByName(diagnosticName.getUnownedSlice());
                    if(!diagnosticInfo)
                    {
                        getSink()->diagnose(attr->args[0], Diagnostics::unknownDiagnosticName, diagnosticName);
                    }

                    allowAttr->diagnostic = diagnosticInfo;
                }
                else
                {
                    if(attr->args.getCount() == 0)
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

            // Manage scope
            RefPtr<RefObject> attrInstance = attrDecl->syntaxClass.createInstance();
            auto attr = attrInstance.as<Attribute>();
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
            UInt argCount = attr->args.getCount();
            UInt paramCounter = 0;
            bool mismatch = false;
            for(auto paramDecl : attrDecl->getMembersOfType<ParamDecl>())
            {
                UInt paramIndex = paramCounter++;
                if( paramIndex < argCount )
                {
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
                        // For now just copy the expression over.

                        attr->args.add(paramDecl->initExpr);
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
            if(!validateAttribute(attr, attrDecl))
            {
                return uncheckedAttr;
            }


            return attr;
        }

        RefPtr<Modifier> checkModifier(
            RefPtr<Modifier>        m,
            ModifiableSyntaxNode*   syntaxNode)
        {
            if(auto hlslUncheckedAttribute = as<UncheckedAttribute>(m))
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

            /// Perform checking of interface conformaces for this decl and all its children
        void checkInterfaceConformancesRec(Decl* decl)
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

        void visitModuleDecl(ModuleDecl* programNode)
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

        bool doesSignatureMatchRequirement(
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

        bool doesGenericSignatureMatchRequirement(
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

        bool doesTypeSatisfyAssociatedTypeRequirement(
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

        // Does the given `memberDecl` work as an implementation
        // to satisfy the requirement `requiredMemberDeclRef`
        // from an interface?
        //
        // If it does, then inserts a witness into `witnessTable`
        // and returns `true`, otherwise returns `false`
        bool doesMemberSatisfyRequirement(
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

        // State used while checking if a declaration (either a type declaration
        // or an extension of that type) conforms to the interfaces it claims
        // via its inheritance clauses.
        //
        struct ConformanceCheckingContext
        {
            Dictionary<DeclRef<InterfaceDecl>, RefPtr<WitnessTable>>    mapInterfaceToWitnessTable;
        };

        // Find the appropriate member of a declared type to
        // satisfy a requirement of an interface the type
        // claims to conform to.
        //
        // The type declaration `typeDecl` has declared that it
        // conforms to the interface `interfaceDeclRef`, and
        // `requiredMemberDeclRef` is a required member of
        // the interface.
        //
        // If a satisfying value is found, registers it in
        // `witnessTable` and returns `true`, otherwise
        // returns `false`.
        //
        bool findWitnessForInterfaceRequirement(
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

        // Check that the type declaration `typeDecl`, which
        // declares conformance to the interface `interfaceDeclRef`,
        // (via the given `inheritanceDecl`) actually provides
        // members to satisfy all the requirements in the interface.
        RefPtr<WitnessTable> checkInterfaceConformance(
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

        RefPtr<WitnessTable> checkConformanceToType(
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

        // Check that the type (or extension) declaration `declRef`,
        // which declares that it inherits from another type via
        // `inheritanceDecl` actually does what it needs to
        // for that inheritance to be valid.
        bool checkConformance(
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

        void checkExtensionConformance(ExtensionDecl* decl)
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

        void checkAggTypeConformance(AggTypeDecl* decl)
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

        bool isIntegerBaseType(BaseType baseType)
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

        // Validate that `type` is a suitable type to use
        // as the tag type for an `enum`
        void validateEnumTagType(Type* type, SourceLoc const& loc)
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

        void visitEnumDecl(EnumDecl* decl)
        {
            if (decl->IsChecked(getCheckedState()))
                return;

            // We need to be careful to avoid recursion in the
            // type-checking logic. We will do the minimal work
            // to make the type usable in the first phase, and
            // then check the actual cases in the second phase.
            //
            if(this->checkingPhase == CheckingPhase::Header)
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
            else if( checkingPhase == CheckingPhase::Body )
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

        void visitEnumCaseDecl(EnumCaseDecl* decl)
        {
            if (decl->IsChecked(getCheckedState()))
                return;

            if(checkingPhase == CheckingPhase::Body)
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

        void visitDeclGroup(DeclGroup* declGroup)
        {
            for (auto decl : declGroup->decls)
            {
                dispatchDecl(decl);
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

        void visitGlobalGenericParamDecl(GlobalGenericParamDecl* decl)
        {
            if (decl->IsChecked(getCheckedState())) return;
            if (checkingPhase == CheckingPhase::Header)
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

        void visitAssocTypeDecl(AssocTypeDecl* decl)
        {
            if (decl->IsChecked(getCheckedState())) return;
            if (checkingPhase == CheckingPhase::Header)
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

        void checkStmt(Stmt* stmt)
        {
            if (!stmt) return;
            dispatchStmt(stmt);
            checkModifiers(stmt);
        }

        void visitFuncDecl(FuncDecl* functionNode)
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

                if (auto typeParamDecl = as<GenericTypeParamDecl>(dd))
                    outParams.add(typeParamDecl);
                else if (auto valueParamDecl = as<GenericValueParamDecl>(dd))
                    outParams.add(valueParamDecl);
                else if (auto constraintDecl = as<GenericTypeConstraintDecl>(dd))
                    outConstraints.add(constraintDecl);
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

        RefPtr<GenericSubstitution> createDummySubstitutions(
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

        void ValidateFunctionRedeclaration(FuncDecl* funcDecl)
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

        void visitScopeDecl(ScopeDecl*)
        {
            // Nothing to do
        }

        void visitParamDecl(ParamDecl* paramDecl)
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

        void VisitFunctionDeclaration(FuncDecl *functionNode)
        {
            if (functionNode->IsChecked(DeclCheckState::CheckedHeader)) return;
            functionNode->SetCheckState(DeclCheckState::CheckingHeader);
            auto oldFunc = this->function;
            this->function = functionNode;

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
            dispatchDecl(stmt->decl);
            checkModifiers(stmt->decl);
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
            const Index outerStmtCount = outerStmts.getCount();
            for (Index ii = outerStmtCount; ii > 0; --ii)
            {
                auto outerStmt = outerStmts[ii-1];
                auto found = as<T>(outerStmt);
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
            outerStmts.add(stmt);
        }

        void PopOuterStmt(Stmt* /*stmt*/)
        {
            outerStmts.removeAt(outerStmts.getCount() - 1);
        }

        RefPtr<Expr> checkPredicateExpr(Expr* expr)
        {
            RefPtr<Expr> e = expr;
            e = CheckTerm(e);
            e = coerce(getSession()->getBoolType(), e);
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
                        stmt->Expression = coerce(function->ReturnType.Ptr(), stmt->Expression);
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
            if (auto constantVal = as<ConstantIntVal>(val))
                return constantVal->value;

            // TODO(tfoley): Need to track intervals so that this isn't just a lie...
            return 1;
        }

        void maybeInferArraySizeForVariable(VarDeclBase* varDecl)
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

        void validateArraySizeForVariable(VarDeclBase* varDecl)
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

        void visitVarDecl(VarDecl* varDecl)
        {
            CheckVarDeclCommon(varDecl);
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

        Linkage* getLinkage() { return m_linkage; }
        NamePool* getNamePool() { return getLinkage()->getNamePool(); }

        Name* getName(String const& text)
        {
            return getNamePool()->getName(text);
        }

        RefPtr<IntVal> TryConstantFoldExpr(
            InvokeExpr* invokeExpr)
        {
            // We need all the operands to the expression

            // Check if the callee is an operation that is amenable to constant-folding.
            //
            // For right now we will look for calls to intrinsic functions, and then inspect
            // their names (this is bad and slow).
            auto funcDeclRefExpr = invokeExpr->FunctionExpr.as<DeclRefExpr>();
            if (!funcDeclRefExpr) return nullptr;

            auto funcDeclRef = funcDeclRefExpr->declRef;
            auto intrinsicMod = funcDeclRef.getDecl()->FindModifier<IntrinsicOpModifier>();
            if (!intrinsicMod)
            {
                // We can't constant fold anything that doesn't map to a builtin
                // operation right now.
                //
                // TODO: we should really allow constant-folding for anything
                // that can be lowered to our bytecode...
                return nullptr;
            }



            // Let's not constant-fold operations with more than a certain number of arguments, for simplicity
            static const int kMaxArgs = 8;
            if (invokeExpr->Arguments.getCount() > kMaxArgs)
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

                if (auto constArgVal = as<ConstantIntVal>(argVal))
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
            CASE(<<);
            CASE(>>);
            CASE(&);
            CASE(|);
            CASE(^);
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
            while (auto parenExpr = as<ParenExpr>(expr))
            {
                expr = parenExpr->base;
            }

            // TODO(tfoley): more serious constant folding here
            if (auto intLitExpr = as<IntegerLiteralExpr>(expr))
            {
                return GetIntVal(intLitExpr);
            }

            // it is possible that we are referring to a generic value param
            if (auto declRefExpr = as<DeclRefExpr>(expr))
            {
                auto declRef = declRefExpr->declRef;

                if (auto genericValParamRef = declRef.as<GenericValueParamDecl>())
                {
                    // TODO(tfoley): handle the case of non-`int` value parameters...
                    return new GenericParamIntVal(genericValParamRef);
                }

                // We may also need to check for references to variables that
                // are defined in a way that can be used as a constant expression:
                if(auto varRef = declRef.as<VarDeclBase>())
                {
                    auto varDecl = varRef.getDecl();

                    // In HLSL, `static const` is used to mark compile-time constant expressions
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
                }
                else if(auto enumRef = declRef.as<EnumCaseDecl>())
                {
                    // The cases in an `enum` declaration can also be used as constant expressions,
                    if(auto tagExpr = getTagExpr(enumRef))
                    {
                        return TryConstantFoldExpr(tagExpr.Ptr());
                    }
                }
            }

            if(auto castExpr = as<TypeCastExpr>(expr))
            {
                auto val = TryConstantFoldExpr(castExpr->Arguments[0].Ptr());
                if(val)
                    return val;
            }
            else if (auto invokeExpr = as<InvokeExpr>(expr))
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
            // Check if type is acceptable for an integer constant expression
            if(auto basicType = as<BasicExpressionType>(exp->type.type))
            {
                if(!isIntegerBaseType(basicType->baseType))
                    return nullptr;
            }
            else
            {
                return nullptr;
            }

            // Consider operations that we might be able to constant-fold...
            return TryConstantFoldExpr(exp);
        }

        // Enforce that an expression resolves to an integer constant, and get its value
        RefPtr<IntVal> CheckIntegerConstantExpression(Expr* inExpr)
        {
            // No need to issue further errors if the expression didn't even type-check.
            if(IsErrorExpr(inExpr)) return nullptr;

            // First coerce the expression to the expected type
            auto expr = coerce(getSession()->getIntType(),inExpr);

            // No need to issue further errors if the type coercion failed.
            if(IsErrorExpr(expr)) return nullptr;

            auto result = TryCheckIntegerConstantExpression(expr.Ptr());
            if (!result)
            {
                getSink()->diagnose(expr, Diagnostics::expectedIntegerConstantNotConstant);
            }
            return result;
        }

        RefPtr<IntVal> CheckEnumConstantExpression(Expr* expr)
        {
            // No need to issue further errors if the expression didn't even type-check.
            if(IsErrorExpr(expr)) return nullptr;

            // No need to issue further errors if the type coercion failed.
            if(IsErrorExpr(expr)) return nullptr;

            auto result = TryConstantFoldExpr(expr);
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
            if (auto baseTypeType = as<TypeType>(baseType))
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
            else if (auto baseArrayType = as<ArrayExpressionType>(baseType))
            {
                return CheckSimpleSubscriptExpr(
                    subscriptExpr,
                    baseArrayType->baseType);
            }
            else if (auto vecType = as<VectorExpressionType>(baseType))
            {
                return CheckSimpleSubscriptExpr(
                    subscriptExpr,
                    vecType->elementType);
            }
            else if (auto matType = as<MatrixExpressionType>(baseType))
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
                subscriptCallExpr->Arguments.add(subscriptExpr->IndexExpression);

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
            if (functionNode->GetParameters().getCount() != args.getCount())
                return false;
            Index i = 0;
            for (auto param : functionNode->GetParameters())
            {
                if (!param->type.Equals(args[i]->type.Ptr()))
                    return false;
                i++;
            }
            return true;
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

            /// Given an immutable `expr` used as an l-value emit a special diagnostic if it was derived from `this`.
        void maybeDiagnoseThisNotLValue(Expr* expr)
        {
            // We will try to handle expressions of the form:
            //
            //      e ::= "this"
            //          | e . name
            //          | e [ expr ]
            //
            // We will unwrap the `e.name` and `e[expr]` cases in a loop.
            RefPtr<Expr> e = expr;
            for(;;)
            {
                if(auto memberExpr = as<MemberExpr>(e))
                {
                    e = memberExpr->BaseExpression;
                }
                else if(auto subscriptExpr = as<IndexExpr>(e))
                {
                    e = subscriptExpr->BaseExpression;
                }
                else
                {
                    break;
                }
            }
            //
            // Now we check to see if we have a `this` expression,
            // and if it is immutable.
            if(auto thisExpr = as<ThisExpr>(e))
            {
                if(!thisExpr->type.IsLeftValue)
                {
                    getSink()->diagnose(thisExpr, Diagnostics::thisIsImmutableByDefault);
                }
            }
        }

        RefPtr<Expr> visitAssignExpr(AssignExpr* expr)
        {
            expr->left = CheckExpr(expr->left);

            auto type = expr->left->type;

            expr->right = coerce(type, CheckTerm(expr->right));

            if (!type.IsLeftValue)
            {
                if (as<ErrorType>(type))
                {
                    // Don't report an l-value issue on an erroneous expression
                }
                else
                {
                    getSink()->diagnose(expr, Diagnostics::assignNonLValue);

                    // As a special case, check if the LHS expression is derived
                    // from a `this` parameter (implicitly or explicitly), which
                    // is immutable. We can give the user a bit more context into
                    // what is going on.
                    //
                    maybeDiagnoseThisNotLValue(expr->left);
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

        void visitExtensionDecl(ExtensionDecl* decl)
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
                decl->Members.add(getterDecl);
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


        //

        struct Constraint
        {
            Decl*		decl; // the declaration of the thing being constraints
            RefPtr<Val>	val; // the value to which we are constraining it
            bool satisfied = false; // Has this constraint been met?
        };

        // A collection of constraints that will need to be satisfied (solved)
        // in order for checking to succeed.
        struct ConstraintSystem
        {
            // A source location to use in reporting any issues
            SourceLoc loc;

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
        RefPtr<DeclaredSubtypeWitness> createSimpleSubtypeWitness(
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
                // that `type` has been proven to be *equal*
                // to `interfaceDeclRef`.
                //
                SLANG_UNEXPECTED("reflexive type witness");
                UNREACHABLE_RETURN(nullptr);
            }

            // We might have one or more steps in the breadcrumb trail, e.g.:
            //
            //      {A : B} {B : C} {C : D}
            //
            // The chain is stored as a reversed linked list, so that
            // the first entry would be the `(C : D)` relationship
            // above.
            //
            // We need to walk the list and build up a suitable witness,
            // which in the above case would look like:
            //
            //      Transitive(
            //          Transitive(
            //              Declared({A : B}),
            //              {B : C}),
            //          {C : D})
            //
            // Because of the ordering of the breadcrumb trail, along
            // with the way the `Transitive` case nests, we will be
            // building these objects outside-in, and keeping
            // track of the "hole" where the next step goes.
            //
            auto bb = inBreadcrumbs;

            // `witness` here will hold the first (outer-most) object
            // we create, which is the overall result.
            RefPtr<SubtypeWitness> witness;

            // `link` will point at the remaining "hole" in the
            // data structure, to be filled in.
            RefPtr<SubtypeWitness>* link = &witness;

            // As long as there is more than one breadcrumb, we
            // need to be creating transitive witnesses.
            while(bb->prev)
            {
                // On the first iteration when processing the list
                // above, the breadcrumb would be for `{ C : D }`,
                // and so we'd create:
                //
                //      Transitive(
                //          [...],
                //          { C : D})
                //
                // where `[...]` represents the "hole" we leave
                // open to fill in next.
                //
                RefPtr<TransitiveSubtypeWitness> transitiveWitness = new TransitiveSubtypeWitness();
                transitiveWitness->sub = bb->sub;
                transitiveWitness->sup = bb->sup;
                transitiveWitness->midToSup = bb->declRef;

                // Fill in the current hole, and then set the
                // hole to point into the node we just created.
                *link = transitiveWitness;
                link = &transitiveWitness->subToMid;

                // Move on with the list.
                bb = bb->prev;
            }

            // If we exit the loop, then there is only one breadcrumb left.
            // In our running example this would be `{ A : B }`. We create
            // a simple (declared) subtype witness for it, and plug the
            // final hole, after which there shouldn't be a hole to deal with.
            RefPtr<DeclaredSubtypeWitness> declaredWitness = createSimpleSubtypeWitness(bb);
            *link = declaredWitness;

            // We now know that our original `witness` variable has been
            // filled in, and there are no other holes.
            return witness;
        }

            /// Is the given interface one that a tagged-union type can conform to?
            ///
            /// If a tagged union type `__TaggedUnion(A,B)` is going to be
            /// plugged in for a type parameter `T : IFoo` then we need to
            /// be sure that the interface `IFoo` doesn't have anything
            /// that could lead to unsafe/unsound behavior. This function
            /// checks that all the requirements on the interfaceare safe ones.
            ///
        bool isInterfaceSafeForTaggedUnion(
            DeclRef<InterfaceDecl> interfaceDeclRef)
        {
            for( auto memberDeclRef : getMembers(interfaceDeclRef) )
            {
                if(!isInterfaceRequirementSafeForTaggedUnion(interfaceDeclRef, memberDeclRef))
                    return false;
            }

            return true;
        }

            /// Is the given interface requirement one that a tagged-union type can satisfy?
            ///
            /// Unsafe requirements include any `static` requirements,
            /// any associated types, and also any requirements that make
            /// use of the `This` type (once we support it).
            ///
        bool isInterfaceRequirementSafeForTaggedUnion(
            DeclRef<InterfaceDecl>  interfaceDeclRef,
            DeclRef<Decl>           requirementDeclRef)
        {
            if(auto callableDeclRef = requirementDeclRef.as<CallableDecl>())
            {
                // A `static` method requirement can't be satisfied by a
                // tagged union, because there is no tag to dispatch on.
                //
                if(requirementDeclRef.getDecl()->HasModifier<HLSLStaticModifier>())
                    return false;

                // TODO: We will eventually want to check that any callable
                // requirements do not use the `This` type or any associated
                // types in ways that could lead to errors.
                //
                // For now we are disallowing interfaces that have associated
                // types completely, and we haven't implemented the `This`
                // type, so we should be safe.

                return true;
            }
            else
            {
                return false;
            }
        }

        bool doesTypeConformToInterfaceImpl(
            RefPtr<Type>            originalType,
            RefPtr<Type>            type,
            DeclRef<InterfaceDecl>  interfaceDeclRef,
            RefPtr<Val>*            outWitness,
            TypeWitnessBreadcrumb*  inBreadcrumbs)
        {
            // for now look up a conformance member...
            if(auto declRefType = as<DeclRefType>(type))
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

                if( auto aggTypeDeclRef = declRef.as<AggTypeDecl>() )
                {
                    checkDecl(aggTypeDeclRef.getDecl());

                    for( auto inheritanceDeclRef : getMembersOfTypeWithExt<InheritanceDecl>(aggTypeDeclRef))
                    {
                        checkDecl(inheritanceDeclRef.getDecl());

                        // Here we will recursively look up conformance on the type
                        // that is being inherited from. This is dangerous because
                        // it might lead to infinite loops.
                        //
                        // TODO: A better approach would be to create a linearized list
                        // of all the interfaces that a given type directly or indirectly
                        // inherits, and store it with the type, so that we don't have
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
                else if( auto genericTypeParamDeclRef = declRef.as<GenericTypeParamDecl>() )
                {
                    // We need to enumerate the constraints placed on this type by its outer
                    // generic declaration, and see if any of them guarantees that we
                    // satisfy the given interface..
                    auto genericDeclRef = genericTypeParamDeclRef.GetParent().as<GenericDecl>();
                    SLANG_ASSERT(genericDeclRef);

                    for( auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(genericDeclRef) )
                    {
                        auto sub = GetSub(constraintDeclRef);
                        auto sup = GetSup(constraintDeclRef);

                        auto subDeclRef = as<DeclRefType>(sub);
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
            else if(auto taggedUnionType = as<TaggedUnionType>(type))
            {
                // A tagged union type conforms to an interface if all of
                // the constituent types in the tagged union conform.
                //
                // We will iterate over the "case" types in the tagged
                // union, and check if they conform to the interface.
                // Along the way we will collect the conformance witness
                // values *if* we are being asked to produce a witness
                // value for the tagged union itself (that is, if
                // `outWitness` is non-null).
                //
                List<RefPtr<Val>> caseWitnesses;
                for(auto caseType : taggedUnionType->caseTypes)
                {
                    RefPtr<Val> caseWitness;

                    if(!doesTypeConformToInterfaceImpl(
                        caseType,
                        caseType,
                        interfaceDeclRef,
                        outWitness ? &caseWitness : nullptr,
                        nullptr))
                    {
                        return false;
                    }

                    if(outWitness)
                    {
                        caseWitnesses.add(caseWitness);
                    }
                }

                // We also need to validate the requirements on
                // the interface to make sure that they are suitable for
                // use with a tagged-union type.
                //
                // For example, if the interface includes a `static` method
                // (which can therefore be called without a particular instance),
                // then we wouldn't know what implementation of that method
                // to use because there is no tag value to dispatch on.
                //
                // We will start out being conservative about what we accept
                // here, just to keep things simple.
                //
                if(!isInterfaceSafeForTaggedUnion(interfaceDeclRef))
                    return false;

                // If we reach this point then we have a concrete
                // witness for each of the case types, and that is
                // enough to build a witness for the tagged union.
                //
                if(outWitness)
                {
                    RefPtr<TaggedUnionSubtypeWitness> taggedUnionWitness = new TaggedUnionSubtypeWitness();
                    taggedUnionWitness->sub = taggedUnionType;
                    taggedUnionWitness->sup = DeclRefType::Create(getSession(), interfaceDeclRef);
                    taggedUnionWitness->caseWitnesses.swapWith(caseWitnesses);

                    *outWitness = taggedUnionWitness;
                }
                return true;
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

        /// Does there exist an implicit conversion from `fromType` to `toType`?
        bool canConvertImplicitly(
            RefPtr<Type> toType,
            RefPtr<Type> fromType)
        {
            // Can we convert at all?
            ConversionCost conversionCost;
            if(!canCoerce(toType, fromType, &conversionCost))
                return false;

            // Is the conversion cheap enough to be done implicitly?
            if(conversionCost >= kConversionCost_GeneralConversion)
                return false;

            return true;
        }

        RefPtr<Type> TryJoinTypeWithInterface(
            RefPtr<Type>            type,
            DeclRef<InterfaceDecl>      interfaceDeclRef)
        {
            // The most basic test here should be: does the type declare conformance to the trait.
            if(DoesTypeConformToInterface(type, interfaceDeclRef))
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
            RefPtr<Type> bestType;
            if(auto basicType = type.dynamicCast<BasicExpressionType>())
            {
                for(Int baseTypeFlavorIndex = 0; baseTypeFlavorIndex < Int(BaseType::CountOf); baseTypeFlavorIndex++)
                {
                    // Don't consider `type`, since we already know it doesn't work.
                    if(baseTypeFlavorIndex == Int(basicType->baseType))
                        continue;

                    // Look up the type in our session.
                    auto candidateType = type->getSession()->getBuiltinType(BaseType(baseTypeFlavorIndex));
                    if(!candidateType)
                        continue;

                    // We only want to consider types that implement the target interface.
                    if(!DoesTypeConformToInterface(candidateType, interfaceDeclRef))
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

        // Try to compute the "join" between two types
        RefPtr<Type> TryJoinTypes(
            RefPtr<Type>  left,
            RefPtr<Type>  right)
        {
            // Easy case: they are the same type!
            if (left->Equals(right))
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
                if (auto typeParam = m.as<GenericTypeParamDecl>())
                {
                    RefPtr<Type> type = nullptr;
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
                    RefPtr<IntVal> val = nullptr;
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
                            if(!val->EqualsVal(cVal))
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

            RefPtr<GenericSubstitution> solvedSubst = new GenericSubstitution();
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
                auto sub = GetSub(constraintDeclRef);
                auto sup = GetSup(constraintDeclRef);

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
            Index argCount = 0;
            RefPtr<Expr>* args = nullptr;
            RefPtr<Type>* argTypes = nullptr;

            Index getArgCount() { return argCount; }
            RefPtr<Expr>& getArg(Index index) { return args[index]; }
            RefPtr<Type>& getArgType(Index index)
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

        bool TryCheckOverloadCandidateArity(
            OverloadResolveContext&		context,
            OverloadCandidate const&	candidate)
        {
            UInt argCount = context.getArgCount();
            ParamCounts paramCounts = { 0, 0 };
            switch (candidate.flavor)
            {
            case OverloadCandidate::Flavor::Func:
                paramCounts = CountParameters(GetParameters(candidate.item.declRef.as<CallableDecl>()));
                break;

            case OverloadCandidate::Flavor::Generic:
                paramCounts = CountParameters(candidate.item.declRef.as<GenericDecl>());
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

            if(auto prefixExpr = as<PrefixExpr>(expr))
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
            else if(auto postfixExpr = as<PostfixExpr>(expr))
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
            auto genericDeclRef = candidate.item.declRef.as<GenericDecl>();

            // We will go ahead and hang onto the arguments that we've
            // already checked, since downstream validation might need
            // them.
            auto genSubst = new GenericSubstitution();
            candidate.subst = genSubst;
            auto& checkedArgs = genSubst->args;

            Index aa = 0;
            for (auto memberRef : getMembers(genericDeclRef))
            {
                if (auto typeParamRef = memberRef.as<GenericTypeParamDecl>())
                {
                    if (aa >= context.argCount)
                    {
                        return false;
                    }
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
                    checkedArgs.add(typeExp.type);
                }
                else if (auto valParamRef = memberRef.as<GenericValueParamDecl>())
                {
                    auto arg = context.getArg(aa++);

                    if (context.mode == OverloadResolveContext::Mode::JustTrying)
                    {
                        ConversionCost cost = kConversionCost_None;
                        if (!canCoerce(GetType(valParamRef), arg->type, &cost))
                        {
                            return false;
                        }
                        candidate.conversionCostSum += cost;
                    }

                    arg = coerce(GetType(valParamRef), arg);
                    auto val = ExtractGenericArgInteger(arg);
                    checkedArgs.add(val);
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
            Index argCount = context.getArgCount();

            List<DeclRef<ParamDecl>> params;
            switch (candidate.flavor)
            {
            case OverloadCandidate::Flavor::Func:
                params = GetParameters(candidate.item.declRef.as<CallableDecl>()).ToArray();
                break;

            case OverloadCandidate::Flavor::Generic:
                return TryCheckGenericOverloadCandidateTypes(context, candidate);

            default:
                SLANG_UNEXPECTED("unknown flavor of overload candidate");
                break;
            }

            // Note(tfoley): We might have fewer arguments than parameters in the
            // case where one or more parameters had defaults.
            SLANG_RELEASE_ASSERT(argCount <= params.getCount());

            for (Index ii = 0; ii < argCount; ++ii)
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
                    else if (!canCoerce(GetType(param), argType, &cost))
                    {
                        return false;
                    }
                    candidate.conversionCostSum += cost;
                }
                else
                {
                    arg = coerce(GetType(param), arg);
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

            if(auto supDeclRefType = as<DeclRefType>(sup))
            {
                auto supDeclRef = supDeclRefType->declRef;
                if(auto supInterfaceDeclRef = supDeclRef.as<InterfaceDecl>())
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

            auto genericDeclRef = candidate.item.declRef.as<GenericDecl>();
            SLANG_ASSERT(genericDeclRef); // otherwise we wouldn't be a generic candidate...

            // We should have the existing arguments to the generic
            // handy, so that we can construct a substitution list.

            auto subst = candidate.subst.as<GenericSubstitution>();
            SLANG_ASSERT(subst);

            subst->genericDecl = genericDeclRef.getDecl();
            subst->outer = genericDeclRef.substitutions.substitutions;

            for( auto constraintDecl : genericDeclRef.getDecl()->getMembersOfType<GenericTypeConstraintDecl>() )
            {
                auto subset = genericDeclRef.substitutions;
                subset.substitutions = subst;
                DeclRef<GenericTypeConstraintDecl> constraintDeclRef(
                    constraintDecl, subset);

                auto sub = GetSub(constraintDeclRef);
                auto sup = GetSup(constraintDeclRef);

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

            candidate.status = OverloadCandidate::Status::Applicable;
        }

        // Create the representation of a given generic applied to some arguments
        RefPtr<Expr> createGenericDeclRef(
            RefPtr<Expr>            baseExpr,
            RefPtr<Expr>            originalExpr,
            RefPtr<GenericSubstitution>   subst)
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

            DeclRef<Decl> innerDeclRef(GetInner(baseGenericRef), subst);

            RefPtr<Expr> base;
            if (auto mbrExpr = as<MemberExpr>(baseExpr))
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
                        RefPtr<AppExprBase> callExpr = as<InvokeExpr>(context.originalExpr);
                        if(!callExpr)
                        {
                            callExpr = new InvokeExpr();
                            callExpr->loc = context.loc;

                            for(Index aa = 0; aa < context.argCount; ++aa)
                                callExpr->Arguments.add(context.getArg(aa));
                        }


                        callExpr->FunctionExpr = baseExpr;
                        callExpr->type = QualType(candidate.resultType);

                        // A call may yield an l-value, and we should take a look at the candidate to be sure
                        if(auto subscriptDeclRef = candidate.item.declRef.as<SubscriptDecl>())
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
                        candidate.subst.as<GenericSubstitution>());
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
            if(left->status == OverloadCandidate::Status::Applicable)
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
            ctorItem.breadcrumbs = new LookupResultItem::Breadcrumb(
                LookupResultItem::Breadcrumb::Kind::Member,
                typeItem.declRef,
                typeItem.breadcrumbs);

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
            auto parentGeneric = as<GenericDecl>(parentDecl);
            return parentGeneric;
        }

        // Try to find a unification for two values
        bool TryUnifyVals(
            ConstraintSystem&	constraints,
            RefPtr<Val>			fst,
            RefPtr<Val>			snd)
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
            return false;
        }

        bool tryUnifySubstitutions(
            ConstraintSystem&       constraints,
            RefPtr<Substitutions>   fst,
            RefPtr<Substitutions>   snd)
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

        bool tryUnifyGenericSubstitutions(
            ConstraintSystem&           constraints,
            RefPtr<GenericSubstitution> fst,
            RefPtr<GenericSubstitution> snd)
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

            constraints.constraints.add(constraint);

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

            constraints.constraints.add(constraint);

            return true;
        }

        bool TryUnifyIntParam(
            ConstraintSystem&       constraints,
            DeclRef<VarDeclBase> const&   varRef,
            RefPtr<IntVal>          val)
        {
            if(auto genericValueParamRef = varRef.as<GenericValueParamDecl>())
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

        bool TryUnifyTypes(
            ConstraintSystem&       constraints,
            RefPtr<Type>  fst,
            RefPtr<Type>  snd)
        {
            if (fst->Equals(snd)) return true;

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
                    if(typeParamDecl->ParentDecl == constraints.genericDecl )
                        return TryUnifyTypeParam(constraints, typeParamDecl, snd);
                }
            }

            if (auto sndDeclRefType = as<DeclRefType>(snd))
            {
                auto sndDeclRef = sndDeclRefType->declRef;

                if (auto typeParamDecl = as<GenericTypeParamDecl>(sndDeclRef.getDecl()))
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

        // Is the candidate extension declaration actually applicable to the given type
        DeclRef<ExtensionDecl> ApplyExtensionToType(
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
            DeclRef<GenericDecl>    genericDeclRef,
            OverloadResolveContext& context)
        {
            checkDecl(genericDeclRef.getDecl());

            ConstraintSystem constraints;
            constraints.loc = context.loc;
            constraints.genericDecl = genericDeclRef.getDecl();

            // Construct a reference to the inner declaration that has any generic
            // parameter substitutions in place already, but *not* any substutions
            // for the generic declaration we are currently trying to infer.
            auto innerDecl = GetInner(genericDeclRef);
            DeclRef<Decl> unspecializedInnerRef = DeclRef<Decl>(innerDecl, genericDeclRef.substitutions);

            // Check what type of declaration we are dealing with, and then try
            // to match it up with the arguments accordingly...
            if (auto funcDeclRef = unspecializedInnerRef.as<CallableDecl>())
            {
                auto params = GetParameters(funcDeclRef).ToArray();

                Index argCount = context.getArgCount();
                Index paramCount = params.getCount();

                // Bail out on mismatch.
                // TODO(tfoley): need more nuance here
                if (argCount != paramCount)
                {
                    return DeclRef<Decl>(nullptr, nullptr);
                }

                for (Index aa = 0; aa < argCount; ++aa)
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

            // Also check for generic constructors.
            //
            // TODO: There is way too much duplication between this case and the extension
            // handling below, and all of this is *also* duplicative with the ordinary
            // overload resolution logic for function.
            //
            // The right solution is to handle a "constructor" call expression by
            // first doing member lookup in the type (for initializer members, which
            // should all share a common name), and then to do overload resolution using
            // the (possibly overloaded) result of that lookup.
            //
            for (auto genericDeclRef : getMembersOfType<GenericDecl>(aggTypeDeclRef))
            {
                if (auto ctorDecl = as<ConstructorDecl>(genericDeclRef.getDecl()->inner))
                {
                    DeclRef<Decl> innerRef = SpecializeGenericForOverload(genericDeclRef, context);
                    if (!innerRef)
                        continue;

                    DeclRef<ConstructorDecl> innerCtorRef = innerRef.as<ConstructorDecl>();
                    AddCtorOverloadCandidate(typeItem, type, innerCtorRef, context, resultType);
                }
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
                    if (auto ctorDecl = genericDeclRef.getDecl()->inner.as<ConstructorDecl>())
                    {
                        DeclRef<Decl> innerRef = SpecializeGenericForOverload(genericDeclRef, context);
                        if (!innerRef)
                            continue;

                        DeclRef<ConstructorDecl> innerCtorRef = innerRef.as<ConstructorDecl>();

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
            auto genericDeclRef = typeDeclRef.GetParent().as<GenericDecl>();
            SLANG_ASSERT(genericDeclRef);

            for(auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(genericDeclRef))
            {
                // Does this constraint pertain to the type we are working on?
                //
                // We want constraints of the form `T : Foo` where `T` is the
                // generic parameter in question, and `Foo` is whatever we are
                // constraining it to.
                auto subType = GetSub(constraintDeclRef);
                auto subDeclRefType = as<DeclRefType>(subType);
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
            if (auto declRefType = as<DeclRefType>(type))
            {
                auto declRef = declRefType->declRef;
                if (auto aggTypeDeclRef = declRef.as<AggTypeDecl>())
                {
                    AddAggTypeOverloadCandidates(LookupResultItem(aggTypeDeclRef), type, aggTypeDeclRef, context, resultType);
                }
                else if(auto genericTypeParamDeclRef = declRef.as<GenericTypeParamDecl>())
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

            if (auto funcDeclRef = item.declRef.as<CallableDecl>())
            {
                AddFuncOverloadCandidate(item, funcDeclRef, context);
            }
            else if (auto aggTypeDeclRef = item.declRef.as<AggTypeDecl>())
            {
                auto type = DeclRefType::Create(
                    getSession(),
                    aggTypeDeclRef);
                AddAggTypeOverloadCandidates(item, type, aggTypeDeclRef, context, type);
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
                auto type = getNamedType(getSession(), typeDefDeclRef);
                AddTypeOverloadCandidates(GetType(typeDefDeclRef), context, type);
            }
            else if( auto genericTypeParamDeclRef = item.declRef.as<GenericTypeParamDecl>() )
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
                auto lookupResult = overloadedExpr->lookupResult2;
                SLANG_RELEASE_ASSERT(lookupResult.isOverloaded());
                for(auto item : lookupResult.items)
                {
                    AddDeclRefOverloadCandidates(item, context);
                }
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
            auto parentGenericDeclRef = parentDeclRef.as<GenericDecl>();
            if(parentGenericDeclRef)
            {
                parentDeclRef = parentGenericDeclRef.GetParent();
            }

            // Depending on what the parent is, we may want to format things specially
            if(auto aggTypeDeclRef = parentDeclRef.as<AggTypeDecl>())
            {
                formatDeclPath(sb, aggTypeDeclRef);
                sb << ".";
            }

            sb << getText(declRef.GetName());

            // If the parent declaration is a generic, then we need to print out its
            // signature
            if( parentGenericDeclRef )
            {
                auto genSubst = declRef.substitutions.substitutions.as<GenericSubstitution>();
                SLANG_RELEASE_ASSERT(genSubst);
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
            if (auto funcDeclRef = declRef.as<CallableDecl>())
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
            else if(auto genericDeclRef = declRef.as<GenericDecl>())
            {
                sb << "<";
                bool first = true;
                for (auto paramDeclRef : getMembers(genericDeclRef))
                {
                    if(auto genericTypeParam = paramDeclRef.as<GenericTypeParamDecl>())
                    {
                        if (!first) sb << ", ";
                        first = false;

                        sb << getText(genericTypeParam.GetName());
                    }
                    else if(auto genericValParam = paramDeclRef.as<GenericValueParamDecl>())
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
            OverloadResolveContext context;
            // check if this is a stdlib operator call, if so we want to use cached results
            // to speed up compilation
            bool shouldAddToCache = false;
            OperatorOverloadCacheKey key;
            TypeCheckingCache* typeCheckingCache = getSession()->getTypeCheckingCache();
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
            auto funcExpr = expr->FunctionExpr;
            auto funcExprType = funcExpr->type;

            // If we are trying to apply an erroneous expression, then just bail out now.
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

            context.originalExpr = expr;
            context.funcLoc = funcExpr->loc;

            context.argCount = expr->Arguments.getCount();
            context.args = expr->Arguments.getBuffer();
            context.loc = expr->loc;

            if (auto funcMemberExpr = as<MemberExpr>(funcExpr))
            {
                context.baseExpr = funcMemberExpr->BaseExpression;
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
                for (auto arg : expr->Arguments)
                {
                    if (IsErrorExpr(arg))
                    {
                        return CreateErrorExpr(expr);
                    }
                }

                Name* funcName = nullptr;
                if (auto baseVar = as<VarExpr>(funcExpr))
                    funcName = baseVar->name;
                else if(auto baseMemberRef = as<MemberExpr>(funcExpr))
                    funcName = baseMemberRef->name;

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
                        String declString = getDeclSignatureString(candidate.item);

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
                getSink()->diagnose(expr->FunctionExpr, Diagnostics::expectedFunction, funcExprType);
                expr->type = QualType(getSession()->getErrorType());
                return expr;
            }
        }

        void AddGenericOverloadCandidate(
            LookupResultItem		baseItem,
            OverloadResolveContext&	context)
        {
            if (auto genericDeclRef = baseItem.declRef.as<GenericDecl>())
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

        RefPtr<Expr> visitGenericAppExpr(GenericAppExpr* genericAppExpr)
        {
            // Start by checking the base expression and arguments.
            auto& baseExpr = genericAppExpr->FunctionExpr;
            baseExpr = CheckTerm(baseExpr);
            auto& args = genericAppExpr->Arguments;
            for (auto& arg : args)
            {
                arg = CheckTerm(arg);
            }

            return checkGenericAppWithCheckedArgs(genericAppExpr);
        }

            /// Check a generic application where the operands have already been checked.
        RefPtr<Expr> checkGenericAppWithCheckedArgs(GenericAppExpr* genericAppExpr)
        {
            // We are applying a generic to arguments, but there might be multiple generic
            // declarations with the same name, so this becomes a specialized case of
            // overload resolution.

            auto& baseExpr = genericAppExpr->FunctionExpr;
            auto& args = genericAppExpr->Arguments;

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

                    auto overloadedExpr = new OverloadedExpr2();
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

        RefPtr<Expr> visitTaggedUnionTypeExpr(TaggedUnionTypeExpr* expr)
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
            if (auto invoke = as<InvokeExpr>(rs.Ptr()))
            {
                // if this is still an invoke expression, test arguments passed to inout/out parameter are LValues
                if(auto funcType = as<FuncType>(invoke->FunctionExpr->type))
                {
                    Index paramCount = funcType->getParamCount();
                    for (Index pp = 0; pp < paramCount; ++pp)
                    {
                        auto paramType = funcType->getParamType(pp);
                        if (as<OutTypeBase>(paramType) || as<RefType>(paramType))
                        {
                            // `out`, `inout`, and `ref` parameters currently require
                            // an *exact* match on the type of the argument.
                            //
                            // TODO: relax this requirement by allowing an argument
                            // for an `inout` parameter to be converted in both
                            // directions.
                            //
                            if( pp < expr->Arguments.getCount() )
                            {
                                auto argExpr = expr->Arguments[pp];
                                if( !argExpr->type.IsLeftValue )
                                {
                                    getSink()->diagnose(
                                        argExpr,
                                        Diagnostics::argumentExpectedLValue,
                                        pp);

                                    if( auto implicitCastExpr = as<ImplicitCastExpr>(argExpr) )
                                    {
                                        getSink()->diagnose(
                                            argExpr,
                                            Diagnostics::implicitCastUsedAsLValue,
                                            implicitCastExpr->Arguments[0]->type,
                                            implicitCastExpr->type);
                                    }

                                    maybeDiagnoseThisNotLValue(argExpr);
                                }
                            }
                            else
                            {
                                // There are two ways we could get here, both involving
                                // a call where the number of argument expressions is
                                // less than the number of parameters on the callee:
                                //
                                // 1. There might be fewer arguments than parameters
                                // because the trailing parameters should be defaulted
                                //
                                // 2. There might be fewer arguments than parameters
                                // because the call is incorrect.
                                //
                                // In case (2) an error would have already been diagnosed,
                                // and we don't want to emit another cascading error here.
                                //
                                // In case (1) this implies the user declared an `out`
                                // or `inout` parameter with a default argument expression.
                                // That should be an error, but it should be detected
                                // on the declaration instead of here at the use site.
                                //
                                // Thus, it makes sense to ignore this case here.
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

            // LEGACY FEATURE: As a backwards-compatibility feature
            // for HLSL, we will allow for a cast to a `struct` type
            // from a literal zero, with the semantics of default
            // initialization.
            //
            if( auto declRefType = typeExp.type.as<DeclRefType>() )
            {
                if(auto structDeclRef = declRefType->declRef.as<StructDecl>())
                {
                    if( expr->Arguments.getCount() == 1 )
                    {
                        auto arg = expr->Arguments[0];
                        if( auto intLitArg = arg.as<IntegerLiteralExpr>() )
                        {
                            if(getIntegerLiteralValue(intLitArg->token) == 0)
                            {
                                // At this point we have confirmed that the cast
                                // has the right form, so we want to apply our special case.
                                //
                                // TODO: If/when we allow for user-defined initializer/constructor
                                // definitions we would have to be careful here because it is
                                // possible that the target type has defined an initializer/constructor
                                // that takes a single `int` parmaeter and means to call that instead.
                                //
                                // For now that should be a non-issue, and in a pinch such a user
                                // could use `T(0)` instead of `(T) 0` to get around this special
                                // HLSL legacy feature.

                                // We will type-check code like:
                                //
                                //      MyStruct s = (MyStruct) 0;
                                //
                                // the same as:
                                //
                                //      MyStruct s = {};
                                //
                                // That is, we construct an empty initializer list, and then coerce
                                // that initializer list expression to the desired type (letting
                                // the code for handling initializer lists work out all of the
                                // details of what is/isn't valid). This choice means we get
                                // to benefit from the existing codegen support for initializer
                                // lists, rather than needing the `(MyStruct) 0` idiom to be
                                // special-cased in later stages of the compiler.
                                //
                                // Note: we use an empty initializer list `{}` instead of an
                                // initializer list with a single zero `{0}`, which is semantically
                                // significant if the first field of `MyStruct` had its own
                                // default initializer defined as part of the `struct` definition.
                                // Basically we have chosen to interpret the "cast from zero" syntax
                                // as sugar for default initialization, and *not* specifically
                                // for zero-initialization. That choice could be revisited if
                                // users express displeasure. For now there isn't enough usage
                                // of explicit default initializers for `struct` fields to
                                // make this a major concern (since they aren't supported in HLSL).
                                //
                                RefPtr<InitializerListExpr> initListExpr = new InitializerListExpr();
                                auto checkedInitListExpr = visitInitializerListExpr(initListExpr);
                                return coerce(typeExp.type, initListExpr);
                            }
                        }
                    }
                }
            }


            // Now process this like any other explicit call (so casts
            // and constructor calls are semantically equivalent).
            return CheckInvokeExprWithCheckedOperands(expr);
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

    #define CASE(NAME)                                  \
        RefPtr<Expr> visit##NAME(NAME* expr)            \
        {                                               \
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), expr,  \
                "should not appear in input syntax");   \
            return expr;                                \
        }

        CASE(DerefExpr)
        CASE(SwizzleExpr)
        CASE(OverloadedExpr)
        CASE(OverloadedExpr2)
        CASE(AggTypeCtorExpr)
        CASE(CastToInterfaceExpr)
        CASE(LetExpr)
        CASE(ExtractExistentialValueExpr)

    #undef CASE

        //
        //
        //

        RefPtr<Expr> MaybeDereference(RefPtr<Expr> inExpr)
        {
            RefPtr<Expr> expr = inExpr;
            for (;;)
            {
                auto baseType = expr->type;
                if (auto pointerLikeType = as<PointerLikeType>(baseType))
                {
                    auto elementType = QualType(pointerLikeType->elementType);
                    elementType.IsLeftValue = baseType.IsLeftValue;

                    auto derefExpr = new DerefExpr();
                    derefExpr->base = expr;
                    derefExpr->type = elementType;

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

            for (Index i = 0; i < swizzleText.getLength(); i++)
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
            if (auto constantElementCount = as<ConstantIntVal>(baseElementCount))
            {
                return CheckSwizzleExpr(memberRefExpr, baseElementType, constantElementCount->value);
            }
            else
            {
                getSink()->diagnose(memberRefExpr, Diagnostics::unimplemented, "swizzle on vector of unknown size");
                return CreateErrorExpr(memberRefExpr);
            }
        }

        // Look up a static member
        // @param expr Can be StaticMemberExpr or MemberExpr
        // @param baseExpression Is the underlying type expression determined from resolving expr
        RefPtr<Expr> _lookupStaticMember(RefPtr<DeclRefExpr> expr, RefPtr<Expr> baseExpression)
        {
            auto& baseType = baseExpression->type;

            if (auto typeType = as<TypeType>(baseType))
            {
                // We are looking up a member inside a type.
                // We want to be careful here because we should only find members
                // that are implicitly or explicitly `static`.
                //
                // TODO: this duplicates a *lot* of logic with the case below.
                // We need to fix that.
                auto type = typeType->type;

                if (as<ErrorType>(type))
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
                    return lookupMemberResultFailure(expr, baseType);
                }

                // We need to confirm that whatever member we
                // are trying to refer to is usable via static reference.
                //
                // TODO: eventually we might allow a non-static
                // member to be adapted by turning it into something
                // like a closure that takes the missing `this` parameter.
                //
                // E.g., a static reference to a method could be treated
                // as a value with a function type, where the first parameter
                // is `type`.
                //
                // The biggest challenge there is that we'd need to arrange
                // to generate "dispatcher" functions that could be used
                // to implement that function, in the case where we are
                // making a static reference to some kind of polymorphic declaration.
                //
                // (Also, static references to fields/properties would get even
                // harder, because you'd have to know whether a getter/setter/ref-er
                // is needed).
                //
                // For now let's just be expedient and disallow all of that, because
                // we can always add it back in later.

                if (!lookupResult.isOverloaded())
                {
                    // The non-overloaded case is relatively easy. We just want
                    // to look at the member being referenced, and check if
                    // it is allowed in a `static` context:
                    //
                    if (!isUsableAsStaticMember(lookupResult.item))
                    {
                        getSink()->diagnose(
                            expr->loc,
                            Diagnostics::staticRefToNonStaticMember,
                            type,
                            expr->name);
                    }
                }
                else
                {
                    // The overloaded case is trickier, because we should first
                    // filter the list of candidates, because if there is anything
                    // that *is* usable in a static context, then we should assume
                    // the user just wants to reference that. We should only
                    // issue an error if *all* of the items that were discovered
                    // are non-static.
                    bool anyNonStatic = false;
                    List<LookupResultItem> staticItems;
                    for (auto item : lookupResult.items)
                    {
                        // Is this item usable as a static member?
                        if (isUsableAsStaticMember(item))
                        {
                            // If yes, then it will be part of the output.
                            staticItems.add(item);
                        }
                        else
                        {
                            // If no, then we might need to output an error.
                            anyNonStatic = true;
                        }
                    }

                    // Was there anything non-static in the list?
                    if (anyNonStatic)
                    {
                        // If we had some static items, then that's okay,
                        // we just want to use our newly-filtered list.
                        if (staticItems.getCount())
                        {
                            lookupResult.items = staticItems;
                        }
                        else
                        {
                            // Otherwise, it is time to report an error.
                            getSink()->diagnose(
                                expr->loc,
                                Diagnostics::staticRefToNonStaticMember,
                                type,
                                expr->name);
                        }
                    }
                    // If there were no non-static items, then the `items`
                    // array already represents what we'd get by filtering...
                }

                return createLookupResultExpr(
                    lookupResult,
                    baseExpression,
                    expr->loc);
            }
            else if (as<ErrorType>(baseType))
            {
                return CreateErrorExpr(expr);
            }

            // Failure
            return lookupMemberResultFailure(expr, baseType);
        }

        RefPtr<Expr> visitStaticMemberExpr(StaticMemberExpr* expr)
        {
            expr->BaseExpression = CheckExpr(expr->BaseExpression);

            // Not sure this is needed -> but guess someone could do 
            expr->BaseExpression = MaybeDereference(expr->BaseExpression);

            // If the base of the member lookup has an interface type
            // *without* a suitable this-type substitution, then we are
            // trying to perform lookup on a value of existential type,
            // and we should "open" the existential here so that we
            // can expose its structure.
            //

            expr->BaseExpression = maybeOpenExistential(expr->BaseExpression);
            // Do a static lookup
            return _lookupStaticMember(expr, expr->BaseExpression);
        }

        RefPtr<Expr> lookupMemberResultFailure(
            DeclRefExpr*     expr,
            QualType const& baseType)
        {
            // Check it's a member expression
            SLANG_ASSERT(as<StaticMemberExpr>(expr) || as<MemberExpr>(expr));

            getSink()->diagnose(expr, Diagnostics::noMemberOfNameInType, expr->name, baseType);
            expr->type = QualType(getSession()->getErrorType());
            return expr;
        }

        RefPtr<Expr> visitMemberExpr(MemberExpr * expr)
        {
            expr->BaseExpression = CheckExpr(expr->BaseExpression);

            expr->BaseExpression = MaybeDereference(expr->BaseExpression);

            // If the base of the member lookup has an interface type
            // *without* a suitable this-type substitution, then we are
            // trying to perform lookup on a value of existential type,
            // and we should "open" the existential here so that we
            // can expose its structure.
            //
            expr->BaseExpression = maybeOpenExistential(expr->BaseExpression);

            auto & baseType = expr->BaseExpression->type;

            // Note: Checking for vector types before declaration-reference types,
            // because vectors are also declaration reference types...
            //
            // Also note: the way this is done right now means that the ability
            // to swizzle vectors interferes with any chance of looking up
            // members via extension, for vector or scalar types.
            //
            // TODO: Matrix swizzles probably need to be handled at some point.
            if (auto baseVecType = as<VectorExpressionType>(baseType))
            {
                return CheckSwizzleExpr(
                    expr,
                    baseVecType->elementType,
                    baseVecType->elementCount);
            }
            else if(auto baseScalarType = as<BasicExpressionType>(baseType))
            {
                // Treat scalar like a 1-element vector when swizzling
                return CheckSwizzleExpr(
                    expr,
                    baseScalarType,
                    1);
            }
            else if(auto typeType = as<TypeType>(baseType))
            {
                return _lookupStaticMember(expr, expr->BaseExpression);
            }
            else if (as<ErrorType>(baseType))
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
                    return lookupMemberResultFailure(expr, baseType);
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

        // Perform semantic checking of an object-oriented `this`
        // expression.
        RefPtr<Expr> visitThisExpr(ThisExpr* expr)
        {
            // A `this` expression will default to immutable.
            expr->type.IsLeftValue = false;

            // We will do an upwards search starting in the current
            // scope, looking for a surrounding type (or `extension`)
            // declaration that could be the referrant of the expression.
            auto scope = expr->scope;
            while (scope)
            {
                auto containerDecl = scope->containerDecl;

                if( auto funcDeclBase = as<FunctionDeclBase>(containerDecl) )
                {
                    if( funcDeclBase->HasModifier<MutatingAttribute>() )
                    {
                        expr->type.IsLeftValue = true;
                    }
                }
                else if (auto aggTypeDecl = as<AggTypeDecl>(containerDecl))
                {
                    checkDecl(aggTypeDecl);

                    // Okay, we are using `this` in the context of an
                    // aggregate type, so the expression should be
                    // of the corresponding type.
                    expr->type.type = DeclRefType::Create(
                        getSession(),
                        makeDeclRef(aggTypeDecl));
                    return expr;
                }
                else if (auto extensionDecl = as<ExtensionDecl>(containerDecl))
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
                    expr->type.type = extensionDecl->targetType.type;
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
        SLANG_ASSERT(decl);
        return (!decl->primaryDecl) || (decl == decl->primaryDecl);
    }

    RefPtr<Type> checkProperType(
        Linkage*        linkage,
        TypeExp         typeExp,
        DiagnosticSink* sink)
    {
        SemanticsVisitor visitor(
            linkage,
            sink);
        auto typeOut = visitor.CheckProperType(typeExp);
        return typeOut.type;
    }


    FuncDecl* findFunctionDeclByName(
        Module*                 translationUnit,
        Name*                   name,
        DiagnosticSink*         sink)
    {
        auto translationUnitSyntax = translationUnit->getModuleDecl();

        // Make sure we've got a query-able member dictionary
        buildMemberDictionary(translationUnitSyntax);

        // We will look up any global-scope declarations in the translation
        // unit that match the name of our entry point.
        Decl* firstDeclWithName = nullptr;
        if (!translationUnitSyntax->memberDictionary.TryGetValue(name, firstDeclWithName))
        {
            // If there doesn't appear to be any such declaration, then we are done.

            sink->diagnose(translationUnitSyntax, Diagnostics::entryPointFunctionNotFound, name);

            return nullptr;
        }

        // We found at least one global-scope declaration with the right name,
        // but (1) it might not be a function, and (2) there might be
        // more than one function.
        //
        // We'll walk the linked list of declarations with the same name,
        // to see what we find. Along the way we'll keep track of the
        // first function declaration we find, if any:
        FuncDecl* entryPointFuncDecl = nullptr;
        for (auto ee = firstDeclWithName; ee; ee = ee->nextInContainerWithSameName)
        {
            // Is this declaration a function?
            if (auto funcDecl = as<FuncDecl>(ee))
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

                    sink->diagnose(translationUnitSyntax, Diagnostics::ambiguousEntryPoint, name);

                    // List all of the declarations that the user *might* mean
                    for (auto ff = firstDeclWithName; ff; ff = ff->nextInContainerWithSameName)
                    {
                        if (auto candidate = as<FuncDecl>(ff))
                        {
                            sink->diagnose(candidate, Diagnostics::entryPointCandidate, candidate->getName());
                        }
                    }

                    // Bail out.
                    return nullptr;
                }
            }
        }

        return entryPointFuncDecl;
    }

    static bool isValidThreadDispatchIDType(Type* type)
    {
        // Can accept a single int/unit
        {
            auto basicType = as<BasicExpressionType>(type);
            if (basicType)
            {
                return (basicType->baseType == BaseType::Int || basicType->baseType == BaseType::UInt);
            }
        }
        // Can be an int/uint vector from size 1 to 3
        {
            auto vectorType = as<VectorExpressionType>(type);
            if (!vectorType)
            {
                return false;
            }
            auto elemCount = as<ConstantIntVal>(vectorType->elementCount);
            if (elemCount->value < 1 || elemCount->value > 3)
            {
                return false;
            }
            // Must be a basic type
            auto basicType = as<BasicExpressionType>(vectorType->elementType);
            if (!basicType)
            {
                return false;
            }

            // Must be integral
            return (basicType->baseType == BaseType::Int || basicType->baseType == BaseType::UInt);
        }
    }

        /// Recursively walk `paramDeclRef` and add any existential/interface specialization parameters to `ioSpecializationParams`.
    static void _collectExistentialSpecializationParamsRec(
        SpecializationParams&   ioSpecializationParams,
        DeclRef<VarDeclBase>    paramDeclRef);

        /// Recursively walk `type` and add any existential/interface specialization parameters to `ioSpecializationParams`.
    static void _collectExistentialSpecializationParamsRec(
        SpecializationParams&   ioSpecializationParams,
        Type*                   type)
    {
        // Whether or not something is an array does not affect
        // the number of existential slots it introduces.
        //
        while( auto arrayType = as<ArrayExpressionType>(type) )
        {
            type = arrayType->baseType;
        }

        if( auto parameterGroupType = as<ParameterGroupType>(type) )
        {
            _collectExistentialSpecializationParamsRec(
                ioSpecializationParams,
                parameterGroupType->getElementType());
            return;
        }

        if( auto declRefType = as<DeclRefType>(type) )
        {
            auto typeDeclRef = declRefType->declRef;
            if( auto interfaceDeclRef = typeDeclRef.as<InterfaceDecl>() )
            {
                // Each leaf parameter of interface type adds a specialization
                // parameter, which determines the concrete type(s) that may
                // be provided as arguments for that parameter.
                //
                SpecializationParam specializationParam;
                specializationParam.flavor = SpecializationParam::Flavor::ExistentialType;
                specializationParam.object = type;
                ioSpecializationParams.add(specializationParam);
            }
            else if( auto structDeclRef = typeDeclRef.as<StructDecl>() )
            {
                // A structure type should recursively introduce
                // existential slots for its fields.
                //
                for( auto fieldDeclRef : GetFields(structDeclRef) )
                {
                    if(fieldDeclRef.getDecl()->HasModifier<HLSLStaticModifier>())
                        continue;

                    _collectExistentialSpecializationParamsRec(
                        ioSpecializationParams,
                        fieldDeclRef);
                }
            }
        }

        // TODO: We eventually need to handle cases like constant
        // buffers and parameter blocks that may have existential
        // element types.
    }

    static void _collectExistentialSpecializationParamsRec(
        SpecializationParams&   ioSpecializationParams,
        DeclRef<VarDeclBase>    paramDeclRef)
    {
        _collectExistentialSpecializationParamsRec(
            ioSpecializationParams,
            GetType(paramDeclRef));
    }


        /// Collect any interface/existential specialization parameters for `paramDeclRef` into `ioParamInfo` and `ioSpecializationParams`
    static void _collectExistentialSpecializationParamsForShaderParam(
        ShaderParamInfo&        ioParamInfo,
        SpecializationParams&   ioSpecializationParams,
        DeclRef<VarDeclBase>    paramDeclRef)
    {
        Index beginParamIndex = ioSpecializationParams.getCount();
        _collectExistentialSpecializationParamsRec(ioSpecializationParams, paramDeclRef);
        Index endParamIndex = ioSpecializationParams.getCount();
        
        ioParamInfo.firstSpecializationParamIndex = beginParamIndex;
        ioParamInfo.specializationParamCount = endParamIndex - beginParamIndex;
    }

    void EntryPoint::_collectGenericSpecializationParamsRec(Decl* decl)
    {
        if(!decl)
            return;

        _collectGenericSpecializationParamsRec(decl->ParentDecl);

        auto genericDecl = as<GenericDecl>(decl);
        if(!genericDecl)
            return;

        for(auto m : genericDecl->Members)
        {
            if(auto genericTypeParam = as<GenericTypeParamDecl>(m))
            {
                SpecializationParam param;
                param.flavor = SpecializationParam::Flavor::GenericType;
                param.object = genericTypeParam;
                m_genericSpecializationParams.add(param);
            }
            else if(auto genericValParam = as<GenericValueParamDecl>(m))
            {
                SpecializationParam param;
                param.flavor = SpecializationParam::Flavor::GenericValue;
                param.object = genericValParam;
                m_genericSpecializationParams.add(param);
            }
        }
    }

        /// Enumerate the existential-type parameters of an `EntryPoint`.
        ///
        /// Any parameters found will be added to the list of existential slots on `this`.
        ///
    void EntryPoint::_collectShaderParams()
    {
        // We don't currently treat an entry point as having any
        // *global* shader parameters.
        //
        // TODO: We could probably clean up the code a bit by treating
        // an entry point as introducing a global shader parameter
        // that is based on the implicit "parameters struct" type
        // of the entry point itself.

        // We collect the generic parameters of the entry point,
        // along with those of any outer generics first.
        //
        _collectGenericSpecializationParamsRec(getFuncDecl());

        // After geneic specialization parameters have been collected,
        // we look through the value parameters of the entry point
        // function and see if any of them introduce existential/interface
        // specialization parameters.
        //
        // Note: we defensively test whether there is a function decl-ref
        // because this routine gets called from the constructor, and
        // a "dummy" entry point will have a null pointer for the function.
        //
        if( auto funcDeclRef = getFuncDeclRef() )
        {
            for( auto paramDeclRef : GetParameters(funcDeclRef) )
            {
                ShaderParamInfo shaderParamInfo;
                shaderParamInfo.paramDeclRef = paramDeclRef;

                _collectExistentialSpecializationParamsForShaderParam(
                    shaderParamInfo,
                    m_existentialSpecializationParams,
                    paramDeclRef);

                m_shaderParams.add(shaderParamInfo);
            }
        }
    }

    // Validate that an entry point function conforms to any additional
    // constraints based on the stage (and profile?) it specifies.
    void validateEntryPoint(
        EntryPoint*     entryPoint,
        DiagnosticSink* sink)
    {
        auto entryPointFuncDecl = entryPoint->getFuncDecl();
        auto stage = entryPoint->getStage();

        // TODO: We currently do minimal checking here, but this is the
        // right place to perform the following validation checks:
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

        auto entryPointName = entryPointFuncDecl->getName();

        auto module = getModule(entryPointFuncDecl);
        auto linkage = module->getLinkage();


        // Every entry point needs to have a stage specified either via
        // command-line/API options, or via an explicit `[shader("...")]` attribute.
        //
        if( stage == Stage::Unknown )
        {
            sink->diagnose(entryPointFuncDecl, Diagnostics::entryPointHasNoStage, entryPointName);
        }

        if( stage == Stage::Hull )
        {
            // TODO: We could consider *always* checking any `[patchconsantfunc("...")]`
            // attributes, so that they need to resolve to a function.

            auto attr = entryPointFuncDecl->FindModifier<PatchConstantFuncAttribute>();

            if (attr)
            {
                if (attr->args.getCount() != 1)
                {
                    sink->diagnose(attr, Diagnostics::badlyDefinedPatchConstantFunc, entryPointName);
                    return;
                }

                Expr* expr = attr->args[0];
                StringLiteralExpr* stringLit = as<StringLiteralExpr>(expr);

                if (!stringLit)
                {
                    sink->diagnose(expr, Diagnostics::badlyDefinedPatchConstantFunc, entryPointName);
                    return;
                }

                // We look up the patch-constant function by its name in the module
                // scope of the translation unit that declared the HS entry point.
                //
                // TODO: Eventually we probably want to do the lookup in the scope
                // of the parent declarations of the entry point. E.g., if the entry
                // point is a member function of a `struct`, then its patch-constant
                // function should be allowed to be another member function of
                // the same `struct`.
                //
                // In the extremely long run we may want to support an alternative to
                // this attribute-based linkage between the two functions that
                // make up the entry point.
                //
                Name* name = linkage->getNamePool()->getName(stringLit->value);
                FuncDecl* patchConstantFuncDecl = findFunctionDeclByName(
                    module,
                    name,
                    sink);
                if (!patchConstantFuncDecl)
                {
                    sink->diagnose(expr, Diagnostics::attributeFunctionNotFound, name, "patchconstantfunc");
                    return;
                }

                attr->patchConstantFuncDecl = patchConstantFuncDecl;
            }
        }
        else if(stage == Stage::Compute)
        {
            for(const auto& param : entryPointFuncDecl->GetParameters())
            {
                if(auto semantic = param->FindModifier<HLSLSimpleSemantic>())
                {
                    const auto& semanticToken = semantic->name;

                    String lowerName = String(semanticToken.Content).toLower();

                    if(lowerName == "sv_dispatchthreadid")
                    {
                        Type* paramType = param->getType();

                        if(!isValidThreadDispatchIDType(paramType))
                        {
                            String typeString = paramType->ToString();
                            sink->diagnose(param->loc, Diagnostics::invalidDispatchThreadIDType, typeString);
                            return;
                        }
                    }
                }
            }
        }
    }

    // Given an entry point specified via API or command line options,
    // attempt to find a matching AST declaration that implements the specified
    // entry point. If such a function is found, then validate that it actually
    // meets the requirements for the selected stage/profile.
    //
    // Returns an `EntryPoint` object representing the (unspecialized)
    // entry point if it is found and validated, and null otherwise.
    //
    RefPtr<EntryPoint> findAndValidateEntryPoint(
        FrontEndEntryPointRequest*  entryPointReq)
    {
        // The first step in validating the entry point is to find
        // the (unique) function declaration that matches its name.
        //
        // TODO: We may eventually want/need to extend this to
        // account for nested names like `SomeStruct.vsMain`, or
        // indeed even to handle generics.
        //
        auto compileRequest = entryPointReq->getCompileRequest();
        auto translationUnit = entryPointReq->getTranslationUnit();
        auto linkage = compileRequest->getLinkage();
        auto sink = compileRequest->getSink();
        auto translationUnitSyntax = translationUnit->getModuleDecl();

        auto entryPointName = entryPointReq->getName();

        // Make sure we've got a query-able member dictionary
        buildMemberDictionary(translationUnitSyntax);

        // We will look up any global-scope declarations in the translation
        // unit that match the name of our entry point.
        Decl* firstDeclWithName = nullptr;
        if( !translationUnitSyntax->memberDictionary.TryGetValue(entryPointName, firstDeclWithName) )
        {
            // If there doesn't appear to be any such declaration, then
            // we need to diagnose it as an error, and then bail out.
            sink->diagnose(translationUnitSyntax, Diagnostics::entryPointFunctionNotFound, entryPointName);
            return nullptr;
        }

        // We found at least one global-scope declaration with the right name,
        // but (1) it might not be a function, and (2) there might be
        // more than one function.
        //
        // We'll walk the linked list of declarations with the same name,
        // to see what we find. Along the way we'll keep track of the
        // first function declaration we find, if any:
        //
        FuncDecl* entryPointFuncDecl = nullptr;
        for(auto ee = firstDeclWithName; ee; ee = ee->nextInContainerWithSameName)
        {
            // We want to support the case where the declaration is
            // a generic function, so we will automatically
            // unwrap any outer `GenericDecl` we find here.
            //
            auto decl = ee;
            if(auto genericDecl = as<GenericDecl>(decl))
                decl = genericDecl->inner;

            // Is this declaration a function?
            if (auto funcDecl = as<FuncDecl>(decl))
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

                    sink->diagnose(translationUnitSyntax, Diagnostics::ambiguousEntryPoint, entryPointName);

                    // List all of the declarations that the user *might* mean
                    for (auto ff = firstDeclWithName; ff; ff = ff->nextInContainerWithSameName)
                    {
                        if (auto candidate = as<FuncDecl>(ff))
                        {
                            sink->diagnose(candidate, Diagnostics::entryPointCandidate, candidate->getName());
                        }
                    }

                    // Bail out.
                    return nullptr;
                }
            }
        }

        // Did we find a function declaration in our search?
        if(!entryPointFuncDecl)
        {
            // If not, then we need to diagnose the error.
            // For convenience, we will point to the first
            // declaration with the right name, that wasn't a function.
            sink->diagnose(firstDeclWithName, Diagnostics::entryPointSymbolNotAFunction, entryPointName);
            return nullptr;
        }

        // TODO: it is possible that the entry point was declared with
        // profile or target overloading. Is there anything that we need
        // to do at this point to filter out declarations that aren't
        // relevant to the selected profile for the entry point?

        // We found something, and can start doing some basic checking.
        //
        // If the entry point specifies a stage via a `[shader("...")]` attribute,
        // then we might be able to infer a stage for the entry point request if
        // it didn't have one, *or* issue a diagnostic if there is a mismatch.
        //
        auto entryPointProfile = entryPointReq->getProfile();
        if( auto entryPointAttribute = entryPointFuncDecl->FindModifier<EntryPointAttribute>() )
        {
            auto entryPointStage = entryPointProfile.GetStage();
            if( entryPointStage == Stage::Unknown )
            {
                entryPointProfile.setStage(entryPointAttribute->stage);
            }
            else if( entryPointAttribute->stage != entryPointStage )
            {
                sink->diagnose(entryPointFuncDecl, Diagnostics::specifiedStageDoesntMatchAttribute, entryPointName, entryPointStage, entryPointAttribute->stage);
            }
        }
        else
        {
            // TODO: Should we attach a `[shader(...)]` attribute to an
            // entry point that didn't have one, so that we can have
            // a more uniform representation in the AST?
        }

        RefPtr<EntryPoint> entryPoint = EntryPoint::create(
            linkage,
            makeDeclRef(entryPointFuncDecl),
            entryPointProfile);

        // Now that we've *found* the entry point, it is time to validate
        // that it actually meets the constraints for the chosen stage/profile.
        //
        validateEntryPoint(entryPoint, sink);

        return entryPoint;
    }

    /// Get the name a variable will use for reflection purposes
Name* getReflectionName(VarDeclBase* varDecl)
{
    if (auto reflectionNameModifier = varDecl->FindModifier<ParameterGroupReflectionName>())
        return reflectionNameModifier->nameAndLoc.name;

    return varDecl->getName();
}

// Information tracked when doing a structural
// match of types.
struct StructuralTypeMatchStack
{
    DeclRef<VarDeclBase>        leftDecl;
    DeclRef<VarDeclBase>        rightDecl;
    StructuralTypeMatchStack*   parent;
};

static void diagnoseParameterTypeMismatch(
    DiagnosticSink*             sink,
    StructuralTypeMatchStack*   inStack)
{
    SLANG_ASSERT(inStack);

    // The bottom-most entry in the stack should represent
    // the shader parameters that kicked things off
    auto stack = inStack;
    while(stack->parent)
        stack = stack->parent;

    sink->diagnose(stack->leftDecl, Diagnostics::shaderParameterDeclarationsDontMatch, getReflectionName(stack->leftDecl));
    sink->diagnose(stack->rightDecl, Diagnostics::seeOtherDeclarationOf, getReflectionName(stack->rightDecl));
}

// Two types that were expected to match did not.
// Inform the user with a suitable message.
static void diagnoseTypeMismatch(
    DiagnosticSink*             sink,
    StructuralTypeMatchStack*   inStack)
{
    auto stack = inStack;
    SLANG_ASSERT(stack);
    diagnoseParameterTypeMismatch(sink, stack);

    auto leftType = GetType(stack->leftDecl);
    auto rightType = GetType(stack->rightDecl);

    if( stack->parent )
    {
        sink->diagnose(stack->leftDecl, Diagnostics::fieldTypeMisMatch, getReflectionName(stack->leftDecl), leftType, rightType);
        sink->diagnose(stack->rightDecl, Diagnostics::seeOtherDeclarationOf, getReflectionName(stack->rightDecl));

        stack = stack->parent;
        if( stack )
        {
            while( stack->parent )
            {
                sink->diagnose(stack->leftDecl, Diagnostics::usedInDeclarationOf, getReflectionName(stack->leftDecl));
                stack = stack->parent;
            }
        }
    }
    else
    {
        sink->diagnose(stack->leftDecl, Diagnostics::shaderParameterTypeMismatch, leftType, rightType);
    }
}

// Two types that were expected to match did not.
// Inform the user with a suitable message.
static void diagnoseTypeFieldsMismatch(
    DiagnosticSink*             sink,
    DeclRef<Decl> const&        left,
    DeclRef<Decl> const&        right,
    StructuralTypeMatchStack*   stack)
{
    diagnoseParameterTypeMismatch(sink, stack);

    sink->diagnose(left, Diagnostics::fieldDeclarationsDontMatch, left.GetName());
    sink->diagnose(right, Diagnostics::seeOtherDeclarationOf, right.GetName());

    if( stack )
    {
        while( stack->parent )
        {
            sink->diagnose(stack->leftDecl, Diagnostics::usedInDeclarationOf, getReflectionName(stack->leftDecl));
            stack = stack->parent;
        }
    }
}

static void collectFields(
    DeclRef<AggTypeDecl>    declRef,
    List<DeclRef<VarDecl>>& outFields)
{
    for( auto fieldDeclRef : getMembersOfType<VarDecl>(declRef) )
    {
        if(fieldDeclRef.getDecl()->HasModifier<HLSLStaticModifier>())
            continue;

        outFields.add(fieldDeclRef);
    }
}

static bool validateTypesMatch(
    DiagnosticSink*             sink,
    Type*                       left,
    Type*                       right,
    StructuralTypeMatchStack*   stack);

static bool validateIntValuesMatch(
    DiagnosticSink*             sink,
    IntVal*                     left,
    IntVal*                     right,
    StructuralTypeMatchStack*   stack)
{
    if(left->EqualsVal(right))
        return true;

    // TODO: are there other cases we need to handle here?

    diagnoseTypeMismatch(sink, stack);
    return false;
}


static bool validateValuesMatch(
    DiagnosticSink*             sink,
    Val*                        left,
    Val*                        right,
    StructuralTypeMatchStack*   stack)
{
    if( auto leftType = dynamicCast<Type>(left) )
    {
        if( auto rightType = dynamicCast<Type>(right) )
        {
            return validateTypesMatch(sink, leftType, rightType, stack);
        }
    }

    if( auto leftInt = dynamicCast<IntVal>(left) )
    {
        if( auto rightInt = dynamicCast<IntVal>(right) )
        {
            return validateIntValuesMatch(sink, leftInt, rightInt, stack);
        }
    }

    if( auto leftWitness = dynamicCast<SubtypeWitness>(left) )
    {
        if( auto rightWitness = dynamicCast<SubtypeWitness>(right) )
        {
            return true;
        }
    }

    diagnoseTypeMismatch(sink, stack);
    return false;
}

static bool validateGenericSubstitutionsMatch(
    DiagnosticSink*             sink,
    GenericSubstitution*        left,
    GenericSubstitution*        right,
    StructuralTypeMatchStack*   stack)
{
    if( !left )
    {
        if( !right )
        {
            return true;
        }

        diagnoseTypeMismatch(sink, stack);
        return false;
    }



    Index argCount = left->args.getCount();
    if( argCount != right->args.getCount() )
    {
        diagnoseTypeMismatch(sink, stack);
        return false;
    }

    for( Index aa = 0; aa < argCount; ++aa )
    {
        auto leftArg = left->args[aa];
        auto rightArg = right->args[aa];

        if(!validateValuesMatch(sink, leftArg, rightArg, stack))
            return false;
    }

    return true;
}

static bool validateThisTypeSubstitutionsMatch(
    DiagnosticSink*             /*sink*/,
    ThisTypeSubstitution*       /*left*/,
    ThisTypeSubstitution*       /*right*/,
    StructuralTypeMatchStack*   /*stack*/)
{
    // TODO: actual checking.
    return true;
}

static bool validateSpecializationsMatch(
    DiagnosticSink*             sink,
    SubstitutionSet             left,
    SubstitutionSet             right,
    StructuralTypeMatchStack*   stack)
{
    auto ll = left.substitutions;
    auto rr = right.substitutions;
    for(;;)
    {
        // Skip any global generic substitutions.
        if(auto leftGlobalGeneric = as<GlobalGenericParamSubstitution>(ll))
        {
            ll = leftGlobalGeneric->outer;
            continue;
        }
        if(auto rightGlobalGeneric = as<GlobalGenericParamSubstitution>(rr))
        {
            rr = rightGlobalGeneric->outer;
            continue;
        }

        // If either ran out, then we expect both to have run out.
        if(!ll || !rr)
            return !ll && !rr;

        auto leftSubst = ll;
        auto rightSubst = rr;

        ll = ll->outer;
        rr = rr->outer;

        if(auto leftGeneric = as<GenericSubstitution>(leftSubst))
        {
            if(auto rightGeneric = as<GenericSubstitution>(rightSubst))
            {
                if(validateGenericSubstitutionsMatch(sink, leftGeneric, rightGeneric, stack))
                {
                    continue;
                }
            }
        }
        else if(auto leftThisType = as<ThisTypeSubstitution>(leftSubst))
        {
            if(auto rightThisType = as<ThisTypeSubstitution>(rightSubst))
            {
                if(validateThisTypeSubstitutionsMatch(sink, leftThisType, rightThisType, stack))
                {
                    continue;
                }
            }
        }

        return false;
    }

    return true;
}

// Determine if two types "match" for the purposes of `cbuffer` layout rules.
//
static bool validateTypesMatch(
    DiagnosticSink*             sink,
    Type*                       left,
    Type*                       right,
    StructuralTypeMatchStack*   stack)
{
    if(left->Equals(right))
        return true;

    // It is possible that the types don't match exactly, but
    // they *do* match structurally.

    // Note: the following code will lead to infinite recursion if there
    // are ever recursive types. We'd need a more refined system to
    // cache the matches we've already found.

    if( auto leftDeclRefType = as<DeclRefType>(left) )
    {
        if( auto rightDeclRefType = as<DeclRefType>(right) )
        {
            // Are they references to matching decl refs?
            auto leftDeclRef = leftDeclRefType->declRef;
            auto rightDeclRef = rightDeclRefType->declRef;

            // Do the reference the same declaration? Or declarations
            // with the same name?
            //
            // TODO: we should only consider the same-name case if the
            // declarations come from translation units being compiled
            // (and not an imported module).
            if( leftDeclRef.getDecl() == rightDeclRef.getDecl()
                || leftDeclRef.GetName() == rightDeclRef.GetName() )
            {
                // Check that any generic arguments match
                if( !validateSpecializationsMatch(
                    sink,
                    leftDeclRef.substitutions,
                    rightDeclRef.substitutions,
                    stack) )
                {
                    return false;
                }

                // Check that any declared fields match too.
                if( auto leftStructDeclRef = leftDeclRef.as<AggTypeDecl>() )
                {
                    if( auto rightStructDeclRef = rightDeclRef.as<AggTypeDecl>() )
                    {
                        List<DeclRef<VarDecl>> leftFields;
                        List<DeclRef<VarDecl>> rightFields;

                        collectFields(leftStructDeclRef, leftFields);
                        collectFields(rightStructDeclRef, rightFields);

                        Index leftFieldCount = leftFields.getCount();
                        Index rightFieldCount = rightFields.getCount();

                        if( leftFieldCount != rightFieldCount )
                        {
                            diagnoseTypeFieldsMismatch(sink, leftDeclRef, rightDeclRef, stack);
                            return false;
                        }

                        for( Index ii = 0; ii < leftFieldCount; ++ii )
                        {
                            auto leftField = leftFields[ii];
                            auto rightField = rightFields[ii];

                            if( leftField.GetName() != rightField.GetName() )
                            {
                                diagnoseTypeFieldsMismatch(sink, leftDeclRef, rightDeclRef, stack);
                                return false;
                            }

                            auto leftFieldType = GetType(leftField);
                            auto rightFieldType = GetType(rightField);

                            StructuralTypeMatchStack subStack;
                            subStack.parent = stack;
                            subStack.leftDecl = leftField;
                            subStack.rightDecl = rightField;

                            if(!validateTypesMatch(sink, leftFieldType,rightFieldType, &subStack))
                                return false;
                        }
                    }
                }

                // Everything seemed to match recursively.
                return true;
            }
        }
    }

    // If we are looking at `T[N]` and `U[M]` we want to check that
    // `T` is structurally equivalent to `U` and `N` is the same as `M`.
    else if( auto leftArrayType = as<ArrayExpressionType>(left) )
    {
        if( auto rightArrayType = as<ArrayExpressionType>(right) )
        {
            if(!validateTypesMatch(sink, leftArrayType->baseType, rightArrayType->baseType, stack) )
                return false;

            if(!validateValuesMatch(sink, leftArrayType->ArrayLength, rightArrayType->ArrayLength, stack))
                return false;

            return true;
        }
    }

    diagnoseTypeMismatch(sink, stack);
    return false;
}

// This function is supposed to determine if two global shader
// parameter declarations represent the same logical parameter
// (so that they should get the exact same binding(s) allocated).
//
static bool doesParameterMatch(
    DiagnosticSink*         sink,
    DeclRef<VarDeclBase>    varDeclRef,
    DeclRef<VarDeclBase>    existingVarDeclRef)
{
    StructuralTypeMatchStack stack;
    stack.parent = nullptr;
    stack.leftDecl = varDeclRef;
    stack.rightDecl = existingVarDeclRef;

    validateTypesMatch(sink, GetType(varDeclRef), GetType(existingVarDeclRef), &stack);

    return true;
}

    void Module::_collectShaderParams()
    {
        auto moduleDecl = m_moduleDecl;

        // We are going to walk the global declarations in the body of the
        // module, and use those to build up our lists of:
        //
        // * Global shader parameters
        // * Specialization parameters (both generic and interface/existential)
        // * Requirements (`import`ed modules)
        //
        // For requirements, we want to be careful to only
        // add each required module once (in case the same
        // module got `import`ed multiple times), so we
        // will keep a set of the modules we've already
        // seen and processed.
        //
        HashSet<Module*> requiredModuleSet;

        for( auto globalDecl : moduleDecl->Members )
        {
            if(auto globalVar = globalDecl.as<VarDecl>())
            {
                // We do not want to consider global variable declarations
                // that don't represents shader parameters. This includes
                // things like `static` globals and `groupshared` variables.
                //
                if(!isGlobalShaderParameter(globalVar))
                    continue;

                // At this point we know we have a global shader parameter.

                GlobalShaderParamInfo shaderParamInfo;
                shaderParamInfo.paramDeclRef = makeDeclRef(globalVar.Ptr());

                // We need to consider what specialization parameters
                // are introduced by this shader parameter. This step
                // fills in fields on `shaderParamInfo` so that we
                // can assocaite specialization arguments supplied later
                // with the correct parameter.
                //
                _collectExistentialSpecializationParamsForShaderParam(
                    shaderParamInfo,
                    m_specializationParams,
                    makeDeclRef(globalVar.Ptr()));

                m_shaderParams.add(shaderParamInfo);
            }
            else if( auto globalGenericParam = as<GlobalGenericParamDecl>(globalDecl) )
            {
                // A global generic type parameter declaration introduces
                // a suitable specialization parameter.
                //
                SpecializationParam specializationParam;
                specializationParam.flavor = SpecializationParam::Flavor::GenericType;
                specializationParam.object = globalGenericParam;
                m_specializationParams.add(specializationParam);
            }
            else if( auto importDecl = as<ImportDecl>(globalDecl) )
            {
                // An `import` declaration creates a requirement dependency
                // from this module to another module.
                //
                auto importedModule = getModule(importDecl->importedModuleDecl);
                if(!requiredModuleSet.Contains(importedModule))
                {
                    requiredModuleSet.Add(importedModule);
                    m_requirements.add(importedModule);
                }
            }
        }
    }

    Index Module::getRequirementCount()
    {
        return m_requirements.getCount();
    }

    RefPtr<ComponentType> Module::getRequirement(Index index)
    {
        return m_requirements[index];
    }

    void Module::acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo)
    {
        visitor->visitModule(this, as<ModuleSpecializationInfo>(specializationInfo));
    }


        /// Enumerate the parameters of a `LegacyProgram`.
    void LegacyProgram::_collectShaderParams(DiagnosticSink* sink)
    {
        // We need to collect all of the global shader parameters
        // referenced by the compile request, and for each we
        // need to do a few things:
        //
        // * We need to determine if the parameter is a duplicate/redeclaration
        //   of the "same" parameter in another translation unit, and collapse
        //   those into one logical shader parameter if so.
        //
        // * We need to determine what existential type slots are introduced
        //   by the parameter, and associate that information with the parameter.
        //
        // To deal with the first issue, we will maintain a map from a parameter
        // name to the index of an existing parameter with that name.
        //
        // TODO: Eventually we should deprecate support for the
        // deduplication feature of `LegaqcyProgram`, at which point
        // this entire type and all its complications can be eliminated
        // from the code (that includes a lot of support in the "parameter
        // binding" step for shader parameters with multiple declarations).
        // Until that point this type will have a fair amount of duplication
        // with stuff in `Module` and `CompositeComponentType`.

        // We use a dictionary to keep track of any shader parameter
        // we've alrady collected with a given name.
        //
        Dictionary<Name*, Int> mapNameToParamIndex;

        for( auto translationUnit : m_translationUnits )
        {
            auto module = translationUnit->getModule();
            auto moduleDecl = module->getModuleDecl();
            for( auto globalVar : moduleDecl->getMembersOfType<VarDecl>() )
            {
                // We do not want to consider global variable declarations
                // that don't represents shader parameters. This includes
                // things like `static` globals and `groupshared` variables.
                //
                if(!isGlobalShaderParameter(globalVar))
                    continue;

                // This declaration may represent the same logical parameter
                // as a declaration that came from a different translation unit.
                // If that is the case, we want to re-use the same `ShaderParamInfo`
                // across both parameters.
                //
                // TODO: This logic currently detects *any* global-scope parameters
                // with matching names, but it should eventually be narrowly
                // scoped so that it only applies to parameters from unnamed modules
                // (that is, modules that represent directly-compiled shader files
                // and not `import`ed code).
                //
                // First we look for an existing entry matching the name
                // of this parameter:
                //
                auto paramName = getReflectionName(globalVar);
                Int existingParamIndex = -1;
                if( mapNameToParamIndex.TryGetValue(paramName, existingParamIndex) )
                {
                    // If the parameters have the same name, but don't "match" according to some reasonable rules,
                    // then we will treat them as distinct global parameters.
                    //
                    // Note: all of the mismatch cases currently report errors, so that
                    // compilation will fail on a mismatch.
                    //
                    auto& existingParam = m_shaderParams[existingParamIndex];
                    if( doesParameterMatch(sink, makeDeclRef(globalVar.Ptr()), existingParam.paramDeclRef) )
                    {
                        // If we hit this case, then we had a match, and we should
                        // consider the new variable to be a redclaration of
                        // the existing one.

                        existingParam.additionalParamDeclRefs.add(
                            makeDeclRef(globalVar.Ptr()));
                        continue;
                    }
                }

                Int newParamIndex = Int(m_shaderParams.getCount());
                mapNameToParamIndex.Add(paramName, newParamIndex);

                GlobalShaderParamInfo shaderParamInfo;
                shaderParamInfo.paramDeclRef = makeDeclRef(globalVar.Ptr());

                _collectExistentialSpecializationParamsForShaderParam(
                    shaderParamInfo,
                    m_specializationParams,
                    makeDeclRef(globalVar.Ptr()));

                m_shaderParams.add(shaderParamInfo);
            }
        }
    }

        /// Create a new component type based on `inComponentType`, but with all its requiremetns filled.
    RefPtr<ComponentType> fillRequirements(
        ComponentType* inComponentType)
    {
        auto linkage = inComponentType->getLinkage();

        // We are going to simplify things by solving the problem iteratively.
        // If the current `componentType` has requirements for `A`, `B`, ... etc.
        // then we will create a composite of `componentType`, `A`, `B`, ...
        // and then see if the resulting composite has any requirements.
        //
        // This avoids the problem of trying to compute teh transitive closure
        // of the requirements relationship (while dealing with deduplication,
        // etc.)

        RefPtr<ComponentType> componentType = inComponentType;
        for(;;)
        {
            auto requirementCount = componentType->getRequirementCount();
            if(requirementCount == 0)
                break;

            List<RefPtr<ComponentType>> allComponents;
            allComponents.add(componentType);

            for(Index rr = 0; rr < requirementCount; ++rr)
            {
                auto requirement = componentType->getRequirement(rr);
                allComponents.add(requirement);
            }

            componentType = CompositeComponentType::create(
                linkage,
                allComponents);
        }
        return componentType;
    }

        /// Create a component type to represent the "global scope" of a compile request.
        ///
        /// This component type will include all the modules and their global
        /// parameters from the compile request, but not anything specific
        /// to any entry point functions.
        ///
        /// The layout for this component type will thus represent the things that
        /// a user is likely to want to have stay the same across all compiled
        /// entry points.
        ///
        /// The component type that this function creates is unspecialized, in
        /// that it doesn't take into account any specialization arguments
        /// that might have been supplied as part of the compile request.
        ///
    RefPtr<ComponentType> createUnspecializedGlobalComponentType(
        FrontEndCompileRequest* compileRequest)
    {
        // We want our resulting program to depend on
        // all the translation units the user specified,
        // even if some of them don't contain entry points
        // (this is important for parameter layout/binding).
        //
        // We also want to ensure that the modules for the
        // translation units comes first in the enumerated
        // order for dependencies, to match the pre-existing
        // compiler behavior (at least for now).
        //
        auto linkage = compileRequest->getLinkage();
        auto sink = compileRequest->getSink();

        RefPtr<ComponentType> globalComponentType;
        if(compileRequest->translationUnits.getCount() == 1)
        {
            // The common case is that a compilation only uses
            // a single translation unit, and thus results in
            // a single `Module`. We can then use that module
            // as the component type that represents the global scope.
            //
            globalComponentType = compileRequest->translationUnits[0]->getModule();
        }
        else
        {
            globalComponentType = new LegacyProgram(
                linkage,
                compileRequest->translationUnits,
                sink);
        }

        return fillRequirements(globalComponentType);
    }

        /// Create a component type that represents the global scope for a compile request,
        /// along with any entry point functions.
        ///
        /// The resulting component type will include the global-scope information
        /// first, so its layout will be compatible with the result of
        /// `createUnspecializedGlobalComponentType`.
        ///
        /// The new component type will also add on any entry-point functions
        /// that were requested and will thus include space for their `uniform` parameters.
        /// If multiple entry points were requested then they will be given non-overlapping
        /// parameter bindings, consistent with them being used together in
        /// a single pipeline state, hit group, etc.
        ///
        /// The result of this function is unspecialized and doesn't take into
        /// account any specialization arguments the user might have supplied.
        ///
    RefPtr<ComponentType> createUnspecializedGlobalAndEntryPointsComponentType(
        FrontEndCompileRequest* compileRequest)
    {
        auto linkage = compileRequest->getLinkage();
        auto sink = compileRequest->getSink();

        auto globalComponentType = compileRequest->getGlobalComponentType();

        // The validation of entry points here will be modal, and controlled
        // by whether the user specified any entry points directly via
        // API or command-line options.
        //
        // TODO: We may want to make this choice explicit rather than implicit.
        //
        // First, check if the user requested any entry points explicitly via
        // the API or command line.
        //
        bool anyExplicitEntryPoints = compileRequest->getEntryPointReqCount() != 0;

        List<RefPtr<ComponentType>> allComponentTypes;
        allComponentTypes.add(globalComponentType);

        if( anyExplicitEntryPoints )
        {
            // If there were any explicit requests for entry points to be
            // checked, then we will *only* check those.
            //
            for(auto entryPointReq : compileRequest->getEntryPointReqs())
            {
                auto entryPoint = findAndValidateEntryPoint(
                    entryPointReq);
                if( entryPoint )
                {
                    // TODO: We need to implement an explicit policy
                    // for what should happen if the user specified
                    // entry points via the command-line (or API),
                    // but didn't specify any groups (since the current
                    // compilation API doesn't allow for grouping).
                    //
                    entryPointReq->getTranslationUnit()->entryPoints.add(entryPoint);

                    allComponentTypes.add(entryPoint);
                }
            }

            // TODO: We should consider always processing both categories,
            // and just making sure to only check each entry point function
            // declaration once...
        }
        else
        {
            // Otherwise, scan for any `[shader(...)]` attributes in
            // the user's code, and construct `EntryPoint`s to
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
            Index translationUnitCount = compileRequest->translationUnits.getCount();
            for(Index tt = 0; tt < translationUnitCount; ++tt)
            {
                auto translationUnit = compileRequest->translationUnits[tt];
                for( auto globalDecl : translationUnit->getModuleDecl()->Members )
                {
                    auto maybeFuncDecl = globalDecl;
                    if( auto genericDecl = as<GenericDecl>(maybeFuncDecl) )
                    {
                        maybeFuncDecl = genericDecl->inner;
                    }

                    auto funcDecl = as<FuncDecl>(maybeFuncDecl);
                    if(!funcDecl)
                        continue;

                    auto entryPointAttr = funcDecl->FindModifier<EntryPointAttribute>();
                    if(!entryPointAttr)
                        continue;

                    // We've discovered a valid entry point. It is a function (possibly
                    // generic) that has a `[shader(...)]` attribute to mark it as an
                    // entry point.
                    //
                    // We will now register that entry point as an `EntryPoint`
                    // with an appropriately chosen profile.
                    //
                    // The profile will only include a stage, so that the profile "family"
                    // and "version" are left unspecified. Downstream code will need
                    // to be able to handle this case.
                    //
                    Profile profile;
                    profile.setStage(entryPointAttr->stage);

                    RefPtr<EntryPoint> entryPoint = EntryPoint::create(
                        linkage,
                        makeDeclRef(funcDecl),
                        profile);

                    validateEntryPoint(entryPoint, sink);

                    // Note: in the case that the user didn't explicitly
                    // specify entry points and we are instead compiling
                    // a shader "library," then we do not want to automatically
                    // combine the entry points into groups in the generated
                    // `Program`, since that would be slightly too magical.
                    //
                    // Instead, each entry point will end up in a singleton
                    // group, so that its entry-point parameters lay out
                    // independent of the others.
                    //
                    translationUnit->entryPoints.add(entryPoint);

                    allComponentTypes.add(entryPoint);
                }
            }
        }

        if(allComponentTypes.getCount() > 1)
        {
            auto composite = CompositeComponentType::create(
                linkage,
                allComponentTypes);
            return composite;
        }
        else
        {
            return globalComponentType;
        }
    }

    RefPtr<ComponentType::SpecializationInfo> Module::_validateSpecializationArgsImpl(
        SpecializationArg const*    args,
        Index                       argCount,
        DiagnosticSink*             sink)
    {
        SLANG_ASSERT(argCount == getSpecializationParamCount());

        SemanticsVisitor visitor(getLinkage(), sink);

        RefPtr<Module::ModuleSpecializationInfo> specializationInfo = new Module::ModuleSpecializationInfo();

        for( Index ii = 0; ii < argCount; ++ii )
        {
            auto& arg = args[ii];
            auto& param = m_specializationParams[ii];

            auto argType = arg.val.as<Type>();
            SLANG_ASSERT(argType);

            switch( param.flavor )
            {
            case SpecializationParam::Flavor::GenericType:
                {
                    auto genericTypeParamDecl = param.object.as<GlobalGenericParamDecl>();
                    SLANG_ASSERT(genericTypeParamDecl);

                    // TODO: There is a serious flaw to this checking logic if we ever have cases where
                    // the constraints on one `type_param` can depend on another `type_param`, e.g.:
                    //
                    //      type_param A;
                    //      type_param B : ISidekick<A>;
                    //
                    // In that case, if a user tries to set `B` to `Robin` and `Robin` conforms to
                    // `ISidekick<Batman>`, then the compiler needs to know whether `A` is being
                    // set to `Batman` to know whether the setting for `B` is valid. In this limit
                    // the constraints can be mutually recursive (so `A : IMentor<B>`).
                    //
                    // The only way to check things correctly is to validate each conformance under
                    // a set of assumptions (substitutions) that includes all the type substitutions,
                    // and possibly also all the other constraints *except* the one to be validated.
                    //
                    // We will punt on this for now, and just check each constraint in isolation.

                    // As a quick sanity check, see if the argument that is being supplied for a
                    // global generic type parameter is a reference to *another* global generic
                    // type parameter, since that should always be an error.
                    //
                    if( auto argDeclRefType = argType.as<DeclRefType>() )
                    {
                        auto argDeclRef = argDeclRefType->declRef;
                        if(auto argGenericParamDeclRef = argDeclRef.as<GlobalGenericParamDecl>())
                        {
                            if(argGenericParamDeclRef.getDecl() == genericTypeParamDecl)
                            {
                                // We are trying to specialize a generic parameter using itself.
                                sink->diagnose(genericTypeParamDecl,
                                    Diagnostics::cannotSpecializeGlobalGenericToItself,
                                    genericTypeParamDecl->getName());
                                continue;
                            }
                            else
                            {
                                // We are trying to specialize a generic parameter using a *different*
                                // global generic type parameter.
                                sink->diagnose(genericTypeParamDecl,
                                    Diagnostics::cannotSpecializeGlobalGenericToAnotherGenericParam,
                                    genericTypeParamDecl->getName(),
                                    argGenericParamDeclRef.GetName());
                                continue;
                            }
                        }
                    }

                    ModuleSpecializationInfo::GenericArgInfo genericArgInfo;
                    genericArgInfo.paramDecl = genericTypeParamDecl;
                    genericArgInfo.argVal = argType;
                    specializationInfo->genericArgs.add(genericArgInfo);

                    // Walk through the declared constraints for the parameter,
                    // and check that the argument actually satisfies them.
                    for(auto constraintDecl : genericTypeParamDecl->getMembersOfType<GenericTypeConstraintDecl>())
                    {
                        // Get the type that the constraint is enforcing conformance to
                        auto interfaceType = GetSup(DeclRef<GenericTypeConstraintDecl>(constraintDecl, nullptr));

                        // Use our semantic-checking logic to search for a witness to the required conformance
                        auto witness = visitor.tryGetSubtypeWitness(argType, interfaceType);
                        if (!witness)
                        {
                            // If no witness was found, then we will be unable to satisfy
                            // the conformances required.
                            sink->diagnose(genericTypeParamDecl,
                                Diagnostics::typeArgumentForGenericParameterDoesNotConformToInterface,
                                argType,
                                genericTypeParamDecl->nameAndLoc.name,
                                interfaceType);
                        }

                        ModuleSpecializationInfo::GenericArgInfo constraintArgInfo;
                        constraintArgInfo.paramDecl = constraintDecl;
                        constraintArgInfo.argVal = witness;
                        specializationInfo->genericArgs.add(constraintArgInfo);
                    }
                }
                break;

            case SpecializationParam::Flavor::ExistentialType:
                {
                    auto interfaceType = param.object.as<Type>();
                    SLANG_ASSERT(interfaceType);

                    auto witness = visitor.tryGetSubtypeWitness(argType, interfaceType);
                    if (!witness)
                    {
                            // If no witness was found, then we will be unable to satisfy
                            // the conformances required.
                            sink->diagnose(SourceLoc(),
                                Diagnostics::typeArgumentDoesNotConformToInterface,
                                argType,
                                interfaceType);
                    }

                    ExpandedSpecializationArg expandedArg;
                    expandedArg.val = argType;
                    expandedArg.witness = witness;

                    specializationInfo->existentialArgs.add(expandedArg);
                }
                break;

            default:
                SLANG_UNEXPECTED("unhandled specialization parameter flavor");
            }
        }

        return specializationInfo;
    }


    static void _extractSpecializationArgs(
        ComponentType*              componentType,
        List<RefPtr<Expr>> const&   argExprs,
        List<SpecializationArg>&    outArgs,
        DiagnosticSink*             sink)
    {
        auto linkage = componentType->getLinkage();

        auto argCount = argExprs.getCount();
        for(Index ii = 0; ii < argCount; ++ii )
        {
            auto argExpr = argExprs[ii];
            auto paramInfo = componentType->getSpecializationParam(ii);

            // TODO: We should support non-type arguments here

            auto argType = checkProperType(linkage, TypeExp(argExpr), sink);
            if( !argType )
            {
                // If no witness was found, then we will be unable to satisfy
                // the conformances required.
                sink->diagnose(argExpr,
                    Diagnostics::expectedAType,
                    argExpr->type);
                continue;
            }

            SpecializationArg arg;
            arg.val = argType;
            outArgs.add(arg);
        }
    }

    RefPtr<ComponentType::SpecializationInfo> EntryPoint::_validateSpecializationArgsImpl(
        SpecializationArg const*    inArgs,
        Index                       inArgCount,
        DiagnosticSink*             sink)
    {
        auto args = inArgs;
        auto argCount = inArgCount;

        SemanticsVisitor visitor(getLinkage(), sink);

        // The first N arguments will be for the explicit generic parameters
        // of the entry point (if it has any).
        //
        auto genericSpecializationParamCount = getGenericSpecializationParamCount();
        SLANG_ASSERT(argCount >= genericSpecializationParamCount);

        Result result = SLANG_OK;

        RefPtr<EntryPointSpecializationInfo> info = new EntryPointSpecializationInfo();

        DeclRef<FuncDecl> specializedFuncDeclRef = m_funcDeclRef;
        if(genericSpecializationParamCount)
        {
            // We need to construct a generic application and use
            // the semantic checking machinery to expand out
            // the rest of the arguments via inference...

            auto genericDeclRef = m_funcDeclRef.GetParent().as<GenericDecl>();
            SLANG_ASSERT(genericDeclRef); // otherwise we wouldn't have generic parameters

            RefPtr<GenericSubstitution> genericSubst = new GenericSubstitution();
            genericSubst->outer = genericDeclRef.substitutions.substitutions;
            genericSubst->genericDecl = genericDeclRef.getDecl();

            for(Index ii = 0; ii < genericSpecializationParamCount; ++ii)
            {
                auto specializationArg = args[ii];
                genericSubst->args.add(specializationArg.val);
            }

            for( auto constraintDecl : genericDeclRef.getDecl()->getMembersOfType<GenericTypeConstraintDecl>() )
            {
                auto constraintSubst = genericDeclRef.substitutions;
                constraintSubst.substitutions = genericSubst;

                DeclRef<GenericTypeConstraintDecl> constraintDeclRef(
                    constraintDecl, constraintSubst);

                auto sub = GetSub(constraintDeclRef);
                auto sup = GetSup(constraintDeclRef);

                auto subTypeWitness = visitor.tryGetSubtypeWitness(sub, sup);
                if(subTypeWitness)
                {
                    genericSubst->args.add(subTypeWitness);
                }
                else
                {
                    // TODO: diagnose a problem here
                    sink->diagnose(constraintDecl, Diagnostics::typeArgumentDoesNotConformToInterface, sub, sup);
                    result = SLANG_FAIL;
                    continue;
                }
            }

            specializedFuncDeclRef.substitutions.substitutions = genericSubst;
        }

        info->specializedFuncDeclRef = specializedFuncDeclRef;

        // Once the generic parameters (if any) have been dealt with,
        // any remaining specialization arguments are for existential/interface
        // specialization parameters, attached to the value parameters
        // of the entry point.
        //
        args += genericSpecializationParamCount;
        argCount -= genericSpecializationParamCount;

        auto existentialSpecializationParamCount = getExistentialSpecializationParamCount();
        SLANG_ASSERT(argCount == existentialSpecializationParamCount);

        for( Index ii = 0; ii < existentialSpecializationParamCount; ++ii )
        {
            auto& param = m_existentialSpecializationParams[ii];
            auto& specializationArg = args[ii];

            // TODO: We need to handle all the cases of "flavor" for the `param`s (not just types)

            auto paramType = param.object.as<Type>();
            auto argType = specializationArg.val.as<Type>();

            auto witness = visitor.tryGetSubtypeWitness(argType, paramType);
            if (!witness)
            {
                // If no witness was found, then we will be unable to satisfy
                // the conformances required.
                sink->diagnose(SourceLoc(), Diagnostics::typeArgumentDoesNotConformToInterface, argType, paramType);
                result = SLANG_FAIL;
                continue;
            }

            ExpandedSpecializationArg expandedArg;
            expandedArg.val = specializationArg.val;
            expandedArg.witness = witness;
            info->existentialSpecializationArgs.add(expandedArg);
        }

        return info;
    }

            /// Create a specialization an existing entry point based on specialization argument expressions.
    RefPtr<ComponentType> createSpecializedEntryPoint(
        EntryPoint*                 unspecializedEntryPoint,
        List<RefPtr<Expr>> const&   argExprs,
        DiagnosticSink*             sink)
    {
        // We need to convert all of the `Expr` arguments
        // into `SpecializationArg`s, so that we can bottleneck
        // through the shared logic.
        //
        List<SpecializationArg> args;
        _extractSpecializationArgs(unspecializedEntryPoint, argExprs, args, sink);
        if(sink->GetErrorCount())
            return nullptr;

        return unspecializedEntryPoint->specialize(
            args.getBuffer(),
            args.getCount(),
            sink);
    }

        /// Parse an array of strings as specialization arguments.
        ///
        /// Names in the strings will be parsed in the context of
        /// the code loaded into the given compile request.
        ///
    void parseSpecializationArgStrings(
        EndToEndCompileRequest* endToEndReq,
        List<String> const&     genericArgStrings,
        List<RefPtr<Expr>>&     outGenericArgs)
    {
        auto unspecialiedProgram = endToEndReq->getUnspecializedGlobalComponentType();

        // TODO: Building a list of `scopesToTry` here shouldn't
        // be required, since the `Scope` type itself has the ability
        // for form chains for lookup purposes (e.g., the way that
        // `import` is handled by modifying a scope).
        //
        List<RefPtr<Scope>> scopesToTry;
        for( auto module : unspecialiedProgram->getModuleDependencies() )
            scopesToTry.add(module->getModuleDecl()->scope);

        // We are going to do some semantic checking, so we need to
        // set up a `SemanticsVistitor` that we can use.
        //
        auto linkage = endToEndReq->getLinkage();
        auto sink = endToEndReq->getSink();
        SemanticsVisitor semantics(
            linkage,
            sink);

        // We will be looping over the generic argument strings
        // that the user provided via the API (or command line),
        // and parsing+checking each into an `Expr`.
        //
        // This loop will *not* handle coercing the arguments
        // to be types.
        //
        for(auto name : genericArgStrings)
        {
            RefPtr<Expr> argExpr;
            for (auto & s : scopesToTry)
            {
                argExpr = linkage->parseTypeString(name, s);
                argExpr = semantics.CheckTerm(argExpr);
                if( argExpr )
                {
                    break;
                }
            }

            if(!argExpr)
            {
                sink->diagnose(SourceLoc(), Diagnostics::internalCompilerError, "couldn't parse specialization argument");
                return;
            }

            outGenericArgs.add(argExpr);
        }
    }

    Type* Linkage::specializeType(
        Type*           unspecializedType,
        Int             argCount,
        Type* const*    args,
        DiagnosticSink* sink)
    {
        SLANG_ASSERT(unspecializedType);

        // TODO: We should cache and re-use specialized types
        // when the exact same arguments are provided again later.

        SemanticsVisitor visitor(this, sink);

        SpecializationParams specializationParams;
        _collectExistentialSpecializationParamsRec(specializationParams, unspecializedType);

        assert(specializationParams.getCount() == argCount);

        ExpandedSpecializationArgs specializationArgs;
        for( Int aa = 0; aa < argCount; ++aa )
        {
            auto paramType = specializationParams[aa].object.as<Type>();
            auto argType = args[aa];

            ExpandedSpecializationArg arg;
            arg.val = argType;
            arg.witness = visitor.tryGetSubtypeWitness(argType, paramType);
            specializationArgs.add(arg);
        }

        RefPtr<ExistentialSpecializedType> specializedType = new ExistentialSpecializedType();
        specializedType->baseType = unspecializedType;
        specializedType->args = specializationArgs;

        m_specializedTypes.add(specializedType);

        return specializedType;
    }

        /// Shared implementation logic for the `_createSpecializedProgram*` entry points.
    static RefPtr<ComponentType> _createSpecializedProgramImpl(
        Linkage*                    linkage,
        ComponentType*              unspecializedProgram,
        List<RefPtr<Expr>> const&   specializationArgExprs,
        DiagnosticSink*             sink)
    {
        // If there are no specialization arguments,
        // then the the result of specialization should
        // be the same as the input.
        //
        auto specializationArgCount = specializationArgExprs.getCount();
        if( specializationArgCount == 0 )
        {
            return unspecializedProgram;
        }

        auto specializationParamCount = unspecializedProgram->getSpecializationParamCount();
        if(specializationArgCount != specializationParamCount )
        {
            sink->diagnose(SourceLoc(), Diagnostics::mismatchSpecializationArguments,
                specializationParamCount,
                specializationArgCount);
            return nullptr;
        }

        // We have an appropriate number of arguments for the global specialization parameters,
        // and now we need to check that the arguments conform to the declared constraints.
        //
        SemanticsVisitor visitor(linkage, sink);

        List<SpecializationArg> specializationArgs;
        _extractSpecializationArgs(unspecializedProgram, specializationArgExprs, specializationArgs, sink);
        if(sink->GetErrorCount())
            return nullptr;

        auto specializedProgram = unspecializedProgram->specialize(
            specializationArgs.getBuffer(),
            specializationArgs.getCount(),
            sink);

        return specializedProgram;
    }

        /// Specialize an entry point that was checked by the front-end, based on specialization arguments.
        ///
        /// If the end-to-end compile request included specialization argument strings
        /// for this entry point, then they will be parsed, checked, and used
        /// as arguments to the generic entry point.
        ///
        /// Returns a specialized entry point if everything worked as expected.
        /// Returns null and diagnoses errors if anything goes wrong.
        ///
    RefPtr<ComponentType> createSpecializedEntryPoint(
        EndToEndCompileRequest*                         endToEndReq,
        EntryPoint*                                     unspecializedEntryPoint,
        EndToEndCompileRequest::EntryPointInfo const&   entryPointInfo)
    {
        auto sink = endToEndReq->getSink();
        auto entryPointFuncDecl = unspecializedEntryPoint->getFuncDecl();

        // If the user specified generic arguments for the entry point,
        // then we will need to parse the arguments first.
        //
        List<RefPtr<Expr>> specializationArgExprs;
        parseSpecializationArgStrings(
            endToEndReq,
            entryPointInfo.specializationArgStrings,
            specializationArgExprs);

        // Next we specialize the entry point function given the parsed
        // generic argument expressions.
        //
        auto entryPoint = createSpecializedEntryPoint(
            unspecializedEntryPoint,
            specializationArgExprs,
            sink);

        return entryPoint;
    }

        /// Create a specialized component type for the global scope of the given compile request.
        ///
        /// The specialized program will be consistent with that created by
        /// `createUnspecializedGlobalComponentType`, and will simply fill in
        /// its specialization parameters with the arguments (if any) supllied
        /// as part fo the end-to-end compile request.
        ///
        /// The layout of the new component type will be consistent with that
        /// of the original *if* there are no global generic type parameters
        /// (only interface/existential parameters).
        ///
    RefPtr<ComponentType> createSpecializedGlobalComponentType(
        EndToEndCompileRequest* endToEndReq)
    {
        // The compile request must have already completed front-end processing,
        // so that we have an unspecialized program available, and now only need
        // to parse and check any generic arguments that are being supplied for
        // global or entry-point generic parameters.
        //
        auto unspecializedProgram = endToEndReq->getUnspecializedGlobalComponentType();
        auto linkage = endToEndReq->getLinkage();
        auto sink = endToEndReq->getSink();

        // First, let's parse the specialization argument strings that were
        // provided via the API, so that we can match them
        // against what was declared in the program.
        //
        List<RefPtr<Expr>> globalSpecializationArgs;
        parseSpecializationArgStrings(
            endToEndReq,
            endToEndReq->globalSpecializationArgStrings,
            globalSpecializationArgs);

        // Don't proceed further if anything failed to parse.
        if(sink->GetErrorCount())
            return nullptr;

        // Now we create the initial specialized program by
        // applying the global generic arguments (if any) to the
        // unspecialized program.
        //
        auto specializedProgram = _createSpecializedProgramImpl(
            linkage,
            unspecializedProgram,
            globalSpecializationArgs,
            sink);

        // If anything went wrong with the global generic
        // arguments, then bail out now.
        //
        if(!specializedProgram)
            return nullptr;

        // Next we will deal with the entry points for the
        // new specialized program.
        //
        // If the user specified explicit entry points as part of the
        // end-to-end request, then we only want to process those (and
        // ignore any other `[shader(...)]`-attributed entry points).
        //
        // However, if the user specified *no* entry points as part
        // of the end-to-end request, then we would like to go
        // ahead and consider all the entry points that were found
        // by the front-end.
        //
        Index entryPointCount = endToEndReq->entryPoints.getCount();
        if( entryPointCount == 0 )
        {
            entryPointCount = unspecializedProgram->getEntryPointCount();
            endToEndReq->entryPoints.setCount(entryPointCount);
        }

        return specializedProgram;
    }

        /// Create a specialized program based on the given compile request.
        ///
        /// The specialized program created here includes both the global
        /// scope for all the translation units involved and all the entry
        /// points, and it also includes any specialization arguments
        /// that were supplied.
        ///
        /// It is important to note that this function specializes
        /// the global scope and the entry points in isolation and then
        /// composes them, and that this can lead to different layout
        /// from the result of `createUnspecializedGlobalAndEntryPointsComponentType`.
        ///
        /// If we have a module `M` with entry point `E`, and each has one
        /// specialization parameter, then `createUnspecialized...` will yield:
        ///
        ///     compose(M,E)
        ///
        /// That composed type will have two specialization parameters (the one
        /// from `M` plus the one from `E`) and so we might specialize it to get:
        ///
        ///     specialize(compose(M,E), X, Y)
        ///
        /// while if we use `createSpecialized...` we will get:
        ///
        ///     compose(specialize(M,X), specialize(E,Y))
        ///
        /// While these options are semantically equivalent, they would not lay
        /// out the same way in memory.
        ///
        /// There are many reasons why an application might prefer one over the
        /// other, and an application that cares should use the more explicit
        /// APIs to construct what they want. The behavior of this function
        /// is just to provide a reasonable default for use by end-to-end
        /// compilation (e.g., from the command line).
        ///
    RefPtr<ComponentType> createSpecializedGlobalAndEntryPointsComponentType(
        EndToEndCompileRequest* endToEndReq)
    {
        auto specializedGlobalComponentType = endToEndReq->getSpecializedGlobalComponentType();

        List<RefPtr<ComponentType>> allComponentTypes;
        allComponentTypes.add(specializedGlobalComponentType);

        auto unspecializedGlobalAndEntryPointsComponentType = endToEndReq->getUnspecializedGlobalAndEntryPointsComponentType();
        auto entryPointCount = unspecializedGlobalAndEntryPointsComponentType->getEntryPointCount();

        for(Index ii = 0; ii < entryPointCount; ++ii)
        {
            auto& entryPointInfo = endToEndReq->entryPoints[ii];
            auto unspecializedEntryPoint = unspecializedGlobalAndEntryPointsComponentType->getEntryPoint(ii);

            auto specializedEntryPoint = createSpecializedEntryPoint(endToEndReq, unspecializedEntryPoint, entryPointInfo);
            allComponentTypes.add(specializedEntryPoint);
        }

        RefPtr<ComponentType> composed = CompositeComponentType::create(endToEndReq->getLinkage(), allComponentTypes);
        return composed;
    }


    void checkTranslationUnit(
        TranslationUnitRequest* translationUnit)
    {
        SemanticsVisitor visitor(
            translationUnit->compileRequest->getLinkage(),
            translationUnit->compileRequest->getSink());

        // Apply the visitor to do the main semantic
        // checking that is required on all declarations
        // in the translation unit.
        visitor.checkDecl(translationUnit->getModuleDecl());

        translationUnit->getModule()->_collectShaderParams();
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
}
