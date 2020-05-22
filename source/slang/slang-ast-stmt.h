// slang-ast-stmt.h

#pragma once

#include "slang-ast-base.h"

namespace Slang {

// Syntax class definitions for statements.

class ScopeStmt : public Stmt 
{
    SLANG_ABSTRACT_CLASS(ScopeStmt)

    RefPtr<ScopeDecl> scopeDecl;
};

// A sequence of statements, treated as a single statement
class SeqStmt : public Stmt 
{
    SLANG_CLASS(SeqStmt)

    List<RefPtr<Stmt>> stmts;
};

// The simplest kind of scope statement: just a `{...}` block
class BlockStmt : public ScopeStmt 
{
    SLANG_CLASS(BlockStmt)

    RefPtr<Stmt> body;
};

// A statement that we aren't going to parse or check, because
// we want to let a downstream compiler handle any issues
class UnparsedStmt : public Stmt 
{
    SLANG_CLASS(UnparsedStmt)

    // The tokens that were contained between `{` and `}`
    List<Token> tokens;
};

class EmptyStmt : public Stmt 
{
    SLANG_CLASS(EmptyStmt)
};

class DiscardStmt : public Stmt 
{
    SLANG_CLASS(DiscardStmt)
};

class DeclStmt : public Stmt 
{
    SLANG_CLASS(DeclStmt)

    RefPtr<DeclBase> decl;
};

class IfStmt : public Stmt 
{
    SLANG_CLASS(IfStmt)

    RefPtr<Expr> predicate;
    RefPtr<Stmt> positiveStatement;
    RefPtr<Stmt> negativeStatement;
};

// A statement that can be escaped with a `break`
class BreakableStmt : public ScopeStmt 
{
    SLANG_ABSTRACT_CLASS(BreakableStmt)

};

class SwitchStmt : public BreakableStmt 
{
    SLANG_CLASS(SwitchStmt)

    RefPtr<Expr> condition;
    RefPtr<Stmt> body;
};

// A statement that is expected to appear lexically nested inside
// some other construct, and thus needs to keep track of the
// outer statement that it is associated with...
class ChildStmt : public Stmt 
{
    SLANG_ABSTRACT_CLASS(ChildStmt)

    Stmt* parentStmt = nullptr;
};

// a `case` or `default` statement inside a `switch`
//
// Note(tfoley): A correct AST for a C-like language would treat
// these as a labelled statement, and so they would contain a
// sub-statement. I'm leaving that out for now for simplicity.
class CaseStmtBase : public ChildStmt 
{
    SLANG_ABSTRACT_CLASS(CaseStmtBase)

};

// a `case` statement inside a `switch`
class CaseStmt : public CaseStmtBase 
{
    SLANG_CLASS(CaseStmt)

    RefPtr<Expr> expr;
};

// a `default` statement inside a `switch`
class DefaultStmt : public CaseStmtBase 
{
    SLANG_CLASS(DefaultStmt)
};

// A statement that represents a loop, and can thus be escaped with a `continue`
class LoopStmt : public BreakableStmt 
{
    SLANG_ABSTRACT_CLASS(LoopStmt)

};

// A `for` statement
class ForStmt : public LoopStmt 
{
    SLANG_CLASS(ForStmt)

    RefPtr<Stmt> initialStatement;
    RefPtr<Expr> sideEffectExpression;
    RefPtr<Expr> predicateExpression;
    RefPtr<Stmt> statement;
};

// A `for` statement in a language that doesn't restrict the scope
// of the loop variable to the body.
class UnscopedForStmt : public ForStmt 
{
    SLANG_CLASS(UnscopedForStmt)
;
};

class WhileStmt : public LoopStmt 
{
    SLANG_CLASS(WhileStmt)

    RefPtr<Expr> predicate;
    RefPtr<Stmt> statement;
};

class DoWhileStmt : public LoopStmt 
{
    SLANG_CLASS(DoWhileStmt)

    RefPtr<Stmt> statement;
    RefPtr<Expr> predicate;
};

// A compile-time, range-based `for` loop, which will not appear in the output code
class CompileTimeForStmt : public ScopeStmt 
{
    SLANG_CLASS(CompileTimeForStmt)

    RefPtr<VarDecl> varDecl;
    RefPtr<Expr> rangeBeginExpr;
    RefPtr<Expr> rangeEndExpr;
    RefPtr<Stmt> body;
    RefPtr<IntVal> rangeBeginVal;
    RefPtr<IntVal> rangeEndVal;
};

// The case of child statements that do control flow relative
// to their parent statement.
class JumpStmt : public ChildStmt 
{
    SLANG_ABSTRACT_CLASS(JumpStmt)

};

class BreakStmt : public JumpStmt 
{
    SLANG_CLASS(BreakStmt)
};

class ContinueStmt : public JumpStmt 
{
    SLANG_CLASS(ContinueStmt)
};

class ReturnStmt : public Stmt 
{
    SLANG_CLASS(ReturnStmt)

    RefPtr<Expr> expression;
};

class ExpressionStmt : public Stmt 
{
    SLANG_CLASS(ExpressionStmt)

    RefPtr<Expr> expression;
};

} // namespace Slang
