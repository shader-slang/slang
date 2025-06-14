//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj 
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer

RWStructuredBuffer<float> outputBuffer;

interface IPrintable
{
    int getHash();
};

__generic<int N>
extension StringLiteral<N> : IPrintable
{
	int getHash() { return getStringHash<N>(this); }
}

extension String : IPrintable
{
	int getHash() { return getStringHash(this); }
}

int getHash(IPrintable p) { return p.getHash(); }

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;

	String str = "Hello World! String";
	int hash = getHash("Hello World! Literal");
	int hash2 = getHash(str);
	float inVal = float(tid) + (hash + hash2);

	outputBuffer[tid] = inVal;
}