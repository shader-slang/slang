// column-major-with-row-major-operations.slang

// Metal/CPP/CUDA do not correctly deal with packing currently.
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-mtl -compute

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -dx12
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -emit-spirv-via-glsl

//TEST_INPUT:cbuffer(data=[1.0 2.0 3.0 0.0  4.0 5.0 6.0 0.0  7.0 8.0 9.0 0]):name matrixTestCBuf1
ConstantBuffer<row_major float3x3> matrixTestCBuf1;

//TEST_INPUT:cbuffer(data=[1.0 4.0 7.0 0.0  2.0 5.0 8.0 0.0  3.0 6.0 9.0 0.0]):name matrixTestCBuf2
ConstantBuffer<column_major float3x3> matrixTestCBuf2;

// struct float3x3{float3[3]
// {
//     float3 data1; 
//     float pad1;
//     float3 data2;
//     float pad2;
//     float3 data3;
//     float pad3;
// }

//TEST_INPUT:cbuffer(data=[1.0 2.0 3.0 0.0  4.0 5.0 6.0 0.0]):name NeedsPadding
cbuffer NeedsPadding
{
    float3 data1;
    // float pad1;
    float3 data2;
};

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name output
RWStructuredBuffer<uint> output;

bool floatCheck(float data, float valueToCheckFor)
{
    return data < (valueToCheckFor + 0.001) && data > valueToCheckFor - 0.001;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3x3 matrixTest1;
    matrixTest1 = matrixTestCBuf1;

    float3x3 matrixTest2;
    matrixTest2 = matrixTestCBuf2;

    output[0] = uint(true
            && floatCheck(matrixTest1[0][0], 1)
            && floatCheck(matrixTest1[0][1], 2)
            && floatCheck(matrixTest1[0][2], 3)
            && floatCheck(matrixTest1[1][0], 4)
            && floatCheck(matrixTest1[1][1], 5)
            && floatCheck(matrixTest1[1][2], 6)
            && floatCheck(matrixTest1[2][0], 7)
            && floatCheck(matrixTest1[2][1], 8)
            && floatCheck(matrixTest1[2][2], 9)

            && floatCheck(matrixTest2[0][0], 1)
            && floatCheck(matrixTest2[0][1], 2)
            && floatCheck(matrixTest2[0][2], 3)
            && floatCheck(matrixTest2[1][0], 4)
            && floatCheck(matrixTest2[1][1], 5)
            && floatCheck(matrixTest2[1][2], 6)
            && floatCheck(matrixTest2[2][0], 7)
            && floatCheck(matrixTest2[2][1], 8)
            && floatCheck(matrixTest2[2][2], 9)

            && floatCheck(data1[0], 1)
            && floatCheck(data1[1], 2)
            && floatCheck(data1[2], 3)
            && floatCheck(data2[0], 4)
            && floatCheck(data2[1], 5)
            && floatCheck(data2[2], 6)
        );
    //BUF: 1
}
