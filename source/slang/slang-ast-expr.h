// slang-ast-expr.h

#pragma once

#include "slang-ast-base.h"

namespace Slang {

// Syntax class definitions for expressions.

// Base class for expressions that will reference declarations
class DeclRefExpr: public Expr
{
    SLANG_ABSTRACT_CLASS(DeclRefExpr)

    // The scope in which to perform lookup
    RefPtr<Scope> scope;

    // The declaration of the symbol being referenced
    DeclRef<Decl> declRef;

    // The name of the symbol being referenced
    Name* name;
};

class VarExpr : public DeclRefExpr
{
    SLANG_CLASS(VarExpr)
};

// An expression that references an overloaded set of declarations
// having the same name.
class OverloadedExpr : public Expr
{
    SLANG_CLASS(OverloadedExpr)

    // The name that was looked up and found to be overloaded
    Name* name;

    // Optional: the base expression is this overloaded result
    // arose from a member-reference expression.
    RefPtr<Expr> base;

    // The lookup result that was ambiguous
    LookupResult lookupResult2;
};

// An expression that references an overloaded set of declarations
// having the same name.
class OverloadedExpr2: public Expr
{
    SLANG_CLASS(OverloadedExpr2)

    // Optional: the base expression is this overloaded result
    // arose from a member-reference expression.
    RefPtr<Expr> base;

    // The lookup result that was ambiguous
    List<RefPtr<Expr>> candidiateExprs;
};

class LiteralExpr : public Expr
{
    SLANG_ABSTRACT_CLASS(LiteralExpr)
    // The token that was used to express the literal. This can be
    // used to get the raw text of the literal, including any suffix.
    Token token;
};

class IntegerLiteralExpr : public LiteralExpr
{
    SLANG_CLASS(IntegerLiteralExpr)

    IntegerLiteralValue value;
};

class FloatingPointLiteralExpr: public LiteralExpr
{
    SLANG_CLASS(FloatingPointLiteralExpr)
    FloatingPointLiteralValue value;
};

class BoolLiteralExpr : public LiteralExpr
{
    SLANG_CLASS(BoolLiteralExpr)
    bool value;
};

class StringLiteralExpr : public LiteralExpr
{
    SLANG_CLASS(StringLiteralExpr)

    // TODO: consider storing the "segments" of the string
    // literal, in the case where multiple literals were
    //lined up at the lexer level, e.g.:
    //
    //      "first" "second" "third"
    //
    String value;
};

// An initializer list, e.g. `{ 1, 2, 3 }`
class InitializerListExpr : public Expr
{
    SLANG_CLASS(InitializerListExpr)
    List<RefPtr<Expr>> args;
};

// A base class for expressions with arguments
class ExprWithArgsBase : public Expr
{
    SLANG_ABSTRACT_CLASS(ExprWithArgsBase)

    List<RefPtr<Expr>> arguments;
};

// An aggregate type constructor
class AggTypeCtorExpr : public ExprWithArgsBase
{
    SLANG_CLASS(AggTypeCtorExpr)

    TypeExp base;
};


// A base expression being applied to arguments: covers
// both ordinary `()` function calls and `<>` generic application
class AppExprBase : public ExprWithArgsBase
{
    SLANG_ABSTRACT_CLASS(AppExprBase)

    RefPtr<Expr> functionExpr;
};

class InvokeExpr: public AppExprBase
{
    SLANG_CLASS(InvokeExpr)
};

class OperatorExpr: public InvokeExpr
{
    SLANG_CLASS(OperatorExpr)
};

class InfixExpr: public OperatorExpr
{
    SLANG_CLASS(InfixExpr)
};
class PrefixExpr: public OperatorExpr
{
    SLANG_CLASS(PrefixExpr)
};
class PostfixExpr: public OperatorExpr
{
    SLANG_CLASS(PostfixExpr)
};

class IndexExpr: public Expr
{
    SLANG_CLASS(IndexExpr)

    RefPtr<Expr> baseExpression;
    RefPtr<Expr> indexExpression;
};

class MemberExpr: public DeclRefExpr
{
    SLANG_CLASS(MemberExpr)
    RefPtr<Expr> baseExpression;
};

// Member looked up on a type, rather than a value
class StaticMemberExpr: public DeclRefExpr
{
    SLANG_CLASS(StaticMemberExpr)
    RefPtr<Expr> baseExpression;
};

struct MatrixCoord
{
    bool operator==(const MatrixCoord& rhs) const { return row == rhs.row && col == rhs.col; };
    bool operator!=(const MatrixCoord& rhs) const { return !(*this == rhs); };
    // Rows and columns are zero indexed 
    int row;
    int col;
};

class MatrixSwizzleExpr : public Expr
{
    SLANG_CLASS(MatrixSwizzleExpr)
    RefPtr<Expr> base;
    int elementCount;
    MatrixCoord elementCoords[4];
};

class SwizzleExpr: public Expr
{
    SLANG_CLASS(SwizzleExpr)
    RefPtr<Expr> base;
    int elementCount;
    int elementIndices[4];
};

// A dereference of a pointer or pointer-like type
class DerefExpr: public Expr
{
    SLANG_CLASS(DerefExpr)
    RefPtr<Expr> base;
};

// Any operation that performs type-casting
class TypeCastExpr: public InvokeExpr
{
    SLANG_CLASS(TypeCastExpr)
//    TypeExp TargetType;
//    RefPtr<Expr> Expression;
};

// An explicit type-cast that appear in the user's code with `(type) expr` syntax
class ExplicitCastExpr: public TypeCastExpr
{
    SLANG_CLASS(ExplicitCastExpr)
};

// An implicit type-cast inserted during semantic checking
class ImplicitCastExpr : public TypeCastExpr
{
    SLANG_CLASS(ImplicitCastExpr)
};

    /// A cast from a value to an interface ("existential") type.
class CastToInterfaceExpr: public Expr
{
    SLANG_CLASS(CastToInterfaceExpr)

        /// The value being cast to an interface type
    RefPtr<Expr>    valueArg;

        /// A witness showing that `valueArg` conforms to the chosen interface
    RefPtr<Val>     witnessArg;
};

class SelectExpr: public OperatorExpr
{
    SLANG_CLASS(SelectExpr)
};

class GenericAppExpr: public AppExprBase
{
    SLANG_CLASS(GenericAppExpr)
};

// An expression representing re-use of the syntax for a type in more
// than once conceptually-distinct declaration
class SharedTypeExpr: public Expr
{
    SLANG_CLASS(SharedTypeExpr)
    // The underlying type expression that we want to share
    TypeExp base;
};

class AssignExpr: public Expr
{
    SLANG_CLASS(AssignExpr)
    RefPtr<Expr> left;
    RefPtr<Expr> right;
};

// Just an expression inside parentheses `(exp)`
//
// We keep this around explicitly to be sure we don't lose any structure
// when we do rewriter stuff.
class ParenExpr: public Expr
{
    SLANG_CLASS(ParenExpr)
    RefPtr<Expr> base;
};

// An object-oriented `this` expression, used to
// refer to the current instance of an enclosing type.
class ThisExpr: public Expr
{
    SLANG_CLASS(ThisExpr)
    RefPtr<Scope> scope;
};

// An expression that binds a temporary variable in a local expression context
class LetExpr: public Expr
{
    SLANG_CLASS(LetExpr)
    RefPtr<VarDecl> decl;
    RefPtr<Expr> body;
};

class ExtractExistentialValueExpr: public Expr
{
    SLANG_CLASS(ExtractExistentialValueExpr)
    DeclRef<VarDeclBase> declRef;
};

    /// A type expression of the form `__TaggedUnion(A, ...)`.
    ///
    /// An expression of this form will resolve to a `TaggedUnionType`
    /// when checked.
    ///
class TaggedUnionTypeExpr: public Expr
{
    SLANG_CLASS(TaggedUnionTypeExpr)
    List<TypeExp> caseTypes;
};

    /// A type expression of the form `This`
    ///
    /// Refers to the type of `this` in the current context.
    ///
class ThisTypeExpr: public Expr
{
    SLANG_CLASS(ThisTypeExpr)
    RefPtr<Scope> scope;
};

} // namespace Slang
