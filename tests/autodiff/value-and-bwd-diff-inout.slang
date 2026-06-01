// Q3: surfaced primal y reflects post-call mutation of inout DifferentialPair<T>
// parameters. The wrapped function mutates p inside its body; the y returned
// from value_and_bwd_diff is computed using the post-mutation value.

//TEST:INTERPRET(filecheck=CHECK):

[BackwardDifferentiable]
float withInout(inout DifferentialPair<float> p, float k)
{
    // Mutate p.p before computing the returned value.
    p = DifferentialPair<float>(p.p + 1.0, p.d);
    return p.p * k;
}

void main()
{
    DifferentialPair<float> dpP = DifferentialPair<float>(2.0, 0.0);
    DifferentialPair<float> dpK = DifferentialPair<float>(4.0, 0.0);

    // After mutation p.p == 3.0; surfaced y == 3 * 4 = 12.0
    float y = value_and_bwd_diff(withInout)(dpP, dpK, 1.0);

    // CHECK: y=12.0
    printf("y=%f\n", y);
}
