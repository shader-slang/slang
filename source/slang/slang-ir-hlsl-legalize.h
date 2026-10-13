// slang-ir-hlsl-legalize.h
#pragma once

namespace Slang
{

class DiagnosticSink;
struct IRModule;

void validateBarrierFlagsForHLSL(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
