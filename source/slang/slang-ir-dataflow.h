#pragma once

#include "slang-dataflow.h"
#include "slang-ir-reachability.h"

namespace Slang
{
    class DiagnosticSink;
    struct IRModule;

    struct BasicBlockForwardDataFlow
    {
        using Node = IRBlock*;
        auto successors(Node n) { return n->getSuccessors(); }
        auto predecessors(Node n) { return n->getPredecessors(); }
    };

    struct BasicBlockBackwardDataFlow
    {
        using Node = IRBlock*;
        auto successors(Node n) { return n->getPredecessors(); }
        auto predecessors(Node n) { return n->getSuccessors(); }
    };

    template<typename T>
    auto basicBlockForwardDataFlow(
        T context,
        List<IRBlock*> blocksToQuery,
        ReachabilityContext& reachability,
        typename T::Domain initialInput = T::Domain::bottom())
    {
        static_assert(std::is_base_of_v<BasicBlockForwardDataFlow, T>);
        auto dependsOn = [&reachability](IRBlock* x, IRBlock* y){
            return reachability.isBlockReachable(y, x) && !reachability.isBlockReachable(x, y);
        };
        return dataFlow(
            context,
            blocksToQuery,
            dependsOn,
            initialInput);
    }

    template<typename T>
    auto basicBlockBackwardDataFlow(
        T context,
        List<IRBlock*> blocksToQuery,
        ReachabilityContext& reachability,
        typename T::Domain initialInput = T::Domain::bottom())
    {
        static_assert(std::is_base_of_v<BasicBlockBackwardDataFlow, T>);
        auto dependsOn = [&reachability](IRBlock* y, IRBlock* x){
            return reachability.isBlockReachable(y, x) && !reachability.isBlockReachable(x, y);
        };
        return dataFlow(
            context,
            blocksToQuery,
            dependsOn,
            initialInput);
    }
}
