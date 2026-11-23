// slang-ir-hlsl-legalize.h
#pragma once

namespace Slang
{

struct CapabilitySet;
class DiagnosticSink;
struct IRModule;

void validateBarrierFlagsForHLSL(IRModule* module, DiagnosticSink* sink);

/// Diagnose every `groupshared` parameter of an emitted function. HLSL does not allow a function
/// that keeps a call boundary, such as an `export` function, to have a `groupshared` parameter
/// (HLSL proposal 0043, `groupshared` Arguments), so a `groupshared` parameter that survives
/// inlining cannot be lowered.
void validateGroupSharedParamsForHLSL(
    IRModule* module,
    CapabilitySet const& targetCaps,
    DiagnosticSink* sink);

} // namespace Slang
