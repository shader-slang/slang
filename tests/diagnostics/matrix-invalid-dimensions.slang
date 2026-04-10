// Test that invalid matrix dimensions are rejected at type-construction time.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void test()
{
    matrix<float, 4, 5> m1;
//CHECK: ^^^^^^^^^^^^^^^^^^^ matrix dimension out of valid range

    matrix<float, 5, 4> m2;
//CHECK: ^^^^^^^^^^^^^^^^^^^ matrix dimension out of valid range
}
