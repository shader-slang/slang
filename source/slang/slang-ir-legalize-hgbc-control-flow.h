#pragma once

namespace Slang
{
struct IRModule;

/// Lowers structured control flow to ordinary branches after phi elimination.
/// Run DCE afterward to remove blocks retained only by structural references.
void legalizeControlFlowForHGBC(IRModule* module);
} // namespace Slang
