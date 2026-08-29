//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST:SIMPLE(filecheck=PTX):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

half2 chooseHalf2(half2 left, half2 right, bool chooseLeft)
{
    half2 selected;
    if (chooseLeft)
        selected = left;
    else
        selected = right;
    return selected;
}

half2 adjustHalf2(half2 value)
{
    return -(value + half2(1.0h, 2.0h));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float left = outputBuffer[0] + 7.0;
    float right = outputBuffer[1] + 2.0;
    int integerValue = int(outputBuffer[2]) + 3;

    half first = half(left);
    half second = half(integerValue);
    half2 pair = half2(first, second);
    half2 converted = half2(float2(right, float(integerValue + 1)));
    half2 result = adjustHalf2(chooseHalf2(pair, converted, left > right));
    bool2 compared = result < pair;
    float2 widened = float2(result);
    int2 integers = int2(result);
    half selectedLane = result[integerValue & 1];

    outputBuffer[0] = widened.x;
    outputBuffer[1] = float(integers.y);
    outputBuffer[2] = compared.x ? 1.0 : 0.0;
    outputBuffer[3] = float(selectedLane);

    // CHECK: -8.0
    // CHECK: -5.0
    // CHECK: 1.0
    // CHECK: -5.0
}

// PTX: .visible .entry computeMain
// PTX-DAG: st.global.f32
