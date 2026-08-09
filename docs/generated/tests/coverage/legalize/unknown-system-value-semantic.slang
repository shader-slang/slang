//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T17:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=b4c8d3637c56a76592380b5850772aac25face32ea591824cb9bbf293ce90b35
//META: purpose=Verifies an unrecognized `SV_*` system-value semantic on a Vulkan/GLSL-family target is rejected with error E49999 by the GLSL entry-point legalizer.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the diagnostic path in `createGLSLBuiltinSystemVarStorage`
// (slang-ir-glsl-legalize.cpp:991): when a struct field carries an
// `SV_`-prefixed semantic that the GLSL legalizer does not recognize, it
// falls through to `Diagnostics::UnknownSystemValueSemantic`. The
// legalizer drives both -target glsl and -target spirv (it runs "despite
// the name" on the SPIR-V arm too), so the error fires here on spirv.
// This is the error-path companion to the positive flatten/mapping
// tests; DIAGNOSTIC_TEST validates locally without FileCheck.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage vertex
struct VOut
{
    float4 p : SV_Position;
    float x : SV_BogusSemantic;
//CHECK:  ^ unknown system-value semantic
//CHECK:  ^ unknown system-value semantic 'SV_BogusSemantic'
}
[shader("vertex")]
VOut main()
{
    VOut o;
    o.p = float4(0);
    o.x = 1;
    return o;
}
