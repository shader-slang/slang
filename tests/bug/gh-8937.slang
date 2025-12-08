//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -bindless-space-index 0

// Test that we get a warning when the requested bindless space index is unavailable.

// CHECK: warning 39012: requested bindless space index '0' is unavailable, using the next available index '1'.
uniform StructuredBuffer<DescriptorHandle<Sampler2D>> t;
uniform DescriptorHandle<RWStructuredBuffer<float4>> buffer;

[numthreads(1,1,1)]
void computeMain()
{
    (*buffer)[0] = t[0].SampleLevel(float2(0.0), 0.0);
}

