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
///
/// This pass does not cap combinations of resource constructions or copies of a consuming
/// operation. Independent resource choices can multiply both analysis work and generated code.
///
/// Requires late target IR. Global shader parameters and function-entry parameters are retained
/// inputs; uses of other values in a supported selection graph must belong to block instructions
/// in their owning function, including local debug records. Surviving branch/function metadata
/// must not retain those values.
/// Metadata removed with a graph instruction must have no users outside that instruction's owned
/// subtree.
bool specializeResourceSelections(CodeGenContext* codeGenContext, IRModule* module);
} // namespace Slang
