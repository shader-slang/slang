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
    virtual RefPtr<Val> SubstituteImpl(SubstitutionSet subst, int* ioDiff) override;
)
END_SYNTAX_CLASS()

// A witness to the fact that some proposition is true, encoded
// at the level of the type system.
//
// Given a generic like:
//
//     void example<L>(L light)
//          where L : ILight
//     { ... }
//
// a call to `example()` needs two things for us to be sure
// it is valid:
//
// 1. We need a type `X` to use as the argument for the
//    parameter `L`. We might supply this explicitly, or
//    via inference.
//
// 2. We need a *proof* that whatever `X` we chose conforms
//    to the `ILight` interface.
//
// The easiest way to make such a proof is by construction,
// and a `Witness` represents such a constructive proof.
// Conceptually a proposition like `X : ILight` can be
// seen as a type, and witness prooving that proposition
// is a value of that type.
//
// We construct and store witnesses explicitly during
// semantic checking because they can help us with
// generating downstream code. By following the structure
// of a witness (the structure of a proof) we can, e.g.,
// navigate from the knowledge that `X : ILight` to
// the concrete declarations that provide the implementation
// of `ILight` for `X`.
// 
ABSTRACT_SYNTAX_CLASS(Witness, Val)
END_SYNTAX_CLASS()

// A witness that one type is a subtype of another
// (where by "subtype" we include both inheritance
// relationships and type-conforms-to-interface relationships)
//
// TODO: we may need to tease those apart.
ABSTRACT_SYNTAX_CLASS(SubtypeWitness, Witness)
    FIELD(RefPtr<Type>, sub)
    FIELD(RefPtr<Type>, sup)
END_SYNTAX_CLASS()

SYNTAX_CLASS(TypeEqualityWitness, SubtypeWitness)
RAW(
    virtual bool EqualsVal(Val* val) override;
    virtual String ToString() override;
    virtual int GetHashCode() override;
    virtual RefPtr<Val> SubstituteImpl(SubstitutionSet subst, int * ioDiff) override;
)
END_SYNTAX_CLASS()
// A witness that one type is a subtype of another
// because some in-scope declaration says so
SYNTAX_CLASS(DeclaredSubtypeWitness, SubtypeWitness)
    FIELD(DeclRef<Decl>, declRef);
RAW(
    virtual bool EqualsVal(Val* val) override;
    virtual String ToString() override;
    virtual int GetHashCode() override;
    virtual RefPtr<Val> SubstituteImpl(SubstitutionSet subst, int * ioDiff) override;
)
END_SYNTAX_CLASS()

// A witness that `sub : sup` because `sub : mid` and `mid : sup`
SYNTAX_CLASS(TransitiveSubtypeWitness, SubtypeWitness)
    // Witness that `sub : mid`
    FIELD(RefPtr<SubtypeWitness>, subToMid);

    // Witness that `mid : sup`
    FIELD(DeclRef<Decl>, midToSup);
RAW(
    virtual bool EqualsVal(Val* val) override;
    virtual String ToString() override;
    virtual int GetHashCode() override;
    virtual RefPtr<Val> SubstituteImpl(SubstitutionSet subst, int * ioDiff) override;
)
END_SYNTAX_CLASS()

// A witness taht `sub : sup` because `sub` was wrapped into
// an existential of type `sup`.
SYNTAX_CLASS(ExtractExistentialSubtypeWitness, SubtypeWitness)
RAW(
    // The declaration of the existential value that has been opened
    DeclRef<VarDeclBase> declRef;

    virtual bool EqualsVal(Val* val) override;
    virtual String ToString() override;
    virtual int GetHashCode() override;
    virtual RefPtr<Val> SubstituteImpl(SubstitutionSet subst, int * ioDiff) override;
)
END_SYNTAX_CLASS()

