// Diagnostic: an `out` parameter with an INPUT-only system value
// must be rejected. Complements `sv-invalid-direction.slang` which
// covers the `inout` direction; this test pins the pure-`out` path
// through `validateSystemValueSemantic` in slang-check-shader.cpp
// (around L901-L961) where the OutModifier branch dispatches.
//
// SV_DispatchThreadID is read from the runtime; it cannot be
// written, so declaring it as `out` is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain

//CHECK: E30702
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(out uint3 tid : SV_DispatchThreadID)
{
    tid = uint3(0, 0, 0);
}
