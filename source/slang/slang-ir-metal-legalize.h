#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;
class TargetProgram;

void legalizeIRForMetal(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);
void specializeAddressSpaceForMetal(IRModule* module);

/// Insert a `metalTextureFence` before and after every write to a texture that Metal declares
/// `access::read_write`. The fence after the write lets a later read of that texture by the same
/// thread see it; the fence before keeps the write from moving ahead of an earlier read.
void insertMetalTextureFences(IRModule* module);

} // namespace Slang
