//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// CHECK: %[[C0:[0-9A-Za-z_]+]] = OpSpecConstant %int 32
// CHECK: %[[C1:[0-9A-Za-z_]+]] = OpSpecConstant %int 2
// CHECK: %[[COP0:[0-9A-Za-z_]+]] = OpSpecConstantOp %int SDiv %[[C0]] %[[C1]]
// CHECK: %[[ARR_TYPE:[0-9A-Za-z_]+]] = OpTypeArray %float %[[COP0]]
// CHECK: %[[PT_TYPE:[0-9A-Za-z_]+]] = OpTypePointer Function %[[ARR_TYPE]]

[SpecializationConstant]
const int constValue0 = 32;

[SpecializationConstant]
const int constValue1 = 2;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

void func(out float buffer[constValue0 / constValue1])
{
    for (uint i = 0; i < constValue0 / constValue1; i++)
    {
        buffer[i] = i;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // This test checks that when we are using local constant that are computed from spec constant,
    // the size can be defined in global scope, and type check can pass the when we call `func`.
    const int localConst = constValue0 / constValue1;
    float buffer[localConst];
    // CHECK: OpVariable %[[PT_TYPE]] Function

    func(buffer);

    float temp = 0.0f;
    for (uint i = 0; i < localConst; i++)
    {
        temp += buffer[i] * 2;
    }

    // Result will be (0 + localConst-1) * localConst = 15 * 16 = 240
    outputBuffer[0] = temp;
    // BUF: 240
}
