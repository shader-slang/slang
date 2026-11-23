//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Demo: Wrong message substring
// Change DISABLE_DIAGNOSTIC_TEST to DIAGNOSTIC_TEST to run this test and see the error output

struct S {};

void test()
{
    if (1);
//CHECK:  ^ this message doesn't match
    // The actual message is "empty statement", not "this message doesn't match"
}
