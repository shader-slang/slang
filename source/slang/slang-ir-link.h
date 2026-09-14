// slang-ir-link.h
#pragma once

#include "compiler-core/slang-artifact-associated.h"
#include "slang-compiler.h"

namespace Slang
{
struct IRVarLayout;
class TargetProgram;
class DiagnosticSink;

struct LinkedIR
{
    RefPtr<IRModule> module;
    IRVarLayout* globalScopeVarLayout;
    List<IRFunc*> entryPoints;
    ComPtr<IArtifactPostEmitMetadata> metadata;
};


// Clone the IR values reachable from the given entry point
// into the IR module associated with the specialization state.
// When multiple definitions of a symbol are found, the one
// that is best specialized for the appropriate compilation
// target will be used.
//
LinkedIR linkIR(CodeGenContext* codeGenContext);

/// Gets the finalized target manifest used by structural ray-tracing codegen and reflection.
///
/// This performs link/specialization/open-section completion but does not enter target emission.
RefPtr<IRModule> getOrCreateStructuralRayTracingProgramManifest(
    TargetProgram* targetProgram,
    DiagnosticSink* sink);

// Prelinking is a step that happens immediately after visiting all AST nodes during IR lowering,
// and before any IR validation steps. Prelinking copys all extern symbols that are
// [unsafeForceInlineEarly] into the current module being lowered, so that we can perform necessary
// inlining passes before any data-flow analysis.
// `externalSymbolsToLink` is a list of IRInsts defined in the imported modules that we need to pull
// into the current module.
//
void prelinkIR(Module* module, IRModule* irModule, const List<IRInst*>& externalSymbolsToLink);

// Replace any global constants in the IR module with their
// definitions, if possible.
//
// This pass should always be run shortly after linking the
// IR, to ensure that constants with identical values are
// treated as identical for the purposes of specialization.
//
void replaceGlobalConstants(IRModule* module);
} // namespace Slang
