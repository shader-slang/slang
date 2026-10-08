// Issue #7878 polarity test: the default-constructed-opaque-handle rejection in
// checkUnsupportedInst is specific to Khronos/WGSL. On HLSL/D3D a resource
// `Optional<T>` with a `none` payload lowers to an (uninitialized) resource
// local, which is valid HLSL, so the same source that is rejected for SPIR-V
// must compile cleanly here. This pins the `rejectOpaqueLocals` gate against a
// polarity regression that would otherwise spuriously reject valid HLSL if the
// new `kIROp_DefaultConstruct` check were hoisted out of the guard.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry fs_main -stage fragment

struct TextureArray<T>
{
    T textures[];

    __subscript() -> Optional<T>
    {
        get { return none; }
    }
}

[vk::binding(0, 1)]
TextureArray<Texture2D> textures;

[shader("fragment")]
float4 fs_main() : SV_Target
{
    return textures[].value.Load(int3(0));
}

// CHECK-NOT: opaque type in local variable
// CHECK: fs_main
