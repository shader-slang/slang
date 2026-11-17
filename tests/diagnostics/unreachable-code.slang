//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

// "Obviously" unreachable code.

int foo()
{
	return 1;

	// The following statements can never be reached.
	int x = 0;
/*CHECK:
 ^^^ unreachable code detected
*/
	return x;
}
