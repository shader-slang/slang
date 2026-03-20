//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Verify a recursive PackBranch shape used through a method call on a
// ParameterBlock-backed value is rejected as a cyclic type instead of
// crashing during later specialization/lowering.

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
    void f() {*output = 1;}
}

ParameterBlock<LoopNode<1>> gParams;
uniform int* output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    gParams.f();
}
