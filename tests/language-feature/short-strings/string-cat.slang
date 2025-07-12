
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name output
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 gid : SV_DispatchThreadID)
{
	// Test that a adjacent string literals are concatenated.
	// There only one small issue here in the Visual Code extension (or in the language server):
	// When hovering over str, it is of type StringLiteral<10> as expected,
	// But it is shown only with value "dog", instead of the full sting.
	let str = "dog" " vs " "cat";
	
	// CHECK: A
	output[0] = str.length;
	// CHECK: 64
	output[1] = str[0];
	// CHECK: 20
	output[2] = str[3];
	// CHECK: 74
	output[3] = str[str.length-1];
}