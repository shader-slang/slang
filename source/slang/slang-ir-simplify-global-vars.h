// slang-ir-explicit-global-init.h
#pragma once

namespace Slang
{
struct IRModule;

/// Simplify IRGlobalVars and possibley remove if an alias.
void simplifyGlobalVars(IRModule* module);

}
