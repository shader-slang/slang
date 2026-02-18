//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -mtl -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<int> gOutput;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0]):name=lightData
ByteAddressBuffer lightData;

//TEST_INPUT: ubuffer(data=[0 0  1 0]):name=lightsDesc
StructuredBuffer<LightDescriptor> lightsDesc;

//TEST_INPUT: ubuffer(data=[2]):name=lightsGlobal
StructuredBuffer<LightGlobal> lightsGlobal;

interface ILight
{
    int eval(int x);
}

struct DomeLight : ILight
{
    float3x3 transform;
    float intensity;
    uint flags;
    DescriptorHandle<Texture2D<float4>> hdrTexture;
    DescriptorHandle<SamplerState> linearSampler;

    int eval(int x) { return x + 1; }
}

enum LightType : uint
{
    Point = 0,
    Dome = 1,
}

struct LightDescriptor
{
    LightType lightType;
    uint offset;
}

struct LightGlobal
{
    int numLights;
}

Optional<DomeLight> getFirstDomeLight()
{
    for (int i = 0; i < lightsGlobal[0].numLights; ++i)
    {
        LightDescriptor ld = lightsDesc[i];
        if (ld.lightType == LightType.Dome)
        {
            ILight light = lightData.Load<DomeLight>(ld.offset);
            if (let dome = light as DomeLight)
                return dome;
        }
    }
    return none;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    int result = 0;
    if (let dome = getFirstDomeLight())
    {
        result = dome.eval(tid);
    }

    gOutput[tid] = result;
}

// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 4
