//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//DISABLED_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-wgpu -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);
    
    half f = half(idx) * 1.0h / 4.0h;

    half ft = 0.0h;
    
    // fmod
    ft += trunc(((f % 0.11h) * 100) + 0.5h);
    
    ft += sin(f);
    ft += cos(f);
    ft += tan(f);
    
    ft += asin(f);
    ft += acos(f);
    ft += atan(f);
    
    ft += length(f - 0.5h);
    
    ft += atan2(f, 2.0h);
    
    {
        half sf, cf;
        sincos(f, sf, cf);
        
        ft += sf;
        ft += cf;
    }
    
    ft += rcp(1.0h + f);
    ft += half(sign(f - 0.5h));
    
    ft += saturate(f * 4 - 2.0h);
    
    ft += sqrt(f);
    ft += rsqrt(1.0h + f);
    
    ft += exp2(f);
    ft += exp(f);
        
        
    ft += frac(f * 3);
    ft += ceil(f * 5 - 3);
    
    ft += floor(f * 10 - 7);
    ft += trunc(f * 7);
     
   
    ft += log(f + 10.0h);
    ft += log2(f * 3 + 2);

    {
        half v[] = { 1, 10, 100, 1000 };
        ft += trunc(log10(v[idx]) + 0.5h);
    }
       
    ft += abs(f * 4 - 2.0h);
    
    ft += min(0.5h, f);
    ft += max(f, 0.75h);

    ft += pow(0.5h, f);

    ft += smoothstep(0.2h, 0.7h, f);
    ft += lerp(-100, 100, f);


    ft += clamp(f, 0.1h, 0.3h);

    ft += step(f, 0.5h);

    int vi = asint16(f - f) + idx;
    
    ft += half(vi);

    ft += ldexp(23.2h, f);

    uint16_t vu = asuint16(f);
    ft += asfloat16(vu);
    
    outputBuffer[idx] = int(ft * 16);
}
