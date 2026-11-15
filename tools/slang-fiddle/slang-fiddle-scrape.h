// slang-fiddle-scrape.h
#pragma once

#include "compiler-core/slang-lexer.h"
#if SLANG_USE_SYSTEM_LUA
#include <lua.hpp>
#else
#include "lua/lauxlib.h"
#endif
#include "slang-fiddle-diagnostics.h"

namespace fiddle
{
using namespace Slang;

class Val : public RefObject
{
public:
};

class Node : public Val
{
public:
};

// Fiddle shares token occurrences between its parser and syntax nodes. The mutable node adds
// delimiter-skip information; `Token` represents the raw token and its attached trivia.

/// A token occurrence with mutable bookkeeping shared by Fiddle's parser and syntax nodes.
class TokenWithTriviaNode : public RefObject
{
public:
    TokenType getTypeTag() const { return token.getTypeTag(); }

    Token token;
    Count skipCount = 0;
};

/// A shared handle to a token occurrence and Fiddle's delimiter-skip bookkeeping.
struct TokenWithTrivia
{
public:
    TokenWithTrivia() {}

    TokenWithTrivia(RefPtr<TokenWithTriviaNode> node)
        : node(node)
    {
    }

    SourceLoc getLoc() const { return node->token.getLoc(); }

    Token const& getToken() const { return node->token; }

    TokenType getTypeTag() const { return node ? node->getTypeTag() : TokenType::Count; }

    UnownedStringSlice getLogicalSpelling() const
    {
        return node ? node->token.getLogicalSpelling() : UnownedStringSlice();
    }

    Count getSkipCount() const { return node ? node->skipCount : 0; }

    /// Consumes the first `>` of a `>>` token, leaving its second closer as lookahead.
    ///
    /// Requires a non-empty `OpRsh` value. Leading trivia is consumed with the first
    /// closer; trailing trivia remains with the second closer at its original locations.
    void consumeFirstGreater() const
    {
        SLANG_RELEASE_ASSERT(node);
        SLANG_RELEASE_ASSERT(node->token);
        auto token = node->token;
        auto pool = token.getPool();
        auto secondCloser = pool->createSecondGreaterToken(token.getRawToken().as<RawOpRsh>());
        node->token = pool->createToken(secondCloser, {}, token.getTrailingTrivia().getObjects());
    }

    TriviaList getLeadingTrivia() const { return node->token.getLeadingTrivia(); }
    TriviaList getTrailingTrivia() const { return node->token.getTrailingTrivia(); }

    operator TokenWithTriviaNode*() { return node; }

private:
    RefPtr<TokenWithTriviaNode> node;
};


// Syntax

class Declarator : public Node
{
};

class NameDeclarator : public Declarator
{
public:
    NameDeclarator(TokenWithTrivia nameToken)
        : nameToken(nameToken)
    {
    }

    TokenWithTrivia nameToken;
};

class PtrDeclarator : public Declarator
{
public:
    PtrDeclarator(RefPtr<Declarator> base)
        : base(base)
    {
    }

    RefPtr<Declarator> base;
};

class Expr : public Node
{
public:
};

class ModifierNode : public Node
{
};

class AbstractModifier : public ModifierNode
{
};
class HiddenModifier : public ModifierNode
{
};
class TableModifier : public ModifierNode
{
public:
    TableModifier(String s)
        : tableSource(s)
    {
    }
    String tableSource;
    int tableRef = LUA_NOREF;
};

enum class Mode
{
    Fiddle,
    Cpp,
};

class Decl : public Node
{
public:
    template<typename T>
    T* findModifier()
    {
        for (auto m : modifiers)
        {
            if (auto found = as<T>(m))
                return found;
        }
        return nullptr;
    }

    List<RefPtr<ModifierNode>> modifiers;
    TokenWithTrivia nameToken;
    Mode mode = Mode::Cpp;
};

class ContainerDecl : public Decl
{
public:
    List<RefPtr<Decl>> members;
    Dictionary<String, RefPtr<Decl>> mapNameToMember;
};

class LogicalContainerDecl : public ContainerDecl
{
};

class LogicalNamespaceBase : public LogicalContainerDecl
{
};

class LogicalModule : public LogicalNamespaceBase
{
public:
    /// The pool for all files parsed into this module.
    TokenPool* getTokenPool() const { return m_tokenPool; }

private:
    RefPtr<TokenPool> m_tokenPool = new TokenPool();
};

class LogicalNamespace : public LogicalNamespaceBase
{
};

class PhysicalContainerDecl : public ContainerDecl
{
public:
    LogicalContainerDecl* logicalVersion = nullptr;
};

class SourceUnit : public PhysicalContainerDecl
{
public:
};

class PhysicalNamespaceDecl : public PhysicalContainerDecl
{
public:
};

class AggTypeDecl : public ContainerDecl
{
public:
    RefPtr<Expr> directBaseType;

    List<AggTypeDecl*> directSubTypeDecls;

    bool isSubTypeOf(char const* name);
};

class ClassDecl : public AggTypeDecl
{
};
class StructDecl : public AggTypeDecl
{
};

class VarDecl : public Decl
{
public:
    RefPtr<Expr> type;
    RefPtr<Expr> initExpr;
};

class FiddleMacroInvocation : public Decl
{
public:
    TokenWithTrivia fiddleToken; // the actual `FIDDLE` identifier

    RefPtr<Node> node; // the node whose generated content should get emitted...
};

class FiddleLuaCallInvocation : public Decl
{
public:
    TokenWithTrivia fiddleToken; // the actual `FIDDLE` identifier
    String callString;
    RefPtr<Node> parentDecl;
};

class UncheckedExpr : public Expr
{
};

class CheckedExpr : public Expr
{
};

class NameExpr : public UncheckedExpr
{
public:
    NameExpr(TokenWithTrivia nameToken)
        : nameToken(nameToken)
    {
    }

    TokenWithTrivia nameToken;
};

class LiteralExpr : public UncheckedExpr
{
public:
    LiteralExpr(TokenWithTrivia token)
        : token(token)
    {
    }

    TokenWithTrivia token;
};

class MemberExpr : public UncheckedExpr
{
public:
    MemberExpr(RefPtr<Expr> base, TokenWithTrivia memberNameToken)
        : base(base), memberNameToken(memberNameToken)
    {
    }

    RefPtr<Expr> base;
    TokenWithTrivia memberNameToken;
};

class StaticMemberRef : public UncheckedExpr
{
public:
    StaticMemberRef(RefPtr<Expr> base, TokenWithTrivia memberNameToken)
        : base(base), memberNameToken(memberNameToken)
    {
    }

    RefPtr<Expr> base;
    TokenWithTrivia memberNameToken;
};

typedef Expr Arg;

class PtrType : public UncheckedExpr
{
public:
    PtrType(RefPtr<Expr> base)
        : base(base)
    {
    }

    RefPtr<Expr> base;
};

class SpecializeExpr : public UncheckedExpr
{
public:
    SpecializeExpr() {}

    RefPtr<Expr> base;
    List<RefPtr<Expr>> args;
};

class CallExpr : public UncheckedExpr
{
public:
    CallExpr(RefPtr<Expr> base, List<RefPtr<Arg>> args)
        : base(base), args(args)
    {
    }

    RefPtr<Expr> base;
    List<RefPtr<Arg>> args;
};

class DirectDeclRef : public CheckedExpr
{
public:
    DirectDeclRef(Decl* decl)
        : decl(decl)
    {
    }

    Decl* decl = nullptr;
};

//

Decl* findDecl_(ContainerDecl* outerDecl, UnownedStringSlice const& name);

template<typename T>
T* findDecl(ContainerDecl* outerDecl, UnownedStringSlice const& name)
{
    auto decl = findDecl_(outerDecl, name);
    if (!decl)
        return nullptr;

    auto asType = as<T>(decl);
    if (!asType)
    {
        // TODO: might need this case to be an error...
        return nullptr;
    }

    return asType;
}


/// Parses one file's syntax and registers its declarations in `logicalModule`.
///
/// The returned tree borrows lexical storage from `logicalModule`'s pool, so the caller
/// keeps that module alive while using the tree. Resolving the tree's source locations
/// also requires the manager that owns `inputSourceView` to remain alive.
RefPtr<SourceUnit> parseSourceUnit(
    SourceView* inputSourceView,
    LogicalModule* logicalModule,
    NamePool* namePool,
    DiagnosticSink* sink,
    String outputFileName);

bool hasAnyFiddleInvocations(SourceUnit* sourceUnit);

void checkModule(LogicalModule* module, DiagnosticSink* sink);


void registerScrapedStuffWithScript(LogicalModule* logicalModule);

void emitSourceUnitMacros(
    SourceUnit* sourceUnit,
    StringBuilder& builder,
    DiagnosticSink* sink,
    SourceManager* sourceManager,
    LogicalModule* logicalModule);
} // namespace fiddle
