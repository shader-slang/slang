#pragma once

namespace Slang
{
struct IRModule;

// Expand marked captured-parameter contexts in internal derivative signatures and direct calls.
// Run after tuple lowering and before conversion of aggregate parameters to references, so later
// optimization can handle captured arguments independently. External and reference ABIs are kept.
void expandAutodiffParameterContexts(IRModule* module);
} // namespace Slang
