//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#11454.
// A call to a `[nodiscard]`-marked function whose non-void result is
// discarded at statement level should produce a warning.
// Target-independent: this is a front-end check, so it needs no GPU/target.

[nodiscard]
int computeValue()
{
    return 42;
}

// Result used (returned) -> no warning.
int useResult()
{
    return computeValue();
}

void test()
{
    // Result discarded at statement level -> warning expected on this call.
    computeValue();
//CHECK: ignoring return value of function declared with '[nodiscard]' attribute
}
