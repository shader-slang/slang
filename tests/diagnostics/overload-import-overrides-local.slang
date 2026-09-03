// Regression test for issue #12284. Conversion cost is compared before any scope criterion, so an
// exact-match import outranks a same-module candidate that needs a conversion.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

import "overload-import-overrides-local-helper";

RWStructuredBuffer<int> outputBuffer;

int exampleFunction(int val)
/*CHECK:
    ^^^^^^^^^^^^^^^ 'func exampleFunction(int) -> int' declared in this module is not used; add an explicit cast to call it
*/
{
    return val * 3 + 5;
}

[numthreads(256, 1, 1)]
[shader("compute")]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = exampleFunction(tid.x);
//CHECK:              ^^^^^^^^^^^^^^^ warning E38208
//CHECK:              ^^^^^^^^^^^^^^^ chose 'public func exampleFunction(uint) -> uint' from imported module 'overload-import-overrides-local-helper', which is a better match for these arguments
}
