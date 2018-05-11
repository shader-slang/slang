//TEST:SIMPLE:

// An attempt to truncate a vector should be
// diagnosed with a wanring.

void test(float3 v) {}

void bad(float4 a)
{
	test(a);
}
