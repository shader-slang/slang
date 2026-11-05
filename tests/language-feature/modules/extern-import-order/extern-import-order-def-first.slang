// An `extern` declaration and a definition of the same function must resolve to the definition
// regardless of the order the modules are imported in. Importing the definition before the
// `extern` declaration used to report an ambiguous call, because the extern comparison in
// `CompareLookupResultItems` was not symmetric.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain

// No diagnostics are expected: `f` resolves to the definition.

import externdef;
import externdecl;

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = f(1);
}
