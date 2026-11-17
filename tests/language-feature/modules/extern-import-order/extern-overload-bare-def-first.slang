// A bare reference to a name with a non-extern `int f(int)` and an `extern float f(float)` must
// stay ambiguous: they are different overloads, so `extern` must not make one of them win in
// `CompareLookupResultItems`. The result must not depend on the import order; this file imports
// `overloaddef` first, see extern-overload-bare-ext-first.slang for the swapped order.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry computeMain

import overloaddef;
import overloadext;

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let g = f;
//CHECK: ambiguous reference to 'f'
    outputBuffer[0] = 1.0f;
}
