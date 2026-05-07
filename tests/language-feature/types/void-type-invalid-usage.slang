// void-type-invalid-usage.slang

// Test that void cannot be used as a variable type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void test()
{
    void x;
/*CHECK:
    ^^^^ invalid type 'void'
    ^^^^ invalid type 'void'.
*/
}
