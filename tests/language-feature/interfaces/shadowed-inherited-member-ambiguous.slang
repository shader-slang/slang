//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):-target hlsl -entry main -stage compute

// A diamond of shadowing interfaces makes an inherited member name genuinely
// ambiguous with no single most-derived requirement. When `ILeft` and `IRight`
// each shadow `IBase.X` and `IDiamond` refines both, a nested access `t.X.<...>`
// rooted at `T : IDiamond` has two incomparable candidates (`ILeft.X`,
// `IRight.X`) and must be reported as ambiguous. In particular the dominated base
// requirement `IBase.X` must not become the resolved base (which would let
// `t.X.baseOnly()` compile against it).
//
// Regression test for shader-slang/slang#12513.

interface IInner
{
    int m();
    int baseOnly();
}
interface ILeftInner : IInner {}
interface IRightInner : IInner {}

interface IBase
{
    property X : IInner { get; }
}
interface ILeft : IBase
{
    property X : ILeftInner { get; }
}
interface IRight : IBase
{
    property X : IRightInner { get; }
}
interface IDiamond : ILeft, IRight {}

int useBaseOnly<T : IDiamond>(T t)
{
    return t.X.baseOnly();
//CHECK:     ^ ambiguous reference to 'X'
}

[numthreads(1, 1, 1)]
void main()
{
}
