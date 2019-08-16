//TEST(smoke):SIMPLE:
//TEST(smoke):SIMPLE: -file-system load-file


#include "../b.h"
#include "../c.h"

#include "./../b.h"

float test(float x)
{
	return foo(x) + bar(x);
}