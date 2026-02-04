//DISABLED_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Demo: Substring not found anywhere
// Change DISABLED_DIAGNOSTIC_TEST to DIAGNOSTIC_TEST to run this test and see the error output

struct S {};

void test()
{
    if (1);
//CHECK this substring doesn't exist
    // This substring doesn't appear in any diagnostic
}
