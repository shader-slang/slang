// The silence half of issue #12284. E38208 is on by default, so "it does not fire for ordinary
// code" is as much a part of the contract as "it fires for the hazard" -- but only positive tests
// pin the latter, and a widened trigger would leave them all passing. Exhaustive mode is what makes
// this a guard: every diagnostic the compiler emits must be annotated, so a new E38208 at either
// call site below fails the test rather than going unnoticed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

import "overload-import-overrides-local-negative-helper";

RWStructuredBuffer<int> outputBuffer;

int exampleFunction(int val)
{
    return val * 3 + 5;
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Nothing to override: this module declares no `noCompetitor`, which is the shape of every
    // ordinary core-module or library call.
    outputBuffer[0] = (int)noCompetitor(tid.x);

    // The same-module candidate wins outright. An `int` literal keeps it an exact match, so the
    // case does not depend on int-literal conversion-cost constants that could shift.
    outputBuffer[1] = exampleFunction(7);
}
