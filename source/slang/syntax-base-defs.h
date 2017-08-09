// syntax-base-defs.h

// This file defines the primary base classes for the hierarchy of
// AST nodes and related objects. For example, this is where the
// basic `Decl`, `Stmt`, `Expr`, `Type`, etc. definitions come from.

// Base class for all nodes representing actual syntax
// (thus having a location in the source code)
ABSTRACT_SYNTAX_CLASS(SyntaxNodeBase, RefObject)

    // The primary source location associated with this AST node
    FIELD(CodePosition, Position)

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
ABSTRACT_SYNTAX_CLASS(Val, RefObject)
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
ABSTRACT_SYNTAX_CLASS(ExpressionType, Val)
    RAW(typedef ITypeVisitor Visitor;)

    RAW(virtual void accept(IValVisitor* visitor, void* extra) override;)
    RAW(virtual void accept(ITypeVisitor* visitor, void* extra) = 0;)

RAW(
public:
    Session* getSession() { return this->session; }
    void setSession(Session* s) { this->session = s; }

    virtual String ToString() = 0;

    bool Equals(ExpressionType * type);
    bool Equals(RefPtr<ExpressionType> type);

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
    ExpressionType* GetCanonicalType();

    virtual RefPtr<Val> SubstituteImpl(Substitutions* subst, int* ioDiff) override;

    virtual bool EqualsVal(Val* val) override;
protected:
    virtual bool EqualsImpl(ExpressionType * type) = 0;

    virtual ExpressionType* CreateCanonicalType() = 0;
    ExpressionType* canonicalType = nullptr;

    Session* session = nullptr;
    )
END_SYNTAX_CLASS()

// A substitution represents a binding of certain
// type-level variables to concrete argument values
SYNTAX_CLASS(Substitutions, RefObject)

    // The generic declaration that defines the
    // parametesr we are binding to arguments
    DECL_FIELD(GenericDecl*,  genericDecl)

    // The actual values of the arguments
    SYNTAX_FIELD(List<RefPtr<Val>>, args)

    // Any further substitutions, relating to outer generic declarations
    SYNTAX_FIELD(RefPtr<Substitutions>, outer)

    RAW(
    // Apply a set of substitutions to the bindings in this substitution
    RefPtr<Substitutions> SubstituteImpl(Substitutions* subst, int* ioDiff);

    // Check if these are equivalent substitutiosn to another set
    bool Equals(Substitutions* subst);
    bool operator == (const Substitutions & subst)
    {
        return Equals(const_cast<Substitutions*>(&subst));
    }
    int GetHashCode() const
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

    // The token that was used to name this modifier.
    FIELD(Token, nameToken)
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

    FIELD(Token, Name)

    RAW(
    String const& getName() { return Name.Content; }
    Token const& getNameToken() { return Name; }
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

ABSTRACT_SYNTAX_CLASS(ExpressionSyntaxNode, SyntaxNode)
    RAW(typedef IExprVisitor Visitor;)

    FIELD(QualType, Type)

    RAW(virtual void accept(IExprVisitor* visitor, void* extra) = 0;)

END_SYNTAX_CLASS()

ABSTRACT_SYNTAX_CLASS(StatementSyntaxNode, ModifiableSyntaxNode)
    RAW(typedef IStmtVisitor Visitor;)

    RAW(virtual void accept(IStmtVisitor* visitor, void* extra) = 0;)

END_SYNTAX_CLASS()
