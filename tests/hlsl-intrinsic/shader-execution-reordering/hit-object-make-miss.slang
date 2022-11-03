// hit-object-make-miss.slang

//DISABLE_TEST:SIMPLE: -target dxil -entry computeMain -stage compute -profile sm_6_5 -DNV_SHADER_EXTN_SLOT=u0 

//DISABLE_TEST:COMPARE_COMPUTE_EX:-slang -compute -dx12 -output-using-type -profile sm_6_5 -nvapi-slot u0 
//DISABLE_TEST(compute):COMPARE_COMPUTE:-d3d12 -output-using-type -use-dxil -profile sm_6_5 -render-feature ray-query
//DISABLE_TEST(compute):COMPARE_COMPUTE:-vk -output-using-type -render-feature ray-query

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    int idx = int(dispatchThreadID.x);

    RayDesc ray;  
    ray.Origin = float3(idx, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;
    
    HitObject hit = HitObject::MakeMiss(idx, ray);
    
    int r = int(hit.IsMiss());
    
    outputBuffer[idx] = r;
}
