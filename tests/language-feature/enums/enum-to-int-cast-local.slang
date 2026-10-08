// Regression for #12048: an enum-typed *local* holding a compile-time-constant
// value is SSA-promoted and const-folded away, leaving a degenerate
// `CastEnumToInt(1 : UInt)` (operand and result both the tag type UInt). Because
// `calcRequiredLoweringPassSet` previously flagged `enumType` only on a live
// `IREnumType` — which is gone by then — `lowerEnumType` was skipped and the
// stray cast reached emit as an "unexpected IR opcode" (E99999) on every target.
// The direct-cast form `(uint)XYZ.One` (no local) does NOT reproduce, so keep
// the enum-typed local below.
//
// The SIMPLE legs exercise the direct-emit path that ICEs pre-fix; the CPU
// COMPARE_COMPUTE leg also validates the runtime value.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -shaderobj -output-using-type

enum XYZ : uint
{
    One = 1,
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    XYZ xyz = XYZ.One;

    // Implicit enum -> uint store: the reproducing shape. Pre-fix this ICEs at
    // emit (E99999), so the store never appears and the SIMPLE CHECKs fail.
    // HLSL: outputBuffer{{.*}} = 1U;
    // GLSL: outputBuffer{{.*}} = 1U;
    // BUF: 1
    outputBuffer[0] = xyz;
}
