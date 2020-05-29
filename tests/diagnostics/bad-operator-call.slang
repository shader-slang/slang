// bad-operator-call.slang

//DIAGNOSTIC_TEST(windows):SIMPLE:

// Test that bad calls to operators produce reasonable diagnostic messages.

// Note: This test is currently Windows-only becase our Linux builds
// seem to print references to the stdlib code with paths that don't
// match the Windows build (which generated our baseline).

struct S {}

void test()
{
	int a;
	S b;

	a += b;

	a = a + b;

	a = ~b;

	vector<int, 4> c;
	vector<float, 3> d;

	a += c;

	c = a + c;

	d += c;

	d = c + d;
}