// slang-ir-hlsl-legalize.h
#pragma once

namespace Slang
{

struct CapabilitySet;
class DiagnosticSink;
struct IRModule;

void validateBarrierFlagsForHLSL(IRModule* module, DiagnosticSink* sink);

/// Diagnose every `groupshared` parameter of an emitted function. HLSL cannot pass
/// thread-group-shared memory across a call boundary (DXC error 0043), so a `groupshared`
/// parameter that survives inlining cannot be lowered.
void validateGroupSharedParamsForHLSL(
    IRModule* module,
    CapabilitySet const& targetCaps,
    DiagnosticSink* sink);

} // namespace Slang
