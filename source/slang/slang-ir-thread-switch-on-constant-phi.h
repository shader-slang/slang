// slang-ir-thread-switch-on-constant-phi.h
#pragma once

namespace Slang
{
struct IRModule;

/// Jump-thread `switch` instructions whose selector is a block parameter (phi)
/// that carries a distinct compile-time constant along every incoming path.
///
/// This targets the shape left after inlining, specialization, and
/// scalarization when a branch cascade first selects a constant integer tag and
/// a later `switch` consumes it: the tag is not globally constant (so SCCP
/// cannot fold the `switch`) but is constant along each control-flow edge, so
/// the second runtime dispatch is redundant. The pass rewrites the CFG so that
/// each selecting arm reaches its proven case body directly, then removes the
/// now-dead `switch` -- yielding the same control shape as writing the concrete
/// dispatch in each arm, without cloning the shared continuation.
///
/// Returns true if any function was changed.
bool threadSwitchOnConstantPhi(IRModule* module);
} // namespace Slang
