//DIAGNOSTIC_TEST:SIMPLE(diag=diag):
// use of undefined identifier

void foo()
{
	int a = b;
//diag:  ^ undefined identifier
//diag:  ^ undefined identifier 'b'.
}
