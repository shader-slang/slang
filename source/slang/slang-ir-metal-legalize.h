#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

void legalizeIRForMetal(IRModule* module, DiagnosticSink* sink);
void specializeAddressSpaceForMetal(IRModule* module);

} // namespace Slang
