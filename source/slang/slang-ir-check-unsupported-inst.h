#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class TargetRequest;

void checkUnsupportedInst(IRModule* module, TargetRequest* target, DiagnosticSink* sink);

// Diagnose a texel atomic on a multisampled texture for CUDA/PTX. Runs
// unconditionally (independent of the optimization level) because the CUDA
// emitter cannot represent the multisample resource type and would otherwise
// abort when emitting it, at every optimization level.
void checkUnsupportedTextureAtomic(IRModule* module, TargetRequest* target, DiagnosticSink* sink);
} // namespace Slang
