// Verify that passing a struct-typed argument to abort() produces a clear
// diagnostic (E55211). A struct has no printf-style format specifier, and a
// nested struct inside the explicitly laid out abort message payload cannot
// be given correct member offsets without cloning it into a physical type,
// so only scalar and vector arguments are supported.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -capability abort -skip-spirv-validation
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target glsl -capability abort

struct Pos
{
    float x;
    float y;
    uint id;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Pos p = { 1.0f, 2.0f, tid.y };
    if (tid.y > 100)
    {
        abort("p.id=%u", p);
    }
}

//CHECK: E55211
