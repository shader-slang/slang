//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression test for https://github.com/shader-slang/slang/issues/6703
//
// An associated `static const` requirement (associated constant) used in a
// signature-type position (e.g. `float[T::COUNT]`) must fold to the same
// compile-time constant as the value produced on the generic-substitution
// path. Before the fix, reaching the requirement from a signature-type
// position folded it before the conforming type's witness table was built,
// leaving a symbolic `WitnessLookupIntVal` that never compared equal to the
// literal produced elsewhere, so otherwise-valid array sizes were rejected
// (E30019 / E30523).

interface IAssocConst
{
    static const uint COUNT;
}

struct Data<let N : uint> : IAssocConst
{
    static const uint COUNT = N;
    float values[N];
}

struct Wrapper<T : IAssocConst>
{
    static const uint COUNT = T::COUNT;

    // `T::COUNT` here is reached from a signature-type position.
    uint sum(float[T::COUNT] input)
    {
        uint total = 0;
        for (uint i = 0; i < T::COUNT; ++i)
            total += (uint)input[i];
        return total;
    }
}

// `VALUE::COUNT` is reached from a global `static const let` initializer, a
// signature-type position (`makeInput`'s return type), and a generic-argument
// position (`Data<VALUE::COUNT>`) — the exact combination that failed in #6703.
static const let VALUE = Wrapper<Data<3>>();

float[VALUE::COUNT] makeInput()
{
    float[VALUE::COUNT] result;
    for (uint i = 0; i < VALUE::COUNT; ++i)
        result[i] = float(i + 1) * 100.0;
    return result;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    float[VALUE::COUNT] input = makeInput(); // signature-type + value-access paths
    Data<VALUE::COUNT> sample;               // generic-argument position
    sample.values = input;
    // CHECK: 600
    outputBuffer[0] = VALUE.sum(sample.values);
}
