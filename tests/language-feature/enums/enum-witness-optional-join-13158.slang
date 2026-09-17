// Scope-pinning test for shader-slang/slang#13158. The enum->scalar common-type
// join runs at the ordinary-argument merge, which also reconciles constraints
// discovered while solving optional `where` clauses. A type conforming to
// `IA<E>` and `IB<int>` exposes `U = E` and `U = int` through two optional
// constraints; the enum decays to its `int` tag, so the two reconcile to `int`
// and the call type-checks. This locks that accepted behavior (it is a widening
// of generic inference: the same operands produce a "conflicting requirements
// 'E' and 'int'" error without the enum/scalar join).

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute

interface IA<T>
{
    T ga();
}
interface IB<T>
{
    T gb();
}

enum E : int
{
    A = 1,
}

struct S : IA<E>, IB<int>
{
    E ga() { return E::A; }
    int gb() { return 0; }
}

void pick<T, U>(T t) where optional T : IA<U> where optional T : IB<U> {}

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    S s;
    pick(s);
    output[0] = 0;
}

//CHECK: OpEntryPoint
