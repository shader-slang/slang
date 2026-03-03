//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry test1

// Test that entry points cannot return resource types

//CHECK: error[E38011]
[shader("compute")]
Texture2D<int> test1(uint3 threadID : SV_DispatchThreadID)
{
    Texture2D<int> tex;
    return tex;
}
