#ifndef SLANG_PARSER_H
#define SLANG_PARSER_H

#include "compiler-core/slang-lexer.h"
#include "slang-compiler.h"
#include "slang-syntax.h"

namespace Slang
{
// Parse a source file into an existing translation unit
void parseSourceFile(
    ASTBuilder* astBuilder,
    TranslationUnitRequest* translationUnit,
    SourceLanguage sourceLanguage,
    TokenSpan const& tokens,
    DiagnosticSink* sink,
    Scope* outerScope,
    ContainerDecl* parentDecl);

// Parse a term (type or expression) from a standalone string, outside any module. Used by the
// reflection / string-parse APIs (getTypeFromString, parseExprFromString) and
// specialization-argument parsing. `languageVersion` is the version to assume while parsing, since
// this path has no owning module to read it from; callers pass the session's configured version
// (SLANG_LANGUAGE_VERSION_DEFAULT if unknown).
Expr* parseTermFromSourceFile(
    ASTBuilder* astBuilder,
    TokenSpan const& tokens,
    DiagnosticSink* sink,
    Scope* outerScope,
    NamePool* namePool,
    SourceLanguage sourceLanguage,
    SlangLanguageVersion languageVersion);

struct SemanticsVisitor;

Stmt* parseUnparsedStmt(
    ASTBuilder* astBuilder,
    SemanticsVisitor* semantics,
    TranslationUnitRequest* translationUnit,
    SourceLanguage sourceLanguage,
    TokenSpan const& tokens,
    DiagnosticSink* sink,
    Scope* currentScope,
    Scope* outerScope);

ModuleDecl* populateBaseLanguageModule(ASTBuilder* astBuilder, Scope* scope);

/// Information used to set up SyntaxDecl. Such decls
/// when correctly setup define a callback. For some of the callbacks it's necessary
/// for the `parseUserData` to be set the the associated classInfo
struct SyntaxParseInfo
{
    const char* keywordName;         ///< The keyword associated with this parse
    SyntaxParseCallback callback;    ///< The callback to apply to the parse
    SyntaxClass<NodeBase> classInfo; ///<
};

/// Get all of the predefined SyntaxParseInfos
ConstArrayView<SyntaxParseInfo> getSyntaxParseInfos();

/// Assumes the userInfo is the ReflectClassInfo
NodeBase* parseSimpleSyntax(Parser* parser, void* userData);

} // namespace Slang

#endif
