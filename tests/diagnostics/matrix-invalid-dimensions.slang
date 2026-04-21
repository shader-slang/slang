// Test that invalid matrix dimensions are rejected at type-construction time.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void test()
{
    matrix<float, 4, 5> m1;
//CHECK: error 38206

    matrix<float, 5, 4> m2;
//CHECK: error 38206
}
