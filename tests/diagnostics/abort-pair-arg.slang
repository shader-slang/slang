// Regression test for the `pair`-flavor branch of legalizeAbort in
// slang-ir-legalize-types.cpp: a struct containing both ordinary data and a
// resource legalizes into a (ordinary, special) pair, and legalizeAbort must
// forward only the ordinary half without crashing. The forwarded ordinary
// half is itself a struct, so the SPIR-V legalization pass then rejects it
// with the clean E55211 diagnostic instead of emitting an invalidly laid out
// message payload.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -capability abort -skip-spirv-validation

struct Mixed
{
    uint x;
    Texture2D tex;
}

Mixed gMixed;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x > 100)
    {
        abort("x=%u", gMixed);
    }
}

//CHECK: E55211
