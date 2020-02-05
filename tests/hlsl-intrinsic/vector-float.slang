//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -use-dxil -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float4> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    float vf = idx * (1.0f / (4.0f));
     
    float3 f = float3(0.1f, vf, vf + 0.2f);
    
    // Vector specific
    float3 nv = normalize(f);
    
    
    // Operate over all values
    float3 ft = {};

    // fmod
    ft += float3(int3(((f % 0.11f) * 100) + 0.5));
    
    ft += sin(f);
    ft += cos(f);
    ft += tan(f);
    
    ft += asin(f);
    ft += acos(f);
    ft += atan(f);
    
    ft += atan2(f, 2.0);

#if 0
    {
        // Disabled because not supported on VK (glsl) in vector form
        float3 sf, cf;
        sincos(f, sf, cf);
        
        ft += sf;
        ft += cf;
    }
#endif     
     
#if 0     
    // Disabled because not supported on VK (glsl) in vector form
    ft += rcp(1.0 + f);
#endif

    ft += sign(f - 0.5);
    
    ft += saturate(f * 4 - 2.0);
    
    ft += sqrt(f);
    ft += rsqrt(1.0f + f);
    
    ft += exp2(f);
    ft += exp(f);
     
    ft += frac(f * 3);
    ft += ceil(f * 5 - 3);
    
    ft += floor(f * 10 - 7.01);

    ft += trunc(f * 7);
   
    ft += log(f + 10.0);
    ft += log2(f * 3 + 2);


    {
        float v[] = { 1, 10, 100, 1000 };
        ft += int3(log10(float3(v[idx] + vf) + 0.5f));
    }

       
    ft += abs(f * 4 - 2.0f);
    
    ft += min(0.5, f);
    ft += max(f, 0.75);

    ft += pow(0.5, f);

    ft += smoothstep(0.2, 0.7, f);
    ft += lerp(-100, 100, f);

    ft += clamp(f, 0.1, 0.3);

    ft += step(f, 0.5);

    int3 vi = asint(f - f) + idx;
    
    ft += float3(vi);   

    float4 result = float4(ft + nv, 0) + float4(vf);

    outputBuffer[idx] = result;
}