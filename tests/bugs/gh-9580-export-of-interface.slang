// Non-regression companion to gh-9580-assoc-type-of-export.slang: a fragment entry
// point returning a link-time `export` struct *directly* (rather than an associated
// type of one) already worked before the #9580 fix and must keep working. This is
// the "VARIANT 1" from the issue's reproducer, kept as an explicit guard so the
// associated-type fix in `resolveLinkTimeAssociatedType` cannot regress the
// direct-export path that flows through `lookupExternDeclRefType`.

//TEST:REFLECTION(filecheck=REFL): -target spirv -stage fragment -entry ps_main
//TEST:SIMPLE(filecheck=SPIRV): -target spirv

public interface IFragOutput {
    public __init();
}

public struct ColorOutput : IFragOutput {
    public float3 color : SV_Target0;
    public __init() { color = 0; }
}

export struct FragOut : IFragOutput = ColorOutput;

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
