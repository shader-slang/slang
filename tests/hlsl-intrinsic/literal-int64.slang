//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
// No support for int64_t on D3D11 (no sm 6.0)
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
// No support with Dx12 with dxbc. Needs SM6.0 + dxil
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -profile cs_6_0 -dx12 -use-dxil -shaderobj -render-feature hardware-device
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -render-feature int64
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj

// Note that the behavior we expect here, is a int without suffix is assumed to 
// be an int literal.
// That when making a conversion to a uint64_t, we expect first it to be widened
// to an int64_t, before becoming a uint64_t.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint64_t> outputBuffer;

[numthreads(7, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(int64_t(dispatchThreadID.x));
    
    uint64_t v = 0;
    
    if (idx == 0)
    {
        // Should be 0xffffffff
        v = -1;
    }
    else if (idx == 1)
    {
        // Should be 0xffffffff
        v = -1u;
    }
    else if (idx == 2)
    {
        // Should be 0x80000000
        v = 0x80000000u;
    }
    else if (idx == 3)
    {
        
        // Should be 0xffffffff80000000
        int64_t v0 = 0x80000000;
        v = v0;
    }
    else if (idx == 4)
    {   // Should be 0xffffffff80000000
        v = 0x80000000;
    }
    else if (idx == 5)
    {   // Should be 0xffffffff
        v = ~0; 
    }
    else 
    {   // Should be 0xffffffffffffffff
        v = ~0LL; 
    }
    
    outputBuffer[idx] = v;
}