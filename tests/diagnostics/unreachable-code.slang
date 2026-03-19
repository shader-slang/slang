//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

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
