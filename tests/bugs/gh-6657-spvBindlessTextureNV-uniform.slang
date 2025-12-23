//TEST:REFLECTION(filecheck=CHECK):-target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain

// Test that DescriptorHandle<T> in a structured buffer is correctly reflected
// as uint64_t when spvBindlessTextureNV is enabled

// CHECK: "name": "texture_handle"
// CHECK: "kind": "scalar"
// CHECK: "scalarType": "uint64"
// CHECK: "offset": 16
// CHECK: "size": 8

// Verify that the followingData is at the expected offset
// CHECK: "name": "followingData"
// CHECK: "kind": "vector"
// CHECK: "elementCount": 2
// CHECK: "scalarType": "uint32"
// CHECK: "offset": 24
// CHECK: "size": 8

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
    uint64_t h = (uint64_t)d.texture_handle;
    output[0] = uint(h);
    output[1] = uint(h >> 32);
}
