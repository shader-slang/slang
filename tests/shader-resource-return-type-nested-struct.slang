//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry testNestedStruct

// Test that entry points cannot return structs that transitively contain resource types

struct InnerStruct
{
    Texture2D<float4> texture;
}

struct OuterStruct
{
    InnerStruct inner;
    int value;
}

//CHECK: error 38010: entry point 'testNestedStruct' cannot return type 'OuterStruct' that contains resource types
[shader("compute")]
OuterStruct testNestedStruct(uint3 threadID : SV_DispatchThreadID)
{
    OuterStruct result;
    result.value = 42;
    return result;
}