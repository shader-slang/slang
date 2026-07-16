// Regression test for https://github.com/shader-slang/slang/issues/11730
//
// A braces initializer `{a, b}` that flattens a vector plus a scalar into a
// larger vector must be accepted when passed directly as a function-call
// argument, exactly as it already is for a variable-declaration initializer
// (`float3 c = {a, b}`). The argument form was previously rejected with
// `error 30019: type mismatch in expression`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// Encode a float3 as a single scalar so the flattened component order is observable.
float encode3(float3 v)
{
    return v.x * 100.0 + v.y * 10.0 + v.z;
}

// Encode a float2 as a single scalar.
float encode2(float2 v)
{
    return v.x * 10.0 + v.y;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float2 a = float2(1.0, 2.0);
    float b = 3.0;
    float2 a2 = float2(4.0, 5.0);

    // {float2, float} as a float3 argument: the float2 must flatten to its two
    // components, giving float3(1, 2, 3) -> 1*100 + 2*10 + 3.
    outputBuffer[0] = encode3({a, b});
    // CHECK: 123.0

    // {float2} as a float2 argument: float2(4, 5) -> 4*10 + 5.
    outputBuffer[1] = encode2({a2});
    // CHECK: 45.0
}
