//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Verify an ill-formed PackBranch that forwards the same pack unchanged is
// rejected as a cyclic type when concretely observed through `sizeof`.

struct EmptyNode
{
    __init() {}
}

struct LoopNode<let each P : int>
{
    __packBranch(P, EmptyNode, LoopNode<P>) next = {};
//CHECK:                                    ^^^^ maximum type nesting level exceeded
}

struct Params<let each P : int>
{
    LoopNode<P> node;
}

ParameterBlock<Params<1>> gParams;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uniform int* output)
{
    *output = sizeof(gParams.node);
}
