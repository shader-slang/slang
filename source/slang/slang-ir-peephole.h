// slang-ir-peephole.h
#pragma once

namespace Slang
{
struct IRModule;
struct IRCall;
struct IRInst;
class TargetProgram;

struct PeepholeOptimizationOptions
{
    /// True for the peephole runs in `generateIRForTranslationUnit` that come before linking and
    /// before `checkForUsingUninitializedValues`: the module-wide run after SSA construction and
    /// the per-function runs after mandatory early inlining. Such runs keep `extern` witness-table
    /// lookups unresolved, and keep block params whose incoming values are all undefined so that
    /// each undefined value still gets its own uninitialized-use warning.
    /// `peepholeOptimizeGlobalScope`, which also runs after early inlining, takes no options and
    /// always runs without this flag.
    bool isPrelinking = false;
    static PeepholeOptimizationOptions getPrelinking()
    {
        PeepholeOptimizationOptions result;
        result.isPrelinking = true;
        return result;
    }
};

/// Apply peephole optimizations.
bool peepholeOptimize(TargetProgram* target, IRModule* module, PeepholeOptimizationOptions options);
bool peepholeOptimize(TargetProgram* target, IRInst* func);
bool peepholeOptimize(TargetProgram* target, IRInst* func, PeepholeOptimizationOptions options);
bool peepholeOptimizeInst(TargetProgram* target, IRModule* module, IRInst* inst);
bool peepholeOptimizeGlobalScope(TargetProgram* target, IRModule* module);
bool tryReplaceInstUsesWithSimplifiedValue(TargetProgram* target, IRModule* module, IRInst* inst);
} // namespace Slang
