#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class TargetRequest;

void checkUnsupportedInst(IRModule* module, TargetRequest* target, DiagnosticSink* sink);

// Diagnose any reachable function whose signature uses one of the internal
// parameter-passing modes that have no code-generation support yet
// (`__ref_readonly` / `__ref_writeonly` / `__consume`). This is target-independent
// and must run after inlining/DCE (so an inlined-away or unreferenced use is
// allowed) but on every emit path, including the host-VM and minimum-optimization
// paths that skip the general `checkUnsupportedInst` pass. See #12547.
void checkForUnsupportedParamModes(IRModule* module, DiagnosticSink* sink);

// Like `checkForUnsupportedParamModes`, but restricted to entry-point functions.
// Must run before entry-point uniform-parameter collection (which erases the
// param-mode wrapper); entry points are never inlined away, so an early check on
// them has no false positives.
void checkForUnsupportedEntryPointParamModes(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
