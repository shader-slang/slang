// Regression test for https://github.com/shader-slang/slang/issues/7857
//
// Companion to overload-argument-type-mismatch.slang. Exercises the per-candidate
// mismatch note (E40018) reporting a *non-zero* argument index and a member call
// (which flows through `MemberExpr`), so a regression in the `paramIndex`
// accounting or the member-lookup path would be caught.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Test {}

struct Widget
{
    // The first argument matches; the mismatch is on argument 1.
    float poke(float x, float2 y) { return 0; }
    float poke(float x, float3 y) { return 0; }
}

float test()
{
    Widget w;
    return w.poke(1.0, Test());
//CHECK: no overload for 'poke' applicable to arguments of type (float, Test)
}

// Both candidates report the mismatch at argument index 1.
//CHECK: candidate: func Widget.poke(float, float2) -> float
//CHECK: argument 1 does not match: expected 'vector<float,2>', got 'Test'
//CHECK: candidate: func Widget.poke(float, float3) -> float
//CHECK: argument 1 does not match: expected 'vector<float,3>', got 'Test'
