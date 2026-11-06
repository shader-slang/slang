#pragma once

namespace Slang
{
struct CodeGenContext;
struct IRModule;

/// Eliminate supported IR operations that conditionally compute a resource on Khronos targets.
///
/// Handles resource-valued `IRSelect` instructions and non-entry block parameters, including loop
/// dependencies. For supported accesses to one array or heap, this function selects the index and
/// reconstructs one resource access. For alternatives with different resource constructions, it
/// branches around each operation that uses the chosen resource and merges any non-resource
/// results. Returns whether the module changed; returns false on other targets. Unsupported cases
/// leave their resource-producing operations intact for subsequent specialization or target
/// diagnostics.
bool specializeResourceSelections(CodeGenContext* codeGenContext, IRModule* module);
} // namespace Slang
