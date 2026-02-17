//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test block comment annotations

struct S {};

void test()
{
    if (true);
//CHECK:     ^ potentially unintended empty statement
//CHECK:     ^ potentially unintended empty statement at this location; use {} instead.
    if (true);
//CHECK:     ^ potentially unintended empty statement
//CHECK:     ^ potentially unintended empty statement at this location; use {} instead.
}
