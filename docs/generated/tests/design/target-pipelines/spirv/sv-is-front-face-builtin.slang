//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=3c3c8f77c7e1de8cb96354a3387d70417eaefe0edb55f1e0d96b67d31d068296
//META: purpose=`SV_IsFrontFace` is resolved by the semantic name-match chain to the `gl_FrontFacing` builtin identity and emits a `BuiltIn FrontFacing` decoration.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a second row of the semantic-to-builtin mapping, on a
// fragment-only semantic whose required type is bool rather than an integer
// vector. Branching on the input keeps it live to emit. The CHECK pins the
// FrontFacing builtin decoration, which is re-derived at emit from the same
// semantic name rather than from the gl_* identifier itself.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment

[shader("fragment")]
float4 main(bool front : SV_IsFrontFace) : SV_Target
{
    return front ? float4(1, 0, 0, 1) : float4(0, 1, 0, 1);
}

// CHECK-DAG: OpEntryPoint Fragment
// CHECK-DAG: BuiltIn FrontFacing
