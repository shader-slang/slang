//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry testStructContainingResource

// Test that entry points cannot return structs containing resource types

struct ResourceContainer
{
    Texture2D<float4> texture;
    int value;
}

//CHECK: error 38010: entry point 'testStructContainingResource' cannot return type 'ResourceContainer' that contains resource types
[shader("compute")]
ResourceContainer testStructContainingResource(uint3 threadID : SV_DispatchThreadID)
{
    ResourceContainer result;
    result.value = 42;
    return result;
}