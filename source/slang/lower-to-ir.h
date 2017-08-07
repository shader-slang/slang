// lower.h
#ifndef SLANG_LOWER_TO_IR_H_INCLUDED
#define SLANG_LOWER_TO_IR_H_INCLUDED

// The lowering step translates from a (type-checked) AST into
// our intermediate representation, to facilitate further
// optimization and transformation.

#include "../core/basic.h"

#include "compiler.h"
#include "ir.h"

namespace Slang
{
    class EntryPointRequest;
    class ProgramLayout;
    class TranslationUnitRequest;

    struct ExtensionUsageTracker;

    void lowerEntryPointToIR(
        EntryPointRequest*  entryPoint,
        ProgramLayout*      programLayout,
        CodeGenTarget       target);
}
#endif
