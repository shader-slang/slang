#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;
class TargetProgram;

void legalizeIRForWGSL(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

void specializeAddressSpaceForWGSL(IRModule* module);

} // namespace Slang
