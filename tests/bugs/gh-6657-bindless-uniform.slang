//TEST:REFLECTION(filecheck=CHECK_REFLECTION):-stage compute -entry computeMain -target cpp -no-codegen
//TEST:REFLECTION(filecheck=CHECK_REFLECTION):-stage compute -entry computeMain -target cuda -no-codegen
//TEST:REFLECTION(filecheck=CHECK_METAL):-stage compute -entry computeMain -target metal -no-codegen
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RUNTIME):-cpu -shaderobj -output-using-type

// Test that DescriptorHandle<T> in a structured buffer is correctly reflected
// and works at runtime on bindless targets

// Reflection test: On bindless targets, DescriptorHandle<T> is reflected as T
// Reflection test: Layout should match T (pointer size for Texture2D on bindless targets)
// Reflection test: On 64-bit CPU/CUDA, Texture2D is 8 bytes (pointer or cudaTextureObject_t)

// CPU/CUDA reflection test: Verify that the DescriptorHandle<T> has the size of T with uniform offset
// CHECK_REFLECTION: "name": "texture_handle"
// CHECK_REFLECTION: "kind": "resource"
// CHECK_REFLECTION: "baseShape": "texture2D"
// CHECK_REFLECTION: "offset": 16
// CHECK_REFLECTION: "size": 8

// CPU/CUDA reflection test: Verify that the followingData is at the expected offset
// CHECK_REFLECTION: "name": "followingData"
// CHECK_REFLECTION: "offset": 24

// Metal reflection test: Verify that texture resources use shaderResource binding
// CHECK_METAL: "name": "texture_handle"
// CHECK_METAL: "kind": "resource"
// CHECK_METAL: "baseShape": "texture2D"
// CHECK_METAL: "kind": "shaderResource"
// CHECK_METAL: "index": 0

// Metal reflection test: Verify that followingData still uses uniform offset
// CHECK_METAL: "name": "followingData"
// CHECK_METAL: "kind": "uniform"
// CHECK_METAL: "offset": 16

// Runtime test: Verify that struct layout with pointer-sized handle is correct
// CHECK_RUNTIME: 42
// CHECK_RUNTIME: 43

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 0 0 42 43], stride=4):name=inputData
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=output
StructuredBuffer<Data> inputData;
RWStructuredBuffer<uint> output;

struct Data
{
    float4 someData;
    Texture2D.Handle texture_handle;
    uint2 followingData;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Data d = inputData[tid.x];

    // Verify the struct layout by checking that followingData has the expected values
    // If the handle size is wrong, followingData will have incorrect values
    output[0] = d.followingData.x;
    output[1] = d.followingData.y;
}
