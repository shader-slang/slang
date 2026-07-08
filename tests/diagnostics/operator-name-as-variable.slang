// Regression test for #11664: using an operator name (e.g. `operator+`) as the
// name of a variable was accepted silently and only surfaced as a confusing
// error at a later use site. It should be diagnosed at the declaration.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void computeMain()
{
    int operator+ = 10;
    //CHECK: E20020

    int operator*= = 20;
    //CHECK: E20020
}
