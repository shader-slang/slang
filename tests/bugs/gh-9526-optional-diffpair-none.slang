// Regression test for issue #9526:
// Compiler crash during specialization when passing `none` to `Optional<DifferentialPair<T>>`.
//
// The intent here is compilation-only: the shader is trivial and just ensures the frontend +
// specialization pipeline can handle passing `none` without materializing/specializing a dummy `T`.
//
// Compilation-only regression test: should not crash during specialization.
// Note: specify `-o` so slang-test doesn't treat SPIR-V text on stdout as a failure.
//TEST:SIMPLE: -target spirv -o gh-9526.spv

struct MyStruct : IDifferentiable
{
    int a;
    int b;
}

int processOptionalDiffPair(Optional<DifferentialPair<MyStruct>> s)
{
    if (s.hasValue)
    {
        int x = s.value.p.a + s.value.p.b;
        return x;
    }
    return 0;
}

RWStructuredBuffer<int> outBuffer;

[shader("compute")]
void computeMain()
{
    // "Has value" case (idiomatic): assign a `T` into `Optional<T>` to construct an optional-with-value.
    // Here `T` is `DifferentialPair<MyStruct>`.
    DifferentialPair<MyStruct> dp = diffPair(MyStruct(1, 2));
    Optional<DifferentialPair<MyStruct>> optWithValue = dp;
    outBuffer[0] = processOptionalDiffPair(optWithValue);

    // This used to crash during specialization.
    outBuffer[1] = processOptionalDiffPair(none);
}

