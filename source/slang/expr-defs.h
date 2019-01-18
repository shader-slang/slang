// expr-defs.h

// Syntax class definitions for expressions.


// Base class for expressions that will reference declarations
ABSTRACT_SYNTAX_CLASS(DeclRefExpr, Expr)

// The scope in which to perform lookup
    FIELD(RefPtr<Scope>, scope)

    // The declaration of the symbol being referenced
    DECL_FIELD(DeclRef<Decl>, declRef)

    // The name of the symbol being referenced
    FIELD(Name*, name)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(VarExpr, DeclRefExpr)

// An expression that references an overloaded set of declarations
// having the same name.
SYNTAX_CLASS(OverloadedExpr, Expr)

    // Optional: the base expression is this overloaded result
    // arose from a member-reference expression.
    SYNTAX_FIELD(RefPtr<Expr>, base)

    // The lookup result that was ambiguous
    FIELD(LookupResult, lookupResult2)
END_SYNTAX_CLASS()

// An expression that references an overloaded set of declarations
// having the same name.
SYNTAX_CLASS(OverloadedExpr2, Expr)

    // Optional: the base expression is this overloaded result
    // arose from a member-reference expression.
    SYNTAX_FIELD(RefPtr<Expr>, base)

    // The lookup result that was ambiguous
    FIELD(List<RefPtr<Expr>>, candidiateExprs)
END_SYNTAX_CLASS()

ABSTRACT_SYNTAX_CLASS(LiteralExpr, Expr)
    // The token that was used to express the literal. This can be
    // used to get the raw text of the literal, including any suffix.
    FIELD(Token, token)
END_SYNTAX_CLASS()

SYNTAX_CLASS(IntegerLiteralExpr, LiteralExpr)
    FIELD(IntegerLiteralValue, value)
END_SYNTAX_CLASS()

SYNTAX_CLASS(FloatingPointLiteralExpr, LiteralExpr)
    FIELD(FloatingPointLiteralValue, value)
END_SYNTAX_CLASS()

SYNTAX_CLASS(BoolLiteralExpr, LiteralExpr)
    FIELD(bool, value)
END_SYNTAX_CLASS()

SYNTAX_CLASS(StringLiteralExpr, LiteralExpr)
    // TODO: consider storing the "segments" of the string
    // literal, in the case where multiple literals were
    //lined up at the lexer level, e.g.:
    //
    //      "first" "second" "third"
    //
    FIELD(String, value)
END_SYNTAX_CLASS()

// An initializer list, e.g. `{ 1, 2, 3 }`
SYNTAX_CLASS(InitializerListExpr, Expr)
    SYNTAX_FIELD(List<RefPtr<Expr>>, args)
END_SYNTAX_CLASS()

// A base class for expressions with arguments
ABSTRACT_SYNTAX_CLASS(ExprWithArgsBase, Expr)
    SYNTAX_FIELD(List<RefPtr<Expr>>, Arguments)
END_SYNTAX_CLASS()

// An aggregate type constructor
SYNTAX_CLASS(AggTypeCtorExpr, ExprWithArgsBase)
    SYNTAX_FIELD(TypeExp, base);
END_SYNTAX_CLASS()


// A base expression being applied to arguments: covers
// both ordinary `()` function calls and `<>` generic application
ABSTRACT_SYNTAX_CLASS(AppExprBase, ExprWithArgsBase)
    SYNTAX_FIELD(RefPtr<Expr>, FunctionExpr)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(InvokeExpr, AppExprBase)

SIMPLE_SYNTAX_CLASS(OperatorExpr, InvokeExpr)

SIMPLE_SYNTAX_CLASS(InfixExpr  , OperatorExpr)
SIMPLE_SYNTAX_CLASS(PrefixExpr , OperatorExpr)
SIMPLE_SYNTAX_CLASS(PostfixExpr, OperatorExpr)

SYNTAX_CLASS(IndexExpr, Expr)
    SYNTAX_FIELD(RefPtr<Expr>, BaseExpression)
    SYNTAX_FIELD(RefPtr<Expr>, IndexExpression)
END_SYNTAX_CLASS()

SYNTAX_CLASS(MemberExpr, DeclRefExpr)
    SYNTAX_FIELD(RefPtr<Expr>, BaseExpression)
END_SYNTAX_CLASS()

// Member looked up on a type, rather than a value
SYNTAX_CLASS(StaticMemberExpr, DeclRefExpr)
    SYNTAX_FIELD(RefPtr<Expr>, BaseExpression)
END_SYNTAX_CLASS()

SYNTAX_CLASS(SwizzleExpr, Expr)
    SYNTAX_FIELD(RefPtr<Expr>, base)
    FIELD(int, elementCount)
    FIELD(int, elementIndices[4])
END_SYNTAX_CLASS()

// A dereference of a pointer or pointer-like type
SYNTAX_CLASS(DerefExpr, Expr)
    SYNTAX_FIELD(RefPtr<Expr>, base)
END_SYNTAX_CLASS()

// Any operation that performs type-casting
SYNTAX_CLASS(TypeCastExpr, InvokeExpr)
//    SYNTAX_FIELD(TypeExp, TargetType)
//    SYNTAX_FIELD(RefPtr<Expr>, Expression)
END_SYNTAX_CLASS()

// An explicit type-cast that appear in the user's code with `(type) expr` syntax
SYNTAX_CLASS(ExplicitCastExpr, TypeCastExpr)
END_SYNTAX_CLASS()

// An implicit type-cast inserted during semantic checking
SYNTAX_CLASS(ImplicitCastExpr, TypeCastExpr)
END_SYNTAX_CLASS()

    /// A cast from a value to an interface ("existential") type.
SYNTAX_CLASS(CastToInterfaceExpr, Expr)
RAW(
        /// The value being cast to an interface type
    RefPtr<Expr>    valueArg;

        /// A witness showing that `valueArg` conforms to the chosen interface
    RefPtr<Val>     witnessArg;
)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(SelectExpr, OperatorExpr)

SIMPLE_SYNTAX_CLASS(GenericAppExpr, AppExprBase)

// An expression representing re-use of the syntax for a type in more
// than once conceptually-distinct declaration
SYNTAX_CLASS(SharedTypeExpr, Expr)
    // The underlying type expression that we want to share
    SYNTAX_FIELD(TypeExp, base)
END_SYNTAX_CLASS()

SYNTAX_CLASS(AssignExpr, Expr)
    SYNTAX_FIELD(RefPtr<Expr>, left);
    SYNTAX_FIELD(RefPtr<Expr>, right);
END_SYNTAX_CLASS()

// Just an expression inside parentheses `(exp)`
//
// We keep this around explicitly to be sure we don't lose any structure
// when we do rewriter stuff.
SYNTAX_CLASS(ParenExpr, Expr)
    SYNTAX_FIELD(RefPtr<Expr>, base);
END_SYNTAX_CLASS()

// An object-oriented `this` expression, used to
// refer to the current instance of an enclosing type.
SYNTAX_CLASS(ThisExpr, Expr)
    FIELD(RefPtr<Scope>, scope);
END_SYNTAX_CLASS()

// An expression that binds a temporary variable in a local expression context
SYNTAX_CLASS(LetExpr, Expr)
RAW(
    RefPtr<VarDecl> decl;
    RefPtr<Expr> body;
)
END_SYNTAX_CLASS()

SYNTAX_CLASS(ExtractExistentialValueExpr, Expr)
RAW(
    DeclRef<VarDeclBase> declRef;
)
END_SYNTAX_CLASS()

    /// A type expression of the form `__TaggedUnion(A, ...)`.
    ///
    /// An expression of this form will resolve to a `TaggedUnionType`
    /// when checked.
    ///
SYNTAX_CLASS(TaggedUnionTypeExpr, Expr)
RAW(
    List<TypeExp> caseTypes;
)
END_SYNTAX_CLASS()