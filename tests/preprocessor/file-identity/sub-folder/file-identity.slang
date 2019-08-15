//TEST(smoke):SIMPLE:

#include "../b.h"
#include "../c.h"

#include "./../b.h"

float test(float x)
{
	return foo(x) + bar(x);
}