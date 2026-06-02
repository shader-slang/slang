// Diagnostic: an `inout` parameter with an input-only system value
// must be rejected. `SV_DispatchThreadID` is read from the runtime;
// shaders cannot write it back through an entry-point parameter.
//
// Complements `sv-output-direction-mismatch.slang`, which covers the
// pure-`out` path through system-value validation.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(inout uint3 tid : SV_DispatchThreadID)
//CHECK:                     ^^^ system value semantic 'SV_DispatchThreadID' cannot be used as output in 'compute' shader stage
{
    tid = uint3(0, 0, 0);
}
