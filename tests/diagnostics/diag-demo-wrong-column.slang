//DISABLED_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Demo: Wrong column position
// Change DISABLED_DIAGNOSTIC_TEST to DIAGNOSTIC_TEST to run this test and see the error output

struct S {};

void test()
{
    if (1);
//CHECK        ^ empty statement
    // The diagnostic is actually at column 11, not column 16
}
