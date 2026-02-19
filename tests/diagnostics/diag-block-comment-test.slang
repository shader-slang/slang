//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test block comment annotations

struct S {};

void test()
{
    if (1);
/*CHECK:
        ^ implicit conversion
          ^ empty statement
*/
    if (2);
/*CHECK:
        ^ implicit conversion
          ^ empty statement
*/
}
