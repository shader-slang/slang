// Behavioral test for shader-slang/slang#12829: the generic-parameter-count
// tie-breaker in `compareOverloadCandidateSpecificity` is retained for legacy
// language versions but removed in 202c.
//
// The legacy run exercises the count heuristic directly (fewer required generic
// parameters wins). The 202c run only exercises calls that stay unambiguous once
// the count heuristic is gone: an `[OverloadRank]`-resolved tie (proving tied
// candidates now fall through to the OverloadRank rule) and a
// non-generic-vs-generic call (which is separated earlier by conversion cost and
// must NOT regress into an ambiguity). The now-ambiguous plain cases are checked
// by the companion diagnostic test generic-overload-count-tiebreak-ambiguous.slang.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECKLEGACY):-cpu -output-using-type -DLEGACY_RUN
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK202C):-cpu -output-using-type -std 202c

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

int chooseRanked<T : IFloat>(T value)
{
    return 1;
}
[OverloadRank(1)]
__generic<T : __BuiltinFloatingPointType, let N : int>
int chooseRanked(vector<T, N> value)
{
    return 2;
}

int doThing(int a)
{
    return 10;
}
int doThing<T>(T a)
{
    return 20;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int i = 0;
#ifdef LEGACY_RUN
    // Legacy: the count heuristic prefers the candidate with fewer required
    // generic parameters, so the broad `IFloat` overload and the fixed-size
    // `vector<T,3>` overload win.
    outputBuffer[i++] = choose(float3(0.0));
    // CHECKLEGACY: 1
    outputBuffer[i++] = pick(float3(0.0));
    // CHECKLEGACY: 3
#endif

    // Legacy: the count heuristic fires before OverloadRank, so `IFloat` (fewer
    // params) still wins. 202c: the count heuristic is gone, so the tie falls
    // through to OverloadRank and the ranked `vector<T,N>` overload wins.
    outputBuffer[i++] = chooseRanked(float3(0.0));
    // CHECKLEGACY: 1
    // CHECK202C: 2

    // Non-generic-vs-generic is separated earlier by conversion cost (binding to
    // an unconstrained generic parameter costs more than the exact non-generic
    // match), so it stays unambiguous and picks the non-generic overload in both
    // language versions.
    outputBuffer[i++] = doThing(3);
    // CHECKLEGACY: 10
    // CHECK202C: 10
}
