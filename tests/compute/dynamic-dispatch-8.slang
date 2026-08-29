//TEST(compute):COMPARE_COMPUTE:-dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE:-vk -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cuda -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cuda -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

// NVVM: .visible .entry computeMain()
// NVVM: st.global.u32

// Test dynamic dispatch code gen for extential type parameters.

[anyValueSize(16)]
interface IInterface
{
	int Compute(int inVal);
};

int GenericCompute(IInterface obj, int inVal)
{
	return obj.Compute(inVal);
}

struct Impl : IInterface
{
    int base;
	int Compute(int inVal) { return base + inVal * inVal; }
};

int test(int inVal)
{
    Impl obj;
    obj.base = 1;
	return GenericCompute(obj, inVal);
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
