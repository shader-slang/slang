// An `extern` declaration and a definition of the same function must resolve to the definition
// regardless of the order the modules are imported in. This is the order that always worked; see
// extern-import-order-def-first.slang for the swapped order.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain

// No diagnostics are expected: `f` resolves to the definition.

import externdecl;
import externdef;

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = f(1);
}
