// Verifies the warning fires when the user's shader declares a global
// parameter named `__slang_coverage`. The IR coverage pass under
// `-trace-coverage` synthesizes its own buffer with that name; the
// user declaration is silently shadowed without this diagnostic.
//
// Regresses if a future refactor drops the reserved-name detection
// (e.g. renames `findUserDeclaredCoverageBuffer` without updating the
// `instrumentCoverage` call site, or stops surfacing the diagnostic
// through the linkage's sink).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -trace-coverage -profile cs_5_0

RWStructuredBuffer<uint> __slang_coverage;
//CHECK:                 ^^^^^^^^^^^^^^^^ warning E45100

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    __slang_coverage[0] = 1;
}
