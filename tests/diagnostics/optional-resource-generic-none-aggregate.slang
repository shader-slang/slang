// Issue #7878, aggregate-payload variant. When the Optional payload is a struct
// (or tuple/array) that transitively contains an opaque handle, the lowering's
// `emitDefaultConstruct` wraps the inner `DefaultConstruct` in a `MakeStruct`,
// so the diagnostic keys on that nested inst. This test pins the source location
// with a caret to verify the source-loc *scope* in the Optional lowering reaches
// the nested `DefaultConstruct` (a plain `sourceLoc =` on the outer wrapper would
// miss it and the diagnostic would fall back to `findFirstUseLoc`).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

struct Bundle
{
    Texture2D tex;
}

struct Wrapper<T>
{
    T value;

    Optional<T> getMaybe()
    {
        return none;
//CHECK:       ^^^^ opaque type in local variable is not supported for Khronos targets
//CHECK:       ^^^^ a resource or other opaque-typed value ('Texture2D') cannot be placed in a function-local variable for Khronos targets (SPIR-V/GLSL) or WGSL; this usually comes from selecting a resource with control flow (e.g. a '?:' or 'if'/'else') or returning one from a function
    }
}

[vk::binding(0, 1)]
Wrapper<Bundle> wrapper;

[shader("fragment")]
float4 fs_main() : SV_Target
{
    return wrapper.getMaybe().value.tex.Load(int3(0));
}
