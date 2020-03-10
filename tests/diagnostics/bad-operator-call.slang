// bad-operator-call.slang
//DIAGNOSTIC_TEST:SIMPLE:

// Test that bad calls to operators produce reasonable diagnostic messages.

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