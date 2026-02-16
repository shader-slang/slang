//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test block comment annotations

struct S {};

void test()
{
    if (1);
//CHECK:  ^ potentially unintended empty statement
    if (2);
//CHECK:  ^ potentially unintended empty statement
}
