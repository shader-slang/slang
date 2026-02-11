//DISABLED_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Demo: Diagnostic on wrong line
// Change DISABLED_DIAGNOSTIC_TEST to DIAGNOSTIC_TEST to run this test and see the error output

struct S {};

void test()
{
    int x = 5;
//CHECK ^ unused variable
    // No diagnostic is produced on this line
}
