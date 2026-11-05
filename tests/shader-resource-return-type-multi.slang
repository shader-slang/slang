//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target spirv -stage compute -entry testRWTexture

// Test multiple resource types that should be rejected

//CHECK: error[E38011]
[shader("compute")]
RWTexture2D<float> testRWTexture(uint3 threadID : SV_DispatchThreadID)
{
    RWTexture2D<float> tex;
    return tex;
}
