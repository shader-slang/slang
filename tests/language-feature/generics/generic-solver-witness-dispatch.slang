//TEST:INTERPRET(filecheck=CHECK):

int needsNonEmpty<let each D : int>() where nonempty(D)
{
    return __first(D) + countof(D);
}

[ForwardDifferentiable]
float square(float x)
{
    return x * x;
}

float useFwdDiff(float x)
{
    let result = fwd_diff(square)(DifferentialPair<float>(x, 1.0));
    return result.d;
}

void main()
{
    printf("%d\n", needsNonEmpty<5, 7>());
    // CHECK: 7

    printf("%f\n", useFwdDiff(3.0));
    // CHECK: 6.0
}
