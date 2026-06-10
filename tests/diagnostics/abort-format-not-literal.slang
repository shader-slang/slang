// Verify that passing a non-literal format string to abort() produces a clear
// diagnostic (E55210) instead of silently emitting an empty abort message.

// The test is non-exhaustive because the leftover dynamic-string IR also triggers a
// pre-existing secondary error in SPIR-V emit (the same happens with printf), which
// is not what this test is about.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -capability abort -skip-spirv-validation
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target glsl -capability abort

void doAbort(NativeString s)
{
    abort(s);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x > 100)
    {
        // A data-dependent string never folds to a single literal.
        doAbort(tid.x > 200 ? "too big" : "way too big");
    }
}

//CHECK: E55210
