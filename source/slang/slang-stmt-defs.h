// slang-stmt-defs.h

// Syntax class definitions for statements.

ABSTRACT_SYNTAX_CLASS(ScopeStmt, Stmt)
    SYNTAX_FIELD(RefPtr<ScopeDecl>, scopeDecl)
END_SYNTAX_CLASS()

// A sequence of statements, treated as a single statement
SYNTAX_CLASS(SeqStmt, Stmt)
    SYNTAX_FIELD(List<RefPtr<Stmt>>, stmts)
END_SYNTAX_CLASS()

// The simplest kind of scope statement: just a `{...}` block
SYNTAX_CLASS(BlockStmt, ScopeStmt)
    SYNTAX_FIELD(RefPtr<Stmt>, body);
END_SYNTAX_CLASS()

// A statement that we aren't going to parse or check, because
// we want to let a downstream compiler handle any issues
SYNTAX_CLASS(UnparsedStmt, Stmt)
    // The tokens that were contained between `{` and `}`
    FIELD(List<Token>, tokens)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(EmptyStmt, Stmt)

SIMPLE_SYNTAX_CLASS(DiscardStmt, Stmt)

SYNTAX_CLASS(DeclStmt, Stmt)
    SYNTAX_FIELD(RefPtr<DeclBase>, decl)
END_SYNTAX_CLASS()

SYNTAX_CLASS(IfStmt, Stmt)
    SYNTAX_FIELD(RefPtr<Expr>, Predicate)
    SYNTAX_FIELD(RefPtr<Stmt>, PositiveStatement)
    SYNTAX_FIELD(RefPtr<Stmt>, NegativeStatement)
END_SYNTAX_CLASS()

// A statement that can be escaped with a `break`
ABSTRACT_SYNTAX_CLASS(BreakableStmt, ScopeStmt)
END_SYNTAX_CLASS()

SYNTAX_CLASS(SwitchStmt, BreakableStmt)
    SYNTAX_FIELD(RefPtr<Expr>, condition)
    SYNTAX_FIELD(RefPtr<Stmt>, body)
END_SYNTAX_CLASS()

// A statement that is expected to appear lexically nested inside
// some other construct, and thus needs to keep track of the
// outer statement that it is associated with...
ABSTRACT_SYNTAX_CLASS(ChildStmt, Stmt)
    DECL_FIELD(Stmt*, parentStmt RAW(= nullptr))
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
    SYNTAX_FIELD(RefPtr<Expr>, expr)
END_SYNTAX_CLASS()

// a `default` statement inside a `switch`
SIMPLE_SYNTAX_CLASS(DefaultStmt, CaseStmtBase)

// A statement that represents a loop, and can thus be escaped with a `continue`
ABSTRACT_SYNTAX_CLASS(LoopStmt, BreakableStmt)
END_SYNTAX_CLASS()

// A `for` statement
SYNTAX_CLASS(ForStmt, LoopStmt)
    SYNTAX_FIELD(RefPtr<Stmt>, InitialStatement)
    SYNTAX_FIELD(RefPtr<Expr>, SideEffectExpression)
    SYNTAX_FIELD(RefPtr<Expr>, PredicateExpression)
    SYNTAX_FIELD(RefPtr<Stmt>, Statement)
END_SYNTAX_CLASS()

// A `for` statement in a language that doesn't restrict the scope
// of the loop variable to the body.
SYNTAX_CLASS(UnscopedForStmt, ForStmt);
END_SYNTAX_CLASS()

SYNTAX_CLASS(WhileStmt, LoopStmt)
    SYNTAX_FIELD(RefPtr<Expr>, Predicate)
    SYNTAX_FIELD(RefPtr<Stmt>, Statement)
END_SYNTAX_CLASS()

SYNTAX_CLASS(DoWhileStmt, LoopStmt)
    SYNTAX_FIELD(RefPtr<Stmt>, Statement)
    SYNTAX_FIELD(RefPtr<Expr>, Predicate)
END_SYNTAX_CLASS()

// A compile-time, range-based `for` loop, which will not appear in the output code
SYNTAX_CLASS(CompileTimeForStmt, ScopeStmt)
    SYNTAX_FIELD(RefPtr<VarDecl>, varDecl)
    SYNTAX_FIELD(RefPtr<Expr>, rangeBeginExpr)
    SYNTAX_FIELD(RefPtr<Expr>, rangeEndExpr)
    SYNTAX_FIELD(RefPtr<Stmt>, body)
    SYNTAX_FIELD(RefPtr<IntVal>, rangeBeginVal)
    SYNTAX_FIELD(RefPtr<IntVal>, rangeEndVal)
END_SYNTAX_CLASS()

// The case of child statements that do control flow relative
// to their parent statement.
ABSTRACT_SYNTAX_CLASS(JumpStmt, ChildStmt)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(BreakStmt, JumpStmt)

SIMPLE_SYNTAX_CLASS(ContinueStmt, JumpStmt)

SYNTAX_CLASS(ReturnStmt, Stmt)
    SYNTAX_FIELD(RefPtr<Expr>, Expression)
END_SYNTAX_CLASS()

SYNTAX_CLASS(ExpressionStmt, Stmt)
    SYNTAX_FIELD(RefPtr<Expr>, Expression)
END_SYNTAX_CLASS()
