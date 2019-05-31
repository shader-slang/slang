#ifndef SLANG_SYNTAX_VISITORS_H
#define SLANG_SYNTAX_VISITORS_H

#include "slang-diagnostics.h"
#include "slang-syntax.h"

namespace Slang
{
    class DiagnosticSink;
    class EntryPoint;
    class Linkage;
    class Module;
    class ShaderCompiler;
    class ShaderLinkInfo;
    class ShaderSymbol;

    class TranslationUnitRequest;

    void checkTranslationUnit(
        TranslationUnitRequest* translationUnit);

    // Look for a module that matches the given name:
    // either one we've loaded already, or one we
    // can find vai the search paths available to us.
    //
    // Needed by import declaration checking.
    //
    // TODO: need a better location to declare this.
    RefPtr<Module> findOrImportModule(
        Linkage*            linkage,
        Name*               name,
        SourceLoc const&    loc,
        DiagnosticSink*     sink);
}

#endif
