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
    RefPtr<TypeType> getTypeType(
        Type* type);

        /// Should the given `decl` be treated as a static rather than instance declaration?
    bool isEffectivelyStatic(
        Decl*           decl);

    RefPtr<Type> checkProperType(
        Linkage*        linkage,
        TypeExp         typeExp,
        DiagnosticSink* sink);

    // A flat representation of basic types (scalars, vectors and matrices)
    // that can be used as lookup key in caches
    enum class BasicTypeKey : uint8_t
    {
        Invalid = 0xff,                 ///< Value that can never be a valid type                
    };

    SLANG_FORCE_INLINE BasicTypeKey makeBasicTypeKey(BaseType baseType, IntegerLiteralValue dim1 = 1, IntegerLiteralValue dim2 = 1)
    {
        SLANG_ASSERT(dim1 > 0 && dim2 > 0);
        return BasicTypeKey((uint8_t(baseType) << 4) | ((uint8_t(dim1) - 1) << 2) | (uint8_t(dim2) - 1));
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
                auto elemBasicType = as<BasicExpressionType>(vectorType->elementType);
                return makeBasicTypeKey(elemBasicType->baseType, elemCount->value);
            }
        }
        else if (auto matrixType = as<MatrixExpressionType>(typeIn))
        {
            if (auto elemCount1 = as<ConstantIntVal>(matrixType->getRowCount()))
            {
                if (auto elemCount2 = as<ConstantIntVal>(matrixType->getColumnCount()))
                {
                    auto elemBasicType = as<BasicExpressionType>(matrixType->getElementType());
                    return makeBasicTypeKey(elemBasicType->baseType, elemCount1->value, elemCount2->value);
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

        int GetHashCode()
        {
            return int(type1) << 16 | int(type2);
        }
    };

    struct OperatorOverloadCacheKey
    {
        IROp operatorName;
        BasicTypeKey args[2];
        bool operator == (OperatorOverloadCacheKey key)
        {
            return operatorName == key.operatorName && args[0] == key.args[0] && args[1] == key.args[1];
        }
        int GetHashCode()
        {
            return ((int)(UInt64)(void*)(operatorName) << 16) ^ (int(args[0]) << 8) ^ (int(args[1]));
        }
        bool fromOperatorExpr(OperatorExpr* opExpr)
        {
            // First, lets see if the argument types are ones
            // that we can encode in our space of keys.
            args[0] = BasicTypeKey::Invalid;
            args[1] = BasicTypeKey::Invalid;
            if (opExpr->Arguments.getCount() > 2)
                return false;

            for (Index i = 0; i < opExpr->Arguments.getCount(); i++)
            {
                auto key = makeBasicTypeKey(opExpr->Arguments[i]->type.Ptr());
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

    struct TypeCheckingCache
    {
        Dictionary<OperatorOverloadCacheKey, OverloadCandidate> resolvedOperatorOverloadCache;
        Dictionary<BasicTypeKeyPair, ConversionCost> conversionCostCache;
    };

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

        // TODO(tfoley): What is this and why is it needed?
        RefPtr<Type> typeResult;

        RefPtr<Expr> TranslateTypeNodeImpl(const RefPtr<Expr> & node);
        RefPtr<Type> ExtractTypeFromTypeRepr(const RefPtr<Expr>& typeRepr);
        RefPtr<Type> TranslateTypeNode(const RefPtr<Expr> & node);
        TypeExp TranslateTypeNodeForced(TypeExp const& typeExp);
        TypeExp TranslateTypeNode(TypeExp const& typeExp);

        RefPtr<DeclRefType> getExprDeclRefType(Expr * expr);

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
        RefPtr<Expr> moveTemp(RefPtr<Expr> const& expr, F const& func);

            /// Execute `func` on a variable with the value of `expr`.
            ///
            /// If `expr` is just a reference to an immutable (e.g., `let`) variable
            /// then this might use the existing variable. Otherwise it will create
            /// a new variable to hold `expr`, using `moveTemp()`.
            ///
        template<typename F>
        RefPtr<Expr> maybeMoveTemp(RefPtr<Expr> const& expr, F const& func);

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
            DeclRef<InterfaceDecl>  interfaceDeclRef);

            /// If `expr` has existential type, then open it.
            ///
            /// Returns an expression that opens `expr` if it had existential type.
            /// Otherwise returns `expr` itself.
            ///
            /// See `openExistential` for a discussion of what "opening" an
            /// existential-type value means.
            ///
        RefPtr<Expr> maybeOpenExistential(RefPtr<Expr> expr);

        RefPtr<Expr> ConstructDeclRefExpr(
            DeclRef<Decl>   declRef,
            RefPtr<Expr>    baseExpr,
            SourceLoc       loc);

        RefPtr<Expr> ConstructDerefExpr(
            RefPtr<Expr>    base,
            SourceLoc       loc);

        RefPtr<Expr> createImplicitThisMemberExpr(
            Type*                                           type,
            SourceLoc                                       loc,
            LookupResultItem::Breadcrumb::ThisParameterMode thisParameterMode);

        RefPtr<Expr> ConstructLookupResultExpr(
            LookupResultItem const& item,
            RefPtr<Expr>            baseExpr,
            SourceLoc               loc);

        RefPtr<Expr> createLookupResultExpr(
            LookupResult const&     lookupResult,
            RefPtr<Expr>            baseExpr,
            SourceLoc               loc);

        RefPtr<Expr> ResolveOverloadedExpr(RefPtr<OverloadedExpr> overloadedExpr, LookupMask mask);

        RefPtr<Expr> ExpectATypeRepr(RefPtr<Expr> expr);

        RefPtr<Type> ExpectAType(RefPtr<Expr> expr);

        RefPtr<Type> ExtractGenericArgType(RefPtr<Expr> exp);

        RefPtr<IntVal> ExtractGenericArgInteger(RefPtr<Expr> exp);

        RefPtr<Val> ExtractGenericArgVal(RefPtr<Expr> exp);

        // Construct a type representing the instantiation of
        // the given generic declaration for the given arguments.
        // The arguments should already be checked against
        // the declaration.
        RefPtr<Type> InstantiateGenericType(
            DeclRef<GenericDecl>        genericDeclRef,
            List<RefPtr<Expr>> const&   args);

        // This routine is a bottleneck for all declaration checking,
        // so that we can add some quality-of-life features for users
        // in cases where the compiler crashes
        void dispatchDecl(DeclBase* decl);
        void dispatchStmt(Stmt* stmt);
        void dispatchExpr(Expr* expr);

        // Make sure a declaration has been checked, so we can refer to it.
        // Note that this may lead to us recursively invoking checking,
        // so this may not be the best way to handle things.
        void EnsureDecl(RefPtr<Decl> decl, DeclCheckState state);

        void EnusreAllDeclsRec(RefPtr<Decl> decl);

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

        RefPtr<Expr> CheckTerm(RefPtr<Expr> term);

        RefPtr<Expr> CreateErrorExpr(Expr* expr);

        bool IsErrorExpr(RefPtr<Expr> expr);

        // Capture the "base" expression in case this is a member reference
        RefPtr<Expr> GetBaseExpr(RefPtr<Expr> expr);

    public:

        bool ValuesAreEqual(
            RefPtr<IntVal> left,
            RefPtr<IntVal> right);

        // Compute the cost of using a particular declaration to
        // perform implicit type conversion.
        ConversionCost getImplicitConversionCost(
            Decl* decl);

        bool isEffectivelyScalarForInitializerLists(
            RefPtr<Type>    type);

            /// Should the provided expression (from an initializer list) be used directly to initialize `toType`?
        bool shouldUseInitializerDirectly(
            RefPtr<Type>    toType,
            RefPtr<Expr>    fromExpr);

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
            RefPtr<Type>                inToType,
            RefPtr<Expr>*               outToExpr,
            RefPtr<InitializerListExpr> fromInitializerListExpr,
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
            RefPtr<Type>                toType,
            RefPtr<Expr>*               outToExpr,
            RefPtr<InitializerListExpr> fromInitializerListExpr);

            /// Report that implicit type coercion is not possible.
        bool _failedCoercion(
            RefPtr<Type>    toType,
            RefPtr<Expr>*   outToExpr,
            RefPtr<Expr>    fromExpr);

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
            ConversionCost* outCost);

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
            ConversionCost* outCost = 0);

        RefPtr<TypeCastExpr> createImplicitCastExpr();

        RefPtr<Expr> CreateImplicitCastExpr(
            RefPtr<Type>	toType,
            RefPtr<Expr>	fromExpr);

            /// Create an "up-cast" from a value to an interface type
            ///
            /// This operation logically constructs an "existential" value,
            /// which packages up the value, its type, and the witness
            /// of its conformance to the interface.
            ///
        RefPtr<Expr> createCastToInterfaceExpr(
            RefPtr<Type>    toType,
            RefPtr<Expr>    fromExpr,
            RefPtr<Val>     witness);

            /// Implicitly coerce `fromExpr` to `toType` and diagnose errors if it isn't possible
        RefPtr<Expr> coerce(
            RefPtr<Type>    toType,
            RefPtr<Expr>    fromExpr);

        void CheckVarDeclCommon(RefPtr<VarDeclBase> varDecl);

        // Fill in default substitutions for the 'subtype' part of a type constraint decl
        void CheckConstraintSubType(TypeExp& typeExp);

        void CheckGenericConstraintDecl(GenericTypeConstraintDecl* decl);

        void checkDecl(Decl* decl);

        void checkGenericDeclHeader(GenericDecl* genericDecl);

        void visitGenericDecl(GenericDecl* genericDecl);

        void visitGenericTypeConstraintDecl(GenericTypeConstraintDecl * genericConstraintDecl);

        void visitInheritanceDecl(InheritanceDecl* inheritanceDecl);

        RefPtr<ConstantIntVal> checkConstantIntVal(
            RefPtr<Expr>    expr);

        RefPtr<ConstantIntVal> checkConstantEnumVal(
            RefPtr<Expr>    expr);

        // Check an expression, coerce it to the `String` type, and then
        // ensure that it has a literal (not just compile-time constant) value.
        bool checkLiteralStringVal(
            RefPtr<Expr>    expr,
            String*         outVal);

        void visitSyntaxDecl(SyntaxDecl*);

        void visitAttributeDecl(AttributeDecl*);

        void visitGenericTypeParamDecl(GenericTypeParamDecl*);

        void visitGenericValueParamDecl(GenericValueParamDecl*);

        void visitModifier(Modifier*);

        AttributeDecl* lookUpAttributeDecl(Name* attributeName, Scope* scope);

        bool hasIntArgs(Attribute* attr, int numArgs);
        bool hasStringArgs(Attribute* attr, int numArgs);

        bool getAttributeTargetSyntaxClasses(SyntaxClass<RefObject> & cls, uint32_t typeFlags);

        bool validateAttribute(RefPtr<Attribute> attr, AttributeDecl* attribClassDecl);

        RefPtr<AttributeBase> checkAttribute(
            UncheckedAttribute*     uncheckedAttr,
            ModifiableSyntaxNode*   attrTarget);

        RefPtr<Modifier> checkModifier(
            RefPtr<Modifier>        m,
            ModifiableSyntaxNode*   syntaxNode);

        void checkModifiers(ModifiableSyntaxNode* syntaxNode);

            /// Perform checking of interface conformaces for this decl and all its children
        void checkInterfaceConformancesRec(Decl* decl);

        void visitModuleDecl(ModuleDecl* programNode);

        bool doesSignatureMatchRequirement(
            DeclRef<CallableDecl>   satisfyingMemberDeclRef,
            DeclRef<CallableDecl>   requiredMemberDeclRef,
            RefPtr<WitnessTable>    witnessTable);

        bool doesGenericSignatureMatchRequirement(
            DeclRef<GenericDecl>        genDecl,
            DeclRef<GenericDecl>        requirementGenDecl,
            RefPtr<WitnessTable>        witnessTable);

        bool doesTypeSatisfyAssociatedTypeRequirement(
            RefPtr<Type>            satisfyingType,
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
            RefPtr<WitnessTable>        witnessTable);

        // Check that the type declaration `typeDecl`, which
        // declares conformance to the interface `interfaceDeclRef`,
        // (via the given `inheritanceDecl`) actually provides
        // members to satisfy all the requirements in the interface.
        RefPtr<WitnessTable> checkInterfaceConformance(
            ConformanceCheckingContext* context,
            DeclRef<AggTypeDeclBase>    typeDeclRef,
            InheritanceDecl*            inheritanceDecl,
            DeclRef<InterfaceDecl>      interfaceDeclRef);

        RefPtr<WitnessTable> checkConformanceToType(
            ConformanceCheckingContext* context,
            DeclRef<AggTypeDeclBase>    typeDeclRef,
            InheritanceDecl*            inheritanceDecl,
            Type*                       baseType);

        // Check that the type (or extension) declaration `declRef`,
        // which declares that it inherits from another type via
        // `inheritanceDecl` actually does what it needs to
        // for that inheritance to be valid.
        bool checkConformance(
            DeclRef<AggTypeDeclBase>    declRef,
            InheritanceDecl*            inheritanceDecl);

        void checkExtensionConformance(ExtensionDecl* decl);

        void checkAggTypeConformance(AggTypeDecl* decl);

        void visitAggTypeDecl(AggTypeDecl* decl);

        bool isIntegerBaseType(BaseType baseType);

        // Validate that `type` is a suitable type to use
        // as the tag type for an `enum`
        void validateEnumTagType(Type* type, SourceLoc const& loc);

        void visitEnumDecl(EnumDecl* decl);

        void visitEnumCaseDecl(EnumCaseDecl* decl);

        void visitDeclGroup(DeclGroup* declGroup);

        void visitTypeDefDecl(TypeDefDecl* decl);

        void visitGlobalGenericParamDecl(GlobalGenericParamDecl* decl);

        void visitAssocTypeDecl(AssocTypeDecl* decl);

        void checkStmt(Stmt* stmt);

        void visitFuncDecl(FuncDecl* functionNode);

        void getGenericParams(
            GenericDecl*                        decl,
            List<Decl*>&                        outParams,
            List<GenericTypeConstraintDecl*>    outConstraints);

        bool doGenericSignaturesMatch(
            GenericDecl*    fst,
            GenericDecl*    snd);

        // Check if two functions have the same signature for the purposes
        // of overload resolution.
        bool doFunctionSignaturesMatch(
            DeclRef<FuncDecl> fst,
            DeclRef<FuncDecl> snd);

        RefPtr<GenericSubstitution> createDummySubstitutions(
            GenericDecl* genericDecl);

        void ValidateFunctionRedeclaration(FuncDecl* funcDecl);

        void visitScopeDecl(ScopeDecl*);

        void visitParamDecl(ParamDecl* paramDecl);

        void VisitFunctionDeclaration(FuncDecl *functionNode);

        void visitDeclStmt(DeclStmt* stmt);

        void visitBlockStmt(BlockStmt* stmt);

        void visitSeqStmt(SeqStmt* stmt);

        template<typename T>
        T* FindOuterStmt();

        void visitBreakStmt(BreakStmt *stmt);

        void visitContinueStmt(ContinueStmt *stmt);

        void PushOuterStmt(Stmt* stmt);

        void PopOuterStmt(Stmt* /*stmt*/);

        RefPtr<Expr> checkPredicateExpr(Expr* expr);

        void visitDoWhileStmt(DoWhileStmt *stmt);

        void visitForStmt(ForStmt *stmt);

        RefPtr<Expr> checkExpressionAndExpectIntegerConstant(RefPtr<Expr> expr, RefPtr<IntVal>* outIntVal);
        void visitCompileTimeForStmt(CompileTimeForStmt* stmt);

        void visitSwitchStmt(SwitchStmt* stmt);

        void visitCaseStmt(CaseStmt* stmt);

        void visitDefaultStmt(DefaultStmt* stmt);

        void visitIfStmt(IfStmt *stmt);

        void visitUnparsedStmt(UnparsedStmt*);

        void visitEmptyStmt(EmptyStmt*);

        void visitDiscardStmt(DiscardStmt*);

        void visitReturnStmt(ReturnStmt *stmt);

        IntegerLiteralValue GetMinBound(RefPtr<IntVal> val);

        void maybeInferArraySizeForVariable(VarDeclBase* varDecl);

        void validateArraySizeForVariable(VarDeclBase* varDecl);

        void visitVarDecl(VarDecl* varDecl);

        void visitWhileStmt(WhileStmt *stmt);

        void visitExpressionStmt(ExpressionStmt *stmt);

        RefPtr<Expr> visitBoolLiteralExpr(BoolLiteralExpr* expr);
        RefPtr<Expr> visitIntegerLiteralExpr(IntegerLiteralExpr* expr);
        RefPtr<Expr> visitFloatingPointLiteralExpr(FloatingPointLiteralExpr* expr);
        RefPtr<Expr> visitStringLiteralExpr(StringLiteralExpr* expr);

        IntVal* GetIntVal(IntegerLiteralExpr* expr);

        Linkage* getLinkage() { return m_linkage; }
        NamePool* getNamePool() { return getLinkage()->getNamePool(); }

        Name* getName(String const& text)
        {
            return getNamePool()->getName(text);
        }

        RefPtr<IntVal> TryConstantFoldExpr(
            InvokeExpr* invokeExpr);

        RefPtr<IntVal> TryConstantFoldExpr(
            Expr* expr);

        // Try to check an integer constant expression, either returning the value,
        // or NULL if the expression isn't recognized as a constant.
        RefPtr<IntVal> TryCheckIntegerConstantExpression(Expr* exp);

        // Enforce that an expression resolves to an integer constant, and get its value
        RefPtr<IntVal> CheckIntegerConstantExpression(Expr* inExpr);

        RefPtr<IntVal> CheckEnumConstantExpression(Expr* expr);


        RefPtr<Expr> CheckSimpleSubscriptExpr(
            RefPtr<IndexExpr>   subscriptExpr,
            RefPtr<Type>              elementType);

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
            RefPtr<IntVal>          elementCount);

        RefPtr<Expr> visitIndexExpr(IndexExpr* subscriptExpr);

        RefPtr<Expr> visitParenExpr(ParenExpr* expr);

        //

            /// Given an immutable `expr` used as an l-value emit a special diagnostic if it was derived from `this`.
        void maybeDiagnoseThisNotLValue(Expr* expr);

        RefPtr<Expr> visitAssignExpr(AssignExpr* expr);

        void registerExtension(ExtensionDecl* decl);

        void visitExtensionDecl(ExtensionDecl* decl);

        // Figure out what type an initializer/constructor declaration
        // is supposed to return. In most cases this is just the type
        // declaration that its declaration is nested inside.
        RefPtr<Type> findResultTypeForConstructorDecl(ConstructorDecl* decl);

        void visitConstructorDecl(ConstructorDecl* decl);


        void visitSubscriptDecl(SubscriptDecl* decl);

        void visitAccessorDecl(AccessorDecl* decl);


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
            RefPtr<BasicExpressionType>  scalarType);

        struct TypeWitnessBreadcrumb
        {
            TypeWitnessBreadcrumb*  prev;

            RefPtr<Type>            sub;
            RefPtr<Type>            sup;
            DeclRef<Decl>           declRef;
        };

        // Create a subtype witness based on the declared relationship
        // found in a single breadcrumb
        RefPtr<DeclaredSubtypeWitness> createSimpleSubtypeWitness(
            TypeWitnessBreadcrumb*  breadcrumb);

        RefPtr<Val> createTypeWitness(
            RefPtr<Type>            type,
            DeclRef<InterfaceDecl>  interfaceDeclRef,
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

        bool doesTypeConformToInterfaceImpl(
            RefPtr<Type>            originalType,
            RefPtr<Type>            type,
            DeclRef<InterfaceDecl>  interfaceDeclRef,
            RefPtr<Val>*            outWitness,
            TypeWitnessBreadcrumb*  inBreadcrumbs);

        bool DoesTypeConformToInterface(
            RefPtr<Type>  type,
            DeclRef<InterfaceDecl>        interfaceDeclRef);

        RefPtr<Val> tryGetInterfaceConformanceWitness(
            RefPtr<Type>  type,
            DeclRef<InterfaceDecl>        interfaceDeclRef);

        /// Does there exist an implicit conversion from `fromType` to `toType`?
        bool canConvertImplicitly(
            RefPtr<Type> toType,
            RefPtr<Type> fromType);

        RefPtr<Type> TryJoinTypeWithInterface(
            RefPtr<Type>            type,
            DeclRef<InterfaceDecl>      interfaceDeclRef);

        // Try to compute the "join" between two types
        RefPtr<Type> TryJoinTypes(
            RefPtr<Type>  left,
            RefPtr<Type>  right);

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
        ParamCounts CountParameters(FilteredMemberRefList<ParamDecl> params);

        // count the number of parameters required/allowed for a generic
        ParamCounts CountParameters(DeclRef<GenericDecl> genericRef);

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
        RefPtr<Val> createTypeEqualityWitness(
            Type*  type);

        // If `sub` is a subtype of `sup`, then return a value that
        // can serve as a "witness" for that fact.
        RefPtr<Val> tryGetSubtypeWitness(
            RefPtr<Type>    sub,
            RefPtr<Type>    sup);

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
        RefPtr<Expr> createGenericDeclRef(
            RefPtr<Expr>            baseExpr,
            RefPtr<Expr>            originalExpr,
            RefPtr<GenericSubstitution>   subst);

        // Take an overload candidate that previously got through
        // `TryCheckOverloadCandidate` above, and try to finish
        // up the work and turn it into a real expression.
        //
        // If the candidate isn't actually applicable, this is
        // where we'd start reporting the issue(s).
        RefPtr<Expr> CompleteOverloadCandidate(
            OverloadResolveContext&		context,
            OverloadCandidate&			candidate);

        // Implement a comparison operation between overload candidates,
        // so that the better candidate compares as less-than the other
        int CompareOverloadCandidates(
            OverloadCandidate*	left,
            OverloadCandidate*	right);

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
            RefPtr<FuncType>		/*funcType*/,
            OverloadResolveContext&	/*context*/);

        // Add a candidate callee for overload resolution, based on
        // calling a particular `ConstructorDecl`.
        void AddCtorOverloadCandidate(
            LookupResultItem            typeItem,
            RefPtr<Type>                type,
            DeclRef<ConstructorDecl>    ctorDeclRef,
            OverloadResolveContext&     context,
            RefPtr<Type>                resultType);

        // If the given declaration has generic parameters, then
        // return the corresponding `GenericDecl` that holds the
        // parameters, etc.
        GenericDecl* GetOuterGeneric(Decl* decl);

        // Try to find a unification for two values
        bool TryUnifyVals(
            ConstraintSystem&	constraints,
            RefPtr<Val>			fst,
            RefPtr<Val>			snd);

        bool tryUnifySubstitutions(
            ConstraintSystem&       constraints,
            RefPtr<Substitutions>   fst,
            RefPtr<Substitutions>   snd);

        bool tryUnifyGenericSubstitutions(
            ConstraintSystem&           constraints,
            RefPtr<GenericSubstitution> fst,
            RefPtr<GenericSubstitution> snd);

        bool TryUnifyTypeParam(
            ConstraintSystem&				constraints,
            RefPtr<GenericTypeParamDecl>	typeParamDecl,
            RefPtr<Type>			type);

        bool TryUnifyIntParam(
            ConstraintSystem&               constraints,
            RefPtr<GenericValueParamDecl>	paramDecl,
            RefPtr<IntVal>                  val);

        bool TryUnifyIntParam(
            ConstraintSystem&       constraints,
            DeclRef<VarDeclBase> const&   varRef,
            RefPtr<IntVal>          val);

        bool TryUnifyTypesByStructuralMatch(
            ConstraintSystem&       constraints,
            RefPtr<Type>  fst,
            RefPtr<Type>  snd);

        bool TryUnifyTypes(
            ConstraintSystem&       constraints,
            RefPtr<Type>  fst,
            RefPtr<Type>  snd);

        // Is the candidate extension declaration actually applicable to the given type
        DeclRef<ExtensionDecl> ApplyExtensionToType(
            ExtensionDecl*  extDecl,
            RefPtr<Type>    type);

        // Take a generic declaration and try to specialize its parameters
        // so that the resulting inner declaration can be applicable in
        // a particular context...
        DeclRef<Decl> SpecializeGenericForOverload(
            DeclRef<GenericDecl>    genericDeclRef,
            OverloadResolveContext& context);

        void AddAggTypeOverloadCandidates(
            LookupResultItem        typeItem,
            RefPtr<Type>            type,
            DeclRef<AggTypeDecl>    aggTypeDeclRef,
            OverloadResolveContext& context,
            RefPtr<Type>            resultType);

        void addGenericTypeParamOverloadCandidates(
            DeclRef<GenericTypeParamDecl>   typeDeclRef,
            OverloadResolveContext&         context,
            RefPtr<Type>                    resultType);

        void AddTypeOverloadCandidates(
            RefPtr<Type>	        type,
            OverloadResolveContext&	context,
            RefPtr<Type>            resultType);

        void AddDeclRefOverloadCandidates(
            LookupResultItem		item,
            OverloadResolveContext&	context);

        void AddOverloadCandidates(
            RefPtr<Expr>	funcExpr,
            OverloadResolveContext&			context);

        void formatType(StringBuilder& sb, RefPtr<Type> type);

        void formatVal(StringBuilder& sb, RefPtr<Val> val);

        void formatDeclPath(StringBuilder& sb, DeclRef<Decl> declRef);

        void formatDeclParams(StringBuilder& sb, DeclRef<Decl> declRef);

        void formatDeclSignature(StringBuilder& sb, DeclRef<Decl> declRef);

        String getDeclSignatureString(DeclRef<Decl> declRef);

        String getDeclSignatureString(LookupResultItem item);

        String getCallSignatureString(
            OverloadResolveContext&     context);

        RefPtr<Expr> ResolveInvoke(InvokeExpr * expr);

        void AddGenericOverloadCandidate(
            LookupResultItem		baseItem,
            OverloadResolveContext&	context);

        void AddGenericOverloadCandidates(
            RefPtr<Expr>	baseExpr,
            OverloadResolveContext&			context);

        RefPtr<Expr> visitGenericAppExpr(GenericAppExpr* genericAppExpr);

            /// Check a generic application where the operands have already been checked.
        RefPtr<Expr> checkGenericAppWithCheckedArgs(GenericAppExpr* genericAppExpr);

        RefPtr<Expr> visitSharedTypeExpr(SharedTypeExpr* expr);

        RefPtr<Expr> visitTaggedUnionTypeExpr(TaggedUnionTypeExpr* expr);

        RefPtr<Expr> CheckExpr(RefPtr<Expr> expr);

        RefPtr<Expr> CheckInvokeExprWithCheckedOperands(InvokeExpr *expr);

        RefPtr<Expr> visitInvokeExpr(InvokeExpr *expr);

        RefPtr<Expr> visitVarExpr(VarExpr *expr);

        RefPtr<Expr> visitTypeCastExpr(TypeCastExpr * expr);

        // Get the type to use when referencing a declaration
        QualType GetTypeForDeclRef(DeclRef<Decl> declRef);

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

        RefPtr<Expr> MaybeDereference(RefPtr<Expr> inExpr);

        RefPtr<Expr> CheckSwizzleExpr(
            MemberExpr* memberRefExpr,
            RefPtr<Type>      baseElementType,
            IntegerLiteralValue         baseElementCount);

        RefPtr<Expr> CheckSwizzleExpr(
            MemberExpr*	memberRefExpr,
            RefPtr<Type>		baseElementType,
            RefPtr<IntVal>				baseElementCount);

        // Look up a static member
        // @param expr Can be StaticMemberExpr or MemberExpr
        // @param baseExpression Is the underlying type expression determined from resolving expr
        RefPtr<Expr> _lookupStaticMember(RefPtr<DeclRefExpr> expr, RefPtr<Expr> baseExpression);

        RefPtr<Expr> visitStaticMemberExpr(StaticMemberExpr* expr);

        RefPtr<Expr> lookupMemberResultFailure(
            DeclRefExpr*     expr,
            QualType const& baseType);

        RefPtr<Expr> visitMemberExpr(MemberExpr * expr);

        SemanticsVisitor & operator = (const SemanticsVisitor &) = delete;


        //

        RefPtr<Expr> visitInitializerListExpr(InitializerListExpr* expr);

        void importModuleIntoScope(Scope* scope, ModuleDecl* moduleDecl);

        void visitEmptyDecl(EmptyDecl* /*decl*/);

        void visitImportDecl(ImportDecl* decl);

        // Perform semantic checking of an object-oriented `this`
        // expression.
        RefPtr<Expr> visitThisExpr(ThisExpr* expr);
    };
}
