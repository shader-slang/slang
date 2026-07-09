//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#inlining-and-call-graph
//META: doc_section_digest=d53bcc40d9eaa7a3ac784fc9773eb4780b72aa50e5c0ba67ff2ca45acd271030
//META: purpose=The dll-export pass marks exported entry-point functions with an `export` decoration visible in the IR dump.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "DLL export" row of the inlining / call-graph category
// (`slang-ir-dll-export.cpp`) attaches an export decoration to
// entry-point functions. The IR dump shows
// `[export("...")]` decorating `main`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// CHECK: [export(
// CHECK: func %main
