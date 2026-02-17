//DIAGNOSTIC_TEST:SIMPLE(diag=diag):
// variable with `void` type

void foo()
{
	void a;
/*diag:
 ^ invalid type 'void'
 ^ invalid type 'void'.
*/
}
