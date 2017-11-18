// syntax-base-defs.h

// This file defines the primary base classes for the hierarchy of
// AST nodes and related objects. For example, this is where the
// basic `Decl`, `Stmt`, `Expr`, `type`, etc. definitions come from.

ABSTRACT_SYNTAX_CLASS(NodeBase, RefObject)
    // A helper to access the corresponding class on a concrete instance
    RAW(
    virtual SyntaxClass<NodeBase> getClass() = 0;
    )
END_SYNTAX_CLASS()

// Base class for all nodes representing actual syntax
// (thus having a location in the source code)
ABSTRACT_SYNTAX_CLASS(SyntaxNodeBase, NodeBase)
    // The primary source location associated with this AST node
    FIELD(SourceLoc, loc)

    RAW(
    // Allow dynamic casting with a convenient syntax
    template<typename T>
    T* As()
    {
        return dynamic_cast<T*>(this);
    }
    )

END_SYNTAX_CLASS()

// Base class for compile-time values (most often a type).
// These are *not* syntax nodes, because they do not have
// a unique location, and any two `Val`s representing
// the same value should be conceptually equal.
ABSTRACT_SYNTAX_CLASS(Val, NodeBase)
    RAW(typedef IValVisitor Visitor;)

    RAW(virtual void accept(IValVisitor* visitor, void* extra) = 0;)

    RAW(
    // construct a new value by applying a set of parameter
    // substitutions to this one
    RefPtr<Val> Substitute(Substitutions* subst);

    // Lower-level interface for substition. Like the basic
    // `Substitute` above, but also takes a by-reference
    // integer parameter that should be incremented when
    // returning a modified value (this can help the caller
    // decide whether they need to do anything).
    virtual RefPtr<Val> SubstituteImpl(Substitutions* subst, int* ioDiff);

    virtual bool EqualsVal(Val* val) = 0;
    virtual String ToString() = 0;
    virtual int GetHashCode() = 0;
    bool operator == (const Val & v)
    {
        return EqualsVal(const_cast<Val*>(&v));
    }
    )
END_SYNTAX_CLASS()

// A type, representing a classifier for some term in the AST.
//
// Types can include "sugar" in that they may refer to a
// `typedef` which gives them a good name when printed as
// part of diagnostic messages.
//
// In order to operation on types, though, we often want
// to look past any sugar, and operate on an underlying
// "canonical" type. The reprsentation caches a pointer to
// a canonical type on every type, so we can easily
// operate on the raw representation when needed.
ABSTRACT_SYNTAX_CLASS(Type, Val)
    RAW(typedef ITypeVisitor Visitor;)

    RAW(virtual void accept(IValVisitor* visitor, void* extra) override;)
    RAW(virtual void accept(ITypeVisitor* visitor, void* extra) = 0;)

RAW(
public:
    Session* getSession() { return this->session; }
    void setSession(Session* s) { this->session = s; }

    virtual String ToString() = 0;

    bool Equals(Type * type);
    bool Equals(RefPtr<Type> type);

    bool IsVectorType() { return As<VectorExpressionType>() != nullptr; }
    bool IsArray() { return As<ArrayExpressionType>() != nullptr; }

    template<typename T>
    T* As()
    {
        return dynamic_cast<T*>(GetCanonicalType());
    }

    // Convenience/legacy wrappers for `As<>`
    ArithmeticExpressionType * AsArithmeticType() { return As<ArithmeticExpressionType>(); }
    BasicExpressionType * AsBasicType() { return As<BasicExpressionType>(); }
    VectorExpressionType * AsVectorType() { return As<VectorExpressionType>(); }
    MatrixExpressionType * AsMatrixType() { return As<MatrixExpressionType>(); }
    ArrayExpressionType * AsArrayType() { return As<ArrayExpressionType>(); }

    DeclRefType* AsDeclRefType() { return As<DeclRefType>(); }

    NamedExpressionType* AsNamedType();

    bool IsTextureOrSampler();
    bool IsTexture() { return As<TextureType>() != nullptr; }
    bool IsSampler() { return As<SamplerStateType>() != nullptr; }
    bool IsStruct();
    bool IsClass();
    Type* GetCanonicalType();

    virtual RefPtr<Val> SubstituteImpl(Substitutions* subst, int* ioDiff) override;

    virtual bool EqualsVal(Val* val) override;
protected:
    virtual bool EqualsImpl(Type * type) = 0;

    virtual Type* CreateCanonicalType() = 0;
    Type* canonicalType = nullptr;

    Session* session = nullptr;
    )
END_SYNTAX_CLASS()


// A substitution represents a binding of certain
// type-level variables to concrete argument values
ABSTRACT_SYNTAX_CLASS(Substitutions, RefObject)

    // Any further substitutions, relating to outer generic declarations
    SYNTAX_FIELD(RefPtr<Substitutions>, outer)

    RAW(
    // Apply a set of substitutions to the bindings in this substitution
    virtual RefPtr<Substitutions> SubstituteImpl(Substitutions* subst, int* ioDiff) = 0;

    // Check if these are equivalent substitutiosn to another set
    virtual bool Equals(Substitutions* subst) = 0;
    virtual bool operator == (const Substitutions & subst) = 0;
    virtual int GetHashCode() const = 0;
    )
END_SYNTAX_CLASS()

SYNTAX_CLASS(GenericSubstitution, Substitutions)
    // The generic declaration that defines the
    // parametesr we are binding to arguments
    DECL_FIELD(GenericDecl*, genericDecl)

    // The actual values of the arguments
    SYNTAX_FIELD(List<RefPtr<Val>>, args)
    
    RAW(
    // Apply a set of substitutions to the bindings in this substitution
    virtual RefPtr<Substitutions> SubstituteImpl(Substitutions* subst, int* ioDiff) override;

    // Check if these are equivalent substitutiosn to another set
    virtual bool Equals(Substitutions* subst) override;
    virtual bool operator == (const Substitutions & subst) override
    {
        return Equals(const_cast<Substitutions*>(&subst));
    }
    virtual int GetHashCode() const override
    {
        int rs = 0;
        for (auto && v : args)
        {
            rs ^= v->GetHashCode();
            rs *= 16777619;
        }
        return rs;
    }
    )
END_SYNTAX_CLASS()

SYNTAX_CLASS(ThisTypeSubstitution, Substitutions)
    // The actual type that provides the lookup scope for an associated type
    SYNTAX_FIELD(RefPtr<Val>, sourceType)

    RAW(
    // Apply a set of substitutions to the bindings in this substitution
    virtual RefPtr<Substitutions> SubstituteImpl(Substitutions* subst, int* ioDiff) override;

    // Check if these are equivalent substitutiosn to another set
    virtual bool Equals(Substitutions* subst) override;
    virtual bool operator == (const Substitutions & subst) override
    {
        return Equals(const_cast<Substitutions*>(&subst));
    }
    virtual int GetHashCode() const override
    {
        return sourceType->GetHashCode();
    }
    )
END_SYNTAX_CLASS()

SYNTAX_CLASS(GlobalGenericParamSubstitution, Substitutions)
    // the __generic_param decl to be substituted
    DECL_FIELD(GlobalGenericParamDecl*, paramDecl)
    // the actual type to substitute in
    SYNTAX_FIELD(RefPtr<Val>, actualType)

RAW(
    // Apply a set of substitutions to the bindings in this substitution
    virtual RefPtr<Substitutions> SubstituteImpl(Substitutions* subst, int* ioDiff) override;

    // Check if these are equivalent substitutiosn to another set
    virtual bool Equals(Substitutions* subst) override;
    virtual bool operator == (const Substitutions & subst) override
    {
        return Equals(const_cast<Substitutions*>(&subst));
    }
    virtual int GetHashCode() const override
    {
        int rs = actualType->GetHashCode();
        for (auto && v : witnessTables)
        {
            rs = combineHash(rs, v.Key->GetHashCode());
            rs = combineHash(rs, v.Value->GetHashCode());
        }
        return rs;
    }
    typedef List<KeyValuePair<RefPtr<Type>, RefPtr<Val>>> WitnessTableLookupTable;
)
    // The witness tables for each interface this actual type implements
    SYNTAX_FIELD(WitnessTableLookupTable, witnessTables)
END_SYNTAX_CLASS()

ABSTRACT_SYNTAX_CLASS(SyntaxNode, SyntaxNodeBase)
END_SYNTAX_CLASS()

//
// All modifiers are represented as full-fledged objects in the AST
// (that is, we don't use a bitfield, even for simple/common flags).
// This ensures that we can track source locations for all modifiers.
//
ABSTRACT_SYNTAX_CLASS(Modifier, SyntaxNodeBase)
    RAW(typedef IModifierVisitor Visitor;)

    RAW(virtual void accept(IModifierVisitor* visitor, void* extra) = 0;)

    // Next modifier in linked list of modifiers on same piece of syntax
    SYNTAX_FIELD(RefPtr<Modifier>, next)

    // The keyword that was used to introduce t that was used to name this modifier.
    FIELD(Name*, name)

    RAW(
        Name* getName() { return name; }
        NameLoc getNameAndLoc() { return NameLoc(name, loc); }
    )
END_SYNTAX_CLASS()

// A syntax node which can have modifiers appled
ABSTRACT_SYNTAX_CLASS(ModifiableSyntaxNode, SyntaxNode)

    SYNTAX_FIELD(Modifiers, modifiers)

    RAW(
    template<typename T>
    FilteredModifierList<T> GetModifiersOfType() { return FilteredModifierList<T>(modifiers.first.Ptr()); }

    // Find the first modifier of a given type, or return `nullptr` if none is found.
    template<typename T>
    T* FindModifier()
    {
        return *GetModifiersOfType<T>().begin();
    }

    template<typename T>
    bool HasModifier() { return FindModifier<T>() != nullptr; }
    )
END_SYNTAX_CLASS()


// An intermediate type to represent either a single declaration, or a group of declarations
ABSTRACT_SYNTAX_CLASS(DeclBase, ModifiableSyntaxNode)
    RAW(typedef IDeclVisitor Visitor;)

    RAW(virtual void accept(IDeclVisitor* visitor, void* extra) = 0;)


END_SYNTAX_CLASS()

ABSTRACT_SYNTAX_CLASS(Decl, DeclBase)
    DECL_FIELD(ContainerDecl*, ParentDecl RAW(=nullptr))

    FIELD(NameLoc, nameAndLoc)

    RAW(
    Name*     getName()       { return nameAndLoc.name; }
    SourceLoc getNameLoc()    { return nameAndLoc.loc ; }
    NameLoc   getNameAndLoc() { return nameAndLoc     ; }
    )


    FIELD_INIT(DeclCheckState, checkState, DeclCheckState::Unchecked)

    // The next declaration defined in the same container with the same name
    DECL_FIELD(Decl*, nextInContainerWithSameName RAW(= nullptr))

    RAW(
    bool IsChecked(DeclCheckState state) { return checkState >= state; }
    void SetCheckState(DeclCheckState state)
    {
        SLANG_RELEASE_ASSERT(state >= checkState);
        checkState = state;
    }
    )
END_SYNTAX_CLASS()

ABSTRACT_SYNTAX_CLASS(Expr, SyntaxNode)
    RAW(typedef IExprVisitor Visitor;)

    FIELD(QualType, type)

    RAW(virtual void accept(IExprVisitor* visitor, void* extra) = 0;)

END_SYNTAX_CLASS()

ABSTRACT_SYNTAX_CLASS(Stmt, ModifiableSyntaxNode)
    RAW(typedef IStmtVisitor Visitor;)

    RAW(virtual void accept(IStmtVisitor* visitor, void* extra) = 0;)

END_SYNTAX_CLASS()
