//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):dxbinding(0),glbinding(0),out

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const int id = asint(dispatchThreadID.x);

    {
        int4 i4 = int4(id, id + 1, id + 2, id + 3);
        uint3 u3 = asuint(i4.xyz);
        int2 i2 = asint(u3.xy);
        uint u1 = asuint(i2.x);
        
        outputBuffer[id + 0] = int(u1);
    }
    
    {
        uint v = asuint(id);
        uint4 u4 = uint4(v, v + 1, v + 2, v + 3);
        int3 i3 = asint(u4.xyz);
        uint2 u2 = asuint(i3.xy);
        int i1 = asint(u2.x);
        
        outputBuffer[id + 4] = i1;
    }
    
    {
        // Make i4 holds id as floats so we know they are valid float values and not denormals
        int4 i4 = int4(asint(float(id)), asint(float(id + 1)), asint(float(id + 2)), asint(float(id + 3))); 
        float4 f4 = asfloat(i4);
        uint3 u3 = asuint(f4.xyz);
        float2 f2 = asfloat(u3.xy);
        uint u1 = asuint(f2.x);
        
        float f1 = asfloat(u1);
        int i1 = asint(f1);
        
        float f1_ = asfloat(i1);
        int i1_ = asint(f1_);
        
        outputBuffer[id + 8] = (int)asfloat(i1_); 
    }
}