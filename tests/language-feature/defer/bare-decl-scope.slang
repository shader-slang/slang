//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -stage compute -entry computeMain -target hlsl

// A bare (non-block) deferred declaration is scoped to the deferred statement,
// like `defer { uint i = 1; }`, so its name is not visible to sibling
// statements in the enclosing scope. See shader-slang/slang#12266.

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    defer uint i = 1;
    output[0] = i;
//CHECK:        ^ error E30015
//CHECK:        ^ undefined identifier 'i'.
}
