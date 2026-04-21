// Test that invalid matrix dimensions are rejected at type-construction time.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void test()
{
    // Column count > 4
    matrix<float, 4, 5> m1;
//CHECK: matrix row and column counts must be between 1 and 4

    // Row count > 4
    matrix<float, 5, 4> m2;
//CHECK: matrix row and column counts must be between 1 and 4

    // Row count 0 (boundary value)
    matrix<float, 0, 2> m3;
//CHECK: matrix row and column counts must be between 1 and 4

    // Both dimensions invalid
    matrix<float, 5, 5> m4;
//CHECK: matrix row and column counts must be between 1 and 4

    // Valid boundaries (must not diagnose)
    matrix<float, 1, 1> ok1;
    matrix<float, 4, 4> ok2;
    matrix<float, 1, 4> ok3;
    matrix<float, 4, 1> ok4;
}
