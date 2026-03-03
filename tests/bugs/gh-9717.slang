//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv

//CHECK_SPIRV: OpEntryPoint
struct Foo { float bar[4]; bool baz; };

Texture2D tex[2];
SamplerState sampler; 
ConstantBuffer<Foo> foo;
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
    if (foo.baz)
        output[0] = first(foo, sampler, tex[0]);
    else
        output[0] = first(foo, sampler, tex[0]);
}

float first(Foo view, SamplerState sampler, Texture2D tex) {
    return second(view, sampler, tex);
}

float second(Foo view, SamplerState sampler, Texture2D tex) {
    return tex.SampleLevel(sampler, 0.5f, 0).r;
}
