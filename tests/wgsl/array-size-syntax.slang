//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl

// Test that array sizes are emitted without i32() cast in WGSL

struct Data
{
    float4 values[16];  // Should emit as array<vec4<f32>, 16>, not array<vec4<f32>, i32(16)>
    int indices[8];     // Should emit as array<i32, 8>, not array<i32, i32(8)>
};

// WGSL: var dataBuffer_0 : array<array<Data_std430_0>, 4>;
RWStructuredBuffer<Data> dataBuffer[4];

// WGSL: array<vec4<f32>, 16>
static const float staticArray[32] = {};

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Access array elements to ensure they're not optimized away
    Data d = dataBuffer[0][0];
    d.values[0] = float4(1, 2, 3, 4);
    d.indices[0] = 42;
    dataBuffer[0][0] = d;
    
    float val = staticArray[0];
}