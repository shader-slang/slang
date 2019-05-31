// slang-emit.h
#ifndef SLANG_EMIT_H_INCLUDED
#define SLANG_EMIT_H_INCLUDED

#include "../core/slang-basic.h"

#include "slang-compiler.h"

namespace Slang
{
    class EntryPoint;
    class ProgramLayout;
    class TranslationUnitRequest;

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
