//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal

// Verify that recursive struct types inside a ParameterBlock are rejected
// by the front-end before reaching IR lowering. This exercises the
// validateStructuredBufferElementType / containsRecursiveType check for
// Metal targets and confirms the cycle-guard sentinel in getLoweredTypeInfo
// is not relied upon for correctness.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Node
{
    int data;
    RWStructuredBuffer<Node> children;
//CHECK:                     ^^^^^^^^ recursive type in structured buffer
//CHECK:                     ^^^^^^^^ structured buffer element type 'Node' contains recursive type references
};

ParameterBlock<Node> root;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Node n = root;
}
