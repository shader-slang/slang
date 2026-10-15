// Companion to `gh-12458-generic-builtin-operator-fast-path.slang`: pins autodiff correctness for
// the widened fast path specifically, not just its non-differentiable behavior.
//
// Before this fix, `a + b`/`a * b` inside a `[Differentiable]` generic function constrained on
// `T : __BuiltinFloatingPointType` fell through to full overload resolution, resolving to a
// stdlib `operator+`/`operator*` overload that got inlined before autodiff transformation ran.
// After this fix, the same expression takes the builtin-operator fast path and lowers directly to
// `kIROp_Add`/`kIROp_Mul`, which forward/backward-mode autodiff differentiates via
// `translateBinaryArith` in slang-ir-autodiff-fwd.cpp -- a path concrete `float` operands already
// exercised, but a generic `T` constrained this way did not reach before this PR. Both routes
// should compute the same mathematical derivative; this test exercises the new one directly
// rather than relying on that being incidentally true.

//TEST:INTERPRET(filecheck=CHECK):

[Differentiable]
T addT<T : __BuiltinFloatingPointType>(T a, T b) { return a + b; }

[Differentiable]
T mulT<T : __BuiltinFloatingPointType>(T a, T b) { return a * b; }

void main()
{
    DifferentialPair<float> a = diffPair(3.0, 1.0);
    DifferentialPair<float> b = diffPair(4.0, 0.5);

    // d(a + b) = da + db = 1.0 + 0.5
    var addResult = fwd_diff(addT<float>)(a, b);
    // CHECK: 7.000000 1.500000
    printf("%f %f\n", addResult.p, addResult.d);

    // d(a * b) = a * db + b * da = 3.0 * 0.5 + 4.0 * 1.0
    var mulResult = fwd_diff(mulT<float>)(a, b);
    // CHECK-NEXT: 12.000000 5.500000
    printf("%f %f\n", mulResult.p, mulResult.d);

    // dL/da = b, dL/db = a, for output gradient 1.0
    DifferentialPair<float> pa = diffPair(3.0, 0.0);
    DifferentialPair<float> pb = diffPair(4.0, 0.0);
    bwd_diff(mulT<float>)(pa, pb, 1.0);
    // CHECK-NEXT: 4.000000 3.000000
    printf("%f %f\n", pa.d, pb.d);
}
