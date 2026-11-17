//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Companion executable regression test for
// https://github.com/shader-slang/slang/issues/13996.
// The diamond `IDerived : IBase<int>, IBase<float>` shares the lexical
// inheritance declaration `IBase<T> : IRoot`, so the inherited `IRoot`
// requirement is reachable through both generic applications. This test
// calls the inherited requirement through each application via generic
// constraints, and gives `IBase<T>` a `T`-dependent requirement so a witness
// that resolved through the wrong application would produce the wrong value
// rather than passing silently.

interface IRoot
{
    int read();
}

interface IBase<T> : IRoot
{
    int check(int p, T t);
}

interface IDerived : IBase<int>, IBase<float>
{
}

struct S : IDerived
{
    int read() { return 42; }
    int check(int p, int t) { return p * 10 + t; }
    int check(int p, float t) { return p * 100 + int(t); }
}

int viaInt<U : IBase<int>>(U u)
{
    // Reaches IRoot.read() and IBase<int>.check() through the IBase<int> application.
    return u.read() + u.check(2, 3);
}

int viaFloat<U : IBase<float>>(U u)
{
    // Reaches IRoot.read() and IBase<float>.check() through the IBase<float> application.
    return u.read() + u.check(2, 2.0);
}

RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    // read() = 42; check(2,3) = 23; check(2,2.0) = 202.
    output[0] = viaInt(s);    // 65
    output[1] = viaFloat(s);  // 244
}

// CHECK: 65
// CHECK: 244
