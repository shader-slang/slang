#pragma once

namespace Slang
{
struct IRModule;

void legalizeAtomicOperations(IRModule* module);
} // namespace Slang
