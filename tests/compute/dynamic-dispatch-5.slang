//TEST(compute):COMPARE_COMPUTE:-dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE:-vk -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cuda -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cuda -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

// NVVM: .visible .entry computeMain()
// NVVM: st.global.u32

// Test dynamic dispatch code gen for general `This` type.
[anyValueSize(8)]
interface IInterface
{
	int Compute(int inVal, This other);
};

int GenericCompute<T:IInterface>(int inVal, T obj, T other)
{
	return obj.Compute(inVal, other);
}

struct Impl : IInterface
{
    int base;
	int Compute(int inVal, This other) { return other.base + base + inVal; }
};
int test(int inVal)
{
    Impl obj1, obj2;
    obj1.base = 1;
    obj2.base = 2;
	return GenericCompute<Impl>(inVal, obj1, obj2);
}

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	int inVal = outputBuffer[tid];
	int outVal = test(inVal);
	outputBuffer[tid] = outVal;
}
