#pragma once

namespace Slang
{
struct CodeGenContext;
struct IRModule;

/// Replace resource selections with ordinary state and reconstruct resources at their operations.
///
/// On Khronos targets, the pass handles resource-valued selects and block parameters, including
/// loop dependencies. Alternatives from the same array or heap select indices; alternatives with
/// different resource constructions dispatch the consuming operation. Returns whether IR changed.
/// Unsupported graphs remain intact for subsequent specialization or target diagnostics.
bool specializeResourceSelections(CodeGenContext* codeGenContext, IRModule* module);
} // namespace Slang
