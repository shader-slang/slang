//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=a78fa1c65d9e97005484d3326f0e07d9af56a6dadaad4955b0d779effe950727
//META: purpose=A WaveSize attribute on a compute entry point lowers to a waveSize decoration carrying the requested lane count.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc records `waveSize` with operand `numLanes : IRIntLit` and
// AST origin `[WaveSize(...)]` attribute. Confirm the requested
// integer flows through unchanged.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
[WaveSize(32)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [waveSize(32 : Int)]
// CHECK: func %main
