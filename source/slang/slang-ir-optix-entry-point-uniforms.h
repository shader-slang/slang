// slang-ir-optix-entry-point-uniforms.h
#pragma once

namespace Slang
{

class DiagnosticSink;
struct IRModule;
void collectOptiXEntryPointUniformParams(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
