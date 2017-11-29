//TEST_IGNORE_FILE:

// This file is a "library" used by the `rewriter.hlsl` test.
// It intentionally uses Slang features that can't be supported
// by naive source-to-source translation.

interface IHelper
{
	int help(int inVal);
}

struct MyHelper : IHelper
{
	int help(int inVal)
	{
		return 16 + inVal;
	}
};

__generic<H : IHelper>
int doTest(H helper, int inVal)
{
	return helper.help(inVal);
}

int test(int inVal)
{
	MyHelper helper;
	return doTest<MyHelper>(helper, inVal);
}
