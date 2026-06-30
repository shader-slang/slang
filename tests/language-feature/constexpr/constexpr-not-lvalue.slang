// Test that `constexpr` variables are not lvalues and cannot be assigned to.
// Regression test for the lvalue rule in slang-check-decl.cpp.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

constexpr int x = 5;

[numthreads(1, 1, 1)]
void computeMain()
{
    constexpr int y = 10;
    //CHECK: left of '=' is not an l-value
    x = 6;
    //CHECK: left of '=' is not an l-value
    y = 11;
}
