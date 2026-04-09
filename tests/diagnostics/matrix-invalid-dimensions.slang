// Test that invalid matrix dimensions and element types are rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void test()
{
    matrix<float, 4, 5> m1;
//CHECK: ^^^^^^^^^^^^^^^^^^^ matrix dimension out of valid range

    matrix<float, 5, 4> m2;
//CHECK: ^^^^^^^^^^^^^^^^^^^ matrix dimension out of valid range

    matrix<uint8_t, 4, 4> m3;
//CHECK: ^^^^^^^^^^^^^^^^^^^^^ matrix element type not supported

    matrix<uint64_t, 2, 2> m4;
//CHECK: ^^^^^^^^^^^^^^^^^^^^^^ matrix element type not supported

    matrix<int16_t, 2, 2> m5;
//CHECK: ^^^^^^^^^^^^^^^^^^^^^ matrix element type not supported
}
