// slang-check-impl.h
#pragma once

// This file provides the private interfaces used by
// the various `slang-check-*` files that provide
// the semantic checking infrastructure.

#include "slang-check.h"
#include "slang-compiler.h"
#include "slang-visitor.h"

namespace Slang
{
    
        /// Should the given `decl` be treated as a static rather than instance declaration?
    bool isEffectivelyStatic(
        Decl*           decl);

    Type* checkProperType(
        Linkage*        linkage,
        TypeExp         typeExp,
        DiagnosticSink* sink);

    // A flat representation of basic types (scalars, vectors and matrices)
    // that can be used as lookup key in caches
    enum class BasicTypeKey : uint16_t
    {
        Invalid = 0xffff,                 ///< Value that can never be a valid type
    };

    SLANG_FORCE_INLINE BasicTypeKey makeBasicTypeKey(BaseType baseType, IntegerLiteralValue dim1 = 0, IntegerLiteralValue dim2 = 0)
    {
        SLANG_ASSERT(dim1 >= 0 && dim2 >= 0);
        return BasicTypeKey((uint8_t(baseType) << 8) | (uint8_t(dim1) << 4) | uint8_t(dim2));
    }

    inline BasicTypeKey makeBasicTypeKey(Type* typeIn)
    {
        if (auto basicType = as<BasicExpressionType>(typeIn))
        {
            return makeBasicTypeKey(basicType->baseType);
        }
        else if (auto vectorType = as<VectorExpressionType>(typeIn))
        {
            if (auto elemCount = as<ConstantIntVal>(vectorType->elementCount))
            {
                if( auto elemBasicType = as<BasicExpressionType>(vectorType->elementType) )
                {
                    return makeBasicTypeKey(elemBasicType->baseType, elemCount->value);
                }
            }
        }
        else if (auto matrixType = as<MatrixExpressionType>(typeIn))
        {
            if (auto elemCount1 = as<ConstantIntVal>(matrixType->getRowCount()))
            {
                if (auto elemCount2 = as<ConstantIntVal>(matrixType->getColumnCount()))
                {
                    if( auto elemBasicType = as<BasicExpressionType>(matrixType->getElementType()) )
                    {
                        return makeBasicTypeKey(elemBasicType->baseType, elemCount1->value, elemCount2->value);
                    }
                }
            }
        }
        return BasicTypeKey::Invalid;
    }

    struct BasicTypeKeyPair
    {
        BasicTypeKey type1, type2;
        bool operator==(const BasicTypeKeyPair& rhs) const { return type1 == rhs.type1 && type2 == rhs.type2; }
        bool operator!=(const BasicTypeKeyPair& rhs) const { return !(*this == rhs); }

        bool isValid() const { return type1 != BasicTypeKey::Invalid && type2 != BasicTypeKey::Invalid; }

        HashCode getHashCode()
        {
            return HashCode(int(type1) << 16 | int(type2));
        }
    };

    struct OperatorOverloadCacheKey
    {
        intptr_t operatorName;
        BasicTypeKey args[2];
        bool operator == (OperatorOverloadCacheKey key)
        {
            return operatorName == key.operatorName && args[0] == key.args[0] && args[1] == key.args[1];
        }
        HashCode getHashCode()
        {
            return HashCode(((int)(UInt64)(void*)(operatorName) << 16) ^ (int(args[0]) << 8) ^ (int(args[1])));
        }
        bool fromOperatorExpr(OperatorExpr* opExpr)
        {
            // First, lets see if the argument types are ones
            // that we can encode in our space of keys.
            args[0] = BasicTypeKey::Invalid;
            args[1] = BasicTypeKey::Invalid;
            if (opExpr->arguments.getCount() > 2)
                return false;

            for (Index i = 0; i < opExpr->arguments.getCount(); i++)
            {
                auto key = makeBasicTypeKey(opExpr->arguments[i]->type.Ptr());
                if (key == BasicTypeKey::Invalid)
                {
                    return false;
                }
                args[i]=  key;
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

            if (auto overloadedBase = as<OverloadedExpr>(opExpr->functionExpr))
            {
                for(auto item : overloadedBase->lookupResult2 )
                {
                    // Look at a candidate definition to be called and
                    // see if it gives us a key to work with.
                    //
                    Decl* funcDecl = overloadedBase->lookupResult2.item.declRef.decl;
                    if (auto genDecl = as<GenericDecl>(funcDecl))
                        funcDecl = genDecl->inner;

                    // Reject definitions that have the wrong fixity.
                    //
                    if(prefixExpr && !funcDecl->findModifier<PrefixModifier>())
                        continue;
                    if(postfixExpr && !funcDecl->findModifier<PostfixModifier>())
                        continue;

                    if (auto intrinsicOp = funcDecl->findModifier<IntrinsicOpModifier>())
                    {
                        operatorName = intrinsicOp->op;
                        return true;
                    }
                }
            }
            return false;
        }
    };

    struct OverloadCandidate
    {
        enum class Flavor
        {
            Func,
            Generic,
            UnspecializedGeneric,
            Expr,
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

        // Type of function being applied (for cases where `item` is not used)
        FuncType* funcType = nullptr;

        // The type of the result expression if this candidate is selected
        Type*	resultType = nullptr;

        // A system for tracking constraints introduced on generic parameters
        //            ConstraintSystem constraintSystem;

        // How much conversion cost should be considered for this overload,
        // when ranking candidates.
        ConversionCost conversionCostSum = kConversionCost_None;

        // When required, a candidate can store a pre-checked list of
        // arguments so that we don't have to repeat work across checking
        // phases. Currently this is only needed for generics.
        Substitutions*   subst = nullptr;
    };

    struct TypeCheckingCache
    {
        Dictionary<OperatorOverloadCacheKey, OverloadCandidate> resolvedOperatorOverloadCache;
        Dictionary<BasicTypeKeyPair, ConversionCost> conversionCostCache;
    };

        /// Shared state for a semantics-checking session.
    struct SharedSemanticsContext
    {
        Linkage*        m_linkage   = nullptr;

            /// The (optional) "primary" module that is the parent to everything that will be checked.
        Module*         m_module    = nullptr;

        DiagnosticSink* m_sink      = nullptr;

            /// (optional) modules that comes from previously processed translation units in the
            /// front-end request that are made visible to the module being checked. This allows
            /// `import` to use them instead of trying to find the files in file system.
        LoadedModuleDictionary* m_environmentModules = nullptr;

        DiagnosticSink* getSink()
        {
            return m_sink;
        }

        // We need to track what has been `import`ed into
        // the scope of this semantic checking session,
        // and also to avoid importing the same thing more
        // than once.
        //
        List<ModuleDecl*> importedModulesList;
        HashSet<ModuleDecl*> importedModulesSet;

    public:
        SharedSemanticsContext(
            Linkage*        linkage,
            Module*         module,
            DiagnosticSink* sink,
            LoadedModuleDictionary* environmentModules = nullptr)
            : m_linkage(linkage)
            , m_module(module)
            , m_sink(sink)
            , m_environmentModules(environmentModules)
        {}

        Session* getSession()
        {
            return m_linkage->getSessionImpl();
        }

        Linkage* getLinkage()
        {
            return m_linkage;
        }

        Module* getModule()
        {
            return m_module;
        }

        bool isInLanguageServer()
        {
            if (m_linkage)
                return m_linkage->isInLanguageServer();
            return false;
        }
            /// Get the list of extension declarations that appear to apply to `decl` in this context
        List<ExtensionDecl*> const& getCandidateExtensionsForTypeDecl(AggTypeDecl* decl);

            /// Register a candidate extension `extDecl` for `typeDecl` encountered during checking.
        void registerCandidateExtension(AggTypeDecl* typeDecl, ExtensionDecl* extDecl);

    private:
            /// Mapping from type declarations to the known extensiosn that apply to them
        Dictionary<AggTypeDecl*, RefPtr<CandidateExtensionList>> m_mapTypeDeclToCandidateExtensions;

            /// Is the `m_mapTypeDeclToCandidateExtensions` dictionary valid and up to date?
        bool m_candidateExtensionListsBuilt = false;

            /// Add candidate extensions declared in `moduleDecl` to `m_mapTypeDeclToCandidateExtensions`
        void _addCandidateExtensionsFromModule(ModuleDecl* moduleDecl);
    };

        /// Local/scoped state of the semantic-checking system
        ///
        /// This type is kept distinct from `SharedSemanticsContext` so that we
        /// can avoid unncessary mutable state being propagated through the
        /// checking process.
        ///
        /// Semantic-checking code should make a new local `SemanticsContext`
        /// in cases where it want to check a sub-entity (expression, statement,
        /// declaration, etc.) in a modified or extended context.
        ///
    struct SemanticsContext
    {
    public:
        explicit SemanticsContext(
            SharedSemanticsContext* shared)
            : m_shared(shared)
            , m_sink(shared->getSink())
            , m_astBuilder(shared->getLinkage()->getASTBuilder())
        {}

        SharedSemanticsContext* getShared() { return m_shared; }
        ASTBuilder* getASTBuilder() { return m_astBuilder; }

        DiagnosticSink* getSink() { return m_sink; }

        Session* getSession() { return m_shared->getSession(); }

        Linkage* getLinkage() { return m_shared->m_linkage; }
        NamePool* getNamePool() { return getLinkage()->getNamePool(); }
        SourceManager* getSourceManager() { return getLinkage()->getSourceManager(); }

        SemanticsContext withSink(DiagnosticSink* sink)
        {
            SemanticsContext result(*this);
            result.m_sink = sink;
            return result;
        }

        SemanticsContext withParentFunc(FunctionDeclBase* parentFunc)
        {
            SemanticsContext result(*this);
            result.m_parentFunc = parentFunc;
            result.m_outerStmts = nullptr;
            return result;
        }

            /// Information for tracking one or more outer statements.
            ///
            /// During checking of statements, we need to track what
            /// outer statements are in scope, so that we can resolve
            /// the target for a `break` or `continue` statement (and
            /// validate that such statements are only used in contexts
            /// where such a target exists).
            ///
            /// We use a linked list of `OuterStmtInfo` threaded up
            /// through the recursive call stack to track the statements
            /// that are lexically surrounding the one we are checking.
            ///
        struct OuterStmtInfo
        {
            Stmt* stmt = nullptr;
            OuterStmtInfo* next;
        };

        OuterStmtInfo* getOuterStmts() { return m_outerStmts; }

        SemanticsContext withOuterStmts(OuterStmtInfo* outerStmts)
        {
            SemanticsContext result(*this);
            result.m_outerStmts = outerStmts;
            return result;
        }

        TryClauseType getEnclosingTryClauseType() { return m_enclosingTryClauseType; }

        SemanticsContext withEnclosingTryClauseType(TryClauseType tryClauseType)
        {
            SemanticsContext result(*this);
            result.m_enclosingTryClauseType = tryClauseType;
            return result;
        }

            /// A scope that is local to a particular expression, and
            /// that can be used to allocate temporary bindings that
            /// might be needed by that expression or its sub-expressions.
            ///
            /// The scope is represented as a sequence of nested `LetExpr`s
            /// that introduce the bindings needed in the scope.
            ///
        struct ExprLocalScope
        {
        public:
            void addBinding(LetExpr* binding);

            LetExpr* getOuterMostBinding() const { return m_outerMostBinding; }

        private:
            LetExpr* m_outerMostBinding = nullptr;
            LetExpr* m_innerMostBinding = nullptr;
        };

        ExprLocalScope* getExprLocalScope() { return m_exprLocalScope; }

        SemanticsContext withExprLocalScope(ExprLocalScope* exprLocalScope)
        {
            SemanticsContext result(*this);
            result.m_exprLocalScope = exprLocalScope;
            return result;
        }

    private:
        SharedSemanticsContext* m_shared = nullptr;

        DiagnosticSink* m_sink = nullptr;

        ExprLocalScope* m_exprLocalScope = nullptr;


    protected:
        // TODO: consider making more of this state `private`...

            /// The parent function (if any) that surrounds the statement being checked.
        FunctionDeclBase* m_parentFunc = nullptr;

            /// The linked list of lexically surrounding statements.
        OuterStmtInfo* m_outerStmts = nullptr;

            /// The type of a try clause (if any) enclosing current expr.
        TryClauseType m_enclosingTryClauseType = TryClauseType::None;

        ASTBuilder* m_astBuilder = nullptr;
    };

    struct SemanticsVisitor : public SemanticsContext
    {
        typedef SemanticsContext Super;

        explicit SemanticsVisitor(
            SharedSemanticsContext* shared)
            : Super(shared)
        {}

        SemanticsVisitor(
            SemanticsContext const& context)
            : Super(context)
        {}


    public:
        // Translate Types


        Expr* TranslateTypeNodeImpl(Expr* node);
        Type* ExtractTypeFromTypeRepr(Expr* typeRepr);
        Type* TranslateTypeNode(Expr* node);
        TypeExp TranslateTypeNodeForced(TypeExp const& typeExp);
        TypeExp TranslateTypeNode(TypeExp const& typeExp);

        DeclRefType* getExprDeclRefType(Expr * expr);

            /// Is `decl` usable as a static member?
        bool isDeclUsableAsStaticMember(
            Decl*   decl);

            /// Is `item` usable as a static member?
        bool isUsableAsStaticMember(
            LookupResultItem const& item);

            /// Move `expr` into a temporary variable and execute `func` on that variable.
            ///
            /// Returns an expression that wraps both the creation and initialization of
            /// the temporary, and the computation created by `func`.
            ///
        template<typename F>
        Expr* moveTemp(Expr* const& expr, F const& func);

            /// Execute `func` on a variable with the value of `expr`.
            ///
            /// If `expr` is just a reference to an immutable (e.g., `let`) variable
            /// then this might use the existing variable. Otherwise it will create
            /// a new variable to hold `expr`, using `moveTemp()`.
            ///
        template<typename F>
        Expr* maybeMoveTemp(Expr* const& expr, F const& func);

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
        Expr* openExistential(
            Expr*            expr,
            DeclRef<InterfaceDecl>  interfaceDeclRef);

            /// If `expr` has existential type, then open it.
            ///
            /// Returns an expression that opens `expr` if it had existential type.
            /// Otherwise returns `expr` itself.
            ///
            /// See `openExistential` for a discussion of what "opening" an
            /// existential-type value means.
            ///
        Expr* maybeOpenExistential(Expr* expr);

        Expr* ConstructDeclRefExpr(
            DeclRef<Decl>   declRef,
            Expr*    baseExpr,
            SourceLoc loc,
            Expr*    originalExpr);

        Expr* ConstructDerefExpr(
            Expr*    base,
            SourceLoc       loc);

        Expr* ConstructLookupResultExpr(
            LookupResultItem const& item,
            Expr*            baseExpr,
            SourceLoc loc,
            Expr* originalExpr);

        Expr* createLookupResultExpr(
            Name*                   name,
            LookupResult const&     lookupResult,
            Expr*            baseExpr,
            SourceLoc loc,
            Expr*    originalExpr);

            /// Attempt to "resolve" an overloaded `LookupResult` to only include the "best" results
        LookupResult resolveOverloadedLookup(LookupResult const& lookupResult);

            /// Attempt to resolve `expr` into an expression that refers to a single declaration/value.
            /// If `expr` isn't overloaded, then it will be returned as-is.
            ///
            /// The provided `mask` is used to filter items down to those that are applicable in a given context (e.g., just types).
            ///
            /// If the expression cannot be resolved to a single value then *if* `diagSink` is non-null an
            /// appropriate "ambiguous reference" error will be reported, and an error expression will be returned.
            /// Otherwise, the original expression is returned if resolution fails.
            ///
        Expr* maybeResolveOverloadedExpr(Expr* expr, LookupMask mask, DiagnosticSink* diagSink);

            /// Attempt to resolve `overloadedExpr` into an expression that refers to a single declaration/value.
            ///
            /// Equivalent to `maybeResolveOverloadedExpr` with `diagSink` bound to the sink for the `SemanticsVisitor`.
        Expr* resolveOverloadedExpr(OverloadedExpr* overloadedExpr, LookupMask mask);

            /// Worker reoutine for `maybeResolveOverloadedExpr` and `resolveOverloadedExpr`.
        Expr* _resolveOverloadedExprImpl(OverloadedExpr* overloadedExpr, LookupMask mask, DiagnosticSink* diagSink);

        void diagnoseAmbiguousReference(OverloadedExpr* overloadedExpr, LookupResult const& lookupResult);
        void diagnoseAmbiguousReference(Expr* overloadedExpr);


        Expr* ExpectATypeRepr(Expr* expr);

        Type* ExpectAType(Expr* expr);

        Type* ExtractGenericArgType(Expr* exp);

        IntVal* ExtractGenericArgInteger(Expr* exp, DiagnosticSink* sink);
        IntVal* ExtractGenericArgInteger(Expr* exp);

        Val* ExtractGenericArgVal(Expr* exp);

        // Construct a type representing the instantiation of
        // the given generic declaration for the given arguments.
        // The arguments should already be checked against
        // the declaration.
        Type* InstantiateGenericType(
            DeclRef<GenericDecl>        genericDeclRef,
            List<Expr*> const&   args);

        // These routines are bottlenecks for semantic checking,
        // so that we can add some quality-of-life features for users
        // in cases where the compiler crashes
        //
        void dispatchStmt(Stmt* stmt, SemanticsContext const& context);
        Expr* dispatchExpr(Expr* expr, SemanticsContext const& context);

            /// Ensure that a declaration has been checked up to some state
            /// (aka, a phase of semantic checking) so that we can safely
            /// perform certain operations on it.
            ///
            /// Calling `ensureDecl` may cause the type-checker to recursively
            /// start checking `decl` on top of the stack that is already
            /// doing other semantic checking. Care should be taken when relying
            /// on this function to avoid blowing out the stack or (even worse
            /// creating a circular dependency).
            ///
        void ensureDecl(Decl* decl, DeclCheckState state, SemanticsContext* baseContext = nullptr);

            /// Helper routine allowing `ensureDecl` to be called on a `DeclRef`
        void ensureDecl(DeclRefBase const& declRef, DeclCheckState state)
        {
            ensureDecl(declRef.getDecl(), state);
        }

            /// Helper routine allowing `ensureDecl` to be used on a `DeclBase`
            ///
            /// `DeclBase` is the base clas of `Decl` and `DeclGroup`. When
            /// called on a `DeclGroup` this function just calls `ensureDecl()`
            /// on each declaration in the group.
            ///
        void ensureDeclBase(DeclBase* decl, DeclCheckState state, SemanticsContext* baseContext);

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
            Type**   outProperType,
            DiagnosticSink* diagSink);

        TypeExp CoerceToProperType(TypeExp const& typeExp);

        TypeExp tryCoerceToProperType(TypeExp const& typeExp);

        // Check a type, and coerce it to be proper
        TypeExp CheckProperType(TypeExp typeExp);

        // For our purposes, a "usable" type is one that can be
        // used to declare a function parameter, variable, etc.
        // These turn out to be all the proper types except
        // `void`.
        //
        // TODO(tfoley): consider just allowing `void` as a
        // simple example of a "unit" type, and get rid of
        // this check.
        TypeExp CoerceToUsableType(TypeExp const& typeExp);

        // Check a type, and coerce it to be usable
        TypeExp CheckUsableType(TypeExp typeExp);

        Expr* CheckTerm(Expr* term);

        Expr* CreateErrorExpr(Expr* expr);

        bool IsErrorExpr(Expr* expr);

        // Capture the "base" expression in case this is a member reference
        Expr* GetBaseExpr(Expr* expr);

            /// Validate a declaration to ensure that it doesn't introduce a circularly-defined constant
            ///
            /// Circular definition in a constant may lead to infinite looping or stack overflow in
            /// the compiler, so it needs to be protected against.
            ///
            /// Note that this function does *not* protect against circular definitions in general,
            /// and a program that indirectly initializes a global variable using its own value (e.g.,
            /// by calling a function that indirectly reads the variable) will be allowed and then
            /// exhibit undefined behavior at runtime.
            ///
        void _validateCircularVarDefinition(VarDeclBase* varDecl);

        bool shouldSkipChecking(Decl* decl, DeclCheckState state);
    public:

        bool ValuesAreEqual(
            IntVal* left,
            IntVal* right);

        // Compute the cost of using a particular declaration to
        // perform implicit type conversion.
        ConversionCost getImplicitConversionCost(
            Decl* decl);

        bool isEffectivelyScalarForInitializerLists(
            Type*    type);

            /// Should the provided expression (from an initializer list) be used directly to initialize `toType`?
        bool shouldUseInitializerDirectly(
            Type*    toType,
            Expr*    fromExpr);

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
            Type*                toType,
            Expr**               outToExpr,
            InitializerListExpr* fromInitializerListExpr,
            UInt                       &ioInitArgIndex);

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
            Type*                inToType,
            Expr**               outToExpr,
            InitializerListExpr* fromInitializerListExpr,
            UInt                       &ioArgIndex);

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
            Type*                toType,
            Expr**               outToExpr,
            InitializerListExpr* fromInitializerListExpr);

            /// Report that implicit type coercion is not possible.
        bool _failedCoercion(
            Type*    toType,
            Expr**   outToExpr,
            Expr*    fromExpr);

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
            Type*    toType,
            Expr**   outToExpr,
            Type*    fromType,
            Expr*    fromExpr,
            ConversionCost* outCost);

            /// Check whether implicit type coercion from `fromType` to `toType` is possible.
            ///
            /// If conversion is possible, returns `true` and sets `outCost` to the cost
            /// of the conversion found (if `outCost` is non-null).
            ///
            /// If conversion is not possible, returns `false`.
            ///
        bool canCoerce(
            Type*    toType,
            Type*    fromType,
            Expr*    fromExpr,
            ConversionCost* outCost = 0);

        TypeCastExpr* createImplicitCastExpr();

        Expr* CreateImplicitCastExpr(
            Type*	toType,
            Expr*	fromExpr);

            /// Create an "up-cast" from a value to an interface type
            ///
            /// This operation logically constructs an "existential" value,
            /// which packages up the value, its type, and the witness
            /// of its conformance to the interface.
            ///
        Expr* createCastToInterfaceExpr(
            Type*    toType,
            Expr*    fromExpr,
            Val*     witness);

            /// Implicitly coerce `fromExpr` to `toType` and diagnose errors if it isn't possible
        Expr* coerce(
            Type*    toType,
            Expr*    fromExpr);

        // Fill in default substitutions for the 'subtype' part of a type constraint decl
        void CheckConstraintSubType(TypeExp& typeExp);

        void checkGenericDeclHeader(GenericDecl* genericDecl);

        ConstantIntVal* checkConstantIntVal(
            Expr*    expr);

        ConstantIntVal* checkConstantEnumVal(
            Expr*    expr);

        // Check an expression, coerce it to the `String` type, and then
        // ensure that it has a literal (not just compile-time constant) value.
        bool checkLiteralStringVal(
            Expr*    expr,
            String*         outVal);

        void visitModifier(Modifier*);

        AttributeDecl* lookUpAttributeDecl(Name* attributeName, Scope* scope);

        bool hasIntArgs(Attribute* attr, int numArgs);
        bool hasStringArgs(Attribute* attr, int numArgs);

        bool getAttributeTargetSyntaxClasses(SyntaxClass<NodeBase> & cls, uint32_t typeFlags);

        bool validateAttribute(Attribute* attr, AttributeDecl* attribClassDecl);

        AttributeBase* checkAttribute(
            UncheckedAttribute*     uncheckedAttr,
            ModifiableSyntaxNode*   attrTarget);

        Modifier* checkModifier(
            Modifier*        m,
            ModifiableSyntaxNode*   syntaxNode);

        void checkModifiers(ModifiableSyntaxNode* syntaxNode);

        bool doesSignatureMatchRequirement(
            DeclRef<CallableDecl>   satisfyingMemberDeclRef,
            DeclRef<CallableDecl>   requiredMemberDeclRef,
            RefPtr<WitnessTable>    witnessTable);

        bool doesAccessorMatchRequirement(
            DeclRef<AccessorDecl>   satisfyingMemberDeclRef,
            DeclRef<AccessorDecl>   requiredMemberDeclRef);

        bool doesPropertyMatchRequirement(
            DeclRef<PropertyDecl>   satisfyingMemberDeclRef,
            DeclRef<PropertyDecl>   requiredMemberDeclRef,
            RefPtr<WitnessTable>    witnessTable);

        bool doesGenericSignatureMatchRequirement(
            DeclRef<GenericDecl>        genDecl,
            DeclRef<GenericDecl>        requirementGenDecl,
            RefPtr<WitnessTable>        witnessTable);

        bool doesTypeSatisfyAssociatedTypeRequirement(
            Type*            satisfyingType,
            DeclRef<AssocTypeDecl>  requiredAssociatedTypeDeclRef,
            RefPtr<WitnessTable>    witnessTable);

        // Does the given `memberDecl` work as an implementation
        // to satisfy the requirement `requiredMemberDeclRef`
        // from an interface?
        //
        // If it does, then inserts a witness into `witnessTable`
        // and returns `true`, otherwise returns `false`
        bool doesMemberSatisfyRequirement(
            DeclRef<Decl>               memberDeclRef,
            DeclRef<Decl>               requiredMemberDeclRef,
            RefPtr<WitnessTable>        witnessTable);

        // State used while checking if a declaration (either a type declaration
        // or an extension of that type) conforms to the interfaces it claims
        // via its inheritance clauses.
        //
        struct ConformanceCheckingContext
        {
                /// The type for which conformances are being checked
            Type*           conformingType;

                /// The outer declaration for the conformances being checked (either a type or `extension` declaration)
            ContainerDecl*  parentDecl;

            Dictionary<DeclRef<InterfaceDecl>, RefPtr<WitnessTable>>    mapInterfaceToWitnessTable;
        };

            /// Attempt to synthesize a method that can satisfy `requiredMemberDeclRef` using `lookupResult`.
            ///
            /// On success, installs the syntethesized method in `witnessTable` and returns `true`.
            /// Otherwise, returns `false`.
        bool trySynthesizeMethodRequirementWitness(
            ConformanceCheckingContext* context,
            LookupResult const&         lookupResult,
            DeclRef<FuncDecl>           requiredMemberDeclRef,
            RefPtr<WitnessTable>        witnessTable);

            /// Attempt to synthesize a property that can satisfy `requiredMemberDeclRef` using `lookupResult`.
            ///
            /// On success, installs the syntethesized method in `witnessTable` and returns `true`.
            /// Otherwise, returns `false`.
            ///
        bool trySynthesizePropertyRequirementWitness(
            ConformanceCheckingContext* context,
            LookupResult const&         lookupResult,
            DeclRef<PropertyDecl>       requiredMemberDeclRef,
            RefPtr<WitnessTable>        witnessTable);

            /// Attempt to synthesize a declartion that can satisfy `requiredMemberDeclRef` using `lookupResult`.
            ///
            /// On success, installs the syntethesized declaration in `witnessTable` and returns `true`.
            /// Otherwise, returns `false`.
        bool trySynthesizeRequirementWitness(
            ConformanceCheckingContext* context,
            LookupResult const&         lookupResult,
            DeclRef<Decl>               requiredMemberDeclRef,
            RefPtr<WitnessTable>        witnessTable);

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
            Type*                       subType,
            Type*                       superInterfaceType,
            InheritanceDecl*            inheritanceDecl,
            DeclRef<InterfaceDecl>      superInterfaceDeclRef,
            DeclRef<Decl>               requiredMemberDeclRef,
            RefPtr<WitnessTable>        witnessTable,
            SubtypeWitness*             subTypeConformsToSuperInterfaceWitness);

        // Check that the type declaration `typeDecl`, which
        // declares conformance to the interface `interfaceDeclRef`,
        // (via the given `inheritanceDecl`) actually provides
        // members to satisfy all the requirements in the interface.
        bool checkInterfaceConformance(
            ConformanceCheckingContext* context,
            Type*                       subType,
            Type*                       superInterfaceType,
            InheritanceDecl*            inheritanceDecl,
            DeclRef<InterfaceDecl>      superInterfaceDeclRef,
            SubtypeWitness*             subTypeConformsToSuperInterfaceWitness,
            WitnessTable*               witnessTable);

        RefPtr<WitnessTable> checkInterfaceConformance(
            ConformanceCheckingContext* context,
            Type*                       subType,
            Type*                       superInterfaceType,
            InheritanceDecl*            inheritanceDecl,
            DeclRef<InterfaceDecl>      superInterfaceDeclRef,
            SubtypeWitness*             subTypeConformsToSuperInterfaceWitness);

        bool checkConformanceToType(
            ConformanceCheckingContext* context,
            Type*                       subType,
            InheritanceDecl*            inheritanceDecl,
            Type*                       superType,
            SubtypeWitness*             subIsSuperWitness,
            WitnessTable*               witnessTable);

            /// Check that `type` which has declared that it inherits from (and/or implements)
            /// another type via `inheritanceDecl` actually does what it needs to for that
            /// inheritance to be valid.
        bool checkConformance(
            Type*                       type,
            InheritanceDecl*            inheritanceDecl,
            ContainerDecl*              parentDecl);

        void checkExtensionConformance(ExtensionDecl* decl);

        void checkAggTypeConformance(AggTypeDecl* decl);

        bool isIntegerBaseType(BaseType baseType);

            /// Is `type` a scalar integer type.
        bool isScalarIntegerType(Type* type);

        // Validate that `type` is a suitable type to use
        // as the tag type for an `enum`
        void validateEnumTagType(Type* type, SourceLoc const& loc);

        void checkStmt(Stmt* stmt, SemanticsContext const& context);

        void getGenericParams(
            GenericDecl*                        decl,
            List<Decl*>&                        outParams,
            List<GenericTypeConstraintDecl*>&   outConstraints);

            /// Determine if `left` and `right` have matching generic signatures.
            /// If they do, then outputs a substitution to `ioSubstRightToLeft` that
            /// can be used to specialize `right` to the parameters of `left`.
        bool doGenericSignaturesMatch(
            GenericDecl*                    left,
            GenericDecl*                    right,
            GenericSubstitution**    outSubstRightToLeft);

        // Check if two functions have the same signature for the purposes
        // of overload resolution.
        bool doFunctionSignaturesMatch(
            DeclRef<FuncDecl> fst,
            DeclRef<FuncDecl> snd);

        GenericSubstitution* createDummySubstitutions(
            GenericDecl* genericDecl);

        Result checkRedeclaration(Decl* newDecl, Decl* oldDecl);
        Result checkFuncRedeclaration(FuncDecl* newDecl, FuncDecl* oldDecl);
        void checkForRedeclaration(Decl* decl);

        Expr* checkPredicateExpr(Expr* expr);

        Expr* checkExpressionAndExpectIntegerConstant(Expr* expr, IntVal** outIntVal);

        IntegerLiteralValue GetMinBound(IntVal* val);

        void maybeInferArraySizeForVariable(VarDeclBase* varDecl);

        void validateArraySizeForVariable(VarDeclBase* varDecl);

        IntVal* getIntVal(IntegerLiteralExpr* expr);

        inline IntVal* getIntVal(SubstExpr<IntegerLiteralExpr> expr)
        {
            return getIntVal(expr.getExpr());
        }

        Name* getName(String const& text)
        {
            return getNamePool()->getName(text);
        }

            /// Helper type to detect and catch circular definitions when folding constants,
            /// to prevent the compiler from going into infinite loops or overflowing the stack.
        struct ConstantFoldingCircularityInfo
        {
            ConstantFoldingCircularityInfo(
                Decl*                           decl,
                ConstantFoldingCircularityInfo* next)
                : decl(decl)
                , next(next)
            {}

                /// A declaration whose value is contributing to the constant being folded
            Decl*                           decl = nullptr;

                /// The rest of the links in the chain of declarations being folded
            ConstantFoldingCircularityInfo* next = nullptr;
        };

            /// Try to apply front-end constant folding to determine the value of `invokeExpr`.
        IntVal* tryConstantFoldExpr(
            SubstExpr<InvokeExpr>           invokeExpr,
            ConstantFoldingCircularityInfo* circularityInfo);

            /// Try to apply front-end constant folding to determine the value of `expr`.
        IntVal* tryConstantFoldExpr(
            SubstExpr<Expr>                 expr,
            ConstantFoldingCircularityInfo* circularityInfo);

        bool _checkForCircularityInConstantFolding(
            Decl*                           decl,
            ConstantFoldingCircularityInfo* circularityInfo);

            /// Try to resolve a compile-time constant `IntVal` from the given `declRef`.
        IntVal* tryConstantFoldDeclRef(
            DeclRef<VarDeclBase> const&     declRef,
            ConstantFoldingCircularityInfo* circularityInfo);

            /// Try to extract the value of an integer constant expression, either
            /// returning the `IntVal` value, or null if the expression isn't recognized
            /// as an integer constant.
            ///
        IntVal* tryFoldIntegerConstantExpression(
            SubstExpr<Expr>                 expr,
            ConstantFoldingCircularityInfo* circularityInfo);

        // Enforce that an expression resolves to an integer constant, and get its value
        IntVal* CheckIntegerConstantExpression(Expr* inExpr);
        IntVal* CheckIntegerConstantExpression(Expr* inExpr, DiagnosticSink* sink);

        IntVal* CheckEnumConstantExpression(Expr* expr);


        Expr* CheckSimpleSubscriptExpr(
            IndexExpr*   subscriptExpr,
            Type*              elementType);

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
        VectorExpressionType* createVectorType(
            Type*  elementType,
            IntVal*          elementCount);

        //

            /// Given an immutable `expr` used as an l-value emit a special diagnostic if it was derived from `this`.
        void maybeDiagnoseThisNotLValue(Expr* expr);

        void registerExtension(ExtensionDecl* decl);

        // Figure out what type an initializer/constructor declaration
        // is supposed to return. In most cases this is just the type
        // declaration that its declaration is nested inside.
        Type* findResultTypeForConstructorDecl(ConstructorDecl* decl);

            /// Determine what type `This` should refer to in the context of the given parent `decl`.
        Type* calcThisType(DeclRef<Decl> decl);

            /// Determine what type `This` should refer to in an extension of `type`.
        Type* calcThisType(Type* type);


        //

        struct Constraint
        {
            Decl*		decl = nullptr; // the declaration of the thing being constraints
            Val*	val = nullptr; // the value to which we are constraining it
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
            GenericDecl* genericDecl = nullptr;

            // Constraints we have accumulated, which constrain
            // the possible arguments for those parameters.
            List<Constraint> constraints;
        };

        Type* TryJoinVectorAndScalarType(
            VectorExpressionType* vectorType,
            BasicExpressionType*  scalarType);

        struct TypeWitnessBreadcrumb
        {
            TypeWitnessBreadcrumb*  prev;

            Type*            sub = nullptr;
            Type*            sup = nullptr;
            DeclRef<Decl>           declRef;
        };

            // Create a subtype witness based on the declared relationship
            // found in a single breadcrumb
        DeclaredSubtypeWitness* createSimpleSubtypeWitness(
            TypeWitnessBreadcrumb*  breadcrumb);

            /// Create a witness that `subType` is a sub-type of `superTypeDeclRef`.
            ///
            /// The `inBreadcrumbs` parameter represents a linked list of steps
            /// in the process that validated the sub-type relationship, which
            /// will be used to inform the construction of the witness.
            ///
        Val* createTypeWitness(
            Type*            subType,
            DeclRef<AggTypeDecl>  superTypeDeclRef,
            TypeWitnessBreadcrumb*  inBreadcrumbs);
    
            /// Is the given interface one that a tagged-union type can conform to?
            ///
            /// If a tagged union type `__TaggedUnion(A,B)` is going to be
            /// plugged in for a type parameter `T : IFoo` then we need to
            /// be sure that the interface `IFoo` doesn't have anything
            /// that could lead to unsafe/unsound behavior. This function
            /// checks that all the requirements on the interfaceare safe ones.
            ///
        bool isInterfaceSafeForTaggedUnion(
            DeclRef<InterfaceDecl> interfaceDeclRef);

            /// Is the given interface requirement one that a tagged-union type can satisfy?
            ///
            /// Unsafe requirements include any `static` requirements,
            /// any associated types, and also any requirements that make
            /// use of the `This` type (once we support it).
            ///
        bool isInterfaceRequirementSafeForTaggedUnion(
            DeclRef<InterfaceDecl>  interfaceDeclRef,
            DeclRef<Decl>           requirementDeclRef);

            /// Check whether `subType` is declared a sub-type of `superTypeDeclRef`
            ///
            /// If this function returns `true` (because the subtype relationship holds),
            /// then `outWitness` will be set to a value that serves as a witness
            /// to the subtype relationship.
            ///
            /// This function may be used to validate a transitive subtype relationship
            /// where, e.g., `A : C` becase `A : B` and `B : C`. In such a case, a recursive
            /// call to `_isDeclaredSubtype` may occur where `originalSubType` is `A`,
            /// `subType` is `C`, and `superTypeDeclRef` is `C`. The `inBreadcrumbs` in that
            /// case would include information for the `A : B` relationship, which can be
            /// used to construct a witness for `A : C` from the `A : B` and `B : C` witnesses.
            ///
        bool _isDeclaredSubtype(
            Type*            originalSubType,
            Type*            subType,
            DeclRef<AggTypeDecl>    superTypeDeclRef,
            Val**            outWitness,
            TypeWitnessBreadcrumb*  inBreadcrumbs);

            /// Check whether `subType` is a sub-type of `superTypeDeclRef`.
        bool isDeclaredSubtype(
            Type*            subType,
            DeclRef<AggTypeDecl>    superTypeDeclRef);

            /// Check whether `subType` is a sub-type of `superTypeDeclRef`,
            /// and return a witness to the sub-type relationship if it holds
            /// (return null otherwise).
            ///
        Val* tryGetSubtypeWitness(
            Type*            subType,
            DeclRef<AggTypeDecl>    superTypeDeclRef);

            /// Check whether `type` conforms to `interfaceDeclRef`,
            /// and return a witness to the conformance if it holds
            /// (return null otherwise).
            ///
            /// This function is equivalent to `tryGetSubtypeWitness()`.
            ///
        Val* tryGetInterfaceConformanceWitness(
            Type*            type,
            DeclRef<InterfaceDecl>  interfaceDeclRef);

        Expr* createCastToSuperTypeExpr(
            Type*    toType,
            Expr*    fromExpr,
            Val*     witness);

        Expr* createModifierCastExpr(
            Type*    toType,
            Expr*    fromExpr);

        /// Does there exist an implicit conversion from `fromType` to `toType`?
        bool canConvertImplicitly(
            Type* toType,
            Type* fromType);

        Type* TryJoinTypeWithInterface(
            Type*            type,
            DeclRef<InterfaceDecl>      interfaceDeclRef);

        // Try to compute the "join" between two types
        Type* TryJoinTypes(
            Type*  left,
            Type*  right);

        // Try to solve a system of generic constraints.
        // The `system` argument provides the constraints.
        // The `varSubst` argument provides the list of constraint
        // variables that were created for the system.
        //
        // Returns a new substitution representing the values that
        // we solved for along the way.
        SubstitutionSet TrySolveConstraintSystem(
            ConstraintSystem*		system,
            DeclRef<GenericDecl>          genericDeclRef);


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
            Expr* originalExpr = nullptr;

            // Source location of the "function" part of the expression, if any
            SourceLoc       funcLoc;

            // The original arguments to the call
            Index argCount = 0;
            Expr** args = nullptr;
            Type** argTypes = nullptr;

            Index getArgCount() { return argCount; }
            Expr*& getArg(Index index) { return args[index]; }
            Type*& getArgType(Index index)
            {
                if(argTypes)
                    return argTypes[index];
                else
                    return getArg(index)->type.type;
            }
            Type* getArgTypeForInference(Index index, SemanticsVisitor* semantics)
            {
                if(argTypes)
                    return argTypes[index];
                else
                    return semantics->maybeResolveOverloadedExpr(getArg(index), LookupMask::Default, nullptr)->type;
            }

            bool disallowNestedConversions = false;

            Expr* baseExpr = nullptr;

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
        ParamCounts CountParameters(FilteredMemberRefList<ParamDecl> params);

        // count the number of parameters required/allowed for a generic
        ParamCounts CountParameters(DeclRef<GenericDecl> genericRef);

        bool TryCheckOverloadCandidateClassNewMatchUp(
            OverloadResolveContext& context,
            OverloadCandidate const& candidate);

        bool TryCheckOverloadCandidateArity(
            OverloadResolveContext&		context,
            OverloadCandidate const&	candidate);

        bool TryCheckOverloadCandidateFixity(
            OverloadResolveContext&		context,
            OverloadCandidate const&	candidate);

        bool TryCheckGenericOverloadCandidateTypes(
            OverloadResolveContext&	context,
            OverloadCandidate&		candidate);

        bool TryCheckOverloadCandidateTypes(
            OverloadResolveContext&	context,
            OverloadCandidate&		candidate);

        bool TryCheckOverloadCandidateDirections(
            OverloadResolveContext&		/*context*/,
            OverloadCandidate const&	/*candidate*/);

        // Create a witness that attests to the fact that `type`
        // is equal to itself.
        Val* createTypeEqualityWitness(
            Type*  type);

        // If `sub` is a subtype of `sup`, then return a value that
        // can serve as a "witness" for that fact.
        Val* tryGetSubtypeWitness(
            Type*    sub,
            Type*    sup);

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
            OverloadCandidate const&	candidate);

        // Try to check an overload candidate, but bail out
        // if any step fails
        void TryCheckOverloadCandidate(
            OverloadResolveContext&		context,
            OverloadCandidate&			candidate);

        // Create the representation of a given generic applied to some arguments
        Expr* createGenericDeclRef(
            Expr*            baseExpr,
            Expr*            originalExpr,
            GenericSubstitution*   subst);

        // Take an overload candidate that previously got through
        // `TryCheckOverloadCandidate` above, and try to finish
        // up the work and turn it into a real expression.
        //
        // If the candidate isn't actually applicable, this is
        // where we'd start reporting the issue(s).
        Expr* CompleteOverloadCandidate(
            OverloadResolveContext&		context,
            OverloadCandidate&			candidate);

        // Implement a comparison operation between overload candidates,
        // so that the better candidate compares as less-than the other
        int CompareOverloadCandidates(
            OverloadCandidate*	left,
            OverloadCandidate*	right);

            /// If `declRef` representations a specialization of a generic, returns the number of specialized generic arguments.
            /// Otherwise, returns zero.
            ///
        Int getSpecializedParamCount(DeclRef<Decl> const& declRef);

            /// Compare items `left` and `right` produced by lookup, to see if one should be favored for overloading.
        int CompareLookupResultItems(
            LookupResultItem const& left,
            LookupResultItem const& right);

            /// Compare items `left` and `right` being considered as overload candidates, and determine if one should be favored for structural reasons.
        int compareOverloadCandidateSpecificity(
            LookupResultItem const& left,
            LookupResultItem const& right);

        void AddOverloadCandidateInner(
            OverloadResolveContext& context,
            OverloadCandidate&		candidate);

        void AddOverloadCandidate(
            OverloadResolveContext& context,
            OverloadCandidate&		candidate);

        void AddFuncOverloadCandidate(
            LookupResultItem			item,
            DeclRef<CallableDecl>             funcDeclRef,
            OverloadResolveContext&		context);

        void AddFuncOverloadCandidate(
            FuncType*		/*funcType*/,
            OverloadResolveContext&	/*context*/);

        // Add a candidate callee for overload resolution, based on
        // calling a particular `ConstructorDecl`.
        void AddCtorOverloadCandidate(
            LookupResultItem            typeItem,
            Type*                type,
            DeclRef<ConstructorDecl>    ctorDeclRef,
            OverloadResolveContext&     context,
            Type*                resultType);

        // If the given declaration has generic parameters, then
        // return the corresponding `GenericDecl` that holds the
        // parameters, etc.
        GenericDecl* GetOuterGeneric(Decl* decl);

        // Try to find a unification for two values
        bool TryUnifyVals(
            ConstraintSystem&	constraints,
            Val*			fst,
            Val*			snd);

        bool tryUnifySubstitutions(
            ConstraintSystem&       constraints,
            Substitutions*   fst,
            Substitutions*   snd);

        bool tryUnifyGenericSubstitutions(
            ConstraintSystem&           constraints,
            GenericSubstitution* fst,
            GenericSubstitution* snd);

        bool TryUnifyTypeParam(
            ConstraintSystem&				constraints,
            GenericTypeParamDecl*	typeParamDecl,
            Type*			type);

        bool TryUnifyIntParam(
            ConstraintSystem&               constraints,
            GenericValueParamDecl*	paramDecl,
            IntVal*                  val);

        bool TryUnifyIntParam(
            ConstraintSystem&       constraints,
            DeclRef<VarDeclBase> const&   varRef,
            IntVal*          val);

        bool TryUnifyTypesByStructuralMatch(
            ConstraintSystem&       constraints,
            Type*  fst,
            Type*  snd);

        bool TryUnifyTypes(
            ConstraintSystem&       constraints,
            Type*  fst,
            Type*  snd);

        bool TryUnifyConjunctionType(
            ConstraintSystem&   constraints,
            AndType*            fst,
            Type*               snd);

        // Is the candidate extension declaration actually applicable to the given type
        DeclRef<ExtensionDecl> ApplyExtensionToType(
            ExtensionDecl*  extDecl,
            Type*    type);

        // Take a generic declaration and try to specialize its parameters
        // so that the resulting inner declaration can be applicable in
        // a particular context...
        DeclRef<Decl> SpecializeGenericForOverload(
            DeclRef<GenericDecl>    genericDeclRef,
            OverloadResolveContext& context);

        void AddTypeOverloadCandidates(
            Type*	        type,
            OverloadResolveContext&	context);

        void AddDeclRefOverloadCandidates(
            LookupResultItem		item,
            OverloadResolveContext&	context);

        void AddOverloadCandidates(
            LookupResult const&     result,
            OverloadResolveContext&	context);

        void AddOverloadCandidates(
            Expr*	funcExpr,
            OverloadResolveContext&			context);

        String getCallSignatureString(
            OverloadResolveContext&     context);

        Expr* ResolveInvoke(InvokeExpr * expr);

        void AddGenericOverloadCandidate(
            LookupResultItem		baseItem,
            OverloadResolveContext&	context);

        void AddGenericOverloadCandidates(
            Expr*	baseExpr,
            OverloadResolveContext&			context);

            /// Check a generic application where the operands have already been checked.
        Expr* checkGenericAppWithCheckedArgs(GenericAppExpr* genericAppExpr);

        Expr* CheckExpr(Expr* expr);

        Expr* CheckInvokeExprWithCheckedOperands(InvokeExpr *expr);

        // Get the type to use when referencing a declaration
        QualType GetTypeForDeclRef(DeclRef<Decl> declRef, SourceLoc loc);

        //
        //
        //

        Expr* MaybeDereference(Expr* inExpr);

        Expr* CheckMatrixSwizzleExpr(
            MemberExpr* memberRefExpr,
            Type*      baseElementType,
            IntegerLiteralValue        baseElementRowCount,
            IntegerLiteralValue        baseElementColCount);

        Expr* CheckMatrixSwizzleExpr(
            MemberExpr* memberRefExpr,
            Type*      baseElementType,
            IntVal*        baseElementRowCount,
            IntVal*        baseElementColCount);

        Expr* CheckSwizzleExpr(
            MemberExpr* memberRefExpr,
            Type*      baseElementType,
            IntegerLiteralValue         baseElementCount);

        Expr* CheckSwizzleExpr(
            MemberExpr*	memberRefExpr,
            Type*		baseElementType,
            IntVal*				baseElementCount);

            /// Perform semantic checking of an assignment where the operands have already been checked.
        Expr* checkAssignWithCheckedOperands(AssignExpr* expr);

        // Look up a static member
        // @param expr Can be StaticMemberExpr or MemberExpr
        // @param baseExpression Is the underlying type expression determined from resolving expr
        Expr* _lookupStaticMember(DeclRefExpr* expr, Expr* baseExpression);

        Expr* visitStaticMemberExpr(StaticMemberExpr* expr);

            /// Perform checking operations required for the "base" expression of a member-reference like `base.someField`
        Expr* checkBaseForMemberExpr(Expr* baseExpr);

        Expr* lookupMemberResultFailure(
            DeclRefExpr*     expr,
            QualType const& baseType);

        SharedSemanticsContext& operator=(const SharedSemanticsContext &) = delete;


        //

        void importModuleIntoScope(Scope* scope, ModuleDecl* moduleDecl);

        void suggestCompletionItems(
            CompletionSuggestions::ScopeKind scopeKind, LookupResult const& lookupResult);
    };

    struct SemanticsExprVisitor
        : public SemanticsVisitor
        , ExprVisitor<SemanticsExprVisitor, Expr*>
    {
    public:
        SemanticsExprVisitor(SemanticsContext const& outer)
            : SemanticsVisitor(outer)
        {}

        Expr* visitIncompleteExpr(IncompleteExpr* expr);
        Expr* visitBoolLiteralExpr(BoolLiteralExpr* expr);
        Expr* visitNullPtrLiteralExpr(NullPtrLiteralExpr* expr);
        Expr* visitIntegerLiteralExpr(IntegerLiteralExpr* expr);
        Expr* visitFloatingPointLiteralExpr(FloatingPointLiteralExpr* expr);
        Expr* visitStringLiteralExpr(StringLiteralExpr* expr);

        Expr* visitIndexExpr(IndexExpr* subscriptExpr);

        Expr* visitParenExpr(ParenExpr* expr);

        Expr* visitAssignExpr(AssignExpr* expr);

        Expr* visitGenericAppExpr(GenericAppExpr* genericAppExpr);

        Expr* visitSharedTypeExpr(SharedTypeExpr* expr);

        Expr* visitTaggedUnionTypeExpr(TaggedUnionTypeExpr* expr);

        Expr* visitInvokeExpr(InvokeExpr *expr);

        Expr* visitVarExpr(VarExpr *expr);

        Expr* visitTypeCastExpr(TypeCastExpr * expr);

        Expr* visitTryExpr(TryExpr* expr);

        //
        // Some syntax nodes should not occur in the concrete input syntax,
        // and will only appear *after* checking is complete. We need to
        // deal with this cases here, even if they are no-ops.
        //

    #define CASE(NAME)                                                                           \
        Expr* visit##NAME(NAME* expr)                                                            \
        {                                                                                        \
            if (!getShared()->isInLanguageServer())                                              \
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), expr, "should not appear in input syntax"); \
            expr->type = m_astBuilder->getErrorType();                                           \
            return expr;                                                                         \
        }

        CASE(DerefExpr)
        CASE(MatrixSwizzleExpr)
        CASE(SwizzleExpr)
        CASE(OverloadedExpr)
        CASE(OverloadedExpr2)
        CASE(AggTypeCtorExpr)
        CASE(CastToSuperTypeExpr)
        CASE(ModifierCastExpr)
        CASE(LetExpr)
        CASE(ExtractExistentialValueExpr)

    #undef CASE

        Expr* visitStaticMemberExpr(StaticMemberExpr* expr);

        Expr* visitMemberExpr(MemberExpr * expr);

        Expr* visitInitializerListExpr(InitializerListExpr* expr);

        Expr* visitThisExpr(ThisExpr* expr);
        Expr* visitThisTypeExpr(ThisTypeExpr* expr);
        Expr* visitAndTypeExpr(AndTypeExpr* expr);
        Expr* visitModifiedTypeExpr(ModifiedTypeExpr* expr);

        Expr* visitJVPDifferentiateExpr(JVPDifferentiateExpr* expr);

            /// Perform semantic checking on a `modifier` that is being applied to the given `type`
        Val* checkTypeModifier(Modifier* modifier, Type* type);
    };

    struct SemanticsStmtVisitor
        : public SemanticsVisitor
        , StmtVisitor<SemanticsStmtVisitor>
    {
        SemanticsStmtVisitor(SemanticsContext const& outer)
            : SemanticsVisitor(outer)
        {}

        FunctionDeclBase* getParentFunc() { return m_parentFunc; }

        void checkStmt(Stmt* stmt);

        template<typename T>
        T* FindOuterStmt();

        void visitDeclStmt(DeclStmt* stmt);

        void visitBlockStmt(BlockStmt* stmt);

        void visitSeqStmt(SeqStmt* stmt);

        void visitBreakStmt(BreakStmt *stmt);

        void visitContinueStmt(ContinueStmt *stmt);

        void visitDoWhileStmt(DoWhileStmt *stmt);

        void visitForStmt(ForStmt *stmt);

        void visitCompileTimeForStmt(CompileTimeForStmt* stmt);

        void visitSwitchStmt(SwitchStmt* stmt);

        void visitCaseStmt(CaseStmt* stmt);

        void visitDefaultStmt(DefaultStmt* stmt);

        void visitIfStmt(IfStmt *stmt);

        void visitUnparsedStmt(UnparsedStmt*);

        void visitEmptyStmt(EmptyStmt*);

        void visitDiscardStmt(DiscardStmt*);

        void visitReturnStmt(ReturnStmt *stmt);

        void visitWhileStmt(WhileStmt *stmt);
        
        void visitGpuForeachStmt(GpuForeachStmt *stmt);

        void visitExpressionStmt(ExpressionStmt *stmt);
    };

    struct SemanticsDeclVisitorBase
        : public SemanticsVisitor
    {
        SemanticsDeclVisitorBase(SemanticsContext const& outer)
            : SemanticsVisitor(outer)
        {}

        void checkBodyStmt(Stmt* stmt, FunctionDeclBase* parentDecl)
        {
            checkStmt(stmt, withParentFunc(parentDecl));
        }

        void checkModule(ModuleDecl* programNode);
    };
}
