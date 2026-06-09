// Cross-target check: exercise the SPIRV emit path for value_and_bwd_diff
// (per learning 1780072266959 — emit pipeline must be exercised on at least
// one test that runs under spirv-asm).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;

[BackwardDifferentiable]
float square(float x)
{
    return x * x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    DifferentialPair<float> p = DifferentialPair<float>(3.0, 0.0);
    float y = value_and_bwd_diff(square)(p, 1.0);
    outputBuffer[0] = y;
    outputBuffer[1] = p.d;
}

// Just check that the shader compiles to SPIRV without errors and that
// the ValueAndBwdDiffFuncType resolves through the IR pipeline.
// CHECK: OpEntryPoint
