//DIAGNOSTIC_TEST:SIMPLE(diag=foo,non-exhaustive):

// Test that use of empty statement after an `if` leads to a warning.

struct S {};

void test()
{
    if (1+1==2);
//foo:         ^ potentially unintended empty statement

// the above comment matches the warning diagnostic
		return;
}
