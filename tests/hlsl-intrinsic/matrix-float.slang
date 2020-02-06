// NOTE we can't test on VK/gl at the moment because we don't support intrinsics over matrices on that target currently

//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -use-dxil -output-using-type
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef matrix<float, 2, 2> FloatType;
typedef matrix<int, 2, 2> IntType;
typedef matrix<uint, 2, 2> UIntType;

float calcTotal(vector<float, 2> v)
{
    return v.x + v.y;
}

float calcTotal(FloatType v) 
{ 
    return calcTotal(v[0]) + calcTotal(v[1]);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    float scalarF = idx * (1.0f / (4.0f));

    FloatType ft = {}; 

    FloatType f = { { scalarF + 0.01, scalarF * 0.2f}, { scalarF * 0.3f, scalarF + 0.1f}};
    
#if 0    
    
    // fmod
    ft += IntType(((f % 0.11f) * 100) + 0.5);
#endif
    
    ft += sin(f);
      
    ft += cos(f);
    ft += tan(f);
    
    ft += asin(f);
    ft += acos(f);
    ft += atan(f);
    
    FloatType f2 = { { 2, 2 }, { 2, 2} };
    
    ft += atan2(f, f2);


#if 0   
    {
        FloatType sf, cf;
        sincos(f, sf, cf);
        
        ft += sf;
        ft += cf;
    }
#endif     
    
    FloatType f1 = { { 1, 1 }, { 1, 1} };
    FloatType f0_5 = { { 0.5, 0.5}, { 0.5, 0.5} };
    
    ft += rcp(f1 + f);
    ft += FloatType(sign(f - f0_5));
   
   
#if 0      
    ft += saturate(f * 4 - 2.0);
    
    ft += sqrt(f);
    ft += rsqrt(1.0f + f);
    
    ft += exp2(f);
    ft += exp(f);
        
        
    ft += frac(f * 3);
    ft += ceil(f * 5 - 3);
    
    ft += floor(f * 10 - 7);
    ft += trunc(f * 7);
     
   
    ft += log(f + 10.0);
    ft += log2(f * 3 + 2);

    {
        float scalarVs[] = { 1, 10, 100, 1000 };
        float scalarV = scalarVs[idx];
        FloatType v = scalarV;
        ft += int(log10(v) + 0.5f);
    }
       
    ft += abs(f * 4 - 2.0f);
    
    ft += min(0.5, f);
    ft += max(f, 0.75);

    ft += pow(0.5, f);

    ft += smoothstep(0.2, 0.7, f);
    ft += lerp(-100, 100, f);

    ft += clamp(f, 0.1, 0.3);

    ft += step(f, 0.5);

    IntType vi = asint(f - f) + idx;
    
    ft += matrix<float, 2, 2>(vi);
    
    UIntType vu = asuint(f);
    ft += asfloat(vu);
#endif   
   
    outputBuffer[idx] = calcTotal(ft);
}