//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type -cpu

// Regression test for the O(N^2) compile-time blowup in dynamic-dispatch
// specialization (`specializeModule` / `TranslationContext::resolveInst`).
//
// An *array of existentials* (interface-typed elements) holding many distinct
// implementations and dispatched in a loop forces construction of the N-member
// witness-table / type `set` (and the `TaggedUnion` type built over it) whose
// repeated resolution was quadratic before the fix. This deliberately differs from
// the switch-factory coverage in `dispatch-many-conformances.slang`: it exercises
// the existential-aggregate lowering path, mirroring the `existential_aggregate`
// compile-perf workload that the fix also speeds up. The compile-time guard itself
// lives in the compile-perf suite; this test verifies the dispatch still produces
// correct results after the resolution cache.

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
