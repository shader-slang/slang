// Verifies the split coverage pipeline:
// - after `prepareCoverageInstrumentation`, coverage remains as
//   `IncrementCoverageCounter` markers with assigned slot decorations
// - after `materializeCoverageInstrumentation`, those markers are
//   lowered to the final runtime atomic form
// - the lowered form is preserved both for CPU-style wrapped globals
//   and descriptor-backed direct buffers
// - the test shader uses generic specializations so the slot
//   decorations must survive cloning/specialization before the late
//   materialization pass runs

//DIAGNOSTIC_TEST:SIMPLE(diag=PREP):-target cpp -stage compute -entry computeMain -trace-coverage -dump-ir-after prepareCoverageInstrumentation
//DIAGNOSTIC_TEST:SIMPLE(diag=MAT_CPP):-target cpp -stage compute -entry computeMain -trace-coverage -dump-ir-after materializeCoverageInstrumentation
//DIAGNOSTIC_TEST:SIMPLE(diag=MAT_SPV):-target spirv-asm -stage compute -entry computeMain -trace-coverage -dump-ir-after materializeCoverageInstrumentation

__generic<let Bias : int>
uint genericHelper(uint value)
{
    uint accum = value;
    if (value == 0)
        accum += Bias;
    accum += Bias + 1;
    return accum;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = genericHelper<1>(tid.x);
    accum += genericHelper<2>(tid.x);
}

// PREP: IncrementCoverageCounter
// PREP: CoverageSlotDecoration

// MAT_CPP: atomicAdd
// MAT_CPP: rwstructuredBufferGetElementPtr
// MAT_SPV: atomicAdd
// MAT_SPV: rwstructuredBufferGetElementPtr
