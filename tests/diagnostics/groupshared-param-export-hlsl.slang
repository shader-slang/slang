// `export` keeps a function as a real boundary for HLSL just as `[noinline]` does, so a bare
// `groupshared` parameter on an `export` function is rejected on HLSL for the same reason: HLSL
// cannot pass thread-group-shared memory across a function boundary (shader-slang/slang#10641).
// `export` is HLSL-facing only here -- it does not drive the SPIR-V VariablePointers path -- so it
// triggers only the HLSL diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL,non-exhaustive):-target hlsl

static const uint N = 8;

export void writeSlot(uint tid, groupshared uint scratch[N])
{
    scratch[tid] = tid + 1;
}

// HLSL: 30708
