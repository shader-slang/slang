// slang-ir-missing-return.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;
enum class CodeGenTarget;

void checkForMissingReturns(IRModule* module, DiagnosticSink* sink, CodeGenTarget target);
} // namespace Slang
