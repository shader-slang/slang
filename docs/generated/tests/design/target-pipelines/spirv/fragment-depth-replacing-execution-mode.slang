//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-fragdepth-and-geometry-primitive-refinements
//META: doc_section_digest=2a5194c459ed95b6dfdeca1a6bc72af8d96c833b655ce928b16267ebca672dcb
//META: purpose=A conservative-depth output carries `OpExecutionMode ... DepthReplacing` alongside the `DepthGreater` mode, because Vulkan demands DepthReplacing for any write to `FragDepth`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the companion execution mode that the conservative-depth tests do
// not assert. Writing FragDepth at all requires DepthReplacing, so the
// emitter adds it in addition to whichever conservative mode the semantic
// selected; a shader is invalid without it. The depth value is derived from
// a varying input so the write is not folded away, and the CHECKs pin both
// modes on the same entry point.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment

[shader("fragment")]
float main(float4 pos : SV_Position) : SV_DepthGreaterEqual
{
    return pos.z * 0.5 + 0.25;
}

// CHECK-DAG: OpExecutionMode {{%[a-zA-Z_0-9]+}} DepthReplacing
// CHECK-DAG: OpExecutionMode {{%[a-zA-Z_0-9]+}} DepthGreater
// CHECK-DAG: BuiltIn FragDepth
