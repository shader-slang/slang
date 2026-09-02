// deep-inheritance-chain-witness-cache.slang

// Regression test for the `_specializeInterfaceInheritanceWitness` cache added for #12139: a
// deep, non-diamond-shared interface inheritance chain forces `_calcInheritanceInfo` to
// specialize the same base interface's witness repeatedly across separate calls while composing
// `S`'s transitive facets -- exactly the pattern the new cache memoizes. Checks the actual
// computed value (not just successful compilation), varied per thread, so a wrong or stale
// cached witness would show up as a wrong output rather than silently passing.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface I0
{
    int get();
}
interface I1 : I0
{}
interface I2 : I1
{}
interface I3 : I2
{}
interface I4 : I3
{}
interface I5 : I4
{}
interface I6 : I5
{}
interface I7 : I6
{}
interface I8 : I7
{}
interface I9 : I8
{}
interface I10 : I9
{}
interface I11 : I10
{}
interface I12 : I11
{}
interface I13 : I12
{}
interface I14 : I13
{}
interface I15 : I14
{}
interface I16 : I15
{}
interface I17 : I16
{}
interface I18 : I17
{}
interface I19 : I18
{}
interface I20 : I19
{}

struct S : I20
{
    int value;
    int get() { return value; }
}

// Constrained on the root of the chain, so satisfying `T : I0` for `T = S` walks the full
// 20-level inheritance chain and specializes each level's witness against `S`.
int use<T : I0>(T t)
{
    return t.get() * 2 + 1;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// CHECK: 1
// CHECK: 3
// CHECK: 5
// CHECK: 7

[numthreads(4, 1, 1)]
void computeMain(int3 tid: SV_DispatchThreadID)
{
    S s;
    s.value = tid.x;
    outputBuffer[tid.x] = use(s);
}
