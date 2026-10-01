#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;
class TargetProgram;

void legalizeIRForMetal(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

/// Specialize Metal address spaces from native entry points and target-owned executable roots.
void specializeAddressSpaceForMetal(IRModule* module, ConstArrayView<IRFunc*> additionalRoots = {});

} // namespace Slang
