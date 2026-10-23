// Under slangi a plain %f/%e/%g must read a `double` at its full 8-byte width, not 4 (issue
// #12964). -9.0 is the witness: its low 32 bits are zero, so a 4-byte read of it prints 0.000000,
// making a width bug visible. mkd/mkf keep the values runtime operands.

//TEST:INTERPRET(filecheck=CHECK):

double mkd(double a, double b)
{
    return a * b;
}

float mkf(float a, float b)
{
    return a * b;
}

[Differentiable]
float sq(float x)
{
    return x * x;
}

int main()
{
    double d = mkd(3.0, -3.0);

    // CHECK: f=-9.000000
    printf("f=%f\n", d);
    // CHECK: lf=-9.000000
    printf("lf=%lf\n", d);
    // Exponent digit count is platform-dependent (e+00 vs e+000), so match it loosely.
    // CHECK: e=-9.000000e{{[+]0+}}
    printf("e=%e\n", d);
    // CHECK: le=-9.000000e{{[+]0+}}
    printf("le=%le\n", d);
    // CHECK: g=-9
    printf("g=%g\n", d);
    // CHECK: lg=-9
    printf("lg=%lg\n", d);

    float ff = mkf(3.0f, -3.0f);
    // CHECK: ff=-9.000000
    printf("ff=%f\n", ff);
    // A float with the `l` modifier must read 4 bytes (not 8) and promote to double.
    // CHECK: lff=-9.000000
    printf("lff=%lf\n", ff);

    // `dp.d` is a `float` field of a DifferentialPair, so its operand inherits the pair's wider
    // size — this exercises the emitter-side size pin (without it the field prints garbage).
    var dp = diffPair(3.0, 0.0);
    bwd_diff(sq)(dp, 1.0f);
    // CHECK: field=6.000000
    printf("field=%f\n", dp.d);

    return 0;
}
