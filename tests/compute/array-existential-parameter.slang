// Test using existential shader parameter that is an interface array.

//TEST(compute):COMPARE_COMPUTE:-cpu
//TEST(compute):COMPARE_COMPUTE:-cuda
//TEST(compute):COMPARE_COMPUTE:-cuda -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

// NVVM: .visible .const .align 8 .b8 SLANG_globalParams[32]
// NVVM: .visible .entry computeMain()
// NVVM-COUNT-2: ld.global.u32
// NVVM: shl.b32
// NVVM: st.global.u32

[anyValueSize(8)]
interface IInterface
{
    int run(int input);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<int> gOutputBuffer;

struct Params
{
    IInterface values[2];
};

//TEST_INPUT:set gCb = new{ values: [ new MyImpl{ val: 1 } ] }
ConstantBuffer<Params> gCb;

//TEST_INPUT:set gCb2 = new{ values: [ new MyImpl{ val: 1 } ] }
ConstantBuffer<Params> gCb2;

[numthreads(4, 1, 1)]
void computeMain(uint3       dispatchThreadID : SV_DispatchThreadID)
{
    let tid = dispatchThreadID.x;

    let inputVal : int = int(tid);
    let outputVal = gCb.values[0].run(inputVal) + gCb2.values[0].run(inputVal);

    gOutputBuffer[tid] = outputVal;
}

//TEST_INPUT: globalExistentialType MyImpl
//TEST_INPUT: globalExistentialType __Dynamic

// Type must be marked `public` to ensure it is visible in the generated DLL.
export struct MyImpl : IInterface
{
    int val;
    int run(int input)
    {
        return input + val;
    }
};
