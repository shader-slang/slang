// slang-ir-lower-dynamic-insts.h
#pragma once
#include "../core/slang-linked-list.h"
#include "../core/slang-smart-pointer.h"
#include "slang-ir.h"

namespace Slang
{
// Main entry point for the pass
bool lowerDynamicInsts(IRModule* module, DiagnosticSink* sink);
// void lowerCollectionAndTagInsts(IRModule* module, DiagnosticSink* sink);

void lowerTypeCollections(IRModule* module, DiagnosticSink* sink);
void lowerTagInsts(IRModule* module, DiagnosticSink* sink);

void lowerSequentialIDTagCasts(IRModule* module, DiagnosticSink* sink);
void lowerTagTypes(IRModule* module);
} // namespace Slang
