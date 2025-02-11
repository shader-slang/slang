//DIAGNOSTIC_TEST:SIMPLE: -target hlsl

SamplerState sampler;

struct Foo
{
    bool sample_bar = false;
    Texture2D<float4> bar = {};
};

struct Result
{
    float4 color = float4(0.0);
};

Result process(in Foo foo)
{
    Result result = {};

    if (foo.sample_bar) {
        result.color = foo.bar.Sample(sampler, float2(0.0, 0.0));
    }

    return result;
}

[shader("compute")]
[numthreads(8, 8, 1)]
float4 cs_main(uint3 thread_id : SV_DispatchThreadID) {
    Foo foo;
    const let result = process(foo);
    return result.color;
}
