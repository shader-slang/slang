// Regression test for issue #7878: returning `none` from a generic wrapper's
// `Optional<T>` accessor, then instantiating the wrapper with a resource type,
// used to reach spirv-emit as a `defaultConstruct` of an opaque handle and abort
// with "Unhandled local inst in spirv-emit". The front-end `Optional<T>` check
// (E30902) does not fire here because `T` is only known to be `Texture2D` after
// specialization. The compiler must reject this with a diagnostic rather than
// crash: there is no default/zero value for an opaque handle on Khronos targets.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

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
