#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;
class TargetProgram;

void legalizeIRForWGSL(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

void specializeAddressSpaceForWGSL(IRModule* module);

/// Read a groupshared value through `workgroupUniformLoad` when it decides whether a
/// workgroup barrier runs, so that WGSL's uniformity analysis accepts the barrier.
void insertWorkgroupUniformLoadsForWGSL(IRModule* module, CapabilitySet const& targetCaps);

} // namespace Slang
