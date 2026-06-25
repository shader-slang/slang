//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type -cpu

// Regression test for the O(N^2) compile-time blowup in dynamic-dispatch
// specialization (`specializeModule` / `TranslationContext::resolveInst`).
//
// An interface with many implementations dispatched through a runtime-typed
// existential forces real witness-table dynamic dispatch. Before the fix,
// resolving the N-element witness-table / type set was re-done O(N) times per
// referencing instruction, making compilation quadratic in the number of
// implementations. This test exercises that path (and verifies the dispatch
// still produces correct results); the compile-time guard lives in the
// compile-perf suite.

[anyValueSize(16)]
interface IShape
{
    int eval(int x);
}

struct S0 : IShape { int eval(int x) { return x * 1 + 0; } }
struct S1 : IShape { int eval(int x) { return x * 2 + 1; } }
struct S2 : IShape { int eval(int x) { return x * 3 + 2; } }
struct S3 : IShape { int eval(int x) { return x * 4 + 3; } }
struct S4 : IShape { int eval(int x) { return x * 5 + 4; } }
struct S5 : IShape { int eval(int x) { return x * 6 + 5; } }
struct S6 : IShape { int eval(int x) { return x * 7 + 6; } }
struct S7 : IShape { int eval(int x) { return x * 8 + 7; } }

int dispatch(int id, int x)
{
    IShape s;
    switch (id)
    {
    case 0:  s = S0(); break;
    case 1:  s = S1(); break;
    case 2:  s = S2(); break;
    case 3:  s = S3(); break;
    case 4:  s = S4(); break;
    case 5:  s = S5(); break;
    case 6:  s = S6(); break;
    case 7:  s = S7(); break;
    default: s = S0(); break;
    }
    return s.eval(x);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = dispatch(0, 10);
    // CHECK: 10

    outputBuffer[1] = dispatch(1, 10);
    // CHECK: 21

    outputBuffer[2] = dispatch(2, 10);
    // CHECK: 32

    outputBuffer[3] = dispatch(7, 10);
    // CHECK: 87
}
