#ifndef SLANG_PARSER_H
#define SLANG_PARSER_H

#include "slang-lexer.h"
#include "slang-compiler.h"
#include "slang-syntax.h"

namespace Slang
{
    // Parse a source file into an existing translation unit
    void parseSourceFile(
        ASTBuilder*                     astBuilder,
        TranslationUnitRequest*         translationUnit,
        TokenSpan const&                tokens,
        DiagnosticSink*                 sink,
        RefPtr<Scope> const&            outerScope);

    Expr* parseTermFromSourceFile(
        ASTBuilder*                     astBuilder,
        TokenSpan const&                tokens,
        DiagnosticSink*                 sink,
        RefPtr<Scope> const&            outerScope,
        NamePool*                       namePool,
        SourceLanguage                  sourceLanguage);

    ModuleDecl* populateBaseLanguageModule(
        ASTBuilder*     astBuilder,
        RefPtr<Scope>   scope);
}

#endif
