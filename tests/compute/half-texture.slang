//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -profile cs_6_2
//TEST:CROSS_COMPILE: -target dxil-assembly -entry computeMain -profile cs_6_2

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=16):out
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: Texture2D(size=4):
RWTexture2D<half> halfTexture;
//TEST_INPUT: Texture2D(size=4):
RWTexture2D<half2> halfTexture2;
//TEST_INPUT: Texture2D(size=4):
RWTexture2D<half4> halfTexture4;

//TEST_INPUT: Sampler:
SamplerState 	s;

[numthreads(4, 4, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int2 pos = int2(dispatchThreadID.xy);
    float2 uv = pos * (1.0f / 3.0f);
    int2 pos2 = int2(3 - pos.y, 3 - pos.x);
    
#if 0 
    half  h = halfTexture.Sample(s, uv);
    half2 h2 = halfTexture2.Sample(s, uv);
    half4 h4 = halfTexture4.Sample(s, uv);
#else
    // CHECK: {{.*}} = OpImageRead
    // CHECK: {{.*}} = OpFConvert %half
    half h = halfTexture[pos2];
    // CHECK: {{.*}} = OpImageRead
    // CHECK: {{.*}} = OpFConvert %v2half
    half2 h2 = halfTexture2[pos2];
    // CHECK: {{.*}} = OpImageRead
    // CHECK: {{.*}} = OpFConvert %v4half
    half4 h4 = halfTexture4[pos2];
#endif
    // Store a results
    halfTexture[pos] = h2.x + h2.y;
    halfTexture2[pos] = h4.xy;
    halfTexture4[pos] = half4(h2, h, h);
    
    int index = pos.x + pos.y * 4;
    outputBuffer[index] = index;
}
