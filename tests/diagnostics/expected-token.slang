//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
// expected one token, but got another

int foo()
{
	int a = 3 ]
//CHECK:   ^ unexpected token
//CHECK:   ^ unexpected ']', expected ';'
}
