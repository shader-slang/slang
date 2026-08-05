// Issue #7878, WGSL sibling of optional-resource-generic-none.slang. The new
// `kIROp_DefaultConstruct` rejection in checkUnsupportedInst is gated on
// `isKhronosTarget(target) || isWGPUTarget(target)`, so WGSL is behaviorally
// coupled to this fix: a default-constructed opaque handle (from `Optional<T>`
// specialized to a resource and a `none` payload) must be rejected here too, not
// just on SPIR-V/GLSL.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl

struct TextureArray<T>
{
    T textures[];

    __subscript() -> Optional<T>
    {
        get { return none; }
//CHECK:             ^^^^ opaque type in local variable is not supported for Khronos targets
//CHECK:             ^^^^ a resource or other opaque-typed value ('Texture2D') cannot be placed in a function-local variable for Khronos targets (SPIR-V/GLSL) or WGSL; this usually comes from selecting a resource with control flow (e.g. a '?:' or 'if'/'else') or returning one from a function
    }
}

[vk::binding(0, 1)]
TextureArray<Texture2D> textures;

[shader("fragment")]
float4 fs_main() : SV_Target
{
    return textures[].value.Load(int3(0));
}
