//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Verify an ill-formed PackBranch that forwards the same pack unchanged is
// rejected as a cyclic type when concretely observed through `sizeof`.

struct EmptyNode
{
    __init() {}
}

struct LoopNode<let each P : int>
/*CHECK:
       ^^^^^^^^ type contains cyclic reference
       ^^^^^^^^ type 'LoopNode' contains cyclic reference to itself.
*/
{
    __packBranch(P, EmptyNode, LoopNode<P>) next = {};
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
