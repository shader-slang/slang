// entry-point-uniform-params.slang

// Confirm that `uniform` parameters on
// entry points are allowed, and work as expected.

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute

struct Signs
{
	int a;
}

struct Stuff
{
	int b;
}

struct Things
{
	int c;
}

// A shader parameter at global scope should be assigned
// a register/binding before any related to the entry point.

//TEST_INPUT:cbuffer(data=[1 0 0 0]):dxbinding(0),glbinding(0)
ConstantBuffer<Signs> signs;

[numthreads(4, 1, 1)]
void computeMain(
//TEST_INPUT:cbuffer(data=[2 0 0 0 3 0 0 0]):dxbinding(1),glbinding(1)
	uniform Stuff  stuff,
	uniform Things things,

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(2),out
	uniform RWStructuredBuffer<int> outputBuffer,

	uint3 dispatchThreadID : SV_DispatchThreadID)
{
	int tid = dispatchThreadID.x;

	int val = 0;
	val = val*16 + signs.a;
	val = val*16 + stuff.b;
	val = val*16 + things.c;
	val = val*16 + tid;

	outputBuffer[tid] = val;
}