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

FloatType makeFloatType(float f)
{
    FloatType m = { { f, f }, { f, f } };
    return m;
}

IntType makeIntType(int v)
{
    IntType m = { { v, v }, { v, v } };
    return m;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    float scalarF = idx * (1.0f / (4.0f));

    FloatType ft = {}; 

    FloatType f = { { scalarF + 0.01, scalarF * 0.2f}, { scalarF * 0.3f, scalarF + 0.1f}};
    
    // fmod
    ft += FloatType(IntType(((f % makeFloatType(0.11f)) * makeFloatType(100)) + makeFloatType(0.5)));
    
    ft += sin(f);
      
    ft += cos(f);
    ft += tan(f);
    
    ft += asin(f);
    ft += acos(f);
    ft += atan(f);
    
    ft += atan2(f, makeFloatType(2)); 

#if 0
    // This fails from DXC with a validation error(!)
    {
        FloatType sf, cf;
        sincos(f, sf, cf);
        
        ft += sf;
        ft += cf;
    }
#endif 
    
    ft += rcp(makeFloatType(1.0) + f);
    ft += FloatType(sign(f - makeFloatType(0.5)));
   
    ft += saturate(f * makeFloatType(4) - makeFloatType(2.0));
    
    ft += sqrt(f);
    ft += rsqrt(makeFloatType(1.0f) + f);
    
    ft += exp2(f);
    ft += exp(f);
                
    ft += frac(f * makeFloatType(3));
    ft += ceil(f * makeFloatType(5) - makeFloatType(3));
    
    ft += floor(f * makeFloatType(10) - makeFloatType(7));
    ft += trunc(f * makeFloatType(7));
     
   
    ft += log(f + makeFloatType(10.0));
    ft += log2(f * makeFloatType(3) + makeFloatType(2));

    {
        float scalarVs[] = { 1, 10, 100, 1000 };
        ft += FloatType(IntType(log10(makeFloatType(scalarVs[idx])) + makeFloatType(0.5f)));
    }
       
    ft += abs(f * makeFloatType(4) - makeFloatType(2.0f));
    
    ft += min(makeFloatType(0.5), f);
    ft += max(f, makeFloatType(0.75));

    ft += pow(makeFloatType(0.5), f);

    ft += smoothstep(makeFloatType(0.2), makeFloatType(0.7), f);
    ft += lerp(makeFloatType(-100), makeFloatType(100), f);

    ft += clamp(f, makeFloatType(0.1), makeFloatType(0.3));

    ft += step(f, makeFloatType(0.5));

    IntType vi = asint(f - f) + makeIntType(idx);
    
    ft += FloatType(vi);
    
    UIntType vu = asuint(f);
    ft += asfloat(vu);
   
    outputBuffer[idx] = calcTotal(ft);
}