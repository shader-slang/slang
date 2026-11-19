// slang-ast-stmt.h
#pragma once

#include "slang-ast-base.h"

//
#include "slang-ast-stmt.h.fiddle"

FIDDLE()
namespace Slang
{

// Syntax class definitions for statements.

FIDDLE(abstract)
class ScopeStmt : public Stmt
{
    FIDDLE(...)
    ScopeDecl* scopeDecl = nullptr;
};

// A sequence of statements, treated as a single statement
FIDDLE()
class SeqStmt : public Stmt
{
    FIDDLE(...)
    FIDDLE() List<Stmt*> stmts;
};

// A statement with a label.
FIDDLE()
class LabelStmt : public Stmt
{
    FIDDLE(...)
    FIDDLE() Token label;
    FIDDLE() Stmt* innerStmt;
};

// A lexical block, written with braces or implicit in an HLSL control-flow body.
FIDDLE()
class BlockStmt : public ScopeStmt
{
    FIDDLE(...)
    /// TODO(JS): Having ranges of sourcelocs might be a good addition to AST nodes in general.
    /// The closing brace, or the next token after a body with an implicit block scope.
    SourceLoc closingSourceLoc;

    FIDDLE() Stmt* body = nullptr;
};

// A statement that we aren't going to parse or check, because
// we want to let a downstream compiler handle any issues
FIDDLE()
class UnparsedStmt : public Stmt
{
    FIDDLE(...)
    // The tokens that were contained between `{` and `}`
    List<Token> tokens;
    Scope* currentScope = nullptr;
    Scope* outerScope = nullptr;
    SourceLanguage sourceLanguage;
};

FIDDLE()
class EmptyStmt : public Stmt
{
    FIDDLE(...)
};

FIDDLE()
class DiscardStmt : public Stmt
{
    FIDDLE(...)
};

FIDDLE()
class DeclStmt : public Stmt
{
    FIDDLE(...)
    FIDDLE() DeclBase* decl = nullptr;
};

// A condition of a statement that tests one, in either an expression or a
// declaration form. Any client that only needs the tested value can read it
// with `getPredicateExpr`, which hides which form the clause takes; code that
// must handle each form in its own way dispatches the clause with
// `ConditionClauseVisitor`.
FIDDLE(abstract)
class ConditionClause : public SyntaxNode
{
    FIDDLE(...)
};

// A condition written as an expression, as in `if (a < b)`.
FIDDLE()
class ExprConditionClause : public ConditionClause
{
    FIDDLE(...)

    /// The tested expression; semantic checking updates it in place with the
    /// checked expression.
    FIDDLE() Expr* expr = nullptr;
};

// A condition written as a single-variable declaration, as in `if (int count = c)`.
// The declared variable is visible in the condition, and wherever the statement that
// owns the condition keeps its scope open.
FIDDLE()
class DeclConditionClause : public ConditionClause
{
    FIDDLE(...)

    /// The variable that the condition declares. Always present: the parser only builds
    /// this clause once it has parsed the whole declaration, so the variable is never
    /// left null.
    FIDDLE() VarDeclBase* decl = nullptr;

    /// The tested value, derived during semantic checking; null until then.
    FIDDLE() Expr* predicateExpr = nullptr;
};

// The tested value of a condition clause, after semantic checking; null before
// checking.
Expr* getPredicateExpr(ConditionClause* clause);

FIDDLE()
class IfStmt : public Stmt
{
    FIDDLE(...)
    FIDDLE() ConditionClause* condition = nullptr;
    FIDDLE() Stmt* positiveStatement = nullptr;
    FIDDLE() Stmt* negativeStatement = nullptr;
    FIDDLE() SourceLoc afterLoc;
};

FIDDLE()
class UniqueStmtIDNode : public Decl
{
    FIDDLE(...)
};

// A statement that can be escaped with a `break`
FIDDLE(abstract)
class BreakableStmt : public ScopeStmt
{
    FIDDLE(...)

    /// A unique ID for this statement.
    ///
    /// Used by `ChildStmt` to reference the
    /// enclosing statement.
    ///
    UniqueStmtIDNode* uniqueID = kInvalidUniqueID;

    typedef UniqueStmtIDNode* UniqueID;
    static constexpr UniqueID kInvalidUniqueID = nullptr;
};

FIDDLE()
class SwitchStmt : public BreakableStmt
{
    FIDDLE(...)
    FIDDLE() Expr* condition = nullptr;
    FIDDLE() Stmt* body = nullptr;
};

// A statement that is expected to appear lexically nested inside
// some other construct, and thus needs to keep track of the
// outer statement that it is associated with...
FIDDLE(abstract)
class ChildStmt : public Stmt
{
    FIDDLE(...)

    /// The unique ID of the enclosing statement this
    /// child statement refers to.
    ///
    BreakableStmt::UniqueID targetOuterStmtID = BreakableStmt::kInvalidUniqueID;
};

FIDDLE()
class TargetCaseStmt : public ChildStmt
{
    FIDDLE(...)
    FIDDLE() int32_t capability;
    FIDDLE() Token capabilityToken;
    FIDDLE() Stmt* body = nullptr;
};

FIDDLE()
class TargetSwitchStmt : public BreakableStmt
{
    FIDDLE(...)
    FIDDLE() List<TargetCaseStmt*> targetCases;
};

FIDDLE()
class StageSwitchStmt : public TargetSwitchStmt
{
    FIDDLE(...)
};

FIDDLE()
class IntrinsicAsmStmt : public Stmt
{
    FIDDLE(...)
    FIDDLE() String asmText;

    FIDDLE() List<Expr*> args;
};

// a `case` or `default` statement inside a `switch`
//
// Note(tfoley): A correct AST for a C-like language would treat
// these as a labelled statement, and so they would contain a
// sub-statement. I'm leaving that out for now for simplicity.
FIDDLE(abstract)
class CaseStmtBase : public ChildStmt
{
    FIDDLE(...)
};

// a `case` statement inside a `switch`
FIDDLE()
class CaseStmt : public CaseStmtBase
{
    FIDDLE(...)
    FIDDLE() Expr* expr = nullptr;

    FIDDLE() Val* exprVal = nullptr;
};

// a `default` statement inside a `switch`
FIDDLE()
class DefaultStmt : public CaseStmtBase
{
    FIDDLE(...)
};

// a `default` statement inside a `switch`
FIDDLE()
class GpuForeachStmt : public ScopeStmt
{
    FIDDLE(...)
    FIDDLE() Expr* device = nullptr;
    FIDDLE() Expr* gridDims = nullptr;
    FIDDLE() VarDecl* dispatchThreadID = nullptr;
    FIDDLE() Expr* kernelCall = nullptr;
};

// A statement that represents a loop, and can thus be escaped with a `continue`
FIDDLE(abstract)
class LoopStmt : public BreakableStmt
{
    FIDDLE(...)
};

// A `for` statement
FIDDLE()
class ForStmt : public LoopStmt
{
    FIDDLE(...)
    FIDDLE() Stmt* initialStatement = nullptr;
    FIDDLE() Expr* sideEffectExpression = nullptr;
    FIDDLE() Expr* predicateExpression = nullptr;
    FIDDLE() Stmt* statement = nullptr;
};

// A `for` statement in a language that doesn't restrict the scope
// of the loop variable to the body.
FIDDLE()
class UnscopedForStmt : public ForStmt
{
    FIDDLE(...)
};

FIDDLE()
class WhileStmt : public LoopStmt
{
    FIDDLE(...)
    FIDDLE() ConditionClause* condition = nullptr;
    FIDDLE() Stmt* statement = nullptr;
};

FIDDLE()
class DoWhileStmt : public LoopStmt
{
    FIDDLE(...)
    FIDDLE() Stmt* statement = nullptr;
    FIDDLE() ConditionClause* condition = nullptr;
};

// A compile-time, range-based `for` loop, which will not appear in the output code
FIDDLE()
class CompileTimeForStmt : public ScopeStmt
{
    FIDDLE(...)
    FIDDLE() VarDecl* varDecl = nullptr;
    FIDDLE() Expr* rangeBeginExpr = nullptr;
    FIDDLE() Expr* rangeEndExpr = nullptr;
    FIDDLE() Stmt* body = nullptr;
    FIDDLE() IntVal* rangeBeginVal = nullptr;
    FIDDLE() IntVal* rangeEndVal = nullptr;
};

// The case of child statements that do control flow relative
// to their parent statement.
FIDDLE(abstract)
class JumpStmt : public ChildStmt
{
    FIDDLE(...)
};

FIDDLE()
class BreakStmt : public JumpStmt
{
    FIDDLE(...)
    Token targetLabel;
};

FIDDLE()
class ContinueStmt : public JumpStmt
{
    FIDDLE(...)
};

FIDDLE()
class ReturnStmt : public Stmt
{
    FIDDLE(...)
    FIDDLE() Expr* expression = nullptr;
};

FIDDLE()
class DeferStmt : public Stmt
{
    FIDDLE(...)

    FIDDLE() Stmt* statement = nullptr;
};

FIDDLE()
class ThrowStmt : public Stmt
{
    FIDDLE(...)
    FIDDLE() Expr* expression = nullptr;
};

FIDDLE()
class CatchStmt : public Stmt
{
    FIDDLE(...)
    FIDDLE() ParamDecl* errorVar = nullptr; // null => catch-all
    FIDDLE() Stmt* tryBody = nullptr;
    FIDDLE() Stmt* handleBody = nullptr;
};

FIDDLE()
class ExpressionStmt : public Stmt
{
    FIDDLE(...)
    FIDDLE() Expr* expression = nullptr;
};

FIDDLE()
class RequireCapabilityStmt : public Stmt
{
    FIDDLE(...)
    FIDDLE() List<Token> requiredCaps;
};

} // namespace Slang
