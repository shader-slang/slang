//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
// expected one token, but got EOF

int foo()
{
	int a = 33
//CHECK: unexpected token
//CHECK: unexpected end of file, expected ';'
