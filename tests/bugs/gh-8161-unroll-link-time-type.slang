//TEST:SIMPLE(filecheck=CHECK): -target spirv
// Test that validates [unroll] works with link-time types where [ForceUnroll] would fail

// CHECK: OpEntryPoint

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

struct BarSampler : ISampler
{
    int getSampleCount() { return 2; }
    float sample(int index) { return index * 0.5; }
}

extern struct Sampler : ISampler = FooSampler;

RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    Sampler sampler;
    [unroll]  // This works, while [ForceUnroll] would fail
    for (int i = 0; i < sampler.getSampleCount(); i++)
        output[tid] += sampler.sample(i);
}