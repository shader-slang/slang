//DISABLED_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Demo: Exhaustive mode failure - unannotated diagnostics
// Change DISABLED_DIAGNOSTIC_TEST to DIAGNOSTIC_TEST to run this test and see the error output

struct S {};

void test()
{
    if (1);
//CHECK   ^ empty statement
    // This test only annotates one diagnostic but two are produced
    // In exhaustive mode (default), this fails
}
