//TEST:INTERPRET(filecheck=CHECK):

int load<let N : int, each TIndex>(TIndex indices) where countof(TIndex) == N
{
    return N;
}

int typeDeclaredWitnessBar<let N : int, each I>() where countof(I) == N
{
    return N;
}

// This body is checked while `N` and `I` are still abstract. The call can only
// type-check if `typeDeclaredWitnessFoo` supplies a DeclaredVariadicPackCountWitness
// for the exact `countof(I) == N` proof required by `typeDeclaredWitnessBar`.
int typeDeclaredWitnessFoo<let N : int, each I>() where countof(I) == N
{
    return typeDeclaredWitnessBar<N, I>();
}

int valueDeclaredWitnessBar<let N : int, let each I : int>() where countof(I) == N
{
    return N;
}

// Value packs use the same declared-witness forwarding path as type packs, but
// the pack value is a `ConcreteIntValPack` only after specialization. The
// generic body below therefore exercises the abstract declared proof as well.
int valueDeclaredWitnessFoo<let N : int, let each I : int>() where countof(I) == N
{
    return valueDeclaredWitnessBar<N, I>();
}

int orientedCountWitnessBar<let N : int, each I>() where countof(I) == N
{
    return N;
}

struct OrientedCountWitness<each U>
{
    int inner<each T>(T args) where countof(T) == countof(U)
    {
        return orientedCountWitnessBar<countof(U), T>();
    }
}

struct OuterPackCount<let each N : int>
{
    int innerLeft<each TIndex>(TIndex indices)
        where TIndex == int
        where countof(TIndex) == countof(N)
    {
        return countof(TIndex);
    }
}

interface IParserBoundary {}
struct ParserBoundaryImpl : IParserBoundary {}

struct ParserBoundary<each V>
{
    int inner<T, each U>() where T : IParserBoundary where countof(U) == countof(V)
    {
        return countof(U);
    }
}

int typePackCountOverload<let N : int, each T>(T args) { return 0; }
int typePackCountOverload<let N : int, each T>(T args) where countof(T) == N { return 1; }

void main()
{
    printf("%d\n", load<3>(1, 2, 3));
    // CHECK: 3

    printf("%d\n", typeDeclaredWitnessFoo<5, int, float, bool, uint, double>());
    // CHECK: 5

    printf("%d\n", valueDeclaredWitnessFoo<5, 1, 2, 3, 4, 5>());
    // CHECK: 5

    OrientedCountWitness<int, float> oriented;
    printf("%d\n", oriented.inner(1, 2.0));
    // CHECK: 2

    OuterPackCount<1, 2, 3> outer;

    printf("%d\n", outer.innerLeft(1, 2, 3));
    // CHECK: 3

    ParserBoundary<int, float> parserBoundary;
    printf("%d\n", parserBoundary.inner<ParserBoundaryImpl, uint, double>());
    // CHECK: 2

    printf("%d\n", typePackCountOverload<3>(1, 2.0));
    // CHECK: 0
}
