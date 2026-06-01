// value_and_bwd_diff with an aggregate (struct) result type.
// Verifies the surfaced primal R is a non-scalar IDifferentiable type.
// (Backward-diff wrappers are not supported by the bytecode interpreter, so this
// uses the CPU compute path like the other reverse-mode tests.)

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct Vec2 : IDifferentiable
{
    float x;
    float y;
}

[BackwardDifferentiable]
Vec2 buildVec(float a, float b)
{
    Vec2 v;
    v.x = a * a;
    v.y = b + a;
    return v;
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    DifferentialPair<float> dpA = DifferentialPair<float>(2.0, 0.0);
    DifferentialPair<float> dpB = DifferentialPair<float>(5.0, 0.0);

    Vec2.Differential seed;
    seed.x = 1.0;
    seed.y = 0.0;

    // Surfaced primal: x = 2*2 = 4.0, y = 5 + 2 = 7.0
    Vec2 v = value_and_bwd_diff(buildVec)(dpA, dpB, seed);

    outputBuffer[0] = v.x; // CHECK: 4.0
    outputBuffer[1] = v.y; // CHECK-NEXT: 7.0
}
