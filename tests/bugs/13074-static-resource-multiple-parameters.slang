// Two file- or namespace-scope `static` resources of the same type become two parameters on
// `loadBoth`. The generated call must pass the corresponding source resource to each parameter in
// the same order. Using the same type for both resources makes this test detect an ordering error
// that IR type validation alone cannot catch.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage fragment -O0 -validate-ir

//CHECK-LABEL: float4 loadBoth_{{[0-9]+}}(
//CHECK-SAME: Texture2D<float4 > [[FIRST:firstTexture_[0-9]+]],
//CHECK-SAME: Texture2D<float4 > [[SECOND:secondTexture_[0-9]+]])
//CHECK: return [[FIRST]].Load({{.*}}int(0){{.*}}) + [[SECOND]].Load({{.*}}int(1)
//CHECK-LABEL: float4 main()
//CHECK: return loadBoth_{{[0-9]+}}(textures_{{[0-9]+}}[int(0)], textures_{{[0-9]+}}[int(1)]);

Texture2D<float4> textures[2];

static Texture2D<float4> firstTexture = textures[0];
static Texture2D<float4> secondTexture = textures[1];

[noinline]
float4 loadBoth()
{
    return firstTexture.Load(int3(0, 0, 0)) + secondTexture.Load(int3(1, 0, 0));
}

[shader("fragment")]
float4 main() : SV_Target
{
    return loadBoth();
}
