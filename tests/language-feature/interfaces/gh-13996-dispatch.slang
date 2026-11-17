//TEST(compute,vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute,cuda):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=output

// Companion executable regression test for
// https://github.com/shader-slang/slang/issues/13996.
// The diamond `IDerived : IBase<int>, IBase<float>` shares the lexical
// inheritance declaration `IBase<T> : IRoot`, so the shared `IRoot`
// conformance of `S` must resolve to a single witness table and each
// application's own requirements must stay distinct. This test dispatches
// the `T`-dependent requirement `check` through both applications of the
// generic interface, so a witness that resolved through the wrong
// application would produce the wrong value rather than passing silently.

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

RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    IDerived d = s;
    // Dispatches IBase<int>.check() and IBase<float>.check() through their own
    // witness tables, selecting a different overload per application.
    output[0] = d.check(2, 3);
    output[1] = d.check(2, 2.0);
}

// CHECK: 23
// CHECK: 202
