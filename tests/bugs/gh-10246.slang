//DIAGNOSTIC_TEST:SIMPLE: -target cuda -entry main -stage fragment

// Regression test: CUDA backend ICE on ParameterBlock with texture members
// Issue: https://github.com/shader-slang/slang/issues/10246

struct Params {
    Sampler2D<float4> tex;
};

[shader("fragment")]
float4 main(float2 uv : UV, ParameterBlock<Params> params) : SV_Target {
    return params.tex.Sample(uv);
}
