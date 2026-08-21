#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class TargetRequest;

void checkUnsupportedInst(IRModule* module, TargetRequest* target, DiagnosticSink* sink);

void checkUnsupportedTextureAtomic(IRModule* module, TargetRequest* target, DiagnosticSink* sink);
} // namespace Slang
