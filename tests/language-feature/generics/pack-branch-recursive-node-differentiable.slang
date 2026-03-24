//TEST:INTERPRET(filecheck=CHECK):

// Verify a terminating PackBranch-based recursive node remains valid in the
// abstract differentiable-type registration path.

interface INode : IDifferentiable
{
    __init();
    [Differentiable]
    float sum();
}

struct EmptyNode : INode
{
    float headValue;

    __init()
    {
        headValue = 0.0;
    }

    [Differentiable]
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

    [Differentiable]
    float sum()
    {
        return headValue + restLayers.sum();
    }
}

[Differentiable]
float evalNodeSum<let Head : int, let each Tail : int>(EvalNode<Head, Tail> node)
{
    return node.sum();
}

void main()
{
    EvalNode<4, 8, 16> three = {};
    printf("%f\n", evalNodeSum(three));
    // CHECK: 28.000000
}
