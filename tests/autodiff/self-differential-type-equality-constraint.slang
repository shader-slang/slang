//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type

interface IV : IDifferentiablePtrType{
	int get();
}

struct V : IV
{
	typealias Differential = This;

	int get()
	{
		return 12;
	}
}

int g<T:IV>(DifferentialPtrPair<T> obj)
	where T.Differential == T
{
	return obj.d.get();
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;
[shader("compute")]
void computeMain()
{
	V v = {};
	DifferentialPtrPair<V> p = {v, v};

	// BUFFER: 12
	outputBuffer[0] = g(p);
}
