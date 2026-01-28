//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test block comment annotations

struct S {};

void test()
{
    if (1);
/*CHECK
          ^ empty statement
*/
    if (2);
/*CHECK
          ^ empty statement
*/
}
