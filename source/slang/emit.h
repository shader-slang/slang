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

    // Emit code for a single entry point, based on
    // the input translation unit.
    String emitEntryPoint(
        EntryPointRequest*  entryPoint,
        ProgramLayout*      programLayout,
        CodeGenTarget       target);
}
#endif
