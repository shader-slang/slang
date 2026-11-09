//TEST:INTERPRET(filecheck=CHECK):

interface IFloatPolicy
{
    static const half halfValue;
    static const float floatValue;
    static const double doubleValue;
}

struct FloatPolicy<let N : int> : IFloatPolicy
{
    static const half halfValue = half(N) * half(0.5);
    static const float floatValue = float(N) * 0.25;
    static const double doubleValue = double(N) * 0.125;
}

interface IPolicyContainer
{
    associatedtype Policy : IFloatPolicy;
}

struct PolicyContainer<let N : int> : IPolicyContainer
{
    typealias Policy = FloatPolicy<N>;
}

float readPolicy<P : IFloatPolicy>()
{
    return float(P.halfValue) + P.floatValue + float(P.doubleValue);
}

float readContainer<C : IPolicyContainer>()
{
    return readPolicy<C.Policy>();
}

void main()
{
    printf("%.3f %.3f\n", readPolicy<FloatPolicy<2>>(), readContainer<PolicyContainer<3>>());
    // CHECK: 1.750 2.625
}
