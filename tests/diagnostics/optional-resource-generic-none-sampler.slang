// Issue #7878: the default-constructed-opaque-handle rejection is not specific
// to textures. `findUnstorableOpaqueHandleType` returns the leaf handle for any
// image/sampler/subpass/acceleration-structure type (recursing into aggregates),
// so a generic `Optional<T>` specialized to `SamplerState` with a `none` payload
// must be rejected the same way. This guards against a refactor that regresses a
// non-`Texture2D` handle category while the texture case still passes. The main
// optional-resource-generic-none.slang test pins the source location; here we
// match on the message so the assertion is about the `SamplerState` category.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv

struct Wrapper<T>
{
    T value;

    Optional<T> getMaybe()
    {
        // CHECK: error[E56004]
        // CHECK: 'SamplerState'
        return none;
    }
}

[vk::binding(0, 1)]
Wrapper<SamplerState> wrapper;
Texture2D tex;

[shader("fragment")]
float4 fs_main() : SV_Target
{
    return tex.Sample(wrapper.getMaybe().value, float2(0));
}
