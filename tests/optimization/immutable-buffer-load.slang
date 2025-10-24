// Test that we can use ImmutablePtr<T> to result in more optimized buffer loads
// in SPIR-V and CUDA.

//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV): -target spirv -O0
uniform ImmutablePtr<float4> data;

uniform float4* result;

float4 work(ImmutablePtr<float4> ptr)
{
    return *ptr;
}

[numthreads(1,1,1)]
void computeMain()
{
    // SPV: Restrict
    // PTX: ld.global.nc.v4.f32
    *result = work(data) + float4(1,2,3,4);
}