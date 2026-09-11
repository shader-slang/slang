// Exercises VM `Call` arguments that are narrower than their padded callee parameter slots.
// Reverse-mode `square` builds a call whose first argument is a 4-byte float feeding a callee slot
// padded to 8 bytes; `twice` and the two-input `mulAdd` cover the same call path with different
// argument layouts. All three must copy only the argument's own bytes and return the correct
// gradient rather than over-reading the padding.

//TEST:INTERPRET(filecheck=BC): -disasm
//TEST:INTERPRET(filecheck=CHECK):

[Differentiable]
float square(float x)
{
    return x * x;
}

[Differentiable]
float twice(float x)
{
    return x + x;
}

[Differentiable]
float mulAdd(float x, float y)
{
    return x * y + x;
}

int main()
{
    var dp = diffPair(3.0, 0.0);
    bwd_diff(square)(dp, 1.0f);
    // CHECK: dsquare: 6.000000
    printf("dsquare: %f\n", dp.d);

    var dp2 = diffPair(3.0, 0.0);
    bwd_diff(twice)(dp2, 1.0f);
    // CHECK: dtwice: 2.000000
    printf("dtwice: %f\n", dp2.d);

    var dx = diffPair(3.0, 0.0);
    var dy = diffPair(4.0, 0.0);
    bwd_diff(mulAdd)(dx, dy, 1.0f);
    // CHECK: dmulAdd: 5.000000 3.000000
    printf("dmulAdd: %f %f\n", dx.d, dy.d);

    return 0;
}

// The reverse-mode wrapper for `square` (`s_bwdProp_square`) is called with a 4-byte float as its
// first argument (a `copy.4` destination), the narrow-argument shape that over-read the padded
// callee slot; keep it present so the CHECK gradients stay meaningful. Anchor on the forward-apply
// wrapper (`s_apply_square`), which precedes the call in the same caller, so the captured register
// is the one this call actually passes.
// BC: {{.*}}, s_apply_square
// BC: copy.4 [[ARG0:ws:[0-9A-Fa-f]+]], {{.*}}
// BC: call {{.*}}, s_bwdProp_square, [[ARG0]],
