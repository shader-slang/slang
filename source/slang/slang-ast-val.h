// slang-ast-val.h

#pragma once

#include "slang-ast-base.h"

namespace Slang {

// Syntax class definitions for compile-time values.

// A compile-time integer (may not have a specific concrete value)
class IntVal : public Val 
{
    SLANG_ABSTRACT_AST_CLASS(IntVal)
};

// Trivial case of a value that is just a constant integer
class ConstantIntVal : public IntVal 
{
    SLANG_AST_CLASS(ConstantIntVal)

    IntegerLiteralValue value;

    // Overrides should be public so base classes can access
    bool _equalsValOverride(Val* val);
    void _toTextOverride(StringBuilder& out);
    HashCode _getHashCodeOverride();

protected:
    ConstantIntVal(IntegerLiteralValue inValue)
        : value(inValue)
    {}

};

// The logical "value" of a reference to a generic value parameter
class GenericParamIntVal : public IntVal 
{
    SLANG_AST_CLASS(GenericParamIntVal)

    DeclRef<VarDeclBase> declRef;

    // Overrides should be public so base classes can access
    bool _equalsValOverride(Val* val);
    void _toTextOverride(StringBuilder& out);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);

protected:
    GenericParamIntVal(DeclRef<VarDeclBase> inDeclRef)
        : declRef(inDeclRef)
    {}
};

// An compile time int val as result of some general computation.
class FuncCallIntVal : public IntVal
{
    SLANG_AST_CLASS(FuncCallIntVal)

    bool _equalsValOverride(Val* val);
    void _toTextOverride(StringBuilder& out);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);

    DeclRef<Decl> funcDeclRef;
    Type* funcType;
    List<IntVal*> args;

    static Val* tryFoldImpl(ASTBuilder* astBuilder, DeclRef<Decl> newFuncDecl, List<IntVal*>& newArgs, DiagnosticSink* sink);
};

class WitnessLookupIntVal : public IntVal
{
    SLANG_AST_CLASS(WitnessLookupIntVal)

    bool _equalsValOverride(Val* val);
    void _toTextOverride(StringBuilder& out);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);

    SubtypeWitness* witness;
    Decl* key;
    Type* type;

    static Val* tryFoldOrNull(ASTBuilder* astBuilder, SubtypeWitness* witness, Decl* key);

    static Val* tryFold(ASTBuilder* astBuilder, SubtypeWitness* witness, Decl* key, Type* type);
};

// polynomial expression "2*a*b^3 + 1" will be represented as:
// { constantTerm:1, terms: [ { constFactor:2, paramFactors:[{"a", 1}, {"b", 3}] } ] }
class PolynomialIntValFactor : public NodeBase
{
    SLANG_AST_CLASS(PolynomialIntValFactor)
public:
    IntVal* param;
    IntegerLiteralValue power;
    // for sorting only.
    bool operator<(const PolynomialIntValFactor& other) const
    {
        if (auto thisGenParam = as<GenericParamIntVal>(param))
        {
            if (auto thatGenParam = as<GenericParamIntVal>(other.param))
            {
                if (thisGenParam->equalsVal(thatGenParam))
                    return power < other.power;
                else
                    return thisGenParam->declRef.decl < thatGenParam->declRef.decl;
            }
            else
            {
                return true;
            }
        }
        else
        {
            if (auto thatGenParam = as<GenericParamIntVal>(other.param))
            {
                return false;
            }
            return param == other.param ? power < other.power : param < other.param;
        }
        
    }
    // for sorting only.
    bool operator==(const PolynomialIntValFactor& other) const
    {
        if (auto thisGenParam = as<GenericParamIntVal>(param))
        {
            if (auto thatGenParam = as<GenericParamIntVal>(other.param))
            {
                if (param->equalsVal(other.param) && power == other.power)
                    return true;
            }
            return false;
        }
        return power == other.power && param == other.param;
    }
    bool equals(const PolynomialIntValFactor& other) const
    {
        return power == other.power && param->equalsVal(other.param);
    }
};
class PolynomialIntValTerm : public NodeBase
{
    SLANG_AST_CLASS(PolynomialIntValTerm)
public:
    IntegerLiteralValue constFactor;
    List<PolynomialIntValFactor*> paramFactors;
    bool canCombineWith(const PolynomialIntValTerm& other) const
    {
        if (paramFactors.getCount() != other.paramFactors.getCount())
            return false;
        for (Index i = 0; i < paramFactors.getCount(); i++)
        {
            if (!paramFactors[i]->equals(*other.paramFactors[i]))
                return false;
        }
        return true;
    }
    bool operator<(const PolynomialIntValTerm& other) const
    {
        if (constFactor < other.constFactor)
            return true;
        else if (constFactor == other.constFactor)
        {
            for (Index i = 0; i < paramFactors.getCount(); i++)
            {
                if (i >= other.paramFactors.getCount())
                    return false;
                if (*(paramFactors[i]) < *(other.paramFactors[i]))
                    return true;
                if (*(paramFactors[i]) == *(other.paramFactors[i]))
                {
                }
                else
                {
                    return false;
                }
            }
        }
        return false;
    }
};

class PolynomialIntVal : public IntVal
{
    SLANG_AST_CLASS(PolynomialIntVal)
public:
    
    List<PolynomialIntValTerm*> terms;
    IntegerLiteralValue constantTerm = 0;

    bool isConstant() { return terms.getCount() == 0; }
    // Canonicalize the polynomial. If the polynomial can be simplified to a constant or a genericparam,
    // the method returns the value simplified to.
    // Otherwise, in-place modifications are performed and returns this.
    IntVal* canonicalize(ASTBuilder* builder);

    // Overrides should be public so base classes can access
    bool _equalsValOverride(Val* val);
    void _toTextOverride(StringBuilder& out);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);

    static PolynomialIntVal* neg(ASTBuilder* astBuilder, IntVal* base);
    static PolynomialIntVal* add(ASTBuilder* astBuilder, IntVal* op0, IntVal* op1);
    static PolynomialIntVal* sub(ASTBuilder* astBuilder, IntVal* op0, IntVal* op1);
    static PolynomialIntVal* mul(ASTBuilder* astBuilder, IntVal* op0, IntVal* op1);

};

    /// An unknown integer value indicating an erroneous sub-expression
class ErrorIntVal : public IntVal 
{
    SLANG_AST_CLASS(ErrorIntVal)

    // TODO: We should probably eventually just have an `ErrorVal` here
    // and have all `Val`s that represent ordinary values hold their
    // `Type` so that we can have an `ErrorVal` of any type.

    // Overrides should be public so base classes can access
    bool _equalsValOverride(Val* val);
    void _toTextOverride(StringBuilder& out);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

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
class Witness : public Val 
{
    SLANG_ABSTRACT_AST_CLASS(Witness)
};

// A witness that one type is a subtype of another
// (where by "subtype" we include both inheritance
// relationships and type-conforms-to-interface relationships)
//
// TODO: we may need to tease those apart.
class SubtypeWitness : public Witness 
{
    SLANG_ABSTRACT_AST_CLASS(SubtypeWitness)

    Type* sub = nullptr;
    Type* sup = nullptr;
};

class TypeEqualityWitness : public SubtypeWitness 
{
    SLANG_AST_CLASS(TypeEqualityWitness)

    // Overrides should be public so base classes can access
    bool _equalsValOverride(Val* val);
    void _toTextOverride(StringBuilder& out);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

// A witness that one type is a subtype of another
// because some in-scope declaration says so
class DeclaredSubtypeWitness : public SubtypeWitness 
{
    SLANG_AST_CLASS(DeclaredSubtypeWitness)

    DeclRef<Decl> declRef;

    // Overrides should be public so base classes can access
    bool _equalsValOverride(Val* val);
    void _toTextOverride(StringBuilder& out);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

// A witness that `sub : sup` because `sub : mid` and `mid : sup`
class TransitiveSubtypeWitness : public SubtypeWitness 
{
    SLANG_AST_CLASS(TransitiveSubtypeWitness)

    // Witness that `sub : mid`
    SubtypeWitness* subToMid = nullptr;

    // Witness that `mid : sup`
    SubtypeWitness* midToSup = nullptr;

    // Overrides should be public so base classes can access
    bool _equalsValOverride(Val* val);
    void _toTextOverride(StringBuilder& out);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

// A witness taht `sub : sup` because `sub` was wrapped into
// an existential of type `sup`.
class ExtractExistentialSubtypeWitness : public SubtypeWitness 
{
    SLANG_AST_CLASS(ExtractExistentialSubtypeWitness)

    // The declaration of the existential value that has been opened
    DeclRef<VarDeclBase> declRef;

    // Overrides should be public so base classes can access
    bool _equalsValOverride(Val* val);
    void _toTextOverride(StringBuilder& out);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

// A witness that `sub : sup`, because `sub` is a tagged union
// of the form `A | B | C | ...` and each of `A : sup`,
// `B : sup`, `C : sup`, etc.
//
class TaggedUnionSubtypeWitness : public SubtypeWitness 
{
    SLANG_AST_CLASS(TaggedUnionSubtypeWitness)

    // Witnesses that each of the "case" types in the union
    // is a subtype of `sup`.
    //
    List<Val*> caseWitnesses;


    // Overrides should be public so base classes can access
    bool _equalsValOverride(Val* val);
    void _toTextOverride(StringBuilder& out);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

    /// A witness of the fact that `ThisType(someInterface) : someInterface`
class ThisTypeSubtypeWitness : public SubtypeWitness
{
    SLANG_AST_CLASS(ThisTypeSubtypeWitness)
};

    /// A witness of the fact that a user provided "__Dynamic" type argument is a
    /// subtype to the existential type parameter.
class DynamicSubtypeWitness : public SubtypeWitness
{
    SLANG_AST_CLASS(DynamicSubtypeWitness)
};

    /// A witness that `T : L & R` because `T : L` and `T : R`
class ConjunctionSubtypeWitness : public SubtypeWitness
{
    SLANG_AST_CLASS(ConjunctionSubtypeWitness)

        /// Witness that `sub : sup->left`
    Val* leftWitness;

        /// Witness that `sub : sup->right`
    Val* rightWitness;
};

    /// A witness that `T : X` because `T : X & Y` or `T : Y & X`
class ExtractFromConjunctionSubtypeWitness : public SubtypeWitness
{
    SLANG_AST_CLASS(ExtractFromConjunctionSubtypeWitness)

        /// Witness that `T : L & R` for some `R`
    Val* conunctionWitness;

        /// The zero-based index of the super-type we care about in the conjunction
        ///
        /// If `conunctionWitness` is `T : X & Y` then this index should be zero if
        /// we want to represent `T : X` and one if we want `T : Y`.
        ///
    int indexInConjunction;
};

    /// A value that represents a modifier attached to some other value
class ModifierVal : public Val
{
    SLANG_AST_CLASS(ModifierVal)

    bool _equalsValOverride(Val* val);
    HashCode _getHashCodeOverride();
};

class TypeModifierVal : public ModifierVal
{
    SLANG_AST_CLASS(TypeModifierVal)
};

class ResourceFormatModifierVal : public TypeModifierVal
{
    SLANG_AST_CLASS(ResourceFormatModifierVal)
};

class UNormModifierVal : public ResourceFormatModifierVal
{
    SLANG_AST_CLASS(UNormModifierVal)

    void _toTextOverride(StringBuilder& out);
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

class SNormModifierVal : public ResourceFormatModifierVal
{
    SLANG_AST_CLASS(SNormModifierVal)

    void _toTextOverride(StringBuilder& out);
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

} // namespace Slang
