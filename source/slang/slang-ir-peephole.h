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
    /// True for the front-end runs in `generateIRForTranslationUnit`, before linking and before
    /// `checkForUsingUninitializedValues`. Such runs keep `extern` witness-table lookups
    /// unresolved, and keep block params whose incoming values are all undefined so that each
    /// undefined value still gets its own uninitialized-use warning.
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
