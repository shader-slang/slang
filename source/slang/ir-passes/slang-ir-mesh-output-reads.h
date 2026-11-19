// slang-ir-mesh-output-reads.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;

void checkForMeshOutputReads(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
