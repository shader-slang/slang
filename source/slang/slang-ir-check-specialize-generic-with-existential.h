// slang-ir-check-specialize-generic-with-existential.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;

/// This IR pass decorates IRSpecialize insts whose specialization arguments include existential
/// types, and eagerly emits E33180 for kIROp_InterfaceType args. The added decoration
/// (IRDisallowSpecializationWithExistentialsDecoration) lets the typeflow-specialize pass
/// distinguish invalid existential-type specializations from valid narrowed callees.
void addDecorationsForGenericsSpecializedWithExistentials(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
