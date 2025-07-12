
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -compute -shaderobj

// CHECK: 48
// CHECK-NEXT: 65
// CHECK-NEXT: 6C
// CHECK-NEXT: 6C
// CHECK-NEXT: 6F
// CHECK-NEXT: 20

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<uint> output;

extension RWStructuredBuffer<uint>
{
	void print<int N>(const in ShortString<N> str, uint offset = 0)
	{
		for(uint i = 0; i < N; ++i)
		{
			this[offset + i] = str[i];
		}
	}
};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 gid : SV_DispatchThreadID)
{
	output.print("Hello World!");
}