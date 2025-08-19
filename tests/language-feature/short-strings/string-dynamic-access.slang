
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -compute -shaderobj

// CHECK: 78
// CHECK-NEXT: 79
// CHECK-NEXT: 7A
// CHECK-NEXT: 77

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name output
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 gid : SV_DispatchThreadID)
{
	let str = "xyzw";
	uint n = gid.x % str.length;
	output[gid.x] = str[n];
}