// slang-ir-specialize.h
#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class TargetRequest;

    /// Specialize generic and interface-based code to use concrete types.
bool specializeModule(
    TargetRequest* target,
    IRModule*   module,
    DiagnosticSink* sink);

void finalizeSpecialization(IRModule* module);

}
