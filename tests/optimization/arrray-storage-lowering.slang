//TEST:SIMPLE(filecheck=SPV): -target spirv

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

struct Nested
{
    bool values[16];

    bool getVal(int id) { return values[id]; }
}

struct Params
{
    Nested nested;

    bool getVal(int id) { return nested.getVal(id); }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:set gParams = cbuffer(data=[1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1])
ConstantBuffer<Params> gParams;

// CHECK: 1

[numthreads(1,1,1)]
void computeMain(int id: SV_DispatchThreadID)
{
    outputBuffer[0] = gParams.getVal(id);
}

// SPV-NOT: OpCompositeConstruct