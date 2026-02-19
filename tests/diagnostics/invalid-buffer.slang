//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv

Buffer b0;
/*diag:
^^^^^^ generic type used without arguments
^^^^^^ generic type 'Buffer' used without argument
*/

Buffer<> b1;
/*diag:
^^^^^^ cannot specialize generic
^^^^^^ cannot specialize generic 'Buffer' with the provided arguments.
*/

[numthreads(1,1,1)]
void computeMain()
{
}
