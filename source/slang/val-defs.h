// val-defs.h

// Syntax class definitions for compile-time values.

// A compile-time integer (may not have a specific concrete value)
ABSTRACT_SYNTAX_CLASS(IntVal, Val)
END_SYNTAX_CLASS()

// Trivial case of a value that is just a constant integer
SYNTAX_CLASS(ConstantIntVal, IntVal)
    FIELD(IntegerLiteralValue, value)

    RAW(
    ConstantIntVal()
    {}
    ConstantIntVal(IntegerLiteralValue value)
        : value(value)
    {}

    virtual bool EqualsVal(Val* val) override;
    virtual String ToString() override;
    virtual int GetHashCode() override;
    )
END_SYNTAX_CLASS()

// The logical "value" of a rererence to a generic value parameter
SYNTAX_CLASS(GenericParamIntVal, IntVal)
    DECL_FIELD(DeclRef<VarDeclBase>, declRef)

    RAW(
    GenericParamIntVal()
    {}
    GenericParamIntVal(DeclRef<VarDeclBase> declRef)
        : declRef(declRef)
    {}

    virtual bool EqualsVal(Val* val) override;
    virtual String ToString() override;
    virtual int GetHashCode() override;
    virtual RefPtr<Val> SubstituteImpl(Substitutions* subst, int* ioDiff) override;
)
END_SYNTAX_CLASS()
