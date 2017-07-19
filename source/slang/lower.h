// lower.h
#ifndef SLANG_LOWER_H_INCLUDED
#define SLANG_LOWER_H_INCLUDED

// The "lowering" step takes an input AST written in the complete Slang
// language and turns it into a more minimal format (still using the
// same AST) suitable for emission into lower-level languages.

#include "../core/basic.h"

#include "compiler.h"
#include "syntax.h"

namespace Slang
{
    class EntryPointRequest;
    class ProgramLayout;
    class TranslationUnitRequest;

    struct ExtensionUsageTracker;

    struct LoweredEntryPoint
    {
        // The actual lowered entry point
        RefPtr<FunctionSyntaxNode>  entryPoint;

        // The generated program AST that
        // contains the entry point and any
        // other declarations it uses
        RefPtr<ProgramSyntaxNode>   program;
    };

    // Emit code for a single entry point, based on
    // the input translation unit.
    LoweredEntryPoint lowerEntryPoint(
        EntryPointRequest*      entryPoint,
        ProgramLayout*          programLayout,
        CodeGenTarget           target,
        ExtensionUsageTracker*  extensionUsageTracker);
}
#endif
