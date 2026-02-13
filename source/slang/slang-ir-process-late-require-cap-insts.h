// slang-ir-process-late-require-cap-insts.h
#pragma once

#include "slang.h"

namespace Slang
{
struct CodeGenContext;
class DiagnosticSink;
struct IRModule;

/// Process and eliminate the LateRequireCapability IR insts:
SlangResult processLateRequireCapabilityInsts(IRModule* module, CodeGenContext* codeGenContext, DiagnosticSink* sink);

} // namespace Slang
