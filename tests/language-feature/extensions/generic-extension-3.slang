// Test that multiple functions from different extensions are properly linked.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
interface IValuable
{
	int getValue();
};

__generic <Int : __BuiltinIntegerType>
extension Int : IValuable
{
	int getValue()
	{
		return 0;
	}
};

__generic <Float : __BuiltinFloatingPointType>
extension Float : IValuable
{
	int getValue()
	{
		return 1;
	}
};

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
	uint i = 0;
	float f = float(i) / float(10);	

	// CHECK: 0
	outputBuffer[0] = i.getValue();

	// CHECK: 1
	outputBuffer[1] = f.getValue();
}