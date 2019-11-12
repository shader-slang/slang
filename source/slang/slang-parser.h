#ifndef SLANG_PARSER_H
#define SLANG_PARSER_H

#include "slang-lexer.h"
#include "slang-compiler.h"
#include "slang-syntax.h"

namespace Slang
{
    // Parse a source file into an existing translation unit
    void parseSourceFile(
        TranslationUnitRequest*         translationUnit,
        TokenSpan const&                tokens,
        DiagnosticSink*                 sink,
        RefPtr<Scope> const&            outerScope);

    RefPtr<Expr> parseTermFromSourceFile(
        Session*                        session,
        TokenSpan const&                tokens,
        DiagnosticSink*                 sink,
        RefPtr<Scope> const&            outerScope,
        NamePool*                       namePool,
        SourceLanguage                  sourceLanguage);

    RefPtr<ModuleDecl> populateBaseLanguageModule(
        Session*        session,
        RefPtr<Scope>   scope);
}

#endif
