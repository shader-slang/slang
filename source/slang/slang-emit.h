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



        /// Emit source code for a single entry point.
        ///
        /// This function generates source code for the
        /// entry point with index `entryPointIndex`
        /// inside the program being compiled in `compileRequest`.
        ///
        /// The code is generated for the language specified
        /// as `target` (e.g., HLSL or GLSL).
        ///
        /// The `targetRequest` gives further information about
        /// the compilation target and its options.
        ///
        /// Note: it is possible that `target` and `targetRequest`
        /// do not store the same target format. For example
        /// `target` might be HLSL, while `targetRequest` is
        /// a DXIL target. The split information here tells us
        /// both the immediate target language (HLSL) as well
        /// as the eventual destination format (DXIL) in case
        /// we need to customize the output (e.g., we might
        /// generate different HLSL output if we know it
        /// will be used to generate SPIR-V).
        ///
    SlangResult emitEntryPointsSourceFromIR(
        BackEndCompileRequest*  compileRequest,
        const List<Int>&        entryPointIndices,
        CodeGenTarget           target,
        TargetRequest*          targetRequest,
        SourceResult&           outSource);
}
#endif
