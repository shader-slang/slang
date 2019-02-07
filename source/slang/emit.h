// Emit.h
#ifndef SLANG_EMIT_H_INCLUDED
#define SLANG_EMIT_H_INCLUDED

#include "../core/basic.h"

#include "compiler.h"

namespace Slang
{
    class EntryPoint;
    class ProgramLayout;
    class TranslationUnitRequest;

    struct ExtensionUsageTracker;
    void requireGLSLExtension(
        ExtensionUsageTracker*  tracker,
        String const&           name);

    // Emit code for a single entry point, based on
    // the input translation unit.
    String emitEntryPoint(
        BackEndCompileRequest*  compileRequest,
        EntryPoint*             entryPoint,

        // The target language to generate code in (e.g., HLSL/GLSL)
        CodeGenTarget           target,

        // The full target request
        TargetRequest*          targetRequest);
}
#endif
