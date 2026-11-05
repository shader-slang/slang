#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;
class TargetProgram;

void legalizeIRForMetal(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);
void specializeAddressSpaceForMetal(IRModule* module);

} // namespace Slang
