//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry testRWTexture

// Test multiple resource types that should be rejected

//CHECK: error 38010: entry point 'testRWTexture' cannot return a resource type 'RWTexture2D<float>'
[shader("compute")]
RWTexture2D<float> testRWTexture(uint3 threadID : SV_DispatchThreadID)
{
    RWTexture2D<float> tex;
    return tex;
}
