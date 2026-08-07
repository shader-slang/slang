// Regression test for issue #12284, guarding WHICH local overload the diagnostic names when the
// call site's module declares several applicable ones. The reported candidate must be the overload
// the call would resolve to without the import -- here `f(float)` -- so the suggested cast keeps the
// original behavior. Naming some other applicable overload would point the user at a function their
// code was never calling.
//
// The declaration order is load-bearing: `f(float)` is deliberately neither first nor last of the
// three, so naming it is inconsistent with any rule that selects by position instead of by
// conversion cost. `f(bool)` is applicable to a `half` argument as well -- it is what gets named if
// `f(float)` is removed -- so it is a real competitor rather than filler.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

import "overload-import-overrides-local-best-helper";

RWStructuredBuffer<float> outputBuffer;

int f(int x)
{
    return x * 5;
}

float f(float x)
/*CHECK:
      ^ 'func f(float) -> float' declared in this module is not used; add an explicit cast to call it
*/
{
    return x * 3.0f;
}

bool f(bool b)
{
    return b;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    half h = (half)tid.x;
    outputBuffer[0] = (float)f(h);
//CHECK:                     ^ warning E38208
//CHECK:                     ^ chose 'public func f(half) -> half' from imported module 'overload-import-overrides-local-best-helper', which is a better match for these arguments
}
