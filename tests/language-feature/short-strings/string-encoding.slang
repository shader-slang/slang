
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name output
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 gid : SV_DispatchThreadID)
{
	Array<uint, 3> c = {'文', '字', '列'};
	let str = "文字列";

	// CHECK: 0
	output[0] = (str[0] - c[0]);
	// CHECK: 0
	output[1] = (str[1] - c[1]);
	// CHECK: 0
	output[2] = (str[2] - c[2]);
	// CHECK: 3
	output[3] = str.length;
}