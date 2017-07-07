// expr-defs.h

// Syntax class definitions for expressions.


// Base class for expressions that will reference declarations
ABSTRACT_SYNTAX_CLASS(DeclRefExpr, ExpressionSyntaxNode)

// The scope in which to perform lookup
    FIELD(RefPtr<Scope>, scope)

    // The declaration of the symbol being referenced
    DECL_FIELD(DeclRef<Decl>, declRef)

    // The name of the symbol being referenced
    FIELD(String, name)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(VarExpressionSyntaxNode, DeclRefExpr)

// An expression that references an overloaded set of declarations
// having the same name.
SYNTAX_CLASS(OverloadedExpr, ExpressionSyntaxNode)

    // Optional: the base expression is this overloaded result
    // arose from a member-reference expression.
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, base)

    // The lookup result that was ambiguous
    FIELD(LookupResult, lookupResult2)
END_SYNTAX_CLASS()

SYNTAX_CLASS(ConstantExpressionSyntaxNode, ExpressionSyntaxNode)
    FIELD(Token, token)

    RAW(
    enum class ConstantType
    {
        Int,
        Bool,
        Float,
        String,
    };
    ConstantType ConstType;
    union
    {
        IntegerLiteralValue         integerValue;
        FloatingPointLiteralValue   floatingPointValue;
    };
    String stringValue;
    )
END_SYNTAX_CLASS()

// An initializer list, e.g. `{ 1, 2, 3 }`
SYNTAX_CLASS(InitializerListExpr, ExpressionSyntaxNode)
    SYNTAX_FIELD(List<RefPtr<ExpressionSyntaxNode>>, args)
END_SYNTAX_CLASS()

// A base expression being applied to arguments: covers
// both ordinary `()` function calls and `<>` generic application
ABSTRACT_SYNTAX_CLASS(AppExprBase, ExpressionSyntaxNode)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, FunctionExpr)
    SYNTAX_FIELD(List<RefPtr<ExpressionSyntaxNode>>, Arguments)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(InvokeExpressionSyntaxNode, AppExprBase)

SIMPLE_SYNTAX_CLASS(OperatorExpressionSyntaxNode, InvokeExpressionSyntaxNode)

SIMPLE_SYNTAX_CLASS(InfixExpr  , OperatorExpressionSyntaxNode)
SIMPLE_SYNTAX_CLASS(PrefixExpr , OperatorExpressionSyntaxNode)
SIMPLE_SYNTAX_CLASS(PostfixExpr, OperatorExpressionSyntaxNode)

SYNTAX_CLASS(IndexExpressionSyntaxNode, ExpressionSyntaxNode)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, BaseExpression)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, IndexExpression)
END_SYNTAX_CLASS()

SYNTAX_CLASS(MemberExpressionSyntaxNode, DeclRefExpr)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, BaseExpression)
END_SYNTAX_CLASS()

SYNTAX_CLASS(SwizzleExpr, ExpressionSyntaxNode)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, base)
    FIELD(int, elementCount)
    FIELD(int, elementIndices[4])
END_SYNTAX_CLASS()

// A dereference of a pointer or pointer-like type
SYNTAX_CLASS(DerefExpr, ExpressionSyntaxNode)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, base)
END_SYNTAX_CLASS()

SYNTAX_CLASS(TypeCastExpressionSyntaxNode, ExpressionSyntaxNode)
    SYNTAX_FIELD(TypeExp, TargetType)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, Expression)
END_SYNTAX_CLASS()

SYNTAX_CLASS(ImplicitCastExpr, TypeCastExpressionSyntaxNode)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(SelectExpressionSyntaxNode, OperatorExpressionSyntaxNode)

SIMPLE_SYNTAX_CLASS(GenericAppExpr, AppExprBase)

// An expression representing re-use of the syntax for a type in more
// than once conceptually-distinct declaration
SYNTAX_CLASS(SharedTypeExpr, ExpressionSyntaxNode)
    // The underlying type expression that we want to share
    SYNTAX_FIELD(TypeExp, base)
END_SYNTAX_CLASS()

SYNTAX_CLASS(AssignExpr, ExpressionSyntaxNode)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, left);
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, right);
END_SYNTAX_CLASS()

// Just an expression inside parentheses `(exp)`
//
// We keep this around explicitly to be sure we don't lose any structure
// when we do rewriter stuff.
SYNTAX_CLASS(ParenExpr, ExpressionSyntaxNode)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, base);
END_SYNTAX_CLASS()
