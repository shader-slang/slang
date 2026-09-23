// Each entry point needs its own initialization of the replacement for the file- or
// namespace-scope `static` resource `cachedTexture`. This Metal test also verifies that each
// generated resource value is a function-local `texture2d` value passed to `loadCached`.

//TEST:SIMPLE(filecheck=CHECK): -target metal -O0 -validate-ir -line-directive-mode none

//CHECK-LABEL: float4 loadCached_{{[0-9]+}}(
//CHECK-SAME: int x_{{[0-9]+}},
//CHECK-SAME: texture2d<float, access::sample> [[CACHED:cachedTexture_[0-9]+]])
//CHECK: return (([[CACHED]]).read
//CHECK-LABEL: pixelOutput_{{[0-9]+}} firstMain(
//CHECK: texture2d<float, access::sample> [[FIRST:_S[0-9]+]] = [[INITIALIZER:_S[0-9]+]](
//CHECK: loadCached_{{[0-9]+}}(int(0), [[FIRST]])
//CHECK-LABEL: pixelOutput_{{[0-9]+}} secondMain(
//CHECK: texture2d<float, access::sample> [[SECOND:_S[0-9]+]] = [[INITIALIZER]](
//CHECK: loadCached_{{[0-9]+}}(int(1), [[SECOND]])

Texture2D<float4> textures[2];
static Texture2D<float4> cachedTexture = textures[0];

[noinline]
float4 loadCached(int x)
{
    return cachedTexture.Load(int3(x, 0, 0));
}

[shader("fragment")]
float4 firstMain() : SV_Target
{
    return loadCached(0);
}

[shader("fragment")]
float4 secondMain() : SV_Target
{
    return loadCached(1);
}
