//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute

// Test that DescriptorHandle can be constructed from uint64_t in CUDA.
// In CUDA, DescriptorHandle<T> becomes transparent and maps directly to the underlying
// resource type (CUtexObject/CUsurfObject which are ulonglong). This means:
// 1. uint64_t -> DescriptorHandle conversion is a no-op cast
// 2. DescriptorHandle -> uint64_t conversion is a no-op cast  
// 3. Comparison methods (equals, lessThan, etc.) cannot be called directly because
//    the handle type is erased to the resource type at compile time.

// CUDA-DAG: ulonglong textureHandle
// CUDA-DAG: tex2DLod
// CUDA-DAG: void computeMain

uniform uint64_t textureHandle;
uniform uint64_t textureHandle2;

// Cast uint64_t to DescriptorHandle and use it
DescriptorHandle<Sampler2D> getTextureHandle(uint64_t h)
{
    return DescriptorHandle<Sampler2D>(h);
}

// Cast DescriptorHandle to uint64_t
uint64_t getHandleValue(DescriptorHandle<Sampler2D> h)
{
    return (uint64_t)h;
}

// Test that we can compare handles using uint64_t comparisons
// (since direct method calls aren't available in CUDA)
bool compareHandles(uint64_t h1, uint64_t h2)
{
    DescriptorHandle<Sampler2D> handle1 = DescriptorHandle<Sampler2D>(h1);
    DescriptorHandle<Sampler2D> handle2 = DescriptorHandle<Sampler2D>(h2);
    // Convert back to uint64_t for comparison
    return (uint64_t)handle1 == (uint64_t)handle2;
}

uniform RWStructuredBuffer<float4> output;

[numthreads(1,1,1)]
void computeMain()
{
    // Test construction from uint64_t - creates a texture handle
    DescriptorHandle<Sampler2D> tex = DescriptorHandle<Sampler2D>(textureHandle);
    
    // Test conversion to uint64_t and back
    uint64_t handleValue = (uint64_t)tex;
    DescriptorHandle<Sampler2D> tex2 = DescriptorHandle<Sampler2D>(handleValue);
    
    // Test comparison via uint64_t conversion
    bool same = compareHandles(textureHandle, textureHandle2);
    
    // Sample the texture using the handle
    output[0] = tex->SampleLevel(float2(0.0), 0.0);
    output[1] = float4(same ? 1.0 : 0.0, 0.0, 0.0, 0.0);
}
