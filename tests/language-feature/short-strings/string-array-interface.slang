
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -shaderobj
//TEST_DISABLED(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<uint> output;

// CHECK: 3D
// CHECK-NEXT: 43

__generic<typename Array : IArray<uint8_t>>
uint myTest(const in Array a, uint offset)
{
	uint n = uint(a.getCount());
	output[offset] = n;
	for(uint i = 0; i < n; ++i)
	{
		output[i + offset + 1] = uint(a[i]);
	}
	return offset + n + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 gid : SV_DispatchThreadID)
{
	uint i = 0;
	i = myTest("Check that string literals fit the IArray<uint8_t> interface.", i);
}