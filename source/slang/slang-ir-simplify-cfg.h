// slang-ir-simplify-cfg.h
#pragma once

#include "slang-ir-restructure.h"
#include "slang-ir-dominators.h"

namespace Slang
{
struct IRModule;
struct IRGlobalValueWithCode;
struct IRLoop;

struct CFGSimplificationContext
{
    RefPtr<RegionTree> regionTree;
    RefPtr<IRDominatorTree> domTree;
    Dictionary<IRInst*, List<IRInst*>> relatedAddrMap;
};

struct CFGSimplificationOptions
{
    bool removeTrivialSingleIterationLoops = true;
    bool removeSideEffectFreeLoops = true;
    static CFGSimplificationOptions getDefault() { return CFGSimplificationOptions(); }
    static CFGSimplificationOptions getFast() { return CFGSimplificationOptions{false, false}; }
};

bool isTrivialSingleIterationLoop(CFGSimplificationContext& context, IRGlobalValueWithCode* func, IRLoop* loop);

/// Simplifies control flow graph by merging basic blocks that
/// forms a simple linear chain.
/// Returns true if changed.
bool simplifyCFG(IRModule* module, CFGSimplificationOptions options);

bool simplifyCFG(IRGlobalValueWithCode* func, CFGSimplificationOptions options);

} // namespace Slang
