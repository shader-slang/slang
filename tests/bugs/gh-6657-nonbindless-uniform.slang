//TEST:REFLECTION(filecheck=CHECK_REFLECTION):-target spirv -stage compute -entry computeMain
//TEST:REFLECTION(filecheck=CHECK_REFLECTION):-target hlsl -stage compute -entry computeMain -profile sm_6_6
//TEST:REFLECTION(filecheck=CHECK_REFLECTION):-target glsl -stage compute -entry computeMain
//TEST:REFLECTION(filecheck=CHECK_REFLECTION):-target wgsl -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RUNTIME):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_RUNTIME):-dx12 -shaderobj -output-using-type -profile sm_6_6

// Test that DescriptorHandle<T> in a structured buffer is correctly reflected
// and works at runtime on non-bindless targets

// Reflection test: DescriptorHandle is reflected as uint2 (vector of 2 uint32s)
// Reflection test: Layout should match uint2 (size 8, same as uint2)

// CHECK_REFLECTION: "name": "texture_handle"
// CHECK_REFLECTION: "kind": "vector"
// CHECK_REFLECTION: "elementCount": 2
// CHECK_REFLECTION: "scalarType": "uint32"
// CHECK_REFLECTION: "offset": 16
// CHECK_REFLECTION: "size": 8

// Reflection test: Verify that the followingData is at the expected offset
// CHECK_REFLECTION: "name": "followingData"
// CHECK_REFLECTION: "kind": "vector"
// CHECK_REFLECTION: "elementCount": 2
// CHECK_REFLECTION: "scalarType": "uint32"
// CHECK_REFLECTION: "offset": 24
// CHECK_REFLECTION: "size": 8

// Runtime test: Verify that DescriptorHandle can be cast to uint2
// CHECK_RUNTIME: 100
// CHECK_RUNTIME: 200

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 100 200 0 0], stride=4):name=inputData
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

    // Cast handle to uint2
    uint2 h = (uint2)d.texture_handle;

    // Output the handle values to verify they're correct
    output[0] = h.x;
    output[1] = h.y;
}
