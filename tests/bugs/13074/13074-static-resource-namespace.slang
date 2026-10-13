// `cachedTexture` is a file- or namespace-scope `static` resource declared inside a namespace. It
// is not a static data member. Lowering must mark it for replacement, and legalization must pass
// its value to the namespace helper.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage fragment -O0 -validate-ir

//CHECK-LABEL: float4 Cache_load_{{[0-9]+}}(
//CHECK-SAME: Texture2D<float4 > [[CACHED:cachedTexture_[0-9]+]])
//CHECK: return [[CACHED]].Load
//CHECK-LABEL: float4 main()
//CHECK: return Cache_load_{{[0-9]+}}(inputTexture_{{[0-9]+}});

Texture2D<float4> inputTexture;

namespace Cache
{
    static Texture2D<float4> cachedTexture = inputTexture;

    [noinline]
    float4 load()
    {
        return cachedTexture.Load(int3(0, 0, 0));
    }
}

[shader("fragment")]
float4 main() : SV_Target
{
    return Cache::load();
}
