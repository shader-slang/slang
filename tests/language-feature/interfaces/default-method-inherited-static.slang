// Regression test for shader-slang/slang#12814.
//
// A statically specialized generic calls a *default* method declared on a *base*
// interface (`IBase.combineInPlace`), where the concrete type conforms through a
// *derived* interface (`Value : IDerived : IBase`). The default-impl witness entry
// on the nested `Value : IBase` table must be synthesized against that base table,
// not the outer `Value : IDerived` table, otherwise the inner `IBase.combine`
// lookup resolves to null and IR generation asserts.
//
// The bug is a front-end / IR-generation defect and is target-independent: the
// SPIR-V and HLSL compile-only runs below reproduce the crash without a GPU (they
// only assert that codegen succeeds), while the `-cpu` compute run additionally
// validates the computed value, guarding against silent mis-resolution.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain

interface IBase
{
    This combine(This other);
    [mutating]
    void combineInPlace(This other)
    {
        this = combine(other);
    }
}

interface IDerived : IBase
{
}

struct Value : IDerived
{
    int value;
    __init(int value) { this.value = value; }
    This combine(This other) { return This(value + other.value); }
}

T combine<T : IBase>(T left, T right)
{
    left.combineInPlace(right);
    return left;
}

// SPV: OpEntryPoint
// HLSL: void {{.*}}computeMain

//TEST_INPUT: set resultBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> resultBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Value r = combine<Value>(Value(20), Value(22));
    resultBuffer[0] = r.value;
    // CHECK: 42
}
