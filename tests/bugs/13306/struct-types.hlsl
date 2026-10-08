//TEST:COMPILE: tests/bugs/13306/struct-types.hlsl -Gec -target hlsl -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/struct-types.hlsl -Gec -target spirv -entry main -validate-ir
//TEST:SIMPLE_EX(filecheck=CHECK): tests/bugs/13306/struct-types.hlsl -Gec -target hlsl -entry main -DTRY_WRITES

// Semantic checking must distinguish the fields of `Box<uint>` and `Box<Texture2D<float4>>`,
// including when either specialization is nested in another `Box`. It must also inspect
// inherited instance fields while excluding `static` fields from the value being copied.
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
struct DataWithStaticResource
{
    uint value;
    static Texture2D<float4> texture;
};
uniform Box<uint> data;
uniform Box<Texture2D<float4>> resource;
uniform Box<Box<uint>> nestedData;
uniform Box<Box<Texture2D<float4>>> nestedResource;
uniform ResourceDerived inheritedResource;
uniform DataDerived inheritedData;
uniform DataWithStaticResource dataWithStaticResource;
static Box<Box<uint>> staticNestedData;
static DataWithStaticResource staticDataWithStaticResource;
RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    data.value = tid.x;
    nestedData.value.value = tid.x + 1;
    inheritedData.value++;
    dataWithStaticResource.value++;
    staticNestedData = nestedData;
    staticDataWithStaticResource = dataWithStaticResource;
#ifdef TRY_WRITES
    resource.value = inheritedResource.texture;
    // CHECK: error[E30011]
    inheritedResource.value = 1;
    // CHECK: error[E30011]
    nestedResource.value.value = inheritedResource.texture;
    // CHECK: error[E30011]
#endif
    output[tid.x] = resource.value.Load(int3(0)) + inheritedResource.texture.Load(int3(0)) +
        nestedResource.value.value.Load(int3(0)) +
        float4(data.value + inheritedData.value + inheritedResource.value +
            staticNestedData.value.value + staticDataWithStaticResource.value);
}
