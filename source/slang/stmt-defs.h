// stmt-defs.h

// Syntax class definitions for statements.

ABSTRACT_SYNTAX_CLASS(ScopeStmt, StatementSyntaxNode)
    SYNTAX_FIELD(RefPtr<ScopeDecl>, scopeDecl)
END_SYNTAX_CLASS()

// A sequence of statements, treated as a single statement
SYNTAX_CLASS(SeqStmt, StatementSyntaxNode)
    SYNTAX_FIELD(List<RefPtr<StatementSyntaxNode>>, stmts)
END_SYNTAX_CLASS()

// The simplest kind of scope statement: just a `{...}` block
SYNTAX_CLASS(BlockStmt, ScopeStmt)
    SYNTAX_FIELD(RefPtr<StatementSyntaxNode>, body);
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(EmptyStatementSyntaxNode, StatementSyntaxNode)

SIMPLE_SYNTAX_CLASS(DiscardStatementSyntaxNode, StatementSyntaxNode)

SYNTAX_CLASS(VarDeclrStatementSyntaxNode, StatementSyntaxNode)
    SYNTAX_FIELD(RefPtr<DeclBase>, decl)
END_SYNTAX_CLASS()

SYNTAX_CLASS(IfStatementSyntaxNode, StatementSyntaxNode)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, Predicate)
    SYNTAX_FIELD(RefPtr<StatementSyntaxNode>, PositiveStatement)
    SYNTAX_FIELD(RefPtr<StatementSyntaxNode>, NegativeStatement)
END_SYNTAX_CLASS()

// A statement that can be escaped with a `break`
ABSTRACT_SYNTAX_CLASS(BreakableStmt, ScopeStmt)
END_SYNTAX_CLASS()

SYNTAX_CLASS(SwitchStmt, BreakableStmt)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, condition)
    SYNTAX_FIELD(RefPtr<StatementSyntaxNode>, body)
END_SYNTAX_CLASS()

// A statement that is expected to appear lexically nested inside
// some other construct, and thus needs to keep track of the
// outer statement that it is associated with...
ABSTRACT_SYNTAX_CLASS(ChildStmt, StatementSyntaxNode)
    DECL_FIELD(StatementSyntaxNode*, parentStmt RAW(= nullptr))
END_SYNTAX_CLASS()

// a `case` or `default` statement inside a `switch`
//
// Note(tfoley): A correct AST for a C-like language would treat
// these as a labelled statement, and so they would contain a
// sub-statement. I'm leaving that out for now for simplicity.
ABSTRACT_SYNTAX_CLASS(CaseStmtBase, ChildStmt)
END_SYNTAX_CLASS()

// a `case` statement inside a `switch`
SYNTAX_CLASS(CaseStmt, CaseStmtBase)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, expr)
END_SYNTAX_CLASS()

// a `default` statement inside a `switch`
SIMPLE_SYNTAX_CLASS(DefaultStmt, CaseStmtBase)

// A statement that represents a loop, and can thus be escaped with a `continue`
ABSTRACT_SYNTAX_CLASS(LoopStmt, BreakableStmt)
END_SYNTAX_CLASS()

SYNTAX_CLASS(ForStatementSyntaxNode, LoopStmt)
    SYNTAX_FIELD(RefPtr<StatementSyntaxNode>, InitialStatement)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, SideEffectExpression)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, PredicateExpression)
    SYNTAX_FIELD(RefPtr<StatementSyntaxNode>, Statement)
END_SYNTAX_CLASS()


SYNTAX_CLASS(WhileStatementSyntaxNode, LoopStmt)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, Predicate)
    SYNTAX_FIELD(RefPtr<StatementSyntaxNode>, Statement)
END_SYNTAX_CLASS()

SYNTAX_CLASS(DoWhileStatementSyntaxNode, LoopStmt)
    SYNTAX_FIELD(RefPtr<StatementSyntaxNode>, Statement)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, Predicate)
END_SYNTAX_CLASS()

// The case of child statements that do control flow relative
// to their parent statement.
ABSTRACT_SYNTAX_CLASS(JumpStmt, ChildStmt)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(BreakStatementSyntaxNode, JumpStmt)

SIMPLE_SYNTAX_CLASS(ContinueStatementSyntaxNode, JumpStmt)

SYNTAX_CLASS(ReturnStatementSyntaxNode, StatementSyntaxNode)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, Expression)
END_SYNTAX_CLASS()

SYNTAX_CLASS(ExpressionStatementSyntaxNode, StatementSyntaxNode)
    SYNTAX_FIELD(RefPtr<ExpressionSyntaxNode>, Expression)
END_SYNTAX_CLASS()
