// void-type-invalid-usage.slang

// Test that void cannot be used as a variable type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void test()
{
    void x;
/*CHECK:
    ^^^^ error E30009
    ^^^^ invalid type 'void'
*/
}
