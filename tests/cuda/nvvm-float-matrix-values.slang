//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm
//TEST:SIMPLE(filecheck=PTX):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o -

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float a = outputBuffer[0] + 7.0;
    float b = outputBuffer[1] + 2.0;

    float2x2 ma = float2x2(a, b, b, a);
    float2x2 mb = ma + 1.0;
    float2x2 ms = ma + mb;

    float2x2 selected;
    if (a > b)
        selected = ms;
    else
        selected = mb;

    outputBuffer[0] = mb[0][0];
    outputBuffer[1] = selected[1][1];

    // CHECK: 8.0
    // CHECK: 15.0
}

// PTX: .visible .entry computeMain
