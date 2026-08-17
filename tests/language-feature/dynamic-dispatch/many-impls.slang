//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type -cpu

// Coverage test for the existential-aggregate dynamic-dispatch lowering path that the
// O(N^2) `specializeModule` / `TranslationContext::resolveInst` fix touches.
//
// This is a *correctness* test, not a perf guard: it verifies that dispatch still produces
// the right results through that path. The compile-time blowup is wall-clock only —
// reverting the cache leaves these outputs (and the emitted IR) unchanged — so the perf
// regression is guarded by the compile-perf suite (`tools/compile-perf`, `dynamic_dispatch`
// and `existential_aggregate` workloads), not here.
//
// An *array of existentials* (interface-typed elements) holding many distinct implementations
// dispatched in a loop forces construction of the N-member `set` / `TaggedUnion` types whose
// repeated resolution was quadratic. This deliberately differs from the switch-factory
// coverage in `dispatch-many-conformances.slang`.

// anyValueSize(16) is a safe upper bound for the boxed payload: S0..S7 are stateless, so
// the any-value just needs room for the type tag.
[anyValueSize(16)]
interface IShape
{
    int eval(int x);
}

// Each Si evaluates to Si::eval(x) = x*(i+1) + i, so at x = 10 the outputs are
// 10, 21, 32, 43, 54, 65, 76, 87 — i.e. CHECK line i corresponds to shapes[i].
struct S0 : IShape { int eval(int x) { return x * 1 + 0; } }
struct S1 : IShape { int eval(int x) { return x * 2 + 1; } }
struct S2 : IShape { int eval(int x) { return x * 3 + 2; } }
struct S3 : IShape { int eval(int x) { return x * 4 + 3; } }
struct S4 : IShape { int eval(int x) { return x * 5 + 4; } }
struct S5 : IShape { int eval(int x) { return x * 6 + 5; } }
struct S6 : IShape { int eval(int x) { return x * 7 + 6; } }
struct S7 : IShape { int eval(int x) { return x * 8 + 7; } }

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // An array of existentials populated with N distinct concrete implementations:
    // this is what builds the N-member set / tagged-union types.
    IShape shapes[8];
    shapes[0] = S0();
    shapes[1] = S1();
    shapes[2] = S2();
    shapes[3] = S3();
    shapes[4] = S4();
    shapes[5] = S5();
    shapes[6] = S6();
    shapes[7] = S7();

    for (int i = 0; i < 8; i++)
        outputBuffer[i] = shapes[i].eval(10);

    // CHECK: 10
    // CHECK: 21
    // CHECK: 32
    // CHECK: 43
    // CHECK: 54
    // CHECK: 65
    // CHECK: 76
    // CHECK: 87
}
