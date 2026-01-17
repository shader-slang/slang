// slang-ir-lower-switch-to-reconverged-switches.h
#pragma once

namespace Slang
{
struct IRModule;
class TargetProgram;

/// Lower switch statements with fallthrough to use reconverged control flow.
///
/// This transformation splits switches with fallthrough into two switches:
/// 1. First switch: Sets fallthroughSelector and fallthroughStage, all cases break
/// 2. Second switch: Executes fallthrough sequences using if-guards for stages
///
/// This provides deterministic reconvergence behavior for switches with
/// non-trivial fallthrough, addressing undefined behavior in SPIR-V's OpSwitch.
///
/// Non-fallthrough cases remain in the first switch and execute directly.
///
/// See GitHub issue #6441 for details.
void lowerSwitchToReconvergedSwitches(IRModule* module, TargetProgram* targetProgram);

} // namespace Slang
