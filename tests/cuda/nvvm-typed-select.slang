//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -Wno-30056
//TEST:SIMPLE(filecheck=PTX):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    int2 values = int2(index, index + 1);
    bool2 condition = values > 1;
    bool2 whenTrue = values < 4;
    bool2 whenFalse = values == 0;
    bool2 selected = condition ? whenTrue : whenFalse;
    outputBuffer[index] = all(selected) ? 1 : 0;

    // CHECK: 0
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0
}

// PTX: .visible .entry computeMain
// PTX: st.global.u32
