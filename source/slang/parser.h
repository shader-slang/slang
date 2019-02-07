#ifndef RASTER_RENDERER_PARSER_H
#define RASTER_RENDERER_PARSER_H

#include "lexer.h"
#include "compiler.h"
#include "syntax.h"

namespace Slang
{
    // Parse a source file into an existing translation unit
    void parseSourceFile(
        TranslationUnitRequest*         translationUnit,
        TokenSpan const&                tokens,
        DiagnosticSink*                 sink,
        RefPtr<Scope> const&            outerScope);

    RefPtr<Expr> parseTypeFromSourceFile(
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