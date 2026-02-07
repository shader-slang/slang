// slang-ir-check-specialize-generic-with-existential.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;

/// This IR check pass is will add an annotation on `IRSpecialize` instructions when a generic is
/// specialized with an existential type.
void addDecorationsForGenericsSpecializedWithExistentials(IRModule* module);

} // namespace Slang
