// value_and_bwd_diff with an aggregate (struct) result type.
// Verifies the surfaced primal R is a non-scalar IDifferentiable type.

//TEST:INTERPRET(filecheck=CHECK):

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

void main()
{
    DifferentialPair<float> dpA = DifferentialPair<float>(2.0, 0.0);
    DifferentialPair<float> dpB = DifferentialPair<float>(5.0, 0.0);

    Vec2.Differential seed;
    seed.x = 1.0;
    seed.y = 0.0;

    // Surfaced primal: x = 4.0, y = 7.0
    Vec2 v = value_and_bwd_diff(buildVec)(dpA, dpB, seed);

    // CHECK: v.x=4.0
    printf("v.x=%f\n", v.x);
    // CHECK: v.y=7.0
    printf("v.y=%f\n", v.y);
}
