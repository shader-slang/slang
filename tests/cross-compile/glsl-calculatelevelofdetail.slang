//TEST:CROSS_COMPILE(filecheck=CHECK): -profile ps_5_0 -entry main -target glsl

// CHECK: textureQueryLod(sampler2D(t_0,s_0), ({{.*}})).x
// CHECK: textureQueryLod(sampler2D(t_0,s_0), ({{.*}})).y

Texture2D t;
SamplerState s;
float main()
{
    return t.CalculateLevelOfDetail(s, float2(0,0)) + t.CalculateLevelOfDetailUnclamped(s, float2(0,0));
}
