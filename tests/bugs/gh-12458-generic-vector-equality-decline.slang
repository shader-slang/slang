// Companion to `gh-12458-generic-builtin-operator-fast-path.slang`: `vector<T,N> == vector<T,N>`
// for a generic `T : __BuiltinFloatingPointType` (or `__BuiltinIntegerType`/`__BuiltinLogicalType`)
// must decline the builtin-operator fast path and fall through to `glsl.meta.slang`'s
// `bool operator==<T:__BuiltinArithmeticType,N>(vector<T,N>, vector<T,N>)` (`[OverloadRank(15)]`,
// visible on every target regardless of GLSL scope), which reduces the comparison to a single
// `bool` via `all(equal(...))`. Real code (`tests/metal/math-vector.slang`'s generic
// `Test_Vector<T:__BuiltinFloatingPointType,N>`) relies on `vec == vec` type-checking as `bool`
// directly in a boolean context (e.g. `voidResult && zero == outFloat1`); widening the fast path
// to recognize generic element types without this decline made it fire here too, producing the
// raw per-component `vector<bool,N>` instead, which fails to coerce to `bool` and broke that test.
//
// This is a separate file from the main fast-path test, not an addition to it, because it uses
// `COMPARE_COMPUTE` rather than `INTERPRET`: the `all(equal(...))` lowering this case resolves to
// is not yet supported by the bytecode interpreter's codegen (`slangi` fails with "unimplemented:
// VM bytecode gen for inst" on this exact shape), an unrelated, pre-existing gap in that backend,
// not something this fix touches.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

bool eqVecT<T : __BuiltinFloatingPointType>(vector<T, 3> a, vector<T, 3> b)
{
    return a == b;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] =
        eqVecT<float>(vector<float, 3>(1.0, 2.0, 3.0), vector<float, 3>(1.0, 2.0, 3.0)) ? 1 : 0;
    outputBuffer[1] =
        eqVecT<float>(vector<float, 3>(1.0, 2.0, 3.0), vector<float, 3>(1.0, 2.0, 9.0)) ? 1 : 0;
    // CHECK: 1
    // CHECK-NEXT: 0
}
