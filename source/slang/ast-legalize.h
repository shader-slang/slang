// ast-legalize.h
#ifndef SLANG_AST_LEGALIZE_H_INCLUDED
#define SLANG_AST_LEGALIZE_H_INCLUDED

// The AST legalization pass takes an AST and tries to transform
// it to make sure that it is legal for a chosen compilation target.
//
// This can include many different kinds of work:
//
// - If the input was written in HLSL/Slang, but we want GLSL output, then
//   this pass is responsible for converting certain HLSL idioms into
//   their GLSL equivalents. This is not a really good cross-compilation
//   solution (in particular, it does *not* support Slang code that uses
//   and of our more advanced features), and it will eventually be deprecated
//   in favor of the IR-based code generation approach.
//
// - For input written in GLSL, we support a few extensions to the specified
//   language rules (e.g., we support `struct` types that mix resource and
//   uniform types, and we also support `struct` types for vertex inputs).
//   These need to be lowered to vanilla GLSL. This also applies to HLSL if
//   the `-split-mixed-types` flag is set.
//
// - When using the IR to provide portable library code, with entry points
//   written in unchecked (`-no-checking`) HLSL or GLSL, this pass will
//   be applied to the unchecked code, and used to determine what subset of
//   the Slang code must be compiled via the IR.
//
// Note: in the case where input is pure Slang code (or the subset of
// HLSL that we can fully check) this pass is not needed or used at all;
// instead we perform all lowering, legalization, etc. entirely via the IR.
//

#include "../core/basic.h"

#include "compiler.h"
#include "syntax.h"

namespace Slang
{
    class  EntryPointRequest;
    struct ExtensionUsageTracker;
    struct IRSpecializationState;
    class  ProgramLayout;
    class  TranslationUnitRequest;
    struct TypeLegalizationContext;


    struct LoweredEntryPoint
    {
        // The actual lowered entry point
        RefPtr<FuncDecl>  entryPoint;

        // The generated program AST that
        // contains the entry point and any
        // other declarations it uses
        RefPtr<ModuleDecl>   program;

        // A set of declarations that are not present
        // in the generated AST, and are instead stored
        // in the companion IR module
        HashSet<Decl*>  irDecls;
    };

    // Emit code for a single entry point, based on
    // the input translation unit.
    LoweredEntryPoint lowerEntryPoint(
        EntryPointRequest*          entryPoint,
        ProgramLayout*              programLayout,
        CodeGenTarget               target,
        ExtensionUsageTracker*      extensionUsageTracker,
        IRSpecializationState*      irSpecializationState,
        TypeLegalizationContext*    typeLegalizationContext);
}
#endif
