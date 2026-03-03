//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

Tuple t0;
/*diag:
^^^^^ generic type 'Tuple' used without argument
^^^^^ generic type 'Tuple' used without argument
*/

// This should be OK, compiling down to a zero-sized type
Tuple<> t1;

[numthreads(1,1,1)]
void computeMain()
{
}
