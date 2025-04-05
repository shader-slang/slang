// slang-ir-redundancy-removal.h
#pragma once
#include "slang-compiler.h"

namespace Slang
{
struct IRModule;
struct IRGlobalValueWithCode;

bool removeRedundancy(IRModule* module, bool tryToHoist);
bool removeRedundancyInFunc(IRGlobalValueWithCode* func, bool tryToHoist);

bool eliminateRedundantLoadStore(IRGlobalValueWithCode* func);

void removeAvailableInDownstreamModuleDecorations(CodeGenTarget target, IRModule* module);
} // namespace Slang
