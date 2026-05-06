// Verifies the split coverage pipeline:
// - after `prepareCoverageInstrumentation`, coverage remains as
//   `IncrementCoverageCounter` markers
// - after `materializeCoverageInstrumentation`, those markers are
//   lowered to the final runtime atomic form

//DIAGNOSTIC_TEST:SIMPLE(diag=PREP):-target cpp -stage compute -entry computeMain -trace-coverage -dump-ir-after prepareCoverageInstrumentation
//DIAGNOSTIC_TEST:SIMPLE(diag=MAT):-target cpp -stage compute -entry computeMain -trace-coverage -dump-ir-after materializeCoverageInstrumentation

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = 0;
    if (tid.x == 0)
        accum += 1;
    accum += 2;
}

// PREP: IncrementCoverageCounter

// MAT: atomicAdd
// MAT: rwstructuredBufferGetElementPtr
