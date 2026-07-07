#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;
class TargetProgram;
struct RequiredLoweringPassSet;

void legalizeIRForWGSL(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink,
    const RequiredLoweringPassSet& requiredLoweringPassSet);

void specializeAddressSpaceForWGSL(IRModule* module);

} // namespace Slang
