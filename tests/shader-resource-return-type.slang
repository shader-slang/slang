//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry test1

// Test that entry points cannot return resource types

//CHECK: error 38011: entry point 'test1' cannot return type 'Texture2D<int>' that contains resource types
[shader("compute")]
Texture2D<int> test1(uint3 threadID : SV_DispatchThreadID)
{
    Texture2D<int> tex;
    return tex;
}
