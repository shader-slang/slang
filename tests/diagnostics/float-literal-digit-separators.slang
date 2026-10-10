// Digit separators (`_`) are accepted in the integer part, fraction and exponent of decimal
// and hexadecimal floating-point literals. The test is exhaustive, so any diagnostic on these
// spellings fails it; the values are checked by tests/front-end/float-literal-values-{dec,hex}.slang.
// The integer spellings are those of tests/front-end/digit-separator-token-boundaries.slang; `0_7`
// is decimal, so it has no octal warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    float x;
    x = 1_000.5f;
    x = 1.000_5f;
    x = 1e1_0;
    x = 0_0.5;
    x = 0x1_0p3;
    x = 0x1.8p1_0;
    x = 0x1.8_0p1;

    int i;
    i = 1_0.x;
    i = 1_.x;
    i = 0_7;
    i = 0_19;
    i = 0x_FF;
    i = 0x1e_5;
    i = 1__0;
}
