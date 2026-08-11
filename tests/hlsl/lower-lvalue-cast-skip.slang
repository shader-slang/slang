// Companion to lower-lvalue-cast-gated.slang for issue #11917.
//
// When no out/inout implicit L-value cast is present (all `inout` arguments already match
// the parameter type), `lowerLValueCast` has nothing to lower. This is a plain baseline
// that must still compile and emit the direct `inout` call unchanged. Whether the pass is
// actually *skipped* here rather than run as a no-op is a compile-time-only property that
// is not observable in emitted output; that gated-vs-ungated equivalence is shown by the
// byte-identical revert-drill described in the PR, not by this test.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute

void addOne(inout int x)
{
    x = x + 1;
}

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    int v = 3;
    // `v` is already `int`, so the `inout` argument needs no implicit cast: no
    // InOutImplicitCast IR is produced and the gate flag stays false.
    addOne(v);
    outBuf[i] = v;
}

// Positive anchor guards against an empty-output vacuous match; the direct call carries no
// writeback conversion (there is no cast to lower here).
// CHECK: void main(
// CHECK-NOT: int(uint(
