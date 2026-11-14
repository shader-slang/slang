//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that use of empty statement after an `if` leads to a warning.

struct S {}

void test()
{
	if (1+1==2);
//CHECK:    ^ potentially unintended empty statement
//CHECK:    ^ potentially unintended empty statement at this location; use {} instead.
		return;
}
