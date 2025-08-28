//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -output-using-type -shaderobj
// Doesn't work on DX11 currently - locks up on binding
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -output-using-type -shaderobj
// Produces a different result on DX12 with DXBC than expected(!). So disabled for now
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -profile cs_6_0 -output-using-type -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -output-using-type -shaderobj

//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -output-using-type -shaderobj

//TEST_INPUT: RWTexture1D(format=R32Float, size=4, content = one, mipMaps = 1):name rwt1D
RWTexture1D<float> rwt1D;
//TEST_INPUT: RWTexture2D(format=R32Float, size=4, content = one, mipMaps = 1):name rwt2D
RWTexture2D<float> rwt2D;
//TEST_INPUT: RWTexture3D(format=R32Float, size=4, content = one, mipMaps = 1):name rwt3D
RWTexture3D<float> rwt3D;
//TEST_INPUT: RWTexture1D(format=RG32Float, size=4, content = one, mipMaps = 1):name rwt1D_float2
RWTexture1D<float2> rwt1D_float2;
//TEST_INPUT: RWTexture1D(format=RGBA32Float, size=4, content = one, mipMaps = 1):name rwt1D_float4
RWTexture1D<float4> rwt1D_float4;
//TEST_INPUT: RWTexture2D(format=RG32Float, size=4, content = one, mipMaps = 1):name rwt2D_float2
RWTexture2D<float2> rwt2D_float2;
//TEST_INPUT: RWTexture2D(format=RGBA32Float, size=4, content = one, mipMaps = 1):name rwt2D_float4
RWTexture2D<float4> rwt2D_float4;
//TEST_INPUT: RWTexture3D(format=RG32Float, size=4, content = one, mipMaps = 1):name rwt3D_float2
RWTexture3D<float2> rwt3D_float2;
//TEST_INPUT: RWTexture3D(format=RGBA32Float, size=4, content = one, mipMaps = 1):name rwt3D_float4
RWTexture3D<float4> rwt3D_float4;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;
    
    float val = 0.0f;
 
    // float texture operations
    val += rwt1D[idx];
    val += rwt2D[uint2(idx, idx)];
    val += rwt3D[uint3(idx, idx, idx)];
    
    rwt1D[idx] = idx;
    rwt2D[uint2(idx, idx)] = idx;    
    rwt3D[uint3(idx, idx, idx)] = idx;    
            
    val += rwt1D[idx];
    val += rwt2D[uint2(idx, idx)];
    val += rwt3D[uint3(idx, idx, idx)];
    
    // float2 texture operations for 1D
    float2 val2_1d = rwt1D_float2[idx];
    rwt1D_float2[idx] = float2(idx, idx);
    val2_1d = rwt1D_float2[idx];
    val += val2_1d.x;
    val += val2_1d.y;
    
    // float2 texture operations for 2D
    float2 val2 = rwt2D_float2[uint2(idx, idx)];
    rwt2D_float2[uint2(idx, idx)] = float2(idx, idx);
    val2 = rwt2D_float2[uint2(idx, idx)];
    val += val2.x;
    val += val2.y;
    
    // float2 texture operations for 3D
    float2 val2_3d = rwt3D_float2[uint3(idx, idx, idx)];
    rwt3D_float2[uint3(idx, idx, idx)] = float2(idx, idx);
    val2_3d = rwt3D_float2[uint3(idx, idx, idx)];
    val += val2_3d.x;
    val += val2_3d.y;
    
    // float4 texture operations for 1D
    float4 val4_1d = rwt1D_float4[idx];
    rwt1D_float4[idx] = float4(idx, idx, idx, idx);
    val4_1d = rwt1D_float4[idx];
    val += val4_1d.x;
    val += val4_1d.y;
    val += val4_1d.z;
    val += val4_1d.w;
    
    // float4 texture operations for 2D
    float4 val4 = rwt2D_float4[uint2(idx, idx)];
    rwt2D_float4[uint2(idx, idx)] = float4(idx, idx, idx, idx);
    val4 = rwt2D_float4[uint2(idx, idx)];
    val += val4.x;
    val += val4.y;
    val += val4.z;
    val += val4.w;
    
    // float4 texture operations for 3D
    float4 val4_3d = rwt3D_float4[uint3(idx, idx, idx)];
    rwt3D_float4[uint3(idx, idx, idx)] = float4(idx, idx, idx, idx);
    val4_3d = rwt3D_float4[uint3(idx, idx, idx)];
    val += val4_3d.x;
    val += val4_3d.y;
    val += val4_3d.z;
    val += val4_3d.w;
    
    outputBuffer[idx] = val;
}
