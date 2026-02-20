#pragma once

namespace Slang
{
struct IRModule;
struct IRFunc;
bool propagateFuncProperties(IRModule* module);
bool propagatePropertiesForSingleFunc(IRModule* module, IRFunc* f);
} // namespace Slang
