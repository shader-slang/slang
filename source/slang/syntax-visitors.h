#ifndef RASTER_RENDERER_SYNTAX_PRINTER_H
#define RASTER_RENDERER_SYNTAX_PRINTER_H

#include "diagnostics.h"
#include "syntax.h"
#include "compiled-program.h"

namespace Slang
{
    namespace Compiler
    {
        class CompileOptions;
        class ShaderCompiler;
        class ShaderLinkInfo;
        class ShaderSymbol;

        SyntaxVisitor * CreateSemanticsVisitor(DiagnosticSink * err, CompileOptions const& options);
    }
}

#endif