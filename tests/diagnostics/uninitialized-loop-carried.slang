//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain

// Regression test for shader-slang/slang#11778: a local scalar read while uninitialized
// was not diagnosed when the read is "loop-carried" (the variable is accumulated across
// loop iterations). After SSA construction the scalar becomes a loop-header phi, and the
// uninitialized seed reaches the read only as the loop's initial phi-argument; the analysis
// previously ignored branch arguments and so missed the genuine first-iteration read.

// BUG CASE: `total` is never initialized. The first iteration of `total += a[i]` reads
// uninitialized data, so a warning is expected at that read. The post-loop `return total`
// is NOT flagged here: by the time control reaches it the loop has run on every reaching
// path, so it is left to the normal definite-assignment analysis (matching how a non-loop
// `x += k; return x;` warns only at the `+=`, not at the `return`).
float loop_carried(float a[4])
{
    float total;
    for (int i = 0; i < 4; i++)
        total += a[i];
//CHK:        ^^ E41016
//CHK:        ^^ use of uninitialized variable 'total'
    return total;
}

// NEGATIVE CASE: `total` is initialized before the loop, so the loop entry value is not
// uninitialized and no warning should be produced.
float loop_carried_initialized(float a[4])
{
    float total = 0.0f;
    for (int i = 0; i < 4; i++)
        total += a[i];
    return total;
}

[Shader("compute")]
[NumThreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
}
