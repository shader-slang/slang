// groupshared.slang

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm
//TEST:SIMPLE(filecheck=CHECK_NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o -
// Not supported in LLVM: barriers are not currently available
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX: -llvm -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out, name=gBuffer
RWStructuredBuffer<int> gBuffer;

static const int A = 4;
static const int B = 1;
static const int THREAD_COUNT = A * B;

groupshared int gA[THREAD_COUNT];
int test(int val)
{
	gA[val] = val;
	GroupMemoryBarrierWithGroupSync();
	val = gA[val ^ 1];

/* TODO: once function-scope `static` works
	static groupshared int gB[THREAD_COUNT];

	gB[val] = val;
	GroupMemoryBarrierWithGroupSync();
	val = gB[val ^ 2];
*/

	return val;
}

[numthreads(THREAD_COUNT, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;

	int val = int(tid);
	val = test(val);
	gBuffer[tid] = val;
}

// At the suite's -O0 level, the global store remains in the entry while shared access and the
// barrier body remain in two ordinary helpers.
// CHECK_NVVM: st.global.u32
// CHECK_NVVM: .shared .align 4 .b8 {{.*}}[16]
// CHECK_NVVM: st.shared.u32
// CHECK_NVVM: call.uni
// CHECK_NVVM: ld.shared.u32
// CHECK_NVVM: bar.sync 0
