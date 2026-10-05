//TEST:COMPILE: tests/bugs/13306/struct-types.hlsl -Gec -target hlsl -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/struct-types.hlsl -Gec -target spirv -entry main -validate-ir
//TEST:SIMPLE_EX(filecheck=CHECK): tests/bugs/13306/struct-types.hlsl -Gec -target hlsl -entry main -DTRY_WRITES

// Copy eligibility must use substituted field types and inherited storage. Checking only a
// struct's own fields, or an unspecialized generic declaration, would miss these resources.
struct Box<T>
{
    T value;
};
struct ResourceBase
{
    Texture2D<float4> texture;
};
struct ResourceDerived : ResourceBase
{
    uint value;
};
struct DataBase
{
    uint value;
};
struct DataDerived : DataBase
{
    uint other;
};
uniform Box<uint> data;
uniform Box<Texture2D<float4>> resource;
uniform ResourceDerived inheritedResource;
uniform DataDerived inheritedData;
RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    data.value = tid.x;
    inheritedData.value++;
#ifdef TRY_WRITES
    resource.value = inheritedResource.texture;
    // CHECK: error[E30011]
    inheritedResource.value = 1;
    // CHECK: error[E30011]
#endif
    output[tid.x] = resource.value.Load(int3(0)) + inheritedResource.texture.Load(int3(0)) +
        float4(data.value + inheritedData.value + inheritedResource.value);
}
