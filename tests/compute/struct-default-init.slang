// struct-default-init.slang
//TEST(compute):COMPARE_COMPUTE: -shaderobj
//TEST(compute):COMPARE_COMPUTE: -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE: -cuda -shaderobj
//TEST(compute):COMPARE_COMPUTE: -cuda -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

// NVVM: .visible .const .align 8 .b8 SLANG_globalParams[16]
// NVVM: .visible .entry computeMain()
// NVVM-COUNT-3: shl.b32
// NVVM: st.global.u32

struct Test
{
	int a = 0;
	int b = 1;
	int c = 0;
	int d = 1 + 1;
}

int test(int inVal)
{
	Test myArray[4] = {
		{ 3, 4, 5, 6 },
		{ 7, 8, 9, },
		{ 10, 11 },
		{ 12, }
	};

	Test t = myArray[inVal];
	return t.a * 4096
	     + t.b * 256
	     + t.c * 16
	     + t.d;
}

//TEST_INPUT:ubuffer(data=[9 9 9 9], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;

	int inVal = int(tid);
	int outVal = test(inVal);

	outputBuffer[tid] = outVal;
}
