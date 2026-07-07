// A compute entry point whose numthreads size is itself a generic value
// parameter, used by smoke-test.py to exercise slang_wasm_compile_specialized_entry_point.
// Mirrors the fixture in tools/slang-unit-test/unit-test-generic-entrypoint.cpp:
// specializing `x` to different constants must produce different compiled code,
// which is how the smoke test proves the specialization argument actually took
// effect without needing to decode the target binary.

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(x, 1, 1)]
void computeMain<int x>(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = x * 100 + 7;
}

// A second entry point, generic over a type rather than a value, used to
// exercise slang_wasm_spec_args_add_type (the name-based specialization
// path via findTypeByName, distinct from the constant-expression path
// computeMain above exercises).
interface IValue
{
    int getValue();
}
struct ValueA : IValue
{
    int getValue() { return 1; }
}
struct ValueB : IValue
{
    int getValue() { return 2; }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainTyped<T : IValue>(uint3 tid : SV_DispatchThreadID)
{
    T value;
    outputBuffer[tid.x] = value.getValue();
}
