// Test that invalid matrix dimensions are rejected at type-construction time.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void test()
{
    matrix<float, 4, 5> m1;
//CHECK: matrix row and column counts must be between 1 and 4

    matrix<float, 5, 4> m2;
//CHECK: matrix row and column counts must be between 1 and 4
}
