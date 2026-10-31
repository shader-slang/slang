//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target cuda -entry computeMain -stage compute

// Regression guard for https://github.com/shader-slang/slang/issues/10641.
//
// When a method satisfies an interface requirement only through a synthesized witness (here,
// because it has an extra defaulted parameter), the witness's parameters must keep the
// requirement's `groupshared` and `const`: the witness forwards them to the method, and a
// `groupshared` parameter's argument must itself name group-shared storage (E30711). The test is
// exhaustive with no annotations, so it passes only if this compiles without any diagnostic.

interface IFoo
{
    uint f(const groupshared uint a[8]);
    uint g(groupshared uint a[8]);
}

struct S : IFoo
{
    uint f(const groupshared uint a[8], uint x = 0)
    {
        return a[0] + x;
    }

    uint g(groupshared uint a[8], uint x = 0)
    {
        a[0] = x;
        return 0;
    }
}

groupshared uint sh[8];

uint useFoo<T : IFoo>(T t)
{
    return t.f(sh) + t.g(sh);
}

RWStructuredBuffer<uint> output;

[numthreads(8, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    sh[i] = i;
    S s;
    output[i] = useFoo(s);
}
