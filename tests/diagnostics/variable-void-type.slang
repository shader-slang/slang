//DIAGNOSTIC_TEST:SIMPLE(diag=diag):
// variable with `void` type

void foo()
{
	void a;
/*diag:
 ^^^^ error E30009
 ^^^^ invalid type 'void'.
*/
}
