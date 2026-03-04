// slang-ir-check-specialize-generic-with-existential.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;

/// This IR check pass is will diagnose error when a generic is specialized with
// an existential type.
void addDecorationsForGenericsSpecializedWithExistentials(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
