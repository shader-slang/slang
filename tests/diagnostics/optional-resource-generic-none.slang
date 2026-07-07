// Regression test for issue #7878: returning `none` from a generic wrapper's
// `Optional<T>` accessor, then instantiating the wrapper with a resource type,
// used to reach spirv-emit as a `defaultConstruct` of an opaque handle and abort
// with "Unhandled local inst in spirv-emit". The front-end `Optional<T>` check
// (E30902) does not fire here because `T` is only known to be `Texture2D` after
// specialization. The compiler must reject this with a diagnostic rather than
// crash: there is no default/zero value for an opaque handle on Khronos targets.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv

struct TextureArray<T>
{
    T textures[];

    __subscript() -> Optional<T>
    {
        // CHECK: error[E56004]
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
