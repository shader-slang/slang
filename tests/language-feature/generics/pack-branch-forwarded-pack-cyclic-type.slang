//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Verify a PackBranch that forwards the same pack unchanged is rejected as
// a cyclic type when the recursive structure is concretely observed.

struct EmptyNode
{
    int marker;
    __init() { marker = 0; }
}

struct LoopNode<let each P : int>
/*CHECK:
       ^^^^^^^^ type contains cyclic reference
       ^^^^^^^^ type 'LoopNode' contains cyclic reference to itself.
*/
{
    int marker;
    __packBranch(P, EmptyNode, LoopNode<P>) next = {};
    __init() { marker = 1; }
}

RWStructuredBuffer<int> outBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    LoopNode<1> value = {};
    outBuffer[0] = value.marker;
}
