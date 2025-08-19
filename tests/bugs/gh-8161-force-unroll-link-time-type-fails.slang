//TEST:SIMPLE(filecheck=CHECK): -target spirv
// Test that validates [ForceUnroll] fails with link-time types due to runtime method calls

// CHECK: error 40020

interface ISampler
{
    int getSampleCount();
    float sample(int index);
}

struct FooSampler : ISampler
{
    int getSampleCount() { return 1; }
    float sample(int index) { return 0.0; }
}

extern struct Sampler : ISampler = FooSampler;

RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    Sampler sampler;
    [ForceUnroll]  // This should fail because getSampleCount() is a runtime method call
    for (int i = 0; i < sampler.getSampleCount(); i++)
        output[tid] += sampler.sample(i);
}