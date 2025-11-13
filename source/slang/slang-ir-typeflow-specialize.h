// slang-ir-typeflow-specialize.h
#pragma once
#include "slang-ir.h"

namespace Slang
{

// Convert dynamic insts such as `LookupWitnessMethod`, `ExtractExistentialValue`,
// `ExtractExistentialType`, `ExtractExistentialWitnessTable` and more into specialized versions
// based on the possible values at at the use sites, based on a data-flow-style interprocedural
// analysis.
//
// This pass is intended to be run after all specialization insts with concrete arguments have
// already been processed.
//
// This pass may generate more `Specialize` insts, so it should be run in a loop with
// the standard specialization pass until a no more changes can be made.
//
bool specializeDynamicInsts(IRModule* module, DiagnosticSink* sink);

bool isSetSpecializedGeneric(IRInst* callee);

IROp getSetOpFromType(IRType* type);
} // namespace Slang
