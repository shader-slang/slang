//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry main -stage compute

// The shader from shader-slang/slang#13011. `+` needs `T : IArithmetic`, which the optional
// constraint only inherits, so the unchecked use is diagnosed before `compute<NotFloat>` is
// specialized and code is emitted for an entry point.

struct NotFloat
{
    int x;
    __init(int v) { x = v; }
}

T compute<T>(T a, T b)
    where optional T : __BuiltinFloatingPointType
{
    return a + b;
//CHECK:     ^ no overload for '+'
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    NotFloat r = compute<NotFloat>(NotFloat(1), NotFloat(2));
    outBuf[0] = r.x;
}
