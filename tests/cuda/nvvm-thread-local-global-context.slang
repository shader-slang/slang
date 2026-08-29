//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -Wno-30056
//TEST:SIMPLE(filecheck=PTX):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

static int accumulator = 7;

int accumulate(int value)
{
    accumulator += value;
    return accumulator;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    outputBuffer[index] = accumulate(index);

    // CHECK: 7
    // CHECK-NEXT: 8
    // CHECK-NEXT: 9
    // CHECK-NEXT: 10
}

// PTX: .visible .entry computeMain
// PTX-NOT: .global .align 4 .u32 accumulator
// PTX: st.global.u32
