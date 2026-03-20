//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute

// Verify a terminating PackBranch-based recursive node can still participate
// in the backend layout path without triggering the nesting guards.

interface INode
{
    __init();
    float sum();
}

struct EmptyNode : INode
{
    float headValue;

    __init()
    {
        headValue = 0.0;
    }

    float sum()
    {
        return headValue;
    }
}

struct EvalNode<let Head : int, let each Tail : int> : INode
{
    float headValue;
    __packBranch(Tail, EmptyNode, EvalNode<__first(Tail), __trimFirst(Tail)>) restLayers = {};

    __init()
    {
        headValue = Head;
    }

    float sum()
    {
        return headValue + restLayers.sum();
    }
}

struct Params<let Head : int, let each Tail : int>
{
    EvalNode<Head, Tail> node;
}

ParameterBlock<Params<4, 8, 16>> gParams;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    gParams.node.sum();
}

// CHECK: OpEntryPoint GLCompute
