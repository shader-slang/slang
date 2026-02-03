// slang-ir-mesh-output-reads.h
#pragma once

#include "slang-compiler-options.h"

namespace Slang
{
class DiagnosticSink;
struct IRModule;

void checkForMeshOutputReads(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
