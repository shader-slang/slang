// Digit separators (`_`) are accepted in the integer part, fraction and exponent of decimal
// and hexadecimal floating-point literals. The test is exhaustive, so any diagnostic on these
// spellings fails it; the values are checked by tests/front-end/float-literal-values-{dec,hex}.slang.

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
}
