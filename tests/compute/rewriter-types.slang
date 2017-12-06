//TEST_IGNORE_FILE:

struct MyHelper
{
	int val;
	RWStructuredBuffer<int> buf;
};

struct Other
{
	int val;	
};

int test(int inVal, MyHelper helper, Other other)
{
	return inVal + helper.val + helper.buf[inVal] + other.val;
}
