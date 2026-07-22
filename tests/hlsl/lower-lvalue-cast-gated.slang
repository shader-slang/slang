// Regression for https://github.com/shader-slang/slang/issues/11917.
//
// `lowerLValueCast` is gated on `RequiredLoweringPassSet::lValueCast`. This test locks in
// that the gate FIRES when the module contains an out/inout implicit L-value cast: calling
// an `inout int` parameter with a `uint` argument makes the front end emit a
// `kIROp_InOutImplicitCast`, and the pass must rewrite it into a temporary of the callee
// type plus explicit casts around the call. No emitter has a case for the cast opcode, so
// if the gate predicate were too narrow (a false-negative) the un-lowered cast would reach
// the GLSL emitter and compilation would fail, and the CHECK lines below would not match.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute

void addOne(inout int x)
{
    x = x + 1;
}

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    uint u = 3;
    // `u` is a `uint` l-value passed to an `inout int` parameter: the front end wraps the
    // address in an InOutImplicitCast, which `lowerLValueCast` splits into a temporary `int`
    // that is written back to `u` after the call.
    addOne(u);
    outBuf[i] = u;
}

// The lowering produces the callee-typed temporary and casts around the call site. The
// cast opcode itself never reaches emission; the observable evidence that the pass ran is
// the explicit conversion writing the temporary back to the `uint` original.
// CHECK: void main(
// CHECK: int(uint(
