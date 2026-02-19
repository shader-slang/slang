//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S
{
    float foo : 10;
//CHECK:  ^^^ bit-field type must be integral
//CHECK:  ^^^ bit-field type (float) must be an integral type
    S bar : 10;
/*CHECK:
      ^^^ bit-field type must be integral
      ^^^ bit-field type (S) must be an integral type
*/
};

[numthreads(1, 1, 1)]
void computeMain()
{
}
