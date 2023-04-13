// slang-ir-specialize.h
#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;

    /// Specialize generic and interface-based code to use concrete types.
bool specializeModule(
    IRModule*   module,
    DiagnosticSink* sink);

void finalizeSpecialization(IRModule* module);

}
