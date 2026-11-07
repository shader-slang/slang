// Companion to gh-9580-assoc-type-of-export.slang showing that the single link-time
// resolution bottleneck (`tryResolveAllLinkTimeTypesInDeclRef`) reaches a link-time member
// even when it is *nested* inside another type's generic argument, not just at the top of
// the entry point's result type.
//
// Here the fragment result is `Wrap<ShaderMode::FragOut>`: the link-time associated type
// `ShaderMode::FragOut` (of the `export` wrapper `ShaderMode = SolidMode`) appears as the
// generic argument of `Wrap`. Layout descends into `Wrap`'s field `inner` on its own and
// re-enters the bottleneck for that nested `DeclRefType`, so `FragOut` resolves to the
// concrete `ColorOutput` there — no dedicated recursion over generic arguments is needed.
// Reflection must report the concrete `ColorOutput` field, not an opaque associated type.
// See shader-slang/slang#9580.

//TEST:REFLECTION(filecheck=REFL): -target spirv -stage fragment -entry ps_main
//TEST:SIMPLE(filecheck=SPIRV): -target spirv

public interface IFragOutput {
    public __init();
}

public struct ColorOutput : IFragOutput {
    public float3 color : SV_Target0;
    public __init() { color = 0; }
}

public interface IShaderMode {
    public associatedtype FragOut : IFragOutput;
}

public struct SolidMode : IShaderMode {
    public typedef ColorOutput FragOut;
}

export struct ShaderMode : IShaderMode = SolidMode;

public struct Wrap<T : IFragOutput> : IFragOutput {
    public T inner;
    public __init() { inner = T(); }
}

// SPIRV: OpEntryPoint

// REFL: "name": "ps_main"
// REFL: "result":
// REFL: "name": "ColorOutput"
// REFL: "name": "color"
// REFL: "semanticName": "SV_TARGET"
[shader("fragment")]
Wrap<ShaderMode.FragOut> ps_main() {
    return Wrap<ShaderMode.FragOut>();
}
