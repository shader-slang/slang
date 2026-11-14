//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):


float doSomething(float a)
{
    // Too large with become +inf
    a += 5e+40;
//CHECK: ^^^^^ float literal '5e+40' unrepresentable
    // Will be narrowed to 0
    a += 9e-50;
//CHECK: ^^^^^ smaller than the smallest representable value

    double b = 0.0f;

    // These shouldn't produce warning as they can fit
    b += 5e+40l;
    b += 9e-50l;

    // Cos these don't have l suffix they should also produce warnings
    // and produce -inf and 0
    b += -5e+40;
//CHECK:  ^^^^^ float literal '5e+40' unrepresentable
    b += -9e-50;
//CHECK:  ^^^^^ smaller than the smallest representable value

    return a + float(b);
}
