
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<uint> output;

// CHECK: 3A
// CHECK-NEXT: 43

__generic<typename Array : IArray<uint>>
uint myTest(const in Array a, uint offset)
{
	uint n = uint(a.getCount());
	output[offset] = n;
	for(uint i = 0; i < n; ++i)
	{
		output[i + offset + 1] = a[i];
	}
	return offset + n + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 gid : SV_DispatchThreadID)
{
	uint i = 0;
	i = myTest("Check that string literals fit the IArray<uint> interface.", i);
}