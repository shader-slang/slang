// Diagnostic test for shader-slang/slang#12829: in language version 202c the
// generic-parameter-count tie-breaker is removed, so two applicable generic
// candidates that tie on conversion cost and are not distinguished by any other
// principled rule are now diagnosed as ambiguous instead of being silently
// resolved by required-generic-parameter count.
//
// Both calls below compile cleanly in legacy language versions (the count
// heuristic picks the fewer-parameter candidate); this test pins the 202c
// behavior. `non-exhaustive` is used so the candidate notes attached to each
// ambiguity do not each need their own annotation.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -std 202c

int choose<T : IFloat>(T value)
{
    return 1;
}
__generic<T : __BuiltinFloatingPointType, let N : int>
int choose(vector<T, N> value)
{
    return 2;
}

int pick<T : __BuiltinFloatingPointType>(vector<T, 3> value)
{
    return 3;
}
__generic<T : __BuiltinFloatingPointType, let N : int>
int pick(vector<T, N> value)
{
    return 4;
}

export int testChoose()
{
    return choose(float3(0.0));
}
//CHECK: ambiguous call to 'choose'

export int testPick()
{
    return pick(float3(0.0));
}
//CHECK: ambiguous call to 'pick'
