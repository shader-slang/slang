// Verifies that the hidden coverage buffer is preserved through the
// late-materialization point even when the shader has no user-declared
// global resources. This is the case where the coverage buffer is the
// only global kept alive between the split prepare/materialize passes.

//DIAGNOSTIC_TEST:SIMPLE(diag=MAT):-target cpp -stage compute -entry computeMain -trace-coverage -dump-ir-after materializeCoverageInstrumentation

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = tid.x;
    if (accum == 0)
        accum += 1;
}

// MAT: atomicAdd
// MAT: rwstructuredBufferGetElementPtr
// MAT: __slang_coverage
