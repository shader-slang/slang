//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl

// Test that ConstantBuffer generates var<uniform> and RWStructuredBuffer generates var<storage>

struct UniformData
{
    float4x4 transform;
    float scale;
};

struct StorageData
{
    float values[16];
};

// Should generate var<uniform>
// WGSL: var<uniform> uniformBuffer{{.*}} : UniformData{{.*}};
ConstantBuffer<UniformData> uniformBuffer;

// Should generate var<storage, read_write>
// WGSL: var<storage, read_write> storageBuffer{{.*}} : array<StorageData{{.*}}>;
RWStructuredBuffer<StorageData> storageBuffer;

// Should generate var<storage, read_write>
// WGSL: var<storage, read_write> outputBuffer{{.*}} : array<f32>;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Access uniform buffer (read-only)
    float scale = uniformBuffer.scale;
    float4x4 transform = uniformBuffer.transform;
    
    // Access storage buffers (read-write)
    StorageData data;
    data.values[0] = scale;
    data.values[1] = transform[0][0];
    
    storageBuffer[0] = data;
    outputBuffer[0] = scale + transform[1][1];
}
