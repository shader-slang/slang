// slang-late-require-capability.h
#pragma once

#include "slang.h"

namespace Slang
{
struct CodeGenContext;
class DiagnosticSink;
struct IRModule;

/// Checks that the named capabilities exist.
SlangResult checkLateRequireCapabilityArguments(IRModule* module, DiagnosticSink* sink);

/// Process and eliminate the LateRequireCapability IR insts.
SlangResult processLateRequireCapabilityInsts(IRModule* module, CodeGenContext* codeGenContext, DiagnosticSink* sink);

} // namespace Slang
