//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -vk -shaderobj -output-using-type
//TEST:REFLECTION(filecheck=REFLECTION):-target spirv-asm -stage compute -entry computeMain

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=gInput
RWStructuredBuffer<float> gInput;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<float> gOutput;

//CHK:1.0
//CHK-NEXT:2.0
//REFLECTION: "bindlessSpaceIndex": 1

[Shader("compute")]
[NumThreads(2, 2, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    Texture2D<float4> texture = DescriptorHandle<Texture2D<float4>>(uint2(tid, 0));
    SamplerState sampler = DescriptorHandle<SamplerState>(uint2(tid, 0));
    gOutput[tid] = gInput[tid] + texture.Sample(sampler, float2(0.f,0.f)).x * gOutput[tid];
}
