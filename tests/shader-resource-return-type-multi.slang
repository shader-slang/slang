//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry testRWTexture

// Test multiple resource types that should be rejected

//CHECK: error 38011: entry point 'testRWTexture' cannot return type 'RWTexture2D<float>' that contains resource types
[shader("compute")]
RWTexture2D<float> testRWTexture(uint3 threadID : SV_DispatchThreadID)
{
    RWTexture2D<float> tex;
    return tex;
}
