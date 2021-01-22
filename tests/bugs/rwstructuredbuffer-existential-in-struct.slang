//TEST(compute):COMPARE_COMPUTE:-dx11 -shaderobj

[anyValueSize(8)]
interface IMaterial
{
    float eval();
}

struct MaterialImpl : IMaterial
{
    float eval() { return 0.0f; }
};

struct Parameters
{
    uint ordinary;

//TEST_INPUT: entryPointExistentialType MaterialImpl
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):name=params.materials
    RWStructuredBuffer<IMaterial> materials;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=params.result
    RWStructuredBuffer<float> result;
};

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID, uniform Parameters params, uniform int id)
{
    uint tid = dispatchThreadID.x;
    float rs = params.materials[0].eval();
    params.result[tid] = rs;
}
