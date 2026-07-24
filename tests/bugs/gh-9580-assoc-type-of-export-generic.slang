// Companion to gh-9580-assoc-type-of-export.slang covering a *type-generic* link-time
// wrapper. `resolveLinkTimeAssociatedType` substitutes the wrapper's stored conformance
// witness through the inheritance clause (which binds `T = ColorOutput`) before resolving
// the associated type, so `ShaderMode<ColorOutput>::FragOut` resolves to the concrete
// `ColorOutput` rather than the unbound generic parameter `T`. Without that substitution
// the unbound `T` leaks into layout and the compiler aborts. See shader-slang/slang#9580.

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

public struct SolidMode<T : IFragOutput> : IShaderMode {
    public typedef T FragOut;
}

export struct ShaderMode<T : IFragOutput> : IShaderMode = SolidMode<T>;
typedef ShaderMode<ColorOutput>::FragOut FragOut;

// SPIRV: OpEntryPoint

// REFL: "name": "ps_main"
// REFL: "result":
// REFL: "kind": "struct"
// REFL: "name": "ColorOutput"
// REFL: "name": "color"
// REFL: "semanticName": "SV_TARGET"
[shader("fragment")]
FragOut ps_main() {
    FragOut output;
    return output;
}
