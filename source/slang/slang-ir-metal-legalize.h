#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;
class TargetProgram;

void legalizeIRForMetal(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);
void specializeAddressSpaceForMetal(IRModule* module);

/// Insert a `metalTextureFence` after every write to a texture that Metal declares
/// `access::read_write`, so that a later read of that texture by the same thread sees the write.
void insertMetalTextureFences(IRModule* module);

} // namespace Slang
