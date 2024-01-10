#pragma once

#include "../core/slang-basic.h"

#include <algorithm>
#include <limits>
#include <queue>

namespace Slang
{

template<typename D>
struct InOutPair
{
    D in;
    D out;
};

//
// Parameters:
// - context: This object provides the transfer function
// - ns: The list of nodes for which we should return the eventual output of
//       their transfer function. The closure of predecessor nodes must be
//       finite. Should not contain duplicates.
// - isSuccessor:
//      A function f(x, y) which returns true if x depends on the output of y
//      and y does not depend on x. If this returns true then y will be
//      evaluated before x and the fixed point will be reached with fewer
//      recomputations.
// - maxNumUpdates:
//      (optional) the upper limit for how many times to evaluate the
//      transfer function at each node.
//
// Returns:
// - A list of eventual inputs and outputs of the transfer functions of the
//   nodes specified in `ns`
//
template<typename T, typename P>
List<InOutPair<typename T::Domain>> dataFlow(
    T& context,
    List<typename T::Node> ns,
    P isSuccessor,
    typename T::Domain bottom,
    typename T::Domain initialInput,
    UInt maxNumUpdates = std::numeric_limits<UInt>::max())
{
    using Node = typename T::Node;
    using Domain = typename T::Domain;
    struct Info
    {
        Domain in;
        Domain out;
        bool dirty;
        UInt numUpdates;
    };

    // If we can't evaluate our transfer function, return the only possible value
    if(maxNumUpdates == 0)
        return List<InOutPair<Domain>>::makeRepeated(InOutPair<Domain>{initialInput, bottom}, ns.getCount());

    // Our algorithm state, `infos` tracks the input output of each node as
    // well as if it should be recomputed
    // TODO: We could prune known-stable nodes from this dictionary as we go...
    Dictionary<Node, Info> infos;
    std::priority_queue<Node, std::vector<Node>, P> workQueue(isSuccessor);

    // Construct our work queue
    List<Node> dependencies = ns;
    const Info initialInfo{bottom, bottom, true, 0};
    // Add our roots to the dependencies to explore
    for(auto n : dependencies)
    {
        infos.add(n, initialInfo);
        workQueue.push(n);
    }
    // Explore the dependencies, adding children as we go
    for(int i = 0; i < dependencies.getCount(); ++i)
    {
        auto n = dependencies[i];

        // For each of the predecessors of this node, if we haven't already
        // added it to infos, add it to that, the workQueue and the list of
        // dependencies to explore.
        bool hasNoPredecessors = true;
        for(const auto& p : context.predecessors(n))
        {
            hasNoPredecessors = false;
            if(infos.addIfNotExists(p, initialInfo))
            {
                dependencies.add(p);
                workQueue.push(p);
            }
        }
        // Is we have no predecessor nodes then we should use the initial input
        // the user passed us.
        if(hasNoPredecessors)
        {
            auto i = infos.tryGetValue(n);
            SLANG_ASSERT(i);
            i->in = initialInput;
        }
    }

    // Keep evaluating the transfer function the top of the work queue and
    // adding any changed children to the work queue until nothing is changing
    // any more
    //
    // This terminates because:
    // - The computation graph is finite
    // - Output values for each vertex stop changing
    //  - Either: maxNumUpdates is reached for a vertex
    //  - Or: Domain is a bounded lattice of finite height, and as such,all
    //        output values will eventually be absorbed into ⊤, the transfer
    //        function must be monotonic
    while(!workQueue.empty())
    {
        Node n = workQueue.top();
        workQueue.pop();
        auto& info = *infos.tryGetValue(n);
        info.dirty = false;

        const bool outputChanged = context.transfer(n, info.out, info.in);
        info.numUpdates++;

        // If nothing has changed, no need to update any other state
        if(!outputChanged)
            continue;

        for(auto s : context.successors(n))
        {
            auto* sInfo = infos.tryGetValue(s);
            // If this isn't in our dependency set, skip
            if(!sInfo)
                continue;
            // If this has reached the update limit, skip
            if(sInfo->numUpdates >= maxNumUpdates)
                continue;
            const bool inputChanged = sInfo->in.join(info.out);
            // If this input hasn't changed, skip
            if(!inputChanged)
                continue;

            // Add it to the queue if it isn't in the queue
            if(sInfo->dirty)
                continue;
            sInfo->dirty = true;
            workQueue.push(s);
        }
    }

    // Extract our results
    List<InOutPair<Domain>> ret;
    ret.reserve(ns.getCount());
    for(auto n : ns)
    {
        auto& sInfo = *infos.tryGetValue(n);
        ret.add(InOutPair<Domain>{std::move(sInfo.in), std::move(sInfo.out)});
    }
    return ret;
}
}
