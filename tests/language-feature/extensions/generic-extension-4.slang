// Test that multiple functions from different extensions are properly linked through their respective witness table.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
interface IValuableImpl
{
	int getValue();
}

interface IValuable
{
	associatedtype Impl : IValuableImpl;
	Impl getImpl();
};

__generic <Int : __BuiltinIntegerType>
extension Int : IValuableImpl
{
	int getValue()
	{
		return 0;
	}
};

__generic <Float : __BuiltinFloatingPointType>
extension Float : IValuableImpl
{
	int getValue()
	{
		return 1;
	}
};

__generic <ValuableImpl : IValuableImpl>
extension ValuableImpl : IValuable
{
	typealias Impl = ValuableImpl;
	ValuableImpl getImpl()
	{
		return this;
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
	outputBuffer[0] = i.getImpl().getValue();

	// CHECK: 1
	outputBuffer[1] = f.getImpl().getValue();
}