//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv

Buffer b0;
/*diag:
^^^^^^ generic type 'Buffer' used without argument
*/

Buffer<> b1;
/*diag:
^^^^^^ wrong number of generic arguments
^^^^^^ generic 'Buffer' expects 2 generic argument(s) but 0 were provided
*/

[numthreads(1,1,1)]
void computeMain()
{
}
