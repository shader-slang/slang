// TEST:SIMPLE(filecheck=SPV): -target spirv

// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly

struct DoubleNested
{
    int4x3 matrix;
    int getMatVal(int i, int j) { return matrix[i][j]; }
}

struct Nested
{
    bool values[4];
    DoubleNested doubleNested;
    int getVal(int id) { return (int)values[0] + doubleNested.getMatVal(0, 1); }
}

struct Params
{
    Nested nested;

    int getVal(int id) { return nested.getVal(id) + nested.getVal(id + 1); }
}

// TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int4> outputBuffer;

// TEST_INPUT:set gParams = cbuffer(data=[1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1])
ConstantBuffer<Params> gParams;

// TEST_INPUT: set gDoubleNested = ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12])
uniform DoubleNested *gDoubleNested;

// CHECK: 9

[numthreads(1,1,1)]
void computeMain(int id: SV_DispatchThreadID)
{
    outputBuffer[0].xyz = gParams.getVal(id) + gDoubleNested.getMatVal(1, 1);
}

// SPV-NOT: OpCompositeConstruct
