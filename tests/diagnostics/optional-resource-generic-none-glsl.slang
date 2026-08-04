// Issue #7878, GLSL sibling of optional-resource-generic-none.slang. The new
// `kIROp_DefaultConstruct` rejection is gated on `isKhronosTarget(target)`, which
// covers both SPIR-V and GLSL. GLSL has a distinct emit path (slang-emit-glsl.cpp)
// and could regress independently, so pin the diagnostic for `-target glsl` too.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl

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
