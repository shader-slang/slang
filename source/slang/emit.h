// Emit.h
#ifndef SLANG_EMIT_H_INCLUDED
#define SLANG_EMIT_H_INCLUDED

#include "../core/basic.h"

#include "compiler.h"

namespace Slang
{
    class EntryPointRequest;
    class ProgramLayout;
    class TranslationUnitRequest;

    struct ExtensionUsageTracker;
    void requireGLSLExtension(
        ExtensionUsageTracker*  tracker,
        String const&           name);

    // Emit code for a single entry point, based on
    // the input translation unit.
    String emitEntryPoint(
        EntryPointRequest*  entryPoint,
        ProgramLayout*      programLayout,

        // The target language to generate code in (e.g., HLSL/GLSL)
        CodeGenTarget       target,

        // The full target request
        TargetRequest*      targetRequest);
}
#endif
