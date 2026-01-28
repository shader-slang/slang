//DIAGNOSTIC_TEST:SIMPLE(diag=foo):

// Test that use of empty statement after an `if` leads to an warning.

struct S {};

void test()
{
    if (1+1==2);
//foo          ^ empty statement

// the above comment should match on the output: tests/diagnostics/rich-diag-test.slang(9): warning 20101: potentially unintended empty statement at this location; use {} instead.
		return;
}
