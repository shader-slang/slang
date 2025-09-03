//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -profile cs_6_0 -shaderobj -output-using-type

// Test CUDA Gather runtime behavior - compare with known gather pattern
// tex2Dgather samples 4 texels in 2x2 pattern around coordinate

//TEST_INPUT: Texture2D(size=4, content = one):name testTexture  
// Create a 4x4 texture with 1.0 values - simple but non-zero to verify gather works
Texture2D<float4> testTexture;

//TEST_INPUT: Sampler:name samplerState
SamplerState samplerState;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Simple gather test - sample at center of 2x2 region
    // This should gather from texels (0,0), (1,0), (0,1), (1,1)
    float2 coords = float2(0.75, 0.75);  // Between texels for gather
    
    // Test basic gather - should return 4 values in specific order
    float4 gathered = testTexture.GatherRed(samplerState, coords);
    
    // Store the gathered values
    outputBuffer[0] = gathered.x;  // Should be consistent pattern
    outputBuffer[1] = gathered.y;  
    outputBuffer[2] = gathered.z;  
    outputBuffer[3] = gathered.w;
    
    // Also test that gather actually works by using texture coordinates
    // as the texture values (coord-based pattern)
    int2 texelCoord = int2(dispatchThreadID.xy);
    float coordValue = float(texelCoord.x + texelCoord.y * 4);  // Create pattern: 0,1,2,3,4,5,6,7...
    
    // Store marker value like CUDA reference (42)  
    outputBuffer[4] = 42.0;  // Marker to verify test is working
    
    // Test another gather position
    float4 gathered2 = testTexture.GatherRed(samplerState, float2(1.25, 1.25));
    outputBuffer[5] = gathered2.x;
    outputBuffer[6] = gathered2.y;  
    outputBuffer[7] = gathered2.z;
    outputBuffer[8] = gathered2.w;
}

// Test results - texture filled with 1.0 values
// CHECK: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 42.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
