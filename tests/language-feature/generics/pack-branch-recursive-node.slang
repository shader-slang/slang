//TEST:INTERPRET(filecheck=CHECK):

// Verify a terminating PackBranch-based recursive node chain evaluates
// correctly for concrete pack inputs.

interface INode
{
    __init();
    int sum();
}

struct EmptyNode : INode
{
    __init(){}
    int sum()
    {
        return 0;
    }
}

struct EvalNode<let Head : int, let each Tail : int> : INode
{
    int headValue;
    __init(){ headValue = Head; }
    __packBranch(Tail, EmptyNode, EvalNode<__first(Tail), __trimFirst(Tail)>) restLayers = {};

    int sum()
    {
        return headValue + restLayers.sum();
    }
}

void main()
{
    EvalNode<4> one = {};
    printf("%d\n", one.sum());
    // CHECK: 4

    EvalNode<4, 8, 16> three = {};
    printf("%d\n", three.sum());
    // CHECK: 28
}
