// slang-ir-typeflow-specialize.h
#pragma once
#include "slang-ir.h"

namespace Slang
{

void lowerTypeCollections(IRModule* module, DiagnosticSink* sink);
void lowerTagInsts(IRModule* module, DiagnosticSink* sink);

void lowerSequentialIDTagCasts(IRModule* module, DiagnosticSink* sink);
void lowerTagTypes(IRModule* module);

bool lowerTaggedUnionTypes(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
