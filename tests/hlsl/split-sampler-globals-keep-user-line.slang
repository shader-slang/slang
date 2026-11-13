// The texture and sampler globals that a `Sampler2D` global splits into keep
// the location of the user's `Sampler2D` global, so each gets a `#line` in
// this file (a bare `#line N`, because the file is already current) rather
// than a `#line` naming `core.meta.slang` or `hlsl.meta.slang`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<float4> outBuf;
Sampler2D combined;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = combined.SampleLevel(float2(0.5, 0.5), 0);
}

// CHECK: #line {{[0-9]+}}{{$}}
// CHECK-NEXT: Texture2D{{.*}} combined_texture_
// CHECK: #line {{[0-9]+}}{{$}}
// CHECK-NEXT: SamplerState combined_sampler_
