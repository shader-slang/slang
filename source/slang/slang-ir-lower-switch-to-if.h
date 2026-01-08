// slang-ir-lower-switch-to-if.h
#pragma once

namespace Slang
{
struct IRModule;
class TargetProgram;

/// Lower switch statements to if-else chains wrapped in do-while(false).
///
/// This transformation provides deterministic reconvergence behavior for
/// switches with non-trivial fallthrough, addressing undefined behavior
/// in SPIR-V's OpSwitch instruction.
///
/// See GitHub issue #6441 for details.
void lowerSwitchToIf(IRModule* module, TargetProgram* targetProgram);

} // namespace Slang
