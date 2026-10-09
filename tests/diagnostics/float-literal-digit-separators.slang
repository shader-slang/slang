// Digit separators (`_`) are accepted in every part of a floating-point literal.
// The test is exhaustive, so any diagnostic fails it; the values are checked by
// tests/front-end/float-literal-values-{dec,hex}.slang.

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
}
