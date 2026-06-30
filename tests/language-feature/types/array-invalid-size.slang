// array-invalid-size.slang

// Test that declaring an array with a negative size produces an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void test()
{
    int arr[-1];
//CHECK:     ^ array size must be non-negative
//CHECK:     ^ array size must be non-negative.
}
