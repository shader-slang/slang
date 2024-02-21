// slang-ir-specialize.h
#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class TargetProgram;

    /// Specialize generic and interface-based code to use concrete types.
bool specializeModule(
    TargetProgram* target,
    IRModule*   module,
    DiagnosticSink* sink);

void finalizeSpecialization(IRModule* module);

}
