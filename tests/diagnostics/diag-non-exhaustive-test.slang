//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test non-exhaustive mode - only checks that annotations match
// Doesn't fail if there are extra diagnostics

struct S {};

void test()
{
    if (1);
//CHECK:  ^ empty statement
    // Note: implicit conversion warning is also produced but not annotated
    // In non-exhaustive mode, this is OK
}
