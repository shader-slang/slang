// Returning the associated type of an `export`/`extern` link-time struct used to
// crash the compiler: the front-end built an opaque varying layout for the
// associated type while link-time IR specialization resolved the concrete return
// type, so GLSL/SPIR-V legalization saw a concrete struct with a null layout and
// crashed in `createGLSLGlobalVaryingsImpl`. See shader-slang/slang#9580.
//
// Here `ShaderMode::FragOut` is the associated type `IShaderMode::FragOut` looked
// up on the `export` wrapper `ShaderMode = SolidMode`; it must resolve to the
// concrete `ColorOutput` for layout, exactly as the direct-export variant (tested
// in gh-9580-export-of-interface.slang) already does. Reflection must report the
// concrete `ColorOutput` result with its `SV_Target0` field, not an opaque type.

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
typedef ShaderMode::FragOut FragOut;

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
